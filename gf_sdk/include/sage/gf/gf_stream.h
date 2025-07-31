#ifndef SAGE_GF_STREAM_H
#define SAGE_GF_STREAM_H

#include <stdint.h>
#include "gf_core.h"
#include "gf_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gf_stream_s gf_stream_t;

typedef struct {
    uint8_t allow_control;
    uint32_t shm_bytes;
} gf_stream_cfg_t;

GF_API int  gf_stream_start(gf_ctx_t* ctx, gf_stream_t** out, const gf_stream_cfg_t* cfg);
GF_API void gf_stream_stop(gf_stream_t* s);
GF_API int  gf_stream_publish(gf_stream_t* s);

typedef enum { GF_CMD_NOP=0, GF_CMD_SNAPSHOT_2S, GF_CMD_MARK, GF_CMD_PACEGUARD_SET } gf_cmd_t;

typedef struct {
    gf_cmd_t cmd;
    uint32_t u32;
    char     str[64];
} gf_cmd_msg_t;

GF_API int gf_stream_poll_cmd(gf_stream_t* s, gf_cmd_msg_t* out);
GF_API int gf_stream_send_capabilities(gf_stream_t* s, const char** caps, uint32_t n);

#ifdef __cplusplus
}
#endif

#endif // SAGE_GF_STREAM_H
