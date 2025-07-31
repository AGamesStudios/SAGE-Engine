#include "sage/gf/gf_core.h"
#include "sage/gf/gf_errors.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>



#define TO_Q8_8(ms) ((uint16_t)((ms)*256.0f+0.5f))
#define FROM_Q8_8(q) ((float)(q)/256.0f)

static void set_last_error(gf_ctx_t* ctx, int32_t code, uint32_t detail,
                           uint32_t line, const char* where){
    if(!ctx) return;
    ctx->last_err.code = code;
    ctx->last_err.detail = detail;
    ctx->last_err.line = line;
    strncpy(ctx->last_err.where, where, sizeof(ctx->last_err.where)-1);
    ctx->last_err.where[sizeof(ctx->last_err.where)-1] = '\0';
}

static uint16_t select_p95(const uint16_t* a, uint32_t n, uint16_t* scratch){
    memcpy(scratch, a, n*sizeof(uint16_t));
    uint32_t k = (uint32_t)(0.95f*(n-1));
    uint32_t left=0,right=n-1;
    while(left<right){
        uint16_t pivot = scratch[(left+right)/2];
        uint32_t i=left,j=right;
        while(i<=j){
            while(scratch[i]<pivot) i++;
            while(scratch[j]>pivot) j--;
            if(i<=j){ uint16_t t=scratch[i];scratch[i]=scratch[j];scratch[j]=t;i++;j--; }
        }
        if(k<=j) right=j; else if(k>=i) left=i; else break;
    }
    return scratch[k];
}

static uint16_t allan_jitter(const uint16_t* dt, uint32_t n){
    if(n<2) return 0;
    uint64_t sum=0;
    for(uint32_t i=0;i<n-1;i++){
        int32_t diff=(int32_t)dt[i+1]-dt[i];
        sum += (uint64_t)(diff*diff);
    }
    double val = sqrt((double)sum/(2.0*(n-1)));
    return TO_Q8_8(val);
}

int gf_init(gf_ctx_t** ctx, const gf_cfg_t* cfg){
    if(!ctx || !cfg) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    *ctx = NULL;
    uint32_t cap = cfg->ring_capacity;
    if(cap==0){
        float w = cfg->window_ms/1000.0f * (float)cfg->target_fps;
        cap = (uint32_t)(w + 0.5f);
        if(cap < cfg->target_fps*2) cap = cfg->target_fps*2;
    }
    size_t buf_sz = sizeof(gf_ctx_t) + sizeof(uint16_t)*cap*4;
    gf_ctx_t* c = (gf_ctx_t*)malloc(buf_sz);
    if(!c) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_ENOMEM);
    memset(c,0,sizeof(*c));
    c->cfg = *cfg;
    c->cap = cap;
    uint16_t* base = (uint16_t*)(c+1);
    c->ft_q8_8 = base; base += cap;
    c->lat_q8_8 = base; base += cap;
    c->dt_q8_8 = base; base += cap;
    c->scratch = base; base += cap;
    c->drop_thresh_q8_8 = TO_Q8_8(1000.0f / (float)cfg->drop_fps);
    c->drops_ema_q16_16 = 0;
    c->last_ft = 0;
    c->head = c->count = 0;
    *ctx = c;
    return GF_OK;
}

void gf_shutdown(gf_ctx_t* ctx){
    free(ctx);
}

int gf_update(gf_ctx_t* ctx, uint16_t ft, uint16_t lat, uint32_t dc, uint32_t vis, const float cam[2]){
    if(!ctx) return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    (void)dc; (void)vis; (void)cam;
    ctx->head = (ctx->head+1)%ctx->cap;
    if(ctx->count<ctx->cap) ctx->count++;
    ctx->ft_q8_8[ctx->head]=ft;
    ctx->lat_q8_8[ctx->head]=lat;
    uint16_t dt = (ctx->count>1)?(ft>ctx->last_ft?ft-ctx->last_ft:ctx->last_ft-ft):0;
    ctx->dt_q8_8[ctx->head]=dt;
    ctx->last_ft = ft;
    if(ft>ctx->drop_thresh_q8_8){
        ctx->drops_ema_q16_16 = (uint32_t)((1.0f-ctx->cfg.ema_alpha)*ctx->drops_ema_q16_16 + ctx->cfg.ema_alpha*(1<<16));
    }else{
        ctx->drops_ema_q16_16 = (uint32_t)((1.0f-ctx->cfg.ema_alpha)*ctx->drops_ema_q16_16);
    }
    return GF_OK;
}

int gf_metrics(gf_ctx_t* ctx, gf_metrics_t* out){
    if(!ctx || !out){
        set_last_error(ctx, GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL),0,__LINE__,"core/metrics");
        return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_EINVAL);
    }
    memset(out,0,sizeof(*out));
    uint32_t n = ctx->count;
    if(n==0){
        set_last_error(ctx, GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_ESTATE),0,__LINE__,"core/metrics");
        return GF_ERR(GF_FAC_CORE,GF_MOD_MATH,GF_ESTATE);
    }
    uint16_t ft95 = select_p95(ctx->ft_q8_8, n, ctx->scratch);
    uint16_t lat95 = select_p95(ctx->lat_q8_8, n, ctx->scratch);
    uint16_t jit = allan_jitter(ctx->dt_q8_8, n);
    float gf_fps = (1000.0f / FROM_Q8_8(ft95)) * (1.0f - fminf(FROM_Q8_8(jit)/16.7f,1.0f)) * (1.0f - FROM_Q8_8(lat95)/400.0f);
    if(gf_fps<0) gf_fps=0; if(gf_fps>3000) gf_fps=3000;
    float x = gf_fps / (float)ctx->cfg.target_fps;
    float core = 100.0f/(1.0f+expf(-7.5f*(x-0.9f)));
    float raw = core*0.7f - fminf(FROM_Q8_8((uint16_t)(ctx->drops_ema_q16_16>>8))*2.0f,20.0f);
    int idx = (int)(raw+0.5f); if(idx<0) idx=0; if(idx>100) idx=100;
    out->gf_index = (uint8_t)idx;
    out->ft_p95_q8_8 = ft95;
    out->lat_p95_q8_8 = lat95;
    out->jitter_allan_q8_8 = jit;
    out->drops_rate_q8_8 = (uint16_t)(ctx->drops_ema_q16_16>>8);
    out->gf_fps_q8_8 = TO_Q8_8(gf_fps);
    out->flags = 0;
    if(FROM_Q8_8(jit) > 4.0f) out->flags |= GF_FLAG_PACING;
    if(FROM_Q8_8(lat95) > 90.0f) out->flags |= GF_FLAG_INPUT_LAG;
    if(FROM_Q8_8(jit) > 2.0f) out->flags |= GF_FLAG_MICROSTUTTER;
    return GF_OK;
}

const char* gf_hint(gf_ctx_t* ctx){
    static char hint[64];
    gf_metrics_t m;
    if(gf_metrics(ctx,&m)!=GF_OK) return NULL;
    if(m.flags & GF_FLAG_PACING){
        strcpy(hint,"Pacing unstable - tune Delta-Render");
        return hint;
    }
    if(m.flags & GF_FLAG_INPUT_LAG){
        strcpy(hint,"High input latency - check FrameSync");
        return hint;
    }
    if(m.flags & GF_FLAG_MICROSTUTTER){
        strcpy(hint,"Micro stutter detected - optimize");
        return hint;
    }
    return NULL;
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
