"""Rendering statistics collector."""

from collections import deque

stats = {
    "frame_id": 0,
    "fps": 0.0,
    "fps_avg": 0.0,
    "fps_min": 0.0,
    "fps_max": 0.0,
    "fps_1pct_low": 0.0,
    "ms_update": 0.0,
    "ms_draw": 0.0,
    "ms_flush": 0.0,
    "ms_frame": 0.0,
    "frame_ms": 0.0,
    "frame_time": 0.0,
    "frame_time_avg": 0.0,
    "dirty_tiles": 0,
    "total_tiles": 0,
    "dirty_ratio": 0.0,
    "draw_calls": 0,
    "sprites_drawn": 0,
    "text_glyphs_rendered": 0,
    "textures_bound": 0,
    "textures_loaded": 0,
    "atlas_hits": 0,
    "atlas_misses": 0,
    "texture_memory_kb": 0,
    "memory_peak": 0,
    "transform_nodes_updated": 0,
    "transform_mul_count": 0,
    "culling_tested": 0,
    "culling_rejected": 0,
    "culling_drawn": 0,
    "transform_total_objects": 0,
    "transform_visible_objects": 0,
    "transform_culled_objects": 0,
    "transform_max_depth": 0,
    "fps_jitter": 0.0,
    "frame_stability_score": 0.0,
    "sleep_time": 0.0,
    "triangles_drawn": 0,
    "zbuffer_hits": 0,
    "frame3d_time": 0.0,
}

_history: deque[float] = deque(maxlen=120)


def update_fps(frame_ms: float) -> None:
    """Update FPS statistics from ``frame_ms``."""
    if frame_ms <= 0:
        return
    fps = 1000.0 / frame_ms
    _history.append(fps)
    stats["fps"] = fps
    stats["fps_avg"] = sum(_history) / len(_history)
    stats["fps_min"] = min(_history)
    stats["fps_max"] = max(_history)
    stats["fps_jitter"] = abs(fps - stats["fps_avg"])
    if stats["fps_max"] > 0:
        stats["frame_stability_score"] = stats["fps_min"] / stats["fps_max"] * 100.0
    sorted_hist = sorted(_history)
    idx = max(0, int(len(sorted_hist) * 0.01) - 1)
    stats["fps_1pct_low"] = sorted_hist[idx]
    try:
        from ..logger import logger

        avg = stats["fps_avg"]
        if avg and abs(fps - avg) / avg > 0.3:
            logger.warn(
                "[render][fps] Spike detected: current %.1f avg %.1f",
                fps,
                avg,
                tag="render",
            )
    except Exception:
        pass


def reset_frame() -> None:
    """Reset per-frame statistics."""
    stats["sprites_drawn"] = 0
    stats["text_glyphs_rendered"] = 0
    stats["textures_bound"] = 0
    stats["textures_loaded"] = 0
    stats["atlas_hits"] = 0
    stats["atlas_misses"] = 0
    stats["draw_calls"] = 0
    stats["transform_nodes_updated"] = 0
    stats["transform_mul_count"] = 0
    stats["culling_tested"] = 0
    stats["culling_rejected"] = 0
    stats["culling_drawn"] = 0
    stats["transform_total_objects"] = 0
    stats["transform_visible_objects"] = 0
    stats["transform_culled_objects"] = 0
    stats["transform_max_depth"] = 0
    stats["ms_update"] = 0.0
    stats["ms_draw"] = 0.0
    stats["ms_flush"] = 0.0
    stats["ms_frame"] = 0.0
    stats["frame_ms"] = 0.0
    stats["frame_time"] = 0.0
    stats["sleep_time"] = 0.0
    stats["triangles_drawn"] = 0
    stats["zbuffer_hits"] = 0
    stats["frame3d_time"] = 0.0
    try:  # avoid circular import at module load
        from ..texture.cache import TextureCache

        stats["texture_memory_kb"] = TextureCache.memory_usage() // 1024
        if stats["texture_memory_kb"] > stats["memory_peak"]:
            stats["memory_peak"] = stats["texture_memory_kb"]
    except Exception:
        stats["texture_memory_kb"] = 0

