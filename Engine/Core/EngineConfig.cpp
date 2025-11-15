#include "EngineConfig.h"
#include "Core/Logger.h"
// Added includes for PNG decoding and spritesheet analysis
#include "Graphics/Core/Utils/PNGLoader.h"
#include "Graphics/Core/Resources/Spritesheet.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

namespace SAGE {

EngineConfig LoadEngineConfig(const std::string& path) {
    EngineConfig cfg; // defaults
    std::ifstream in(path);
    if(!in.is_open()) {
        SAGE_WARNING("EngineConfig: could not open '%s', using defaults", path.c_str());
        return cfg;
    }
    std::stringstream ss; ss << in.rdbuf();
    std::string text = ss.str();
    // Very naive JSON-ish parsing (expects { "key": value, ... })
    auto extractString = [&](const std::string& key){
        auto pos = text.find("\""+key+"\"");
        if(pos==std::string::npos) return std::string();
        pos = text.find(':', pos);
        if(pos==std::string::npos) return std::string();
        pos++;
        while(pos<text.size() && (text[pos]==' '||text[pos]=='\t')) pos++;
        if(pos<text.size() && text[pos]=='\"') {
            size_t end = text.find('"', pos+1);
            if(end!=std::string::npos) return text.substr(pos+1, end-pos-1);
        }
        return std::string();
    };
    auto extractUInt = [&](const std::string& key){
        auto pos = text.find("\""+key+"\"");
        if(pos==std::string::npos) return 0u;
        pos = text.find(':', pos);
        if(pos==std::string::npos) return 0u;
        pos++;
        std::string num;
        while(pos<text.size() && (isdigit(text[pos])||text[pos]==' ')) { if(isdigit(text[pos])) num.push_back(text[pos]); pos++; }
        if(num.empty()) return 0u; return static_cast<unsigned>(std::stoul(num));
    };
    auto extractBool = [&](const std::string& key){
        auto pos = text.find("\""+key+"\"");
        if(pos==std::string::npos) return false;
        pos = text.find(':', pos);
        if(pos==std::string::npos) return false;
        pos++;
        std::string word;
        while(pos<text.size() && (isalpha(text[pos])||text[pos]==' ')) { if(isalpha(text[pos])) word.push_back(text[pos]); pos++; }
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        return word=="true";
    };

    std::string assetsDir = extractString("assetsDirectory");
    if(!assetsDir.empty()) cfg.AssetsDirectory = assetsDir;
    std::string sheetPath = extractString("spritesheetPath");
    if(!sheetPath.empty()) cfg.SpritesheetPath = sheetPath;
    unsigned cw = extractUInt("cellW"); if(cw) cfg.DefaultSheetCellW = cw;
    unsigned ch = extractUInt("cellH"); if(ch) cfg.DefaultSheetCellH = ch;
    unsigned margin = extractUInt("margin"); cfg.DefaultSheetMargin = margin; // allow 0
    unsigned spacing = extractUInt("spacing"); cfg.DefaultSheetSpacing = spacing; // allow 0
    bool autoAnalyze = extractBool("autoAnalyze"); cfg.AutoAnalyzeSpritesheet = autoAnalyze;

    SAGE_INFO("EngineConfig loaded: assets='%s' sheet='%s' cell=(%u,%u) margin=%u spacing=%u autoAnalyze=%d", cfg.AssetsDirectory.c_str(), cfg.SpritesheetPath.c_str(), cfg.DefaultSheetCellW, cfg.DefaultSheetCellH, cfg.DefaultSheetMargin, cfg.DefaultSheetSpacing, cfg.AutoAnalyzeSpritesheet);
    if(cfg.AutoAnalyzeSpritesheet) {
        // Attempt to open spritesheet file and run lightweight pixel-based analysis
        std::ifstream f(cfg.SpritesheetPath, std::ios::binary | std::ios::ate);
        if(!f.is_open()) {
            SAGE_WARNING("EngineConfig: autoAnalyze enabled but cannot open '%s'", cfg.SpritesheetPath.c_str());
            return cfg;
        }
        auto size = f.tellg(); f.seekg(0);
        std::vector<unsigned char> buffer(static_cast<size_t>(size));
        if(!f.read(reinterpret_cast<char*>(buffer.data()), size)) {
            SAGE_WARNING("EngineConfig: failed to read '%s' for analysis", cfg.SpritesheetPath.c_str());
            return cfg;
        }
        // Use PNG decoder to get pixels
        auto img = SAGE::Image::PNGImageDecoder::LoadFromMemory(buffer.data(), buffer.size());
        if(!img.IsValid()) {
            SAGE_WARNING("EngineConfig: PNG decode failed for '%s'", cfg.SpritesheetPath.c_str());
            return cfg;
        }
        auto analysis = SAGE::Spritesheet::Analyze(img.width, img.height, img.pixels, 8, 128, true, true);
        if(analysis.bestIndex >= 0) {
            const auto& cand = analysis.candidates[analysis.bestIndex];
            cfg.AnalyzedCellW = cand.cellW;
            cfg.AnalyzedCellH = cand.cellH;
            cfg.AnalyzedColumns = cand.cols;
            cfg.AnalyzedRows = cand.rows;
            cfg.AnalysisSucceeded = true;
            SAGE_INFO("EngineConfig: auto-analysis succeeded cell=(%u,%u) grid=(%u x %u)", cand.cellW, cand.cellH, cand.cols, cand.rows);
        } else {
            SAGE_WARNING("EngineConfig: auto-analysis found no suitable grid for '%s'", cfg.SpritesheetPath.c_str());
        }
    }
    return cfg;
}

}
