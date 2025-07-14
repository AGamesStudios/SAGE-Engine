# RFC: Adaptor System

- **Authors:** Core Team
- **Status:** accepted
- **Created:** 2025-07-14

## Summary
Formalise the adaptor registration API so that third-party packages can plug into the engine via entry points.

## Motivation
Allow developers to provide alternative render, audio or input backends without modifying the core.

## Design
Each adaptor is a package under ``sage_adaptors`` exposing ``register()`` and ``get_capabilities()``. Entry points under ``sage_adaptor`` automatically load these packages.

