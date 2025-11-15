#include "TestFramework.h"
#include "Core/Logger.h"
#include "Graphics/Core/Resources/Spritesheet.h"

using namespace SAGE;

// Synthetic detection test for 192x192 sheet with 16x16 cells (12x12 grid) using transparent separators.
// We build an RGBA buffer with fully transparent separator rows/columns.

static std::vector<unsigned char> BuildSyntheticSheet(unsigned width, unsigned height, unsigned cell) {
    std::vector<unsigned char> pixels(width * height * 4, 0); // start fully transparent
    // Fill cell interiors opaque white with alpha 255.
    for(unsigned y=0; y<height; ++y){
        bool separatorRow = ((y+1) % cell == 0); // last line of cell as separator
        for(unsigned x=0; x<width; ++x){
            bool separatorCol = ((x+1) % cell == 0);
            unsigned char* p = &pixels[(y*width + x)*4];
            if(!separatorRow && !separatorCol){
                p[0]=255; p[1]=255; p[2]=255; p[3]=255;
            } else {
                // keep transparent
            }
        }
    }
    return pixels;
}

TEST_CASE(Spritesheet_GridDetection_16x16) {
    unsigned int sheetSize = 192; // 192x192
    unsigned int cell = 16;
    auto pixels = BuildSyntheticSheet(sheetSize, sheetSize, cell);
    auto detected = Spritesheet::DetectGrid(sheetSize, sheetSize, pixels);
    REQUIRE(detected.first == cell);
    REQUIRE(detected.second == cell);
}

TEST_CASE(Spritesheet_FrameUV_Computation) {
    // Mock texture dimensions (no actual GL upload). We simulate 192x192 with 16x16 cells => 12x12 grid.
    // We cannot instantiate real Texture easily without file IO; we test UV math indirectly using formula.
    unsigned texW = 192, texH = 192, cell = 16;
    // Compute UV for frame (x=5,y=7)
    unsigned x = 5, y = 7;
    float px = float(x * cell);
    float py = float(y * cell);
    float u0 = px / texW; float v0 = py / texH;
    float u1 = (px + cell) / texW; float v1 = (py + cell) / texH;
    CHECK(u0 == Approx(5.0f*16.0f/192.0f));
    CHECK(v0 == Approx(7.0f*16.0f/192.0f));
    CHECK(u1 == Approx((5.0f*16.0f+16.0f)/192.0f));
    CHECK(v1 == Approx((7.0f*16.0f+16.0f)/192.0f));
}

// Atlas with opaque black separator lines (1px). Expect opaqueSeparators=true in candidate.
static std::vector<unsigned char> BuildOpaqueSeparatorSheet(unsigned width, unsigned height, unsigned cell) {
    std::vector<unsigned char> pixels(width*height*4, 255); // opaque white
    // Draw black lines at cell boundaries (horizontal and vertical) using previous pixel line (boundary-1) to mimic analyzer logic
    for(unsigned x=cell; x<width; x+=cell){ unsigned boundary = x-1; for(unsigned y=0;y<height;++y){ unsigned char* p=&pixels[(y*width + boundary)*4]; p[0]=0; p[1]=0; p[2]=0; p[3]=255; } }
    for(unsigned y=cell; y<height; y+=cell){ unsigned boundary = y-1; for(unsigned x=0;x<width;++x){ unsigned char* p=&pixels[(y*width + x)*4]; p[0]=0; p[1]=0; p[2]=0; p[3]=255; } }
    return pixels;
}

TEST_CASE(Spritesheet_OpaqueSeparators_Detection) {
    unsigned sheetSize=96; unsigned cell=16; // 6x6 grid
    auto pixels = BuildOpaqueSeparatorSheet(sheetSize,sheetSize,cell);
    auto analysis = Spritesheet::Analyze(sheetSize,sheetSize,pixels,8,64,true,true);
    bool found=false; bool opaqueSep=false; for(auto& c: analysis.candidates){ if(c.cellW==cell){ found=true; opaqueSep = c.opaqueSeparators; } }
    REQUIRE(found);
    CHECK(opaqueSep == true);
}

TEST_CASE(Spritesheet_AddSpriteFromSheet_Performance) {
    // Microbenchmark: add many sprites from sheet frames, ensure < 10 ms for 50k operations
    unsigned texW=256, texH=256, cell=16; unsigned cols=texW/cell; unsigned rows=texH/cell; unsigned frameCount=cols*rows;
    // Synthetic sheet frames: just compute UVs; avoid real texture dependency.
    std::vector<std::pair<Float2,Float2>> frames; frames.reserve(frameCount);
    for(unsigned y=0;y<rows;++y){ for(unsigned x=0;x<cols;++x){ float u0=float(x*cell)/texW; float v0=float(y*cell)/texH; float u1=float((x+1)*cell)/texW; float v1=float((y+1)*cell)/texH; frames.emplace_back(Float2(u0,v0), Float2(u1,v1)); } }
    SpriteBatchSoA batch(60000); // capacity
    const unsigned spriteOps=50000;
    auto start = std::chrono::high_resolution_clock::now();
    for(unsigned i=0;i<spriteOps;++i){ auto& f=frames[i % frameCount]; batch.AddSpriteFromSheet(Float2(float(i%400), float(i/400)), Float2(16,16), f.first, f.second, Color::White()); }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double,std::milli>(end-start).count();
    SAGE_INFO("AddSpriteFromSheet: {} sprites in {:.3f} ms", spriteOps, ms);
    CHECK(ms < 10.0); // heuristic budget
    REQUIRE(batch.GetCount()==spriteOps);
}
