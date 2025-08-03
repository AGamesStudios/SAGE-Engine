"""Rendering statistics collector."""

stats = {
    "frame_id": 0,
    "fps": 0.0,
    "frame_ms": 0.0,
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
    "time_spent_ms": 0.0,
    "transform_nodes_updated": 0,
    "transform_mul_count": 0,
    "culling_tested": 0,
    "culling_rejected": 0,
    "culling_drawn": 0,
}


def reset_frame() -> None:
    """Reset per-frame statistics."""
    stats["sprites_drawn"] = 0
    stats["text_glyphs_rendered"] = 0
    stats["textures_bound"] = 0
    stats["textures_loaded"] = 0
    stats["atlas_hits"] = 0
    stats["atlas_misses"] = 0
    stats["transform_nodes_updated"] = 0
    stats["transform_mul_count"] = 0
    stats["culling_tested"] = 0
    stats["culling_rejected"] = 0
    stats["culling_drawn"] = 0
    try:  # avoid circular import at module load
        from ..texture.cache import TextureCache

        stats["texture_memory_kb"] = TextureCache.memory_usage() // 1024
        if stats["texture_memory_kb"] > stats["memory_peak"]:
            stats["memory_peak"] = stats["texture_memory_kb"]
    except Exception:
        stats["texture_memory_kb"] = 0

