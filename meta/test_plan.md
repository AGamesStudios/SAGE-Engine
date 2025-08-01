# Test Plan

## Goals
- Verify core loop stability
- Ensure roles and scene operations behave consistently
- Cover rendering and resource loading paths

## Scenarios
1. Unit tests for time, timers, events and DAG execution.
2. Integration test loading a scene and running 10 frames headless.
3. Visual test comparing a saved frame to a golden image.
4. Stress test with 10k objects for 100 frames checking memory usage.

## Coverage
Current pytest suite covers ~60% of modules. Rendering and async loaders remain partially untested.
