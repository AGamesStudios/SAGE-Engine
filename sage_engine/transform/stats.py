"""Per-frame statistics for transforms and culling."""

stats = {
    "nodes_updated": 0,
    "mul_count": 0,
    "culling_tested": 0,
    "culling_rejected": 0,
    "culling_drawn": 0,
    "total_objects": 0,
    "visible_objects": 0,
    "culled_objects": 0,
    "max_depth": 0,
}


def reset_frame() -> None:
    """Reset per-frame transform statistics."""
    stats["nodes_updated"] = 0
    stats["mul_count"] = 0
    stats["culling_tested"] = 0
    stats["culling_rejected"] = 0
    stats["culling_drawn"] = 0
    stats["total_objects"] = 0
    stats["visible_objects"] = 0
    stats["culled_objects"] = 0
    stats["max_depth"] = 0
