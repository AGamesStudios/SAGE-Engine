#include "TextureManager.h"

#include "Core/Logger.h"
#include "Core/FileSystem.h"

#include <algorithm>

namespace SAGE {

TextureManager& TextureManager::Get() {
	static TextureManager instance;
	return instance;
}

void TextureManager::Init() {
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Initialized) {
		SAGE_WARNING("TextureManager already initialized");
		return;
	}
	m_Textures.clear();
	m_Initialized = true;
	SAGE_INFO("TextureManager initialized");
}

void TextureManager::Shutdown() {
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (!m_Initialized) {
		return;
	}
	m_Textures.clear();
	m_Initialized = false;
	SAGE_INFO("TextureManager shutdown");
}

Ref<Texture> TextureManager::Load(const std::string& name, const std::string& filepath) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (name.empty() || filepath.empty()) {
		SAGE_WARNING("TextureManager::Load: Invalid name or filepath");
		return nullptr;
	}
	if (!FileSystem::IsSafePath(filepath)) {
		SAGE_ERROR("TextureManager::Load: Unsafe path detected '{}' (potential directory traversal)", filepath);
		return nullptr;
	}
	auto it = m_Textures.find(name);
	if (it != m_Textures.end()) {
		return it->second.texture;
	}
	auto texture = CreateRef<Texture>(filepath);
	if (!texture->IsLoaded()) {
		SAGE_ERROR("TextureManager::Load: Failed to load texture '{}' from '{}'", name, filepath);
		return nullptr;
	}
	TextureEntry entry{texture, filepath};
	m_Textures[name] = entry;
	SAGE_TRACE("TextureManager::Load: Loaded texture '{}' from '{}' ({}x{})", name, filepath, texture->GetWidth(), texture->GetHeight());
	return texture;
}

Ref<Texture> TextureManager::Get(const std::string& name) const {
	std::lock_guard<std::mutex> lock(m_Mutex);
	auto it = m_Textures.find(name);
	if (it != m_Textures.end()) return it->second.texture;
	return nullptr;
}

bool TextureManager::Reload(const std::string& name) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	auto it = m_Textures.find(name);
	if (it == m_Textures.end()) {
		SAGE_WARNING("TextureManager::Reload: Texture '{}' not found", name);
		return false;
	}
	const std::string& filepath = it->second.filepath;
	SAGE_INFO("TextureManager::Reload: Reloading texture '{}' from '{}'", name, filepath);
	auto newTexture = CreateRef<Texture>(filepath);
	if (!newTexture->IsLoaded()) {
		SAGE_ERROR("TextureManager::Reload: Failed to reload texture '{}' from '{}'", name, filepath);
		return false;
	}
	it->second.texture = newTexture;
	SAGE_TRACE("TextureManager::Reload: Successfully reloaded texture '{}'", name);
	return true;
}

void TextureManager::Remove(const std::string& name) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_Textures.erase(name);
}

void TextureManager::Clear() {
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_Textures.clear();
}

size_t TextureManager::GetLoadedCount() const {
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Textures.size();
}

void TextureManager::UnloadUnused() {
	std::lock_guard<std::mutex> lock(m_Mutex);
	std::vector<std::string> toRemove;
	for (const auto& [name, entry] : m_Textures) {
		if (entry.texture.use_count() == 1) {
			toRemove.push_back(name);
		}
	}
	for (const auto& name : toRemove) {
		SAGE_TRACE("TextureManager::UnloadUnused: Removing unused texture '{}'", name);
		m_Textures.erase(name);
	}
}

bool TextureManager::IsLoaded(const std::string& name) const {
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Textures.find(name) != m_Textures.end();
}

} // namespace SAGE
