#include "SAGE/SAGE.h"
#include "SAGE/Core/ECSGame.h"
#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Core/ECSComponents.h"

using namespace SAGE;
using namespace SAGE::ECS;

// Components
struct CellComponent {
    int row;
    int col;
    int value = 0; // 0: Empty, 1: X, 2: O
};

struct GameStateComponent {
    int currentPlayer = 1; // 1: X, 2: O
    bool gameOver = false;
    float timer = 0.0f;
};

// System
class TicTacToeSystem : public ISystem {
    Camera2D& m_Camera;
public:
    TicTacToeSystem(Camera2D& camera) : m_Camera(camera) {}

    void Tick(Registry& reg, float deltaTime) override {
        Entity stateEntity = kInvalidEntity;
        reg.ForEach<GameStateComponent>([&](Entity e, GameStateComponent&) {
            stateEntity = e;
        });
        
        if (stateEntity == kInvalidEntity) return;
        
        auto* statePtr = reg.Get<GameStateComponent>(stateEntity);
        if (!statePtr) return;
        auto& state = *statePtr;

        if (state.gameOver) {
            state.timer += deltaTime;
            if (state.timer > 2.0f && Input::IsKeyPressed(KeyCode::Space)) {
                ResetGame(reg, state);
            }
            return;
        }

        if (Input::IsMouseButtonPressed(MouseButton::Left)) {
            Vector2 mouseScreen = Input::GetMousePosition();
            Vector2 mouseWorld = m_Camera.ScreenToWorld(mouseScreen);
            
            bool moveMade = false;
            reg.ForEach<CellComponent, TransformComponent, SpriteComponent>([&](Entity, CellComponent& cell, TransformComponent& trans, SpriteComponent& sprite) {
                if (moveMade) return;
                if (cell.value != 0) return;

                // Simple point in rect check
                // Cell size is 180x180 (scale)
                float halfSize = trans.scale.x * 0.5f;
                if (mouseWorld.x >= trans.position.x - halfSize && mouseWorld.x <= trans.position.x + halfSize &&
                    mouseWorld.y >= trans.position.y - halfSize && mouseWorld.y <= trans.position.y + halfSize) {
                    
                    cell.value = state.currentPlayer;
                    sprite.sprite.tint = (state.currentPlayer == 1) ? Color::Red() : Color::Blue();
                    
                    // Check win
                    if (CheckWin(reg, state.currentPlayer)) {
                        SAGE_INFO("Player {} Wins!", state.currentPlayer);
                        state.gameOver = true;
                    } else if (CheckDraw(reg)) {
                        SAGE_INFO("Draw!");
                        state.gameOver = true;
                    }
                    
                    state.currentPlayer = (state.currentPlayer == 1) ? 2 : 1;
                    moveMade = true;
                }
            });
        }
    }

    bool CheckWin(Registry& reg, int player) {
        int grid[3][3] = {0};
        reg.ForEach<CellComponent>([&](Entity, CellComponent& cell) {
            grid[cell.row][cell.col] = cell.value;
        });

        // Rows & Cols
        for (int i = 0; i < 3; i++) {
            if (grid[i][0] == player && grid[i][1] == player && grid[i][2] == player) return true;
            if (grid[0][i] == player && grid[1][i] == player && grid[2][i] == player) return true;
        }
        // Diagonals
        if (grid[0][0] == player && grid[1][1] == player && grid[2][2] == player) return true;
        if (grid[0][2] == player && grid[1][1] == player && grid[2][0] == player) return true;

        return false;
    }

    bool CheckDraw(Registry& reg) {
        bool full = true;
        reg.ForEach<CellComponent>([&](Entity, CellComponent& cell) {
            if (cell.value == 0) full = false;
        });
        return full;
    }

    void ResetGame(Registry& reg, GameStateComponent& state) {
        state.currentPlayer = 1;
        state.gameOver = false;
        state.timer = 0.0f;
        reg.ForEach<CellComponent, SpriteComponent>([&](Entity, CellComponent& cell, SpriteComponent& sprite) {
            cell.value = 0;
            sprite.sprite.tint = Color::White();
        });
        SAGE_INFO("Game Reset");
    }
};

class TicTacToe : public ECSGame {
public:
    TicTacToe() : ECSGame({ "Tic Tac Toe - Engine Test", 800, 600 }) {}

protected:
    void OnInit() override {
        ECSGame::OnInit();
        
        // Center camera on the board
        // Board is 3x3 cells of 180 + 10 gap.
        // Total width = 3*180 + 2*10 = 540 + 20 = 560.
        // Center is at 400, 300.
        // Top-Left of board: 400 - 560/2 = 120.
        
        GetCamera().SetPosition({400, 300});

        auto& reg = GetRegistry();
        auto& scheduler = GetScheduler();

        scheduler.AddSystem<TicTacToeSystem>(GetCamera());

        // Create Game State
        Entity state = reg.CreateEntity();
        reg.Add<GameStateComponent>(state);

        // Create Grid
        auto texture = Texture::CreateWhiteTexture();
        float cellSize = 180.0f;
        float gap = 10.0f;
        float totalSize = 3 * cellSize + 2 * gap;
        float startX = 400.0f - totalSize * 0.5f + cellSize * 0.5f;
        float startY = 300.0f - totalSize * 0.5f + cellSize * 0.5f;

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                Entity e = reg.CreateEntity();
                auto& trans = reg.Add<TransformComponent>(e);
                trans.position = { startX + col * (cellSize + gap), startY + row * (cellSize + gap) };
                trans.scale = { cellSize, cellSize };
                
                auto& sprite = reg.Add<SpriteComponent>(e);
                sprite.sprite = Sprite(texture);
                sprite.sprite.tint = Color::White(); // Empty

                auto& cell = reg.Add<CellComponent>(e);
                cell.row = row;
                cell.col = col;
            }
        }
        
        SAGE_INFO("Tic Tac Toe Initialized. Click to play. Space to reset after game over.");
    }
};

int main() {
    TicTacToe app;
    app.Run();
    return 0;
}
