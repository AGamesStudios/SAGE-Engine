#pragma once
#include <string>
#include <cstdint>

namespace SAGE {

    struct EngineConfig {
        std::string AssetsDirectory = "assets"; // base assets path
        std::string SpritesheetPath = "assets/sheet.png"; // default demo spritesheet
        unsigned DefaultSheetCellW = 16;
        unsigned DefaultSheetCellH = 16;
        unsigned DefaultSheetMargin = 0;
        unsigned DefaultSheetSpacing = 0;
        bool AutoAnalyzeSpritesheet = false; // if true ignore provided cell sizes and run Analyze
        // Populated after auto-analysis (if enabled) to override defaults
        unsigned AnalyzedCellW = 0;
        unsigned AnalyzedCellH = 0;
        unsigned AnalyzedColumns = 0;
        unsigned AnalyzedRows = 0;
        bool AnalysisSucceeded = false;
    };

    // Load EngineConfig from minimal JSON (very small hand-rolled parser to avoid dependency here)
    // Supports keys: assetsDirectory, spritesheetPath, cellW, cellH, margin, spacing, autoAnalyze
    EngineConfig LoadEngineConfig(const std::string& path);
}
