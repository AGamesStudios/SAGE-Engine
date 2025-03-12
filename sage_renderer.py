from OpenGL.GL import *

class SAGERenderer:
    def clear(self):
        """Очищает буферы цвета и глубины."""
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)