#ifndef SAGE_GF_CORE_H
#define SAGE_GF_CORE_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef GF_API
#  ifdef _WIN32
#    ifdef GF_BUILD
#      define GF_API __declspec(dllexport)
#    else
#      define GF_API __declspec(dllimport)
#    endif
#  else
#    define GF_API __attribute__((visibility("default")))
#  endif
#endif

typedef struct gf_cfg_s {
    uint32_t target_fps;
    uint32_t window_ms;
    uint16_t drop_fps;
    float    ema_alpha;
    uint32_t ring_capacity;
} gf_cfg_t;

typedef struct gf_ctx_s gf_ctx_t;

GF_API int  gf_init(gf_ctx_t** ctx, const gf_cfg_t* cfg);
GF_API void gf_shutdown(gf_ctx_t* ctx);
GF_API int  gf_update(gf_ctx_t* ctx,
                     uint16_t frame_time_q8_8,
                     uint16_t input_latency_q8_8,
                     uint32_t draw_calls,
                     uint32_t visible_objects,
                     const float camera_motion_vec[2]);

typedef struct {
    uint8_t  gf_index;
    uint16_t ft_p95_q8_8;
    uint16_t jitter_allan_q8_8;
    uint16_t lat_p95_q8_8;
    uint16_t drops_rate_q8_8;
    uint16_t gf_fps_q8_8;
    uint8_t  flags;
} gf_metrics_t;

GF_API int  gf_metrics(gf_ctx_t* ctx, gf_metrics_t* out);
GF_API const char* gf_hint(gf_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // SAGE_GF_CORE_H
