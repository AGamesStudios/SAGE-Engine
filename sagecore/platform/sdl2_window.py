
# PySDL2 + OpenGL window backend with a minimal GLFW-key compatibility layer.
import ctypes, math
from ctypes import c_int
try:
    import sdl2
    import sdl2.video
except Exception as e:
    raise

# Match the int key-codes used elsewhere (GLFW-compatible numbers)
KEY = {
    "ESC": 256,
    "SPACE": 32,
    "A": 65, "D": 68, "S": 83, "W": 87,
    "LEFT_SHIFT": 340, "LEFT_CONTROL": 341,
    "F5": 294, "F6": 295, "F7": 296, "F8": 297,
}

SDL_TO_KEY = {
    sdl2.SDLK_ESCAPE: KEY["ESC"],
    sdl2.SDLK_SPACE: KEY["SPACE"],
    sdl2.SDLK_a: KEY["A"],
    sdl2.SDLK_d: KEY["D"],
    sdl2.SDLK_s: KEY["S"],
    sdl2.SDLK_w: KEY["W"],
    sdl2.SDLK_F5: KEY["F5"],
    sdl2.SDLK_F6: KEY["F6"],
    sdl2.SDLK_F7: KEY["F7"],
    sdl2.SDLK_F8: KEY["F8"],
}

SDL_KMOD_TO_KEY = {
    sdl2.KMOD_LSHIFT: KEY["LEFT_SHIFT"],
    sdl2.KMOD_LCTRL: KEY["LEFT_CONTROL"],
}

class SDL2Window:
    def __init__(self, width=1280, height=720, title="SAGE Engine (SDL2)", srgb=False):
        if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) != 0:
            raise RuntimeError("SDL_Init failed")
        # GL attributes
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_CONTEXT_MAJOR_VERSION, 3)
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_CONTEXT_MINOR_VERSION, 3)
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_CONTEXT_PROFILE_MASK, sdl2.SDL_GL_CONTEXT_PROFILE_CORE)
        if srgb:
            # request an sRGB capable framebuffer if available
            sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1)
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_DOUBLEBUFFER, 1)
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_DEPTH_SIZE, 24)

        self.width, self.height = int(width), int(height)
        self._win = sdl2.SDL_CreateWindow(
            title.encode('utf-8'),
            sdl2.SDL_WINDOWPOS_CENTERED, sdl2.SDL_WINDOWPOS_CENTERED,
            self.width, self.height,
            sdl2.SDL_WINDOW_OPENGL | sdl2.SDL_WINDOW_RESIZABLE,
        )
        if not self._win:
            sdl2.SDL_Quit()
            raise RuntimeError("SDL_CreateWindow failed")

        self._ctx = sdl2.SDL_GL_CreateContext(self._win)
        if not self._ctx:
            sdl2.SDL_DestroyWindow(self._win); sdl2.SDL_Quit()
            raise RuntimeError("SDL_GL_CreateContext failed")

        sdl2.SDL_GL_MakeCurrent(self._win, self._ctx)
        sdl2.SDL_GL_SetSwapInterval(1)  # vsync

        self._keys_down = set()
        self._pressed = set()
        self._should_close = False
        self._mouse_x = self.width * 0.5
        self._mouse_y = self.height * 0.5

    def _map_key(self, sym, mod):
        if sym in SDL_TO_KEY: return SDL_TO_KEY[sym]
        # modifiers pressed
        for m, k in SDL_KMOD_TO_KEY.items():
            if (mod & m) != 0: self._keys_down.add(k)
        return None

    def poll(self):
        self._pressed.clear()
        event = sdl2.SDL_Event()
        while sdl2.SDL_PollEvent(ctypes.byref(event)) != 0:
            et = event.type
            if et == sdl2.SDL_QUIT:
                self._should_close = True
            elif et == sdl2.SDL_WINDOWEVENT:
                if event.window.event == sdl2.SDL_WINDOWEVENT_RESIZED:
                    self.width, self.height = int(event.window.data1), int(event.window.data2)
            elif et == sdl2.SDL_MOUSEMOTION:
                self._mouse_x, self._mouse_y = float(event.motion.x), float(event.motion.y)
            elif et == sdl2.SDL_KEYDOWN:
                sym = event.key.keysym.sym
                mod = event.key.keysym.mod
                k = self._map_key(sym, mod)
                if k is not None:
                    if k not in self._keys_down: self._pressed.add(k)
                    self._keys_down.add(k)
            elif et == sdl2.SDL_KEYUP:
                sym = event.key.keysym.sym
                k = SDL_TO_KEY.get(sym)
                if k is not None and k in self._keys_down:
                    self._keys_down.remove(k)

    def swap(self):
        sdl2.SDL_GL_SwapWindow(self._win)

    def should_close(self) -> bool:
        return self._should_close

    def is_key_down(self, key: int) -> bool:
        return key in self._keys_down

    def was_key_pressed(self, key: int) -> bool:
        return key in self._pressed

    def get_cursor_pos(self):
        return (self._mouse_x, self._mouse_y)

    def shutdown(self):
        try:
            sdl2.SDL_GL_DeleteContext(self._ctx)
            sdl2.SDL_DestroyWindow(self._win)
        finally:
            sdl2.SDL_Quit()
