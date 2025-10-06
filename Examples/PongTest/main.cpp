// Simple Pong Game - Testing SAGE Engine Core Features
#include <SAGE.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace SAGE;

class PongGame : public Application {
private:
    // Game state
    Vector2 ballPos;
    Vector2 ballVelocity;
    Vector2 paddle1Pos;
    Vector2 paddle2Pos;
    
    float paddleSpeed = 400.0f;
    int score1 = 0;
    int score2 = 0;
    
    // Screen dimensions
    const float SCREEN_WIDTH = 800.0f;
    const float SCREEN_HEIGHT = 600.0f;
    const float PADDLE_WIDTH = 20.0f;
    const float PADDLE_HEIGHT = 100.0f;
    const float BALL_SIZE = 15.0f;
    
public:
    PongGame() : Application("SAGE Engine Test - Pong") {
        srand((unsigned)time(nullptr));
        SAGE_INFO("===========================================");
        SAGE_INFO("  SAGE ENGINE - PONG TEST GAME");
        SAGE_INFO("===========================================");
    }
    
    void OnInit() override {
        SAGE_INFO("Initializing Pong Game...");
        
        // Initialize renderer
        Renderer::Init();
        
        // Initialize paddle positions
        paddle1Pos = {30.0f, SCREEN_HEIGHT / 2.0f};
        paddle2Pos = {SCREEN_WIDTH - 30.0f, SCREEN_HEIGHT / 2.0f};
        
        // Reset ball
        ResetBall();
        
        SAGE_INFO("===========================================");
        SAGE_INFO("Game initialized successfully!");
        SAGE_INFO("===========================================");
        SAGE_INFO("CONTROLS:");
        SAGE_INFO("  Player 1 (Green): W/S keys");
        SAGE_INFO("  Player 2 (Red): UP/DOWN arrows");
        SAGE_INFO("  ESC to quit");
        SAGE_INFO("===========================================");
    }
    
    void OnUpdate(float deltaTime) override {
        // Handle input
        HandleInput(deltaTime);
        
        // Update ball physics
        UpdateBall(deltaTime);
        
        // Check collisions
        CheckCollisions();
        
        // Check for escape
        if (Input::IsKeyPressed(SAGE_KEY_ESCAPE)) {
            SAGE_INFO("===========================================");
            SAGE_INFO("FINAL SCORE: Player 1: %d - Player 2: %d", score1, score2);
            SAGE_INFO("===========================================");
            Close();
        }
    }
    
    void OnRender() override {
        // Clear screen with dark blue background
        Renderer::Clear(0.1f, 0.1f, 0.15f, 1.0f);
        Renderer::BeginScene();
        
        // Draw center line (dashed)
        for (int i = 0; i < 12; i++) {
            QuadDesc centerLine;
            centerLine.position = {SCREEN_WIDTH / 2.0f, i * 50.0f + 25.0f};
            centerLine.size = {5.0f, 30.0f};
            centerLine.color = Color(0.3f, 0.3f, 0.3f, 0.5f);
            Renderer::DrawQuad(centerLine);
        }
        
        // Draw paddle 1 (Green)
        QuadDesc p1Quad;
        p1Quad.position = paddle1Pos;
        p1Quad.size = {PADDLE_WIDTH, PADDLE_HEIGHT};
        p1Quad.color = Color::Green();
        Renderer::DrawQuad(p1Quad);
        
        // Draw paddle 2 (Red)
        QuadDesc p2Quad;
        p2Quad.position = paddle2Pos;
        p2Quad.size = {PADDLE_WIDTH, PADDLE_HEIGHT};
        p2Quad.color = Color::Red();
        Renderer::DrawQuad(p2Quad);
        
        // Draw ball (White)
        QuadDesc ballQuad;
        ballQuad.position = ballPos;
        ballQuad.size = {BALL_SIZE, BALL_SIZE};
        ballQuad.color = Color::White();
        Renderer::DrawQuad(ballQuad);
        
        Renderer::EndScene();
    }
    
