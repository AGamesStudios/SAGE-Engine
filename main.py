import sys
import ctypes

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

# Enhanced shaders with baked global illumination and shadow mapping
VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = texCoord;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
"""

FRAGMENT_SHADER = """
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightColor2;
uniform float ambientStrength;
uniform vec3 objectColor;
uniform sampler2D lightMap;
uniform sampler2D shadowMap;

float shadow_calculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x=-2; x<=2; ++x) {
        for(int y=-2; y<=2; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += currentDepth - 0.005 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}

void main() {
    vec3 baked = texture(lightMap, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = 0.6 * spec * lightColor;

    float shadow = shadow_calculation(FragPosLightSpace, norm, lightDir);
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;
    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 64);
    vec3 specular2 = 0.6 * spec2 * lightColor2;

    vec3 ambient = ambientStrength * baked;
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular) + diffuse2 + specular2;
    vec3 result = lighting * objectColor * baked;
    result = pow(result, vec3(1.0 / 2.2));
    color = vec4(result, 1.0);
}
"""

# simple shaders for the shadow depth map
DEPTH_VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0);
}
"""

DEPTH_FRAGMENT_SHADER = """
#version 330 core
void main() {}
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

def create_depth_program():
    vshader = compile_shader(DEPTH_VERTEX_SHADER, GL_VERTEX_SHADER)
    fshader = compile_shader(DEPTH_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
    program = glCreateProgram()
    glAttachShader(program, vshader)
    glAttachShader(program, fshader)
    glLinkProgram(program)
    if glGetProgramiv(program, GL_LINK_STATUS) != GL_TRUE:
        raise RuntimeError(glGetProgramInfoLog(program))
    glDeleteShader(vshader)
    glDeleteShader(fshader)
    return program

class Engine:
    def __init__(self, width=800, height=600):
        self.width = width
        self.height = height
        self.angle = 0
        self.camera_pos = np.array([4.0, 3.0, 6.0], dtype=np.float32)
        self.light_pos = np.array([5.0, 10.0, 5.0], dtype=np.float32)
        self.light_pos2 = np.array([-4.0, 5.0, -3.0], dtype=np.float32)
        self.light_color2 = np.array([0.6, 0.6, 0.8], dtype=np.float32)
        self.ambient = 0.3
        self.program = None
        self.depth_program = None
        self.lightmap = None
        self.shadow_fbo = None
        self.shadow_map = None
        self.shadow_size = 1024
        self.vao = None

    def init_gl(self):
        glutInit(sys.argv)
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
        glutInitWindowSize(self.width, self.height)
        glutCreateWindow(b"PyOpenGL Scene")
        glEnable(GL_DEPTH_TEST)
        glEnable(GL_MULTISAMPLE)
        glClearColor(0.1, 0.1, 0.1, 1.0)
        glViewport(0, 0, self.width, self.height)
        self.program = create_shader_program()
        self.depth_program = create_depth_program()
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.setup_geometry()
        self.create_lightmap()
        self.create_shadow_buffer()
        glutDisplayFunc(self.render)
        glutIdleFunc(self.update)
        glutReshapeFunc(self.reshape)

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

    def create_shadow_buffer(self):
        self.shadow_fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.shadow_fbo)

        self.shadow_map = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.shadow_map)
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            self.shadow_size,
            self.shadow_size,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            None,
        )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)
        border = (GLfloat * 4)(1.0, 1.0, 1.0, 1.0)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border)
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.shadow_map, 0
        )
        glDrawBuffer(GL_NONE)
        glReadBuffer(GL_NONE)
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER)
        if status != GL_FRAMEBUFFER_COMPLETE:
            print("Shadow framebuffer incomplete:", status)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

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
        eye = self.camera_pos
        view = look_at(eye, [0, 0, 0], [0, 1, 0])
        projection = perspective(45, self.width / self.height, 0.1, 100.0)
        light_view = look_at(self.light_pos, [0,0,0], [0,1,0])
        light_proj = perspective(90, 1.0, 1.0, 25.0)
        light_space = np.dot(light_proj, light_view)
        loc_model = glGetUniformLocation(self.program, 'model')
        loc_view = glGetUniformLocation(self.program, 'view')
        loc_proj = glGetUniformLocation(self.program, 'projection')
        loc_lightspace = glGetUniformLocation(self.program, 'lightSpaceMatrix')
        glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.flatten())
        glUniformMatrix4fv(loc_view, 1, GL_TRUE, view.flatten())
        glUniformMatrix4fv(loc_proj, 1, GL_TRUE, projection.flatten())
        glUniformMatrix4fv(loc_lightspace, 1, GL_TRUE, light_space.flatten())
        glUniform3f(glGetUniformLocation(self.program, 'lightPos'), *self.light_pos)
        glUniform3f(glGetUniformLocation(self.program, 'lightPos2'), *self.light_pos2)
        glUniform3f(glGetUniformLocation(self.program, 'viewPos'), eye[0], eye[1], eye[2])
        glUniform3f(glGetUniformLocation(self.program, 'lightColor'), 1.0, 1.0, 1.0)
        glUniform3f(glGetUniformLocation(self.program, 'lightColor2'), *self.light_color2)
        glUniform1f(glGetUniformLocation(self.program, 'ambientStrength'), self.ambient)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.lightmap)
        glUniform1i(glGetUniformLocation(self.program, 'lightMap'), 0)
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.shadow_map)
        glUniform1i(glGetUniformLocation(self.program, 'shadowMap'), 1)

    def draw_geometry(self, vbo, color):
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(24))
        glEnableVertexAttribArray(2)
        glUniform3f(glGetUniformLocation(self.program, 'objectColor'), *color)
        glDrawArrays(GL_TRIANGLES, 0, 36 if vbo == self.cube_vbo else 6)

    def draw_depth_geometry(self, vbo):
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glDrawArrays(GL_TRIANGLES, 0, 36 if vbo == self.cube_vbo else 6)

    def render(self):
        self.render_depth_pass()
        glViewport(0, 0, self.width, self.height)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        self.set_uniforms()
        self.draw_geometry(self.plane_vbo, (0.8, 0.8, 0.8))
        self.draw_geometry(self.cube_vbo, (0.4, 0.2, 0.8))
        glutSwapBuffers()

    def render_depth_pass(self):
        glUseProgram(self.depth_program)
        angle = np.radians(self.angle)
        c, s = np.cos(angle), np.sin(angle)
        model = np.array([
            [c, 0.0, s, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [-s, 0.0, c, 0.0],
            [0.0, 0.0, 0.0, 1.0]
        ], dtype=np.float32)
        light_view = look_at(self.light_pos, [0,0,0], [0,1,0])
        light_proj = perspective(90, 1.0, 1.0, 25.0)
        light_space = np.dot(light_proj, light_view)
        loc_model = glGetUniformLocation(self.depth_program, 'model')
        loc_light = glGetUniformLocation(self.depth_program, 'lightSpaceMatrix')
        glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.flatten())
        glUniformMatrix4fv(loc_light, 1, GL_TRUE, light_space.flatten())
        glViewport(0, 0, self.shadow_size, self.shadow_size)
        glBindFramebuffer(GL_FRAMEBUFFER, self.shadow_fbo)
        glClear(GL_DEPTH_BUFFER_BIT)
        self.draw_depth_geometry(self.plane_vbo)
        self.draw_depth_geometry(self.cube_vbo)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def update(self):
        self.angle += 0.1
        glutPostRedisplay()

    def reshape(self, w, h):
        self.width = max(1, w)
        self.height = max(1, h)
        glViewport(0, 0, self.width, self.height)

    def run(self):
        self.init_gl()
        glutMainLoop()

if __name__ == '__main__':
    engine = Engine()
    engine.run()
