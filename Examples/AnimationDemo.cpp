#include "SAGE/SAGE.h"
#include "SAGE/Graphics/Animation.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Core/Game.h"

using namespace SAGE;

class AnimationDemo : public Game {
public:
    AnimationDemo() : Game({
        .window = {
            .title = "Animation Demo",
            .width = 800,
            .height = 600
        },
        .renderer = {}
    }) {}

    void OnGameInit() override {
        // Create a procedural spritesheet texture (4x4 grid)
        int frameSize = 32;
        int gridSize = 4;
        int texSize = frameSize * gridSize;
        
        std::vector<unsigned char> data(texSize * texSize * 4);
        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                int col = x / frameSize;
                int row = y / frameSize;
                int index = (y * texSize + x) * 4;
                
                // Checkerboard pattern per frame
                bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;
                unsigned char val = isWhite ? 255 : 200;
                
                // Color tint based on frame index
                data[index + 0] = static_cast<unsigned char>(val * (col + 1) / gridSize); // R
                data[index + 1] = static_cast<unsigned char>(val * (row + 1) / gridSize); // G
                data[index + 2] = val;                        // B
                data[index + 3] = 255;                        // A
            }
        }

        m_Texture = Texture::CreateFromData(texSize, texSize, data.data());

        // Create Animation
        SpriteSheetAnimationBuilder builder(texSize, texSize, frameSize, frameSize);
        
        // Build a "Walk" animation (first row)
        AnimationClip walk = builder.BuildHorizontalStrip("Walk", 0, 4, 0.2f, true);
        m_Animator.AddClip(walk);

        // Build a "Run" animation (second row)
        AnimationClip run = builder.BuildHorizontalStrip("Run", 1, 4, 0.1f, true);
        m_Animator.AddClip(run);

        // Start playing
        m_Animator.Play("Walk");

        // Setup Sprite
        m_Sprite.SetTexture(m_Texture);
        m_Sprite.transform.position = {400.0f, 300.0f};
        m_Sprite.transform.scale = {5.0f, 5.0f}; // Scale up to see better
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
        if (Input::IsKeyPressed(KeyCode::Space)) {
            if (m_Animator.GetCurrentClip() == "Walk") {
                m_Animator.Play("Run");
            } else {
                m_Animator.Play("Walk");
            }
        }
    }

    void OnGameRender() override {
        Renderer::BeginSpriteBatch(&GetCamera());
        Renderer::SubmitSprite(m_Sprite);
        Renderer::FlushSpriteBatch();
    }

private:
    std::shared_ptr<Texture> m_Texture;
    Sprite m_Sprite;
    Animator m_Animator;
};

int main() {
    AnimationDemo app;
    app.Run();
    return 0;
}
