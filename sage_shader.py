from OpenGL.GL import glGetUniformLocation, glUseProgram, glUniformMatrix4fv, glDeleteProgram, GL_FALSE, glUniform3fv
from OpenGL.GL.shaders import compileShader, compileProgram
from OpenGL.GL import GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
from pyglm import glm

class SAGEShader:
    """
    Класс для управления шейдерной программой OpenGL.
    Предоставляет методы для компиляции, активации и очистки шейдеров.
    """

    def __init__(self, vertex_source, fragment_source):
        """Инициализация шейдера с указанием вершинного и фрагментного шейдеров."""
        try:
            self.program = compileProgram(
                compileShader(vertex_source, GL_VERTEX_SHADER),
                compileShader(fragment_source, GL_FRAGMENT_SHADER)
            )
            self.uniforms = {
                "model": glGetUniformLocation(self.program, "model"),
                "view": glGetUniformLocation(self.program, "view"),
                "projection": glGetUniformLocation(self.program, "projection"),
                "lightPos": glGetUniformLocation(self.program, "lightPos"),
                "lightColor": glGetUniformLocation(self.program, "lightColor"),
                "objectColor": glGetUniformLocation(self.program, "objectColor")
            }
            if any(loc == -1 for loc in self.uniforms.values()):
                raise ValueError("Одна или несколько униформных переменных не найдены")
        except Exception as e:
            raise Exception(f"Ошибка компиляции шейдера: {e}")

    def use(self):
        """Активирует шейдерную программу для рендеринга."""
        glUseProgram(self.program)

    def set_mat4(self, name, matrix):
        """Устанавливает матрицу 4x4 в шейдере по имени униформы."""
        loc = self.uniforms.get(name)
        if loc != -1:
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm.value_ptr(matrix))
        else:
            print(f"Предупреждение: униформа '{name}' не найдена")

    def set_vec3(self, name, vector):
        """Устанавливает вектор vec3 в шейдере по имени униформы."""
        loc = self.uniforms.get(name)
        if loc != -1:
            glUniform3fv(loc, 1, glm.value_ptr(vector))
        else:
            print(f"Предупреждение: униформа '{name}' не найдена")

    def cleanup(self):
        """Очищает ресурсы шейдера, удаляя программу."""
        if self.program:
            glDeleteProgram(self.program)
            self.program = None