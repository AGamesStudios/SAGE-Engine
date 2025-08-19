
# sagecore/core/preflight.py
import ctypes

def check_sdl2():
    try:
        import sdl2  # noqa: F401
        return True, "SDL2 Python binding found"
    except Exception as e:
        return False, f"SDL2 not available: {e}"

def check_glfw():
    try:
        import glfw  # noqa: F401
        return True, "GLFW Python binding found"
    except Exception as e:
        return False, f"GLFW not available: {e}"

def check_opengl():
    try:
        from OpenGL import GL  # noqa: F401
        return True, "PyOpenGL available"
    except Exception as e:
        return False, f"OpenGL binding missing: {e}"

def check_vulkan():
    for n in ("vulkan-1.dll", "libvulkan.so.1", "libvulkan.dylib"):
        try:
            ctypes.CDLL(n)
            return True, f"Vulkan loader present: {n}"
        except Exception:
            pass
    return False, "Vulkan loader not found"

def run_preflight(prefer="auto"):
    ok_sdl, msg_sdl = check_sdl2()
    ok_glfw, msg_glfw = check_glfw()
    ok_gl, msg_gl = check_opengl()
    ok_vk, msg_vk = check_vulkan()
    results = {
        "sdl2": (ok_sdl, msg_sdl),
        "glfw": (ok_glfw, msg_glfw),
        "opengl": (ok_gl, msg_gl),
        "vulkan": (ok_vk, msg_vk),
        "recommend_backend": "headless",
    }
    if prefer in ("sdl2","auto") and ok_sdl and ok_gl:
        results["recommend_backend"] = "sdl2"
    elif ok_glfw and ok_gl:
        results["recommend_backend"] = "glfw"
    return results
