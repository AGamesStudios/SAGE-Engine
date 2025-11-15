#include "TestFramework.h"
#include "Editor/EditorScene.h"
#include "Editor/Undo/EditorCommands.h"
#include <nlohmann/json.hpp>
#include <fstream>

using namespace SAGE; using namespace SAGE::Editor;

TEST_CASE("EditorScene_UniqueNames") {
    EditorScene scene;
    auto e1 = scene.CreateEntity("Entity");
    auto e2 = scene.CreateEntity("Entity");
    auto e3 = scene.CreateEntity("Entity");
    REQUIRE(ECS::IsValid(e1)); REQUIRE(ECS::IsValid(e2)); REQUIRE(ECS::IsValid(e3));
    const auto& ents = scene.GetEntities();
    REQUIRE(ents.size() == 3);
    REQUIRE(ents[0].name != ents[1].name);
    REQUIRE(ents[1].name != ents[2].name);
}

TEST_CASE("EditorScene_RenameEdge") {
    EditorScene scene; auto e = scene.CreateEntity("A");
    REQUIRE(scene.RenameEntity(e, "" ) == false); // empty rejected
    REQUIRE(scene.RenameEntity(e, "A") == true); // same name allowed no change
    REQUIRE(scene.RenameEntity(e, "B") == true);
    const auto* rec = scene.FindRecord(e);
    REQUIRE(rec && rec->name == "B");
}

TEST_CASE("EditorScene_Duplicate") {
    EditorScene scene; auto e = scene.CreateEntity("Base");
    auto* sprite = scene.GetSprite(e); REQUIRE(sprite);
    sprite->width = 77.0f; sprite->height = 55.0f;
    ECS::Entity dup = scene.DuplicateEntity(e, "Copy");
    REQUIRE(ECS::IsValid(dup));
    auto* dupSprite = scene.GetSprite(dup); REQUIRE(dupSprite);
    REQUIRE(dupSprite->width == 77.0f);
    REQUIRE(dupSprite->height == 55.0f);
}

TEST_CASE("EditorScene_RoundTripJSON") {
    EditorScene scene; auto e = scene.CreateEntity("Sprite");
    auto* t = scene.GetTransform(e); REQUIRE(t); t->position.x = 12.3f; t->position.y = -4.2f; t->rotation = 33.0f; t->scale.x = 2.0f; t->scale.y = 3.0f;
    auto* s = scene.GetSprite(e); REQUIRE(s); s->width = 64; s->height = 128; s->flipX = true; s->tint.r = 0.2f;

    std::string path = "editor_scene_test.json";
    REQUIRE(scene.SaveToFile(path));

    EditorScene loaded;
    REQUIRE(loaded.LoadFromFile(path));
    const auto& ents = loaded.GetEntities(); REQUIRE(ents.size() == 1);
    auto* lt = loaded.GetTransform(ents[0].id); REQUIRE(lt);
    REQUIRE(lt->position.x == Approx(12.3f));
    REQUIRE(lt->position.y == Approx(-4.2f));
    REQUIRE(lt->rotation == Approx(33.0f));
    auto* ls = loaded.GetSprite(ents[0].id); REQUIRE(ls); REQUIRE(ls->flipX == true); REQUIRE(ls->width == Approx(64.0f));
}

TEST_CASE("UndoRedo_CreateDelete") {
    EditorScene scene;
    UndoStack stack;
    REQUIRE(scene.GetEntities().empty());
    stack.Push(std::make_unique<CreateEntityCommand>("One"), scene);
    REQUIRE(scene.GetEntities().size() == 1);
    stack.Undo(scene); REQUIRE(scene.GetEntities().empty());
    stack.Redo(scene); REQUIRE(scene.GetEntities().size() == 1);

    ECS::Entity e = scene.GetEntities().front().id;
    stack.Push(std::make_unique<DeleteEntityCommand>(e), scene);
    REQUIRE(scene.GetEntities().empty());
    stack.Undo(scene); REQUIRE(scene.GetEntities().size() == 1);
}
