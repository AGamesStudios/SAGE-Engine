#pragma once

#include "Core/Logger.h"
#include "Core/ServiceLocator.h"
#include "Graphics/Interfaces/IShaderManager.h"
#include "Resources/TextureManager.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace SAGE {

/// @brief Event type для FileWatcher
enum class FileWatchEvent {
    Created,
    Modified,
    Deleted,
    Renamed
};

/// @brief File change callback
using FileChangeCallback = std::function<void(const std::string& path, FileWatchEvent event)>;

/// @brief Platform-independent file watcher для hot-reload
class FileWatcher {
public:
    /// @brief Создать file watcher
    /// @param watchPath Директория для отслеживания
    /// @param recursive Отслеживать поддиректории
    explicit FileWatcher(const std::string& watchPath, bool recursive = true)
        : m_WatchPath(watchPath)
        , m_Recursive(recursive)
        , m_Running(false)
    {
        if (!std::filesystem::exists(m_WatchPath)) {
            SAGE_ERROR("FileWatcher: Path does not exist: {}", m_WatchPath);
            return;
        }
        
        SAGE_INFO("FileWatcher: Watching '{}'", m_WatchPath);
    }
    
    ~FileWatcher() {
        Stop();
    }
    
    // Запретить копирование
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    
    /// @brief Начать отслеживание
    void Start() {
        if (m_Running) {
            SAGE_WARNING("FileWatcher already running");
            return;
        }
        
        m_Running = true;
        m_WatchThread = std::thread([this]() { WatchLoop(); });
        
        SAGE_INFO("FileWatcher: Started");
    }
    
    /// @brief Остановить отслеживание
    void Stop() {
        if (!m_Running) return;
        
        m_Running = false;
        
        if (m_WatchThread.joinable()) {
            m_WatchThread.join();
        }
        
        SAGE_INFO("FileWatcher: Stopped");
    }
    
    /// @brief Добавить callback для изменений
    void AddCallback(const std::string& extension, FileChangeCallback callback) {
        std::lock_guard<std::mutex> lock(m_CallbackMutex);
        m_Callbacks[extension].push_back(std::move(callback));
        
        SAGE_INFO("FileWatcher: Added callback for '{}' files", extension);
    }
    
    /// @brief Удалить все callbacks для расширения
    void RemoveCallbacks(const std::string& extension) {
        std::lock_guard<std::mutex> lock(m_CallbackMutex);
        m_Callbacks.erase(extension);
    }
    
    /// @brief Проверить работает ли watcher
    bool IsRunning() const { return m_Running; }

private:
    void WatchLoop() {
#ifdef _WIN32
        WatchLoopWindows();
#else
        WatchLoopPoll();  // Fallback для других платформ
#endif
    }
    
#ifdef _WIN32
    void WatchLoopWindows() {
        HANDLE hDir = CreateFileA(
            m_WatchPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );
        
        if (hDir == INVALID_HANDLE_VALUE) {
            SAGE_ERROR("FileWatcher: Failed to open directory");
            m_Running = false;
            return;
        }
        
        const DWORD bufferSize = 4096;
        std::vector<BYTE> buffer(bufferSize);
        DWORD bytesReturned = 0;
        
        while (m_Running) {
            BOOL success = ReadDirectoryChangesW(
                hDir,
                buffer.data(),
                bufferSize,
                m_Recursive ? TRUE : FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE |
                FILE_NOTIFY_CHANGE_SIZE,
                &bytesReturned,
                nullptr,
                nullptr
            );
            
            if (!success || bytesReturned == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // Parse notifications
            FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());
            
            while (true) {
                // Convert wide string to UTF-8
                int nameLength = info->FileNameLength / sizeof(WCHAR);
                std::wstring wfilename(info->FileName, nameLength);
                std::string filename = WideToUtf8(wfilename);
                
                std::string fullPath = m_WatchPath + "/" + filename;
                
                // Determine event type
                FileWatchEvent event;
                switch (info->Action) {
                    case FILE_ACTION_ADDED:
                        event = FileWatchEvent::Created;
                        break;
                    case FILE_ACTION_REMOVED:
                        event = FileWatchEvent::Deleted;
                        break;
                    case FILE_ACTION_MODIFIED:
                        event = FileWatchEvent::Modified;
                        break;
                    case FILE_ACTION_RENAMED_OLD_NAME:
                    case FILE_ACTION_RENAMED_NEW_NAME:
                        event = FileWatchEvent::Renamed;
                        break;
                    default:
                        event = FileWatchEvent::Modified;
                }
                
                // Trigger callbacks
                TriggerCallbacks(fullPath, event);
                
                // Next notification
                if (info->NextEntryOffset == 0) break;
                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<BYTE*>(info) + info->NextEntryOffset
                );
            }
        }
        
        CloseHandle(hDir);
    }
    
    std::string WideToUtf8(const std::wstring& wide) {
        if (wide.empty()) return {};
        
        int sizeNeeded = WideCharToMultiByte(
            CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr
        );
        
        std::string result(sizeNeeded - 1, 0);
        WideCharToMultiByte(
            CP_UTF8, 0, wide.c_str(), -1, &result[0], sizeNeeded, nullptr, nullptr
        );
        
        return result;
    }
