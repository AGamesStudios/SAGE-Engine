# SAGE Engine - Known Issues (Alpha)

**Last Updated:** 2024  
**Status:** Active Development

## Critical Issues

### Examples Compilation Failures

Most example projects currently fail to compile due to API changes in the engine core. These examples were written against an earlier version of the API and require updates.

**Affected Examples:**
- Box2DPhysicsDemo
- PhysicsSandbox
- RPGSystemsDemo
- UISystemTest
- UISystemTestNew
- NineSliceExample
- And others

**Common Errors:**
1. **AddComponent API Changes**
   - Old: `registry.AddComponent<T>(entity, args...)`
   - Current: `registry.AddComponent<T>(entity)`
   - Impact: All examples using component initialization

2. **Physics API Missing Members**
   - `Physics::Settings` type not found
   - `PhysicsSystem::SetSettings()` removed
   - `PhysicsSystem::GetWorld()` removed

3. **Application Constructor Changes**
   - Old: `Application(title, width, height)`
   - Current: Different signature
   - Impact: All example main() functions

4. **UI System API Changes**
   - Widget constructor signatures changed
   - Mouse event handlers have different parameters
   - UIManager::Init() expects different arguments

5. **Rendering API Changes**
   - `Renderer::Init()` namespace issues
   - Shader access patterns changed
   - RenderBackend interface modified

## Infrastructure Issues

### Fixed ✓
- ✓ Git submodules incomplete (GLFW was missing)
- ✓ Examples disabled in CMake build
- ✓ README claimed non-existent features

### In Progress
- ⚠ Third-party header downloads fail (SSL certificate issues)
  - Workaround: Use `SAGE_OFFLINE_THIRDPARTY=ON`
  - Fallback: Headers exist in ThirdParty/ directory

## Recommended Actions

### For Users

**DO:**
- Use SAGE CLI for project creation
- Build new projects from scratch using current API
- Refer to Engine headers for current API signatures
- Report issues on GitHub

**DON'T:**
- Use example code as-is without modification
- Expect production-ready stability
- Use in commercial projects yet

### For Developers

**Priority Fixes Needed:**
1. Update all examples to current API
2. Document ECS component initialization pattern
3. Restore or document Physics::Settings alternative
4. Fix Application constructor usage
5. Document UI system API changes
6. Add API migration guide

## Working Features

Despite example issues, the following core systems compile successfully:

- ✓ ECS core (Registry, Entity, Systems)
- ✓ Box2D physics integration
- ✓ OpenGL rendering backend
- ✓ Resource management
- ✓ Event system
- ✓ Audio system (miniaudio)
- ✓ ImGui integration

**Testing:** Create a new project with `sage create` and use current API patterns.

## Getting Help

- **GitHub Issues:** Report bugs and request features
- **Documentation:** Check `docs/` for current API reference
- **Engine Headers:** Most authoritative source for current API

## Version History

### Alpha (Current)
- Initial public release
- Core systems functional
- Examples need updates
- API stabilization in progress

---

**Note:** This is Alpha software. Breaking changes are expected. The engine is suitable for learning, experimentation, and feedback, but not production use.
