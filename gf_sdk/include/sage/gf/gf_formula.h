#ifndef SAGE_GF_FORMULA_H
#define SAGE_GF_FORMULA_H

#include <stddef.h>
#include "gf_core.h"
#include "gf_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gff_prog_s gff_prog_t;

GF_API int gff_compile(const char* src, gff_prog_t** out_prog, char* log_buf, size_t log_cap);
GF_API void gff_free(gff_prog_t* prog);
GF_API int gff_apply(gf_ctx_t* ctx, const gff_prog_t* prog);
GF_API int gff_set_param(gf_ctx_t* ctx, const char* name, int32_t value_q16_16);
GF_API int gff_write_file(const gff_prog_t* prog, const char* path);
GF_API int gff_read_file(const char* path, gff_prog_t** out_prog);

#ifdef __cplusplus
}
#endif

#endif // SAGE_GF_FORMULA_H