#endif
    
    // Fallback: polling-based watching (кроссплатформенный)
    void WatchLoopPoll() {
        // Кэш состояния файлов
        std::map<std::string, std::filesystem::file_time_type> fileTimestamps;
        
        // Initial scan
        ScanDirectory(m_WatchPath, fileTimestamps);
        
        while (m_Running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            std::map<std::string, std::filesystem::file_time_type> currentTimestamps;
            ScanDirectory(m_WatchPath, currentTimestamps);
            
            // Check for new/modified files
            for (const auto& [path, timestamp] : currentTimestamps) {
                auto it = fileTimestamps.find(path);
                
                if (it == fileTimestamps.end()) {
                    // New file
                    TriggerCallbacks(path, FileWatchEvent::Created);
                } else if (it->second != timestamp) {
                    // Modified file
                    TriggerCallbacks(path, FileWatchEvent::Modified);
                }
            }
            
            // Check for deleted files
            for (const auto& [path, timestamp] : fileTimestamps) {
                if (currentTimestamps.find(path) == currentTimestamps.end()) {
                    TriggerCallbacks(path, FileWatchEvent::Deleted);
                }
            }
            
            fileTimestamps = std::move(currentTimestamps);
        }
    }
    
    void ScanDirectory(const std::string& path, std::map<std::string, std::filesystem::file_time_type>& timestamps) {
        try {
            if (m_Recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        timestamps[entry.path().string()] = entry.last_write_time();
                    }
                }
            } else {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        timestamps[entry.path().string()] = entry.last_write_time();
                    }
                }
            }
        } catch (const std::exception& e) {
            SAGE_ERROR("FileWatcher: Scan error: {}", e.what());
        }
    }
    
    void TriggerCallbacks(const std::string& path, FileWatchEvent event) {
        // Получить расширение файла
        std::filesystem::path fsPath(path);
        std::string extension = fsPath.extension().string();
        
        std::lock_guard<std::mutex> lock(m_CallbackMutex);
        
        // Callbacks для конкретного расширения
        auto it = m_Callbacks.find(extension);
        if (it != m_Callbacks.end()) {
            for (const auto& callback : it->second) {
                try {
                    callback(path, event);
                } catch (const std::exception& e) {
                    SAGE_ERROR("FileWatcher: Callback exception: {}", e.what());
                }
            }
        }
        
        // Callbacks для всех файлов ("*")
        auto allIt = m_Callbacks.find("*");
        if (allIt != m_Callbacks.end()) {
            for (const auto& callback : allIt->second) {
                try {
                    callback(path, event);
                } catch (const std::exception& e) {
                    SAGE_ERROR("FileWatcher: Callback exception: {}", e.what());
                }
            }
        }
    }

private:
    std::string m_WatchPath;
    bool m_Recursive;
    std::atomic<bool> m_Running;
    std::thread m_WatchThread;
    
    std::map<std::string, std::vector<FileChangeCallback>> m_Callbacks;
    std::mutex m_CallbackMutex;
};

/// @brief Asset hot-reload manager
class HotReloadManager {
public:
    static HotReloadManager& Get() {
        static HotReloadManager instance;
        return instance;
    }
    
    /// @brief Начать отслеживание директории
    void WatchDirectory(const std::string& path) {
        auto watcher = std::make_unique<FileWatcher>(path, true);
        
        // Setup callbacks для различных типов файлов
        watcher->AddCallback(".png", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Texture modified: {}", path);
                // Extract texture name from path
                auto filename = std::filesystem::path(path).stem().string();
                
                // Reload through TextureManager
                TextureManager::Get().Reload(filename);
            }
        });
        
        watcher->AddCallback(".jpg", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Texture modified: {}", path);
                auto filename = std::filesystem::path(path).stem().string();
                TextureManager::Get().Reload(filename);
            }
        });
        
        watcher->AddCallback(".shader", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Shader modified: {}", path);
                // Extract shader name from path
                auto filename = std::filesystem::path(path).stem().string();
                
                // Reload через ServiceLocator
                if (ServiceLocator::HasGlobalInstance()) {
                    auto& services = ServiceLocator::GetGlobalInstance();
                    if (services.HasShaderManager()) {
                        services.GetShaderManager().ReloadShader(filename);
                    }
                }
            }
        });
        
        // GLSL vertex shaders
        watcher->AddCallback(".vert", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Vertex shader modified: {}", path);
                auto filename = std::filesystem::path(path).stem().string();
                
                if (ServiceLocator::HasGlobalInstance()) {
                    auto& services = ServiceLocator::GetGlobalInstance();
                    if (services.HasShaderManager()) {
                        services.GetShaderManager().ReloadShader(filename);
                    }
                }
            }
        });
        
        // GLSL fragment shaders
        watcher->AddCallback(".frag", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Fragment shader modified: {}", path);
                auto filename = std::filesystem::path(path).stem().string();
                
                if (ServiceLocator::HasGlobalInstance()) {
                    auto& services = ServiceLocator::GetGlobalInstance();
                    if (services.HasShaderManager()) {
                        services.GetShaderManager().ReloadShader(filename);
                    }
                }
            }
        });
        
        watcher->AddCallback(".json", [](const std::string& path, FileWatchEvent event) {
            if (event == FileWatchEvent::Modified) {
                SAGE_INFO("HotReload: Scene modified: {}", path);
                // Note: Scene reload integration pending
            }
        });
        
        watcher->Start();
        m_Watchers.push_back(std::move(watcher));
        
        SAGE_INFO("HotReloadManager: Watching '{}'", path);
    }
    
    /// @brief Остановить все watchers
    void StopAll() {
        for (auto& watcher : m_Watchers) {
            watcher->Stop();
        }
        m_Watchers.clear();
        
        SAGE_INFO("HotReloadManager: Stopped all watchers");
    }

private:
    HotReloadManager() = default;
    
    std::vector<std::unique_ptr<FileWatcher>> m_Watchers;
};

} // namespace SAGE
