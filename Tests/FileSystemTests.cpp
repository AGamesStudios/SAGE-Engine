#include "Core/FileSystem.h"
#include "Tests/TestFramework.h"

using namespace SAGE;

TEST(FileSystem_IsSafePath) {
    // Valid safe paths
    ASSERT_TRUE(FileSystem::IsSafePath("textures/player.png"));
    ASSERT_TRUE(FileSystem::IsSafePath("data/config.json"));
    ASSERT_TRUE(FileSystem::IsSafePath("fonts/arial.ttf"));
    ASSERT_TRUE(FileSystem::IsSafePath("subfolder/file.txt"));
    
    // Invalid paths (directory traversal)
    ASSERT_FALSE(FileSystem::IsSafePath("../passwords.txt"));
    ASSERT_FALSE(FileSystem::IsSafePath("data/../../etc/passwd"));
    ASSERT_FALSE(FileSystem::IsSafePath("..\\..\\system32"));
    ASSERT_FALSE(FileSystem::IsSafePath("textures/../../../secret.key"));
    
    // Absolute paths (should be rejected for security)
    ASSERT_FALSE(FileSystem::IsSafePath("/etc/passwd"));
    ASSERT_FALSE(FileSystem::IsSafePath("C:\\Windows\\System32"));
    ASSERT_FALSE(FileSystem::IsSafePath("C:/Users/"));
    
    // Empty path
    ASSERT_FALSE(FileSystem::IsSafePath(""));
}

TEST(FileSystem_NormalizePath) {
    // Simple normalization
    std::string normalized = FileSystem::NormalizePath("textures/player.png", "assets");
    ASSERT_FALSE(normalized.empty());
    
    // Directory traversal should be rejected when base directory is set
    std::string unsafe = FileSystem::NormalizePath("../../../etc/passwd", "assets");
    ASSERT_TRUE(unsafe.empty());
    
    // Valid relative path within base
    std::string safe = FileSystem::NormalizePath("data/config.json", "assets");
    ASSERT_FALSE(safe.empty());
}

TEST(FileSystem_GetExtension) {
    ASSERT_EQ(FileSystem::GetExtension("texture.png"), "png");
    ASSERT_EQ(FileSystem::GetExtension("data.JSON"), "json");  // lowercase conversion
    ASSERT_EQ(FileSystem::GetExtension("path/to/file.TXT"), "txt");
    ASSERT_EQ(FileSystem::GetExtension("noext"), "");
    ASSERT_EQ(FileSystem::GetExtension("file.multiple.dots.tar.gz"), "gz");
}

int main(int argc, char** argv) {
    return RunAllTests();
}
