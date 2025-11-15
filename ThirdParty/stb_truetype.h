// Local fallback stub for stb_truetype.h when network download fails
// WARNING: This is a stub implementation that returns zeros/failures for all operations.
// Download real stb_truetype.h from https://github.com/nothings/stb/blob/master/stb_truetype.h
#ifndef STB_TRUETYPE_H_FALLBACK
#define STB_TRUETYPE_H_FALLBACK

#ifdef __cplusplus
extern "C" {
#endif

// Stub implementation - will cause runtime failures in production if used
#ifndef SAGE_ALLOW_STUB_STB
#pragma message("WARNING: Using stub stb_truetype.h - font rendering will fail! Download real header from stb repository.")
#endif

typedef struct stbtt_fontinfo { int dummy; } stbtt_fontinfo;
typedef struct stbtt_packedchar { unsigned short x0,y0,x1,y1; short xoff,yoff,xoff2,yoff2; unsigned short xadvance; } stbtt_packedchar;
typedef struct stbtt_pack_range { float font_size; int first_unicode_codepoint_in_range; int *array_of_unicode_codepoints; int num_chars; stbtt_packedchar *chardata_for_range; unsigned char h_oversample, v_oversample; } stbtt_pack_range;
typedef struct stbtt_pack_context { int dummy; } stbtt_pack_context;

int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset) { 
    (void)info; (void)data; (void)offset; return 0; 
}
float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float pixels) { 
    (void)info; (void)pixels; return 1.0f; 
}
void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap) { 
    (void)info; if(ascent) *ascent=0; if(descent) *descent=0; if(lineGap) *lineGap=0; 
}
int stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2) { 
    (void)info; (void)ch1; (void)ch2; return 0; 
}
int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context) { 
    (void)spc; (void)pixels; (void)width; (void)height; (void)stride_in_bytes; (void)padding; (void)alloc_context; return 0; 
}
void stbtt_PackEnd(stbtt_pack_context *spc) { (void)spc; }
void stbtt_PackSetOversampling(stbtt_pack_context *spc, unsigned char h_oversample, unsigned char v_oversample) { 
    (void)spc; (void)h_oversample; (void)v_oversample; 
}
int stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges) { 
    (void)spc; (void)fontdata; (void)font_index; (void)ranges; (void)num_ranges; return 0; 
}

#ifdef __cplusplus
}
#endif

#endif // STB_TRUETYPE_H_FALLBACK
