import sys

try:
    from OpenGL.GL import *
    from OpenGL.GLUT import *
    from OpenGL.GLU import *
except ImportError as e:
    print("PyOpenGL is required to run this example. Install it via pip.")
    sys.exit(1)

import numpy as np

# Simple vertex and fragment shaders with basic lighting
VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
"""

FRAGMENT_SHADER = """
#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 1.0);
}
"""

def compile_shader(source, shader_type):
    shader = glCreateShader(shader_type)
    glShaderSource(shader, source)
    glCompileShader(shader)
    if glGetShaderiv(shader, GL_COMPILE_STATUS) != GL_TRUE:
        raise RuntimeError(glGetShaderInfoLog(shader))
    return shader

def create_shader_program():
    vertex_shader = compile_shader(VERTEX_SHADER, GL_VERTEX_SHADER)
    fragment_shader = compile_shader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
    program = glCreateProgram()
    glAttachShader(program, vertex_shader)
    glAttachShader(program, fragment_shader)
    glLinkProgram(program)
    if glGetProgramiv(program, GL_LINK_STATUS) != GL_TRUE:
        raise RuntimeError(glGetProgramInfoLog(program))
    glDeleteShader(vertex_shader)
    glDeleteShader(fragment_shader)
    return program

class Engine:
    def __init__(self, width=800, height=600):
        self.width = width
        self.height = height
        self.angle = 0
        self.program = None

    def init_gl(self):
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH)
        glutInitWindowSize(self.width, self.height)
        glutCreateWindow(b"PyOpenGL Scene")
        glEnable(GL_DEPTH_TEST)
        self.program = create_shader_program()
        self.setup_geometry()
        glutDisplayFunc(self.render)
        glutIdleFunc(self.update)

    def setup_geometry(self):
        # cube vertices and normals
        cube_vertices = [
            # positions       # normals
            -0.5, -0.5, -0.5, 0.0,  0.0, -1.0,
             0.5, -0.5, -0.5, 0.0,  0.0, -1.0,
             0.5,  0.5, -0.5, 0.0,  0.0, -1.0,
             0.5,  0.5, -0.5, 0.0,  0.0, -1.0,
            -0.5,  0.5, -0.5, 0.0,  0.0, -1.0,
            -0.5, -0.5, -0.5, 0.0,  0.0, -1.0,

            -0.5, -0.5,  0.5, 0.0,  0.0, 1.0,
             0.5, -0.5,  0.5, 0.0,  0.0, 1.0,
             0.5,  0.5,  0.5, 0.0,  0.0, 1.0,
             0.5,  0.5,  0.5, 0.0,  0.0, 1.0,
            -0.5,  0.5,  0.5, 0.0,  0.0, 1.0,
            -0.5, -0.5,  0.5, 0.0,  0.0, 1.0,

            -0.5,  0.5,  0.5, -1.0, 0.0, 0.0,
            -0.5,  0.5, -0.5, -1.0, 0.0, 0.0,
            -0.5, -0.5, -0.5, -1.0, 0.0, 0.0,
            -0.5, -0.5, -0.5, -1.0, 0.0, 0.0,
            -0.5, -0.5,  0.5, -1.0, 0.0, 0.0,
            -0.5,  0.5,  0.5, -1.0, 0.0, 0.0,

             0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
             0.5,  0.5, -0.5, 1.0, 0.0, 0.0,
             0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
             0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
             0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
             0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

            -0.5, -0.5, -0.5, 0.0, -1.0, 0.0,
             0.5, -0.5, -0.5, 0.0, -1.0, 0.0,
             0.5, -0.5,  0.5, 0.0, -1.0, 0.0,
             0.5, -0.5,  0.5, 0.0, -1.0, 0.0,
            -0.5, -0.5,  0.5, 0.0, -1.0, 0.0,
            -0.5, -0.5, -0.5, 0.0, -1.0, 0.0,

            -0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
             0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
             0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
             0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
            -0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
            -0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
        ]
        self.cube_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.cube_vbo)
        array_type = (GLfloat * len(cube_vertices))
        glBufferData(GL_ARRAY_BUFFER, len(cube_vertices) * 4, array_type(*cube_vertices), GL_STATIC_DRAW)

        plane_vertices = [
            -5.0, 0.0, -5.0, 0.0, 1.0, 0.0,
             5.0, 0.0, -5.0, 0.0, 1.0, 0.0,
             5.0, 0.0,  5.0, 0.0, 1.0, 0.0,
             5.0, 0.0,  5.0, 0.0, 1.0, 0.0,
            -5.0, 0.0,  5.0, 0.0, 1.0, 0.0,
            -5.0, 0.0, -5.0, 0.0, 1.0, 0.0,
        ]
        self.plane_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.plane_vbo)
        array_type = (GLfloat * len(plane_vertices))
        glBufferData(GL_ARRAY_BUFFER, len(plane_vertices) * 4, array_type(*plane_vertices), GL_STATIC_DRAW)

    def set_uniforms(self):
        glUseProgram(self.program)
        model = np.identity(4, dtype=np.float32)
        view = gluLookAt(5,5,5, 0,0,0, 0,1,0)
        projection = gluPerspective(45, self.width/self.height, 0.1, 100.0)
        loc_model = glGetUniformLocation(self.program, 'model')
        loc_view = glGetUniformLocation(self.program, 'view')
        loc_proj = glGetUniformLocation(self.program, 'projection')
        glUniformMatrix4fv(loc_model, 1, GL_FALSE, model)
        glUniformMatrix4fv(loc_view, 1, GL_FALSE, view)
        glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection)
        glUniform3f(glGetUniformLocation(self.program, 'lightPos'), 5.0, 5.0, 5.0)
        glUniform3f(glGetUniformLocation(self.program, 'viewPos'), 5.0, 5.0, 5.0)
        glUniform3f(glGetUniformLocation(self.program, 'lightColor'), 1.0, 1.0, 1.0)

    def draw_geometry(self, vbo, color):
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)
        glUniform3f(glGetUniformLocation(self.program, 'objectColor'), *color)
        glDrawArrays(GL_TRIANGLES, 0, 36 if vbo == self.cube_vbo else 6)

    def render(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        self.set_uniforms()
        self.draw_geometry(self.plane_vbo, (0.8, 0.8, 0.8))
        self.draw_geometry(self.cube_vbo, (0.4, 0.2, 0.8))
        glutSwapBuffers()

    def update(self):
        self.angle += 0.1
        glutPostRedisplay()

    def run(self):
        self.init_gl()
        glutMainLoop()

if __name__ == '__main__':
    engine = Engine()
    engine.run()
