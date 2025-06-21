import sys

try:
    from OpenGL.GL import *
    from OpenGL.GLUT import *
    from OpenGL.GLU import *
except ImportError as e:
    print("PyOpenGL is required to run this example. Install it via pip.")
    sys.exit(1)

import numpy as np

def perspective(fov, aspect, near, far):
    f = 1.0 / np.tan(np.radians(fov) / 2)
    proj = np.zeros((4, 4), dtype=np.float32)
    proj[0, 0] = f / aspect
    proj[1, 1] = f
    proj[2, 2] = (far + near) / (near - far)
    proj[2, 3] = (2 * far * near) / (near - far)
    proj[3, 2] = -1.0
    return proj

def look_at(eye, target, up):
    f = np.array(target) - np.array(eye)
    f = f / np.linalg.norm(f)
    u = np.array(up)
    u = u / np.linalg.norm(u)
    s = np.cross(f, u)
    s = s / np.linalg.norm(s)
    u = np.cross(s, f)

    view = np.identity(4, dtype=np.float32)
    view[0, :3] = s
    view[1, :3] = u
    view[2, :3] = -f
    view[0, 3] = -np.dot(s, eye)
    view[1, 3] = -np.dot(u, eye)
    view[2, 3] = np.dot(f, eye)
    return view

# Enhanced shaders with baked global illumination support
VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = texCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
"""

FRAGMENT_SHADER = """
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D lightMap;

void main() {
    // sample baked global illumination
    vec3 baked = texture(lightMap, TexCoord).rgb;

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor * baked;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor * baked;
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
        self.lightmap = None

    def init_gl(self):
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH)
        glutInitWindowSize(self.width, self.height)
        glutCreateWindow(b"PyOpenGL Scene")
        glEnable(GL_DEPTH_TEST)
        self.program = create_shader_program()
        self.setup_geometry()
        self.create_lightmap()
        glutDisplayFunc(self.render)
        glutIdleFunc(self.update)

    def setup_geometry(self):
        # cube vertices with normals and texture coordinates
        cube_vertices = [
            # positions        normals        u  v
            -0.5, -0.5, -0.5, 0.0,  0.0, -1.0, 0.0, 0.0,
             0.5, -0.5, -0.5, 0.0,  0.0, -1.0, 1.0, 0.0,
             0.5,  0.5, -0.5, 0.0,  0.0, -1.0, 1.0, 1.0,
             0.5,  0.5, -0.5, 0.0,  0.0, -1.0, 1.0, 1.0,
            -0.5,  0.5, -0.5, 0.0,  0.0, -1.0, 0.0, 1.0,
            -0.5, -0.5, -0.5, 0.0,  0.0, -1.0, 0.0, 0.0,

            -0.5, -0.5,  0.5, 0.0,  0.0, 1.0, 0.0, 0.0,
             0.5, -0.5,  0.5, 0.0,  0.0, 1.0, 1.0, 0.0,
             0.5,  0.5,  0.5, 0.0,  0.0, 1.0, 1.0, 1.0,
             0.5,  0.5,  0.5, 0.0,  0.0, 1.0, 1.0, 1.0,
            -0.5,  0.5,  0.5, 0.0,  0.0, 1.0, 0.0, 1.0,
            -0.5, -0.5,  0.5, 0.0,  0.0, 1.0, 0.0, 0.0,

            -0.5,  0.5,  0.5, -1.0, 0.0, 0.0, 1.0, 0.0,
            -0.5,  0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0,
            -0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0,
            -0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0,
            -0.5, -0.5,  0.5, -1.0, 0.0, 0.0, 1.0, 1.0,
            -0.5,  0.5,  0.5, -1.0, 0.0, 0.0, 1.0, 0.0,

             0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0,
             0.5,  0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 0.0,
             0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
             0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
             0.5, -0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 1.0,
             0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0,

            -0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0,
             0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0,
             0.5, -0.5,  0.5, 0.0, -1.0, 0.0, 1.0, 1.0,
             0.5, -0.5,  0.5, 0.0, -1.0, 0.0, 1.0, 1.0,
            -0.5, -0.5,  0.5, 0.0, -1.0, 0.0, 0.0, 1.0,
            -0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0,

            -0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0,
             0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0,
             0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 1.0, 1.0,
             0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 1.0, 1.0,
            -0.5,  0.5,  0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
            -0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0,
        ]
        self.cube_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.cube_vbo)
        array_type = (GLfloat * len(cube_vertices))
        glBufferData(GL_ARRAY_BUFFER, len(cube_vertices) * 4, array_type(*cube_vertices), GL_STATIC_DRAW)

        plane_vertices = [
            -5.0, 0.0, -5.0, 0.0, 1.0, 0.0, 0.0, 0.0,
             5.0, 0.0, -5.0, 0.0, 1.0, 0.0, 1.0, 0.0,
             5.0, 0.0,  5.0, 0.0, 1.0, 0.0, 1.0, 1.0,
             5.0, 0.0,  5.0, 0.0, 1.0, 0.0, 1.0, 1.0,
            -5.0, 0.0,  5.0, 0.0, 1.0, 0.0, 0.0, 1.0,
            -5.0, 0.0, -5.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        ]
        self.plane_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.plane_vbo)
        array_type = (GLfloat * len(plane_vertices))
        glBufferData(GL_ARRAY_BUFFER, len(plane_vertices) * 4, array_type(*plane_vertices), GL_STATIC_DRAW)

    def create_lightmap(self):
        # create a 1x1 white texture as placeholder for baked lighting
        self.lightmap = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.lightmap)
        data = (GLfloat * 3)(1.0, 1.0, 1.0)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, data)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

    def set_uniforms(self):
        glUseProgram(self.program)
        angle = np.radians(self.angle)
        c, s = np.cos(angle), np.sin(angle)
        model = np.array([
            [c, 0.0, s, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [-s, 0.0, c, 0.0],
            [0.0, 0.0, 0.0, 1.0]
        ], dtype=np.float32)
        view = look_at([5, 5, 5], [0, 0, 0], [0, 1, 0])
        projection = perspective(45, self.width / self.height, 0.1, 100.0)
        loc_model = glGetUniformLocation(self.program, 'model')
        loc_view = glGetUniformLocation(self.program, 'view')
        loc_proj = glGetUniformLocation(self.program, 'projection')
        glUniformMatrix4fv(loc_model, 1, GL_FALSE, model)
        glUniformMatrix4fv(loc_view, 1, GL_FALSE, view)
        glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection)
        glUniform3f(glGetUniformLocation(self.program, 'lightPos'), 5.0, 5.0, 5.0)
        glUniform3f(glGetUniformLocation(self.program, 'viewPos'), 5.0, 5.0, 5.0)
        glUniform3f(glGetUniformLocation(self.program, 'lightColor'), 1.0, 1.0, 1.0)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.lightmap)
        glUniform1i(glGetUniformLocation(self.program, 'lightMap'), 0)

    def draw_geometry(self, vbo, color):
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(24))
        glEnableVertexAttribArray(2)
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
