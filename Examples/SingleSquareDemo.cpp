#include <SAGE/SAGE.h>

using namespace SAGE;

class SingleSquareDemo : public Application {
public:
    SingleSquareDemo() : Application({.window = {.title = "Single Square Demo", .width = 800, .height = 600}}) {}

protected:
    void OnUpdate(double deltaTime) override {
        Renderer::BeginFrame();
        Renderer::Clear(Color::Black());

        // Draw a red square in the center of the screen
        // Screen size is 800x600, so center is (400, 300)
        Renderer::DrawQuad({400.0f, 300.0f}, {100.0f, 100.0f}, Color::Red());

        Renderer::EndFrame();
    }
};

int main() {
    SingleSquareDemo app;
    app.Run();
    return 0;
}
