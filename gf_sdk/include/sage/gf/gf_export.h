#ifndef SAGE_GF_EXPORT_H
#define SAGE_GF_EXPORT_H

#include "gf_core.h"
#include "gf_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

GF_API int gf_write_gf(gf_ctx_t* ctx, const char* path);
GF_API int gf_analyze_to_gfr(const char* gf_path, const char* gff_path, const char* gfr_out_path);

#ifdef __cplusplus
}
#endif

#endif // SAGE_GF_EXPORT_H
