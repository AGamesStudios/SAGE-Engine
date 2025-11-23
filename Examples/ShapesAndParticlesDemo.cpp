#include <SAGE/SAGE.h>
#include <vector>
#include <cmath>

using namespace SAGE;

class ShapesAndParticlesDemo : public Application {
public:
    ShapesAndParticlesDemo() 
        : Application({.window = {.title = "Shapes & Particles Demo", .width = 800, .height = 600}, .renderer = {}}) 
    {
        // Configure particle system
        ParticleSystem::EmitterConfig config;
        config.position = {400.0f, 300.0f};
        config.emissionRate = 50.0f;
        config.lifetimeMin = 1.0f;
        config.lifetimeMax = 2.0f;
        config.velocityMin = {-100.0f, -100.0f};
        config.velocityMax = {100.0f, 100.0f};
        config.startColor = Color::Yellow();
        config.endColor = Color::Red();
        config.sizeStart = 15.0f;
        config.sizeEnd = 0.0f;
        config.sizeVariation = 5.0f;
        
        m_ParticleSystem.SetEmitterConfig(config);
        m_ParticleSystem.Start();
    }

protected:
    void OnUpdate(double deltaTime) override {
        m_Time += static_cast<float>(deltaTime);
        
        // Update particles
        // Move emitter in a circle
        auto& config = const_cast<ParticleSystem::EmitterConfig&>(m_ParticleSystem.GetEmitterConfig());
        config.position.x = 400.0f + std::cos(m_Time * 2.0f) * 150.0f;
        config.position.y = 300.0f + std::sin(m_Time * 2.0f) * 150.0f;
        
        m_ParticleSystem.Update(static_cast<float>(deltaTime));

        Renderer::BeginFrame();
        Renderer::Clear(Color{0.1f, 0.1f, 0.15f, 1.0f});

        // Draw Shapes
        // 1. Square (Quad) - Filled only
        Renderer::DrawQuad({100.0f, 100.0f}, {80.0f, 80.0f}, Color::Green());
        
        // 2. Rect with Outline
        Renderer::DrawRect({250.0f, 100.0f}, {80.0f, 80.0f}, Color::Blue(), 2.0f, Color::White());

        // 3. Rect Outline Only
        Renderer::DrawRect({400.0f, 100.0f}, {80.0f, 80.0f}, Color::Transparent(), 3.0f, Color::Yellow());

        // 4. Circle
        Renderer::DrawCircle({700.0f, 100.0f}, 40.0f, Color::Cyan());
        
        // 5. Triangle
        Renderer::DrawTriangle(
            {400.0f, 200.0f},
            {350.0f, 300.0f},
            {450.0f, 300.0f},
            Color::Magenta()
        );

        // 4. Particles
        // We need to manually draw particles if the system doesn't auto-draw
        // The ParticleSystem class has GetParticles(), let's see if we can draw them.
        // The Renderer has DrawParticle.
        
        for (const auto& p : m_ParticleSystem.GetParticles()) {
            if (p.active) {
                Renderer::DrawParticle(p.position, p.size, p.color, p.rotation);
            }
        }

        Renderer::EndFrame();
    }

private:
    ParticleSystem m_ParticleSystem;
    float m_Time = 0.0f;
};

int main() {
    ShapesAndParticlesDemo app;
    app.Run();
    return 0;
}
