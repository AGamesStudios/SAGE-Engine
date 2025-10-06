#include "TestFramework.h"

#include <SAGE.h>
#include <filesystem>
#include <fstream>

#include <Engine/Resources/EmbeddedFonts.h>

using namespace SAGE;
namespace fs = std::filesystem;

namespace {
    inline bool EnsureDirectory(const fs::path& path) {
        std::error_code ec;
        if (fs::exists(path, ec)) {
            return true;
        }
        return fs::create_directories(path, ec);
    }
}

TEST_CASE(FontManager_RegisterFontFileHandlesMissingInput) {
    FontManager::Clear();

    const fs::path tempDir = fs::temp_directory_path() / "sage_engine_tests";
    REQUIRE(EnsureDirectory(tempDir));

    const fs::path missingFont = tempDir / "missing_font.ttf";
    CHECK(!FontManager::RegisterFontFile(missingFont).has_value());

    const fs::path fontPath = tempDir / "autogen_font.ttf";
    const std::vector<unsigned char> fontData = EmbeddedFonts::GetProggyCleanTTF();
    {
        std::ofstream output(fontPath, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output.write(reinterpret_cast<const char*>(fontData.data()), static_cast<std::streamsize>(fontData.size()));
        REQUIRE(output.good());
    }

    auto key = FontManager::RegisterFontFile(fontPath);
    REQUIRE(key.has_value());
    CHECK(FontManager::IsRegistered(*key));

    auto registeredPath = FontManager::GetRegisteredPath(*key);
    REQUIRE(registeredPath.has_value());
    CHECK(fs::equivalent(*registeredPath, fontPath));

    const auto keys = FontManager::GetRegisteredFonts();
    REQUIRE(keys.size() == 1);
    CHECK(keys.front().find("autogen_font") != std::string::npos);

    const auto displayNames = FontManager::GetRegisteredFonts(true);
    REQUIRE(displayNames.size() == 1);
    CHECK(displayNames.front() == "autogen_font");

    auto duplicateKey = FontManager::RegisterFontFile(fontPath);
    REQUIRE(duplicateKey.has_value());
    CHECK(*duplicateKey == *key);
    CHECK(FontManager::GetRegisteredFonts().size() == 1);

    std::error_code ec;
    fs::remove(fontPath, ec);

    FontManager::Clear();
}

TEST_CASE(SoundManager_GracefullyHandlesMissingFile) {
    SoundManager::Clear();

    const bool wasInitialized = AudioSystem::IsInitialized();
    if (!wasInitialized) {
        AudioSystem::Init();
    }

    const fs::path missingPath = fs::temp_directory_path() / "sage_engine_tests" / "missing_sound.wav";
    std::error_code ec;
    std::filesystem::remove(missingPath, ec);

    Ref<Sound> sound = SoundManager::Load("missing_sound", missingPath.string());
    CHECK(!sound);
    CHECK(!SoundManager::Exists("missing_sound"));

    SoundManager::Clear();
    if (!wasInitialized) {
        AudioSystem::Shutdown();
    }
}
