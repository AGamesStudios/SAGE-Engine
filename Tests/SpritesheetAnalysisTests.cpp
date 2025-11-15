#include "Graphics/Core/Resources/Spritesheet.h"
#include <vector>
#include <cassert>
#include <iostream>

// Helper: create a 192x192 RGBA grid with 4x4 cells of 48x48, transparent separators
std::vector<unsigned char> MakeTestSheetPixels() {
    const int W = 192, H = 192, cell = 48;
    std::vector<unsigned char> pixels(W * H * 4, 0);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            // Separator lines: every 48th row/col is transparent
            if ((x % cell == 0) || (y % cell == 0)) {
                pixels[(y * W + x) * 4 + 3] = 0; // alpha = 0
            } else {
                pixels[(y * W + x) * 4 + 0] = 255; // R
                pixels[(y * W + x) * 4 + 1] = 128; // G
                pixels[(y * W + x) * 4 + 2] = 64;  // B
                pixels[(y * W + x) * 4 + 3] = 255; // alpha = opaque
            }
        }
    }
    return pixels;
}

void TestSpritesheetAutoAnalysis() {
    auto pixels = MakeTestSheetPixels();
    auto result = SAGE::Spritesheet::Analyze(192, 192, pixels, 8, 64, true, true);
    assert(result.bestIndex >= 0);
    const auto& cand = result.candidates[result.bestIndex];
    assert(cand.cellW == 48 && cand.cellH == 48);
    assert(cand.cols == 4 && cand.rows == 4);
    std::cout << "Auto-analysis detected cell: " << cand.cellW << "x" << cand.cellH << ", grid: " << cand.cols << "x" << cand.rows << std::endl;
}

int main() {
    TestSpritesheetAutoAnalysis();
    std::cout << "Spritesheet auto-analysis test passed." << std::endl;
    return 0;
}
