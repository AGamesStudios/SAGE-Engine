#include "SAGE/SAGE.h"
#include "SAGE/Graphics/Animation.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Core/Game.h"
#include "SAGE/Graphics/Texture.h"

using namespace SAGE;

class CatAnimationDemo : public Game {
public:
    CatAnimationDemo() : Game({
        .window = {
            .title = "Cat Animation Demo",
            .width = 800,
            .height = 600
        },
        .renderer = {}
    }) {}

    void OnGameInit() override {
        // Load the spritesheet texture
        // Assuming the file is in assets/Basic Charakter Spritesheet.png
        // We need to make sure the path is correct relative to the executable or working directory.
        // Usually assets are in the root or copied to bin.
        // Let's try loading from "assets/Basic Charakter Spritesheet.png"
        m_Texture = Texture::Create("assets/Basic Charakter Spritesheet.png");
        
        if (!m_Texture) {
            // Fallback if texture fails to load (e.g. path issue)
            // Create a placeholder texture
            std::vector<unsigned char> data(4 * 4 * 4, 255); // 4x4 white texture
            m_Texture = Texture::CreateFromData(4, 4, data.data());
            SAGE_ERROR("Failed to load texture, using fallback.");
        }

        // Spritesheet details
        int textureWidth = m_Texture->GetWidth();
        int textureHeight = m_Texture->GetHeight();
        int frameWidth = 48;  // 192 / 4
        int frameHeight = 48; // 192 / 4

        // Create Animation Builder
        SpriteSheetAnimationBuilder builder(textureWidth, textureHeight, frameWidth, frameHeight);
        
        // Build animations
        // Row 0: Idle
        AnimationClip idle = builder.BuildHorizontalStrip("Idle", 0, 4, 0.2f, true);
        m_Animator.AddClip(idle);

        // Row 1: Action
        AnimationClip action = builder.BuildHorizontalStrip("Action", 1, 4, 0.2f, true);
        m_Animator.AddClip(action);

        // Row 2: Walk
        AnimationClip walk = builder.BuildHorizontalStrip("Walk", 2, 4, 0.15f, true);
        m_Animator.AddClip(walk);

        // Row 3: Run (or whatever the 4th row is)
        AnimationClip run = builder.BuildHorizontalStrip("Run", 3, 4, 0.1f, true);
        m_Animator.AddClip(run);

        // Start playing Idle
        m_Animator.Play("Idle");

        // Setup Sprite
        m_Sprite.SetTexture(m_Texture);
        m_Sprite.transform.position = {400.0f, 300.0f};
        m_Sprite.transform.scale = {4.0f, 4.0f}; // Scale up
    }

    void OnGameUpdate(float deltaTime) override {
        // Update Animator
        m_Animator.Update(deltaTime);

        // Apply current frame to sprite
        const auto* frame = m_Animator.GetCurrentFrameData();
        if (frame) {
            m_Sprite.textureRect = frame->uvRect;
        }

        // Input to switch animations
        if (Input::IsKeyPressed(KeyCode::Num1)) {
            m_Animator.Play("Idle");
        }
        if (Input::IsKeyPressed(KeyCode::Num2)) {
            m_Animator.Play("Action");
        }
        if (Input::IsKeyPressed(KeyCode::Num3)) {
            m_Animator.Play("Walk");
        }
        if (Input::IsKeyPressed(KeyCode::Num4)) {
            m_Animator.Play("Run");
        }
    }

    void OnGameRender() override {
        Renderer::BeginSpriteBatch(&GetCamera());
        
        // Draw instructions
        // (Assuming we have a text renderer or just print to console)
        // For now just the sprite
        
        Renderer::SubmitSprite(m_Sprite);
        Renderer::FlushSpriteBatch();
    }

private:
    std::shared_ptr<Texture> m_Texture;
    Sprite m_Sprite;
    Animator m_Animator;
};

int main() {
    CatAnimationDemo app;
    app.Run();
    return 0;
}
