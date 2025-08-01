subsystem: window
status: "✅ stable"
problems:
  - none
recommendations:
  - maintain platform abstractions
---
subsystem: render
status: "✅ stable"
problems:
  - lacks hardware acceleration
recommendations:
  - integrate Vulkan backend (partial stub added)
---
subsystem: graphic
status: "✅ stable"
problems:
  - sprite group batching not implemented
recommendations:
  - add sprite group support (prototype available)
---
subsystem: sprite
status: "✅ stable"
problems:
  - no scalable text rendering
recommendations:
  - implement sprite groups (done)
  - support scalable fonts (partial)
---
subsystem: texture
status: "✅ stable"
problems:
  - no mipmap generation
recommendations:
  - add optional mipmap step (flag added)
---
subsystem: gui
status: "✅ stable"
problems:
  - fallback font limited to Latin characters
recommendations:
  - allow custom fallback sets (implemented)
---
subsystem: input
status: "✅ stable"
problems:
  - platform layers incomplete
recommendations:
  - extend platform coverage
---
subsystem: flowscript
status: "🟡 partial"
problems:
  - runtime only implements skeleton
recommendations:
  - finish parser and VM
---
subsystem: objects_roles
status: "✅ stable"
problems:
  - schema evolution basic
recommendations:
  - improve migration tooling
---
subsystem: world
status: "✅ stable"
problems:
  - large worlds may hit memory limits
recommendations:
  - stream chunks from disk
---
subsystem: raycast
status: "✅ stable"
problems:
  - only AABB shapes
recommendations:
  - support polygons and circles
---
subsystem: animation
status: "✅ stable"
problems:
  - no curve interpolation
recommendations:
  - add easing curves
---
subsystem: scheduler
status: "✅ stable"
problems:
  - timers lack persistence
recommendations:
  - allow save/load (added)
---
subsystem: resource
status: "✅ stable"
problems:
  - no compression on packs
recommendations:
  - implement optional compression (zlib)
---
subsystem: cursor
status: "✅ stable"
problems:
  - custom cursor images not hot-swappable
recommendations:
  - expose runtime update API (set_style added)
---
subsystem: framesync
status: "🟡 partial"
problems:
  - simple busy-wait strategy
recommendations:
  - implement adaptive sync (basic sleep added)