    void OnShutdown() override {
        Renderer::Shutdown();
        SAGE_INFO("===========================================");
        SAGE_INFO("SAGE ENGINE TEST - Pong Game Shutdown");
        SAGE_INFO("===========================================");
    }
    
private:
    void HandleInput(float deltaTime) {
        float displacement = paddleSpeed * deltaTime;
        
        // Player 1 controls (W/S)
        if (Input::IsKeyPressed(SAGE_KEY_W)) {
            paddle1Pos.y = std::min(paddle1Pos.y + displacement, SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f);
        }
        if (Input::IsKeyPressed(SAGE_KEY_S)) {
            paddle1Pos.y = std::max(paddle1Pos.y - displacement, PADDLE_HEIGHT / 2.0f);
        }
        
        // Player 2 controls (Arrow keys)
        if (Input::IsKeyPressed(SAGE_KEY_UP)) {
            paddle2Pos.y = std::min(paddle2Pos.y + displacement, SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f);
        }
        if (Input::IsKeyPressed(SAGE_KEY_DOWN)) {
            paddle2Pos.y = std::max(paddle2Pos.y - displacement, PADDLE_HEIGHT / 2.0f);
        }
    }
    
    void UpdateBall(float deltaTime) {
        ballPos.x += ballVelocity.x * deltaTime;
        ballPos.y += ballVelocity.y * deltaTime;
        
        // Bounce off top/bottom walls
        if (ballPos.y <= BALL_SIZE / 2.0f || ballPos.y >= SCREEN_HEIGHT - BALL_SIZE / 2.0f) {
            ballVelocity.y *= -1.0f;
            ballPos.y = std::max(BALL_SIZE / 2.0f, std::min(ballPos.y, SCREEN_HEIGHT - BALL_SIZE / 2.0f));
        }
    }
    
    void CheckCollisions() {
        // Check paddle 1 collision
        if (ballPos.x - BALL_SIZE / 2.0f <= paddle1Pos.x + PADDLE_WIDTH / 2.0f &&
            ballPos.x >= paddle1Pos.x - PADDLE_WIDTH / 2.0f &&
            ballPos.y >= paddle1Pos.y - PADDLE_HEIGHT / 2.0f &&
            ballPos.y <= paddle1Pos.y + PADDLE_HEIGHT / 2.0f) {
            
            ballVelocity.x = std::abs(ballVelocity.x);
            float hitPos = (ballPos.y - paddle1Pos.y) / (PADDLE_HEIGHT / 2.0f);
            ballVelocity.y = hitPos * 400.0f;
            SAGE_INFO("Paddle 1 hit! Score: %d - %d", score1, score2);
        }
        
        // Check paddle 2 collision
        if (ballPos.x + BALL_SIZE / 2.0f >= paddle2Pos.x - PADDLE_WIDTH / 2.0f &&
            ballPos.x <= paddle2Pos.x + PADDLE_WIDTH / 2.0f &&
            ballPos.y >= paddle2Pos.y - PADDLE_HEIGHT / 2.0f &&
            ballPos.y <= paddle2Pos.y + PADDLE_HEIGHT / 2.0f) {
            
            ballVelocity.x = -std::abs(ballVelocity.x);
            float hitPos = (ballPos.y - paddle2Pos.y) / (PADDLE_HEIGHT / 2.0f);
            ballVelocity.y = hitPos * 400.0f;
            SAGE_INFO("Paddle 2 hit! Score: %d - %d", score1, score2);
        }
        
        // Check scoring
        if (ballPos.x < 0) {
            score2++;
            SAGE_INFO(">>> GOAL! Player 2 scores! Score: %d - %d <<<", score1, score2);
            ResetBall();
        } else if (ballPos.x > SCREEN_WIDTH) {
            score1++;
            SAGE_INFO(">>> GOAL! Player 1 scores! Score: %d - %d <<<", score1, score2);
            ResetBall();
        }
    }
    
    void ResetBall() {
        ballPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        
        // Random direction
        float angle = (rand() % 120 - 60) * 3.14159f / 180.0f;
        float speed = 300.0f;
        ballVelocity.x = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed * std::cos(angle);
        ballVelocity.y = speed * std::sin(angle);
        
        SAGE_INFO("Ball reset - Velocity: (%.1f, %.1f)", ballVelocity.x, ballVelocity.y);
    }
};

SAGE::Application* SAGE::CreateApplication() {
    return new PongGame();
}

int main() {
    auto* app = SAGE::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
