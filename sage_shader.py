from OpenGL.GL import *
from OpenGL.GL.shaders import compileShader, compileProgram
import glm

class SAGEShader:
    def __init__(self, vertex_source, fragment_source):
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

    def use(self):
        glUseProgram(self.program)

    def set_mat4(self, name, matrix):
        loc = self.uniforms.get(name)
        if loc != -1:
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm.value_ptr(matrix))

    def set_vec3(self, name, vector):
        loc = self.uniforms.get(name)
        if loc != -1:
            glUniform3fv(loc, 1, glm.value_ptr(vector))