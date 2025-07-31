#include "sage/gf/gf_export.h"
#include "sage/gf/gf_errors.h"
#include <stdio.h>

GF_API int gf_write_gf(gf_ctx_t* ctx, const char* path) {
    (void)ctx; (void)path; return GF_OK; }

GF_API int gf_analyze_to_gfr(const char* gf_path, const char* gff_path, const char* gfr_path) {
    (void)gf_path; (void)gff_path; (void)gfr_path; return GF_OK; }
