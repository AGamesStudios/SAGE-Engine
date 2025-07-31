#include "sage/gf/gf_core.h"
#include "sage/gf/gf_errors.h"
#include <stdlib.h>
#include <string.h>

struct gf_ctx_s {
    gf_cfg_t cfg;
    uint32_t frame_count;
    uint16_t last_ft;
    uint16_t last_lat;
};

int gf_init(gf_ctx_t** ctx, const gf_cfg_t* cfg){
    if(!ctx || !cfg) return GF_EINVAL;
    *ctx = (gf_ctx_t*)malloc(sizeof(gf_ctx_t));
    if(!*ctx) return GF_ENOMEM;
    (*ctx)->cfg = *cfg;
    (*ctx)->frame_count = 0;
    (*ctx)->last_ft = 0;
    (*ctx)->last_lat = 0;
    return GF_OK;
}

void gf_shutdown(gf_ctx_t* ctx){
    free(ctx);
}

int gf_update(gf_ctx_t* ctx, uint16_t ft, uint16_t lat, uint32_t dc, uint32_t vis, const float cam[2]){
    if(!ctx) return GF_EINVAL;
    ctx->frame_count++;
    ctx->last_ft = ft;
    ctx->last_lat = lat;
    (void)dc; (void)vis; (void)cam;
    return GF_OK;
}

int gf_metrics(gf_ctx_t* ctx, gf_metrics_t* out){
    if(!ctx || !out) return GF_EINVAL;
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
