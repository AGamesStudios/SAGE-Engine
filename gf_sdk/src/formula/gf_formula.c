#include "sage/gf/gf_formula.h"
#include "sage/gf/gf_errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gff_prog_s {
    char* src; /* store original source for now */
};

GF_API int gff_compile(const char* src, gff_prog_t** out_prog, char* log_buf, size_t log_cap) {
    if (!src || !out_prog) return GF_E(GF_EINVAL);
    size_t len = strlen(src);
    if (len > 8192) return GF_E(GF_EOVER);
    gff_prog_t* p = (gff_prog_t*)malloc(sizeof(gff_prog_t));
    if (!p) return GF_E(GF_ENOMEM);
    p->src = (char*)malloc(len + 1);
    if (!p->src) { free(p); return GF_E(GF_ENOMEM); }
    memcpy(p->src, src, len + 1);
    *out_prog = p;
    if (log_buf && log_cap) {
        strncpy(log_buf, "OK", log_cap - 1);
        log_buf[log_cap-1]='\0';
    }
    return GF_OK;
}

GF_API void gff_free(gff_prog_t* prog) {
    if (prog) {
        free(prog->src);
        free(prog);
    }
}

GF_API int gff_apply(gf_ctx_t* ctx, const gff_prog_t* prog) {
    (void)ctx; (void)prog;
    return GF_OK;
}

GF_API int gff_set_param(gf_ctx_t* ctx, const char* name, int32_t val) {
    (void)ctx; (void)name; (void)val;
    return GF_OK;
}

GF_API int gff_write_file(const gff_prog_t* prog, const char* path) {
    if (!prog || !path) return GF_E(GF_EINVAL);
    FILE* f = fopen(path, "wb");
    if (!f) return GF_E(GF_EIO);
    fwrite(prog->src, 1, strlen(prog->src), f);
    fclose(f);
    return GF_OK;
}

GF_API int gff_read_file(const char* path, gff_prog_t** out_prog) {
    if (!path || !out_prog) return GF_E(GF_EINVAL);
    FILE* f = fopen(path, "rb");
    if (!f) return GF_E(GF_EIO);
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return GF_E(GF_ENOMEM); }
    fread(buf,1,sz,f); buf[sz]='\0';
    fclose(f);
    gff_prog_t* p = (gff_prog_t*)malloc(sizeof(gff_prog_t));
    if (!p) { free(buf); return GF_E(GF_ENOMEM); }
    p->src = buf; *out_prog = p; return GF_OK;
}
