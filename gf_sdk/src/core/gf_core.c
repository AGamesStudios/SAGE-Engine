#include "sage/gf/gf_core.h"
#include "sage/gf/gf_errors.h"
#include <stdlib.h>
#include <string.h>

struct gf_ctx_s {
    gf_cfg_t cfg;
    uint32_t frame_count;
    uint16_t last_ft;
    uint16_t last_lat;
    gf_error_info_t last_err;
};

static void set_last_error(gf_ctx_t* ctx, int32_t code, uint32_t detail,
                           uint32_t line, const char* where){
    if(!ctx) return;
    ctx->last_err.code = code;
    ctx->last_err.detail = detail;
    ctx->last_err.line = line;
    strncpy(ctx->last_err.where, where, sizeof(ctx->last_err.where)-1);
    ctx->last_err.where[sizeof(ctx->last_err.where)-1] = '\0';
}

int gf_init(gf_ctx_t** ctx, const gf_cfg_t* cfg){
    if(!ctx || !cfg) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    *ctx = (gf_ctx_t*)malloc(sizeof(gf_ctx_t));
    if(!*ctx) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_ENOMEM);
    (*ctx)->cfg = *cfg;
    (*ctx)->frame_count = 0;
    (*ctx)->last_ft = 0;
    (*ctx)->last_lat = 0;
    memset(&(*ctx)->last_err,0,sizeof((*ctx)->last_err));
    return GF_OK;
}

void gf_shutdown(gf_ctx_t* ctx){
    free(ctx);
}

int gf_update(gf_ctx_t* ctx, uint16_t ft, uint16_t lat, uint32_t dc, uint32_t vis, const float cam[2]){
    if(!ctx) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    ctx->frame_count++;
    ctx->last_ft = ft;
    ctx->last_lat = lat;
    (void)dc; (void)vis; (void)cam;
    return GF_OK;
}

int gf_metrics(gf_ctx_t* ctx, gf_metrics_t* out){
    if(!ctx || !out){
        set_last_error(ctx, GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL),0,__LINE__,"core/metrics");
        return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    }
    memset(out,0,sizeof(*out));
    out->gf_index = 100;
    out->ft_p95_q8_8 = ctx->last_ft;
    out->lat_p95_q8_8 = ctx->last_lat;
    out->gf_fps_q8_8 = (ctx->cfg.target_fps<<8);
    return GF_OK;
}

const char* gf_hint(gf_ctx_t* ctx){
    (void)ctx; return NULL;
}

typedef struct { int code; const char* name; const char* msg; } err_ent_t;

static const err_ent_t err_tab[] = {
    { GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL), "CORE/MATH/EINVAL", "Invalid argument (core/math)" },
    { GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_ENOMEM), "CORE/MATH/ENOMEM", "Out of memory (core/math)" },
    { GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EOVER),  "CORE/MATH/EOVER",  "Numeric overflow (core/math)" },
    { GF_ERR(GF_FAC_FORMULA,GF_MOD_VM,GF_ESTATE), "FORMULA/VM/ESTATE", "VM state error" },
    { GF_ERR(GF_FAC_FORMULA,GF_MOD_SFS,GF_EFORM), "FORMULA/PARSE/EFORM", "Invalid formula" },
    { GF_ERR(GF_FAC_FORMULA,GF_MOD_SFS,GF_EPARSE),"FORMULA/PARSE/EPARSE","Syntax error in formula" },
    { GF_ERR(GF_FAC_STREAM,GF_MOD_IPC,GF_ECAP),  "STREAM/IPC/ECAP", "Capability mismatch (handshake)" },
    { GF_ERR(GF_FAC_STREAM,GF_MOD_IPC,GF_EIO),   "STREAM/IPC/EIO",  "IPC I/O error" },
    { GF_ERR(GF_FAC_STREAM,GF_MOD_IPC,GF_ETIME), "STREAM/IPC/ETIME", "IPC timeout" },
    { GF_ERR(GF_FAC_FORMAT,GF_MOD_SFS,GF_EIO),   "FORMAT/SFS/EIO",  "I/O error (SFS)" },
    { GF_ERR(GF_FAC_FORMAT,GF_MOD_SFS,GF_ECRC),  "FORMAT/SFS/ECRC", "CRC mismatch (SFS)" },
    { GF_ERR(GF_FAC_FORMAT,GF_MOD_SFS,GF_EINVAL),"FORMAT/SFS/EINVAL","Invalid SFS record" }
};

static const err_ent_t* find_err(int code){
    for(size_t i=0;i<sizeof(err_tab)/sizeof(err_tab[0]);++i)
        if(err_tab[i].code==code) return &err_tab[i];
    return NULL;
}

const char* gf_error_name(int code){
    const err_ent_t* e = find_err(code);
    return e ? e->name : "UNKNOWN";
}

const char* gf_strerror(int code){
    const err_ent_t* e = find_err(code);
    if(e) return e->msg;
    if(code==GF_OK) return "OK";
    return "unknown";
}

int gf_last_error(gf_ctx_t* ctx, gf_error_info_t* out){
    if(!ctx || !out) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    *out = ctx->last_err;
    return GF_OK;
}
