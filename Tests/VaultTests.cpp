#include "TestFramework.h"

#include <SAGE.h>

#include <filesystem>

using namespace sage2d;

namespace {
    std::filesystem::path dataDir() {
        static const std::filesystem::path base = std::filesystem::path(__FILE__).parent_path() / "Data";
        return base;
    }
}

TEST_CASE(Vault_CachesResourcesAndRefCounts) {
    Vault vault;
    auto imagePath = dataDir() / "player.png";

    ResId first = vault.image(imagePath);
    CHECK(IsValid(first));
    CHECK(vault.refCount(first) == 1);

    ResId second = vault.image(imagePath);
    CHECK(first == second);
    CHECK(vault.refCount(first) == 2);

    const auto* image = vault.getImage(first);
    REQUIRE(image != nullptr);
    CHECK(image->exists);
    CHECK(std::filesystem::equivalent(image->source, imagePath));

    vault.release(first);
    CHECK(vault.refCount(first) == 1);
    vault.release(first);
    CHECK(vault.refCount(first) == 0);
}

TEST_CASE(Vault_LoadsRoleFromYaml) {
    Vault vault;
    auto rolePath = dataDir() / "role_player.yaml";

    ResId roleId = vault.roleFromFile(rolePath);
    REQUIRE(IsValid(roleId));

    const Role* role = vault.getRole(roleId);
    REQUIRE(role != nullptr);
    CHECK(role->name == "Player");
    REQUIRE(role->sprite.has_value());
    const Sprite& sprite = *role->sprite;
    CHECK(IsValid(sprite.image));
    CHECK(IsValid(sprite.animation));
    CHECK(sprite.size.x == 32.0f);
    CHECK(sprite.size.y == 48.0f);

    const auto* spriteImage = vault.getImage(sprite.image);
    REQUIRE(spriteImage != nullptr);
    CHECK(spriteImage->exists);

    REQUIRE(role->physics.has_value());
    CHECK(role->physics->mass == 1.2f);
    CHECK(role->physics->gravityScale == 0.9f);
    CHECK(role->physics->kinematic == false);

    REQUIRE(role->collider.has_value());
    CHECK(role->collider->w == 28.0f);

    REQUIRE(role->controls.has_value());
    CHECK(role->controls->left == 'A');

    REQUIRE(role->script.has_value());
    CHECK(role->script->binding == "PlayerUpdate");

    // Query by name should reuse the same resource id
    const Role* byName = vault.getRole("player");
    REQUIRE(byName != nullptr);
    CHECK(byName == role);
}

TEST_CASE(Vault_LoadsSkinFromJson) {
    Vault vault;
    auto skinPath = dataDir() / "skin_night.json";

    ResId skinId = vault.skinFromFile(skinPath);
    REQUIRE(IsValid(skinId));

    const Skin* skin = vault.getSkin(skinId);
    REQUIRE(skin != nullptr);
    CHECK(skin->name == "Night");
    CHECK(skin->imageOverrides.at("player") == "player_night.png");
    CHECK(skin->soundOverrides.at("footstep") == "footstep_night.wav");
    CHECK(skin->animationOverrides.at("player_walk") == "walk.anim");

    const Skin* byName = vault.getSkin("night");
    REQUIRE(byName != nullptr);
    CHECK(byName == skin);
}
