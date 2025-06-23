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

# Enhanced shaders with baked global illumination, shadow mapping and SSAO
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
out vec3 FragPosView;
out vec3 NormalView;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

void main() {
    mat4 modelView = view * model;
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    FragPosView = vec3(modelView * vec4(position, 1.0));
    NormalView = mat3(transpose(inverse(modelView))) * normal;
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

uniform vec3 dirLightDir;
uniform vec3 pointLightPos;
uniform vec3 viewPos;
uniform vec3 dirLightColor;
uniform vec3 pointLightColor;
uniform float ambientStrength;
uniform vec3 objectColor;
uniform sampler2D lightMap;
uniform sampler2D shadowMap;
uniform sampler2D ssaoMap;
uniform vec2 screenSize;

float shadow_calculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // 7x7 PCF kernel for softer shadows
    for(int x=-3; x<=3; ++x) {
        for(int y=-3; y<=3; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += currentDepth - 0.005 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 49.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}

void main() {
    vec3 baked = texture(lightMap, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 dir = normalize(-dirLightDir);

    vec3 viewDir = normalize(viewPos - FragPos);
    float diff = max(dot(norm, dir), 0.0);
    vec3 diffuse = diff * dirLightColor;
    vec3 reflectDir = reflect(-dir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * dirLightColor;

    float shadow = shadow_calculation(FragPosLightSpace, norm, dir);
    vec3 ptDir = normalize(pointLightPos - FragPos);
    float diff2 = max(dot(norm, ptDir), 0.0);
    vec3 diffuse2 = diff2 * pointLightColor;
    vec3 reflectDir2 = reflect(-ptDir, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32);
    vec3 specular2 = 0.5 * spec2 * pointLightColor;

    float ao = texture(ssaoMap, gl_FragCoord.xy / screenSize).r;
    vec3 ambient = ambientStrength * baked * ao;
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular) + diffuse2 + specular2;
    vec3 result = lighting * objectColor * baked;
    result = pow(result, vec3(1.0 / 2.2));
    color = vec4(result, 1.0);
}
"""

GBUFFER_FRAGMENT_SHADER = """
#version 330 core
in vec3 FragPosView;
in vec3 NormalView;
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
void main() {
    gPosition = FragPosView;
    gNormal = normalize(NormalView);
}
"""

QUAD_VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;
void main() {
    TexCoord = texCoord;
    gl_Position = vec4(position, 0.0, 1.0);
}
"""

SSAO_FRAGMENT_SHADER = """
#version 330 core
out float FragColor;
in vec2 TexCoord;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[16];
uniform mat4 projection;
const float radius = 0.5;
const float bias = 0.025;
void main() {
    vec3 fragPos = texture(gPosition, TexCoord).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoord).xyz);
    vec3 randomVec = texture(texNoise, TexCoord * 4.0).xyz;
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float occlusion = 0.0;
    for(int i=0;i<16;++i){
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;
        vec4 offset = projection * vec4(samplePos,1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = texture(gPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0,1.0, radius/abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - occlusion / 16.0;
    FragColor = occlusion;
}
"""

SSAO_BLUR_FRAGMENT_SHADER = """
#version 330 core
out float FragColor;
in vec2 TexCoord;
uniform sampler2D ssaoInput;
void main() {
    vec2 texelSize = 1.0 / textureSize(ssaoInput, 0);
    float result = 0.0;
    for(int x=-2; x<=2; ++x){
        for(int y=-2; y<=2; ++y){
            result += texture(ssaoInput, TexCoord + vec2(x,y) * texelSize).r;
        }
    }
    FragColor = result / 25.0;
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

def create_gbuffer_program():
    vshader = compile_shader(VERTEX_SHADER, GL_VERTEX_SHADER)
    fshader = compile_shader(GBUFFER_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
    program = glCreateProgram()
    glAttachShader(program, vshader)
    glAttachShader(program, fshader)
    glLinkProgram(program)
    if glGetProgramiv(program, GL_LINK_STATUS) != GL_TRUE:
        raise RuntimeError(glGetProgramInfoLog(program))
    glDeleteShader(vshader)
    glDeleteShader(fshader)
    return program

def create_ssao_program():
    vshader = compile_shader(QUAD_VERTEX_SHADER, GL_VERTEX_SHADER)
    fshader = compile_shader(SSAO_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
    program = glCreateProgram()
    glAttachShader(program, vshader)
    glAttachShader(program, fshader)
    glLinkProgram(program)
    if glGetProgramiv(program, GL_LINK_STATUS) != GL_TRUE:
        raise RuntimeError(glGetProgramInfoLog(program))
    glDeleteShader(vshader)
    glDeleteShader(fshader)
    return program

def create_ssao_blur_program():
    vshader = compile_shader(QUAD_VERTEX_SHADER, GL_VERTEX_SHADER)
    fshader = compile_shader(SSAO_BLUR_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
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
    def __init__(self, width=800, height=600, quality='high'):
        self.width = width
        self.height = height
        self.low_quality = quality == 'low'
        self.angle = 0
        self.camera_pos = np.array([4.0, 3.0, 6.0], dtype=np.float32)
        self.dir_light = np.array([-0.2, -1.0, -0.3], dtype=np.float32)
        self.dir_color = np.array([1.0, 1.0, 0.9], dtype=np.float32)
        self.point_light = np.array([4.0, 4.0, 4.0], dtype=np.float32)
        self.point_color = np.array([0.6, 0.6, 0.8], dtype=np.float32)
        # stronger ambient light so objects remain visible
        self.ambient = 0.4
        self.program = None
        self.depth_program = None
        self.gbuffer_program = None
        self.ssao_program = None
        self.ssao_blur_program = None
        self.lightmap = None
        self.shadow_fbo = None
        self.shadow_map = None
        self.shadow_size = 1024 if not self.low_quality else 512
        self.vao = None
        self.g_fbo = None
        self.g_position = None
        self.g_normal = None
        self.g_depth = None
        self.ssao_fbo = None
        self.ssao_fbo_blur = None
        self.ssao_color = None
        self.ssao_tex = None
        self.noise_tex = None
        self.ssao_samples = None
        self.quad_vao = None
        self.quad_vbo = None
        self.enable_ssao = not self.low_quality

    def init_gl(self):
        try:
            glutInit(sys.argv)
        except Exception as e:
            print("Failed to initialize GLUT. Ensure an OpenGL context is available:", e)
            raise
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
        glutInitWindowSize(self.width, self.height)
        glutCreateWindow(b"PyOpenGL Scene")
        glEnable(GL_DEPTH_TEST)
        glEnable(GL_MULTISAMPLE)
        glClearColor(0.1, 0.1, 0.1, 1.0)
        glViewport(0, 0, self.width, self.height)
        self.program = create_shader_program()
        self.depth_program = create_depth_program()
        self.gbuffer_program = create_gbuffer_program()
        self.ssao_program = create_ssao_program()
        self.ssao_blur_program = create_ssao_blur_program()
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.setup_geometry()
        self.create_lightmap()
        self.create_shadow_buffer()
        if self.enable_ssao:
            self.create_gbuffer()
            self.create_ssao_resources()
        else:
            self.g_fbo = None
            self.ssao_tex = self.create_white_texture()
        self.create_quad()
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

    def create_white_texture(self):
        tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex)
        data = (GLfloat * 3)(1.0, 1.0, 1.0)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, data)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        return tex

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

    def create_gbuffer(self):
        self.g_fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.g_fbo)

        self.g_position = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.g_position)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, self.width, self.height, 0, GL_RGB, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.g_position, 0)

        self.g_normal = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.g_normal)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, self.width, self.height, 0, GL_RGB, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, self.g_normal, 0)

        self.g_depth = glGenRenderbuffers(1)
        glBindRenderbuffer(GL_RENDERBUFFER, self.g_depth)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, self.width, self.height)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self.g_depth)

        attachments = (GLuint * 2)(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1)
        glDrawBuffers(2, attachments)
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER)
        if status != GL_FRAMEBUFFER_COMPLETE:
            print("G-buffer incomplete:", status)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def create_ssao_resources(self):
        self.ssao_fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.ssao_fbo)

        self.ssao_color = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.ssao_color)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, self.width, self.height, 0, GL_RED, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.ssao_color, 0)
        if glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE:
            print("SSAO framebuffer incomplete")
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        self.ssao_fbo_blur = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.ssao_fbo_blur)
        self.ssao_tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.ssao_tex)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, self.width, self.height, 0, GL_RED, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.ssao_tex, 0)
        if glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE:
            print("SSAO blur framebuffer incomplete")
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        noise = np.random.rand(16,3).astype(np.float32) * 2.0 - 1.0
        self.noise_tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.noise_tex)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)

        samples = []
        for i in range(16):
            sample = np.random.uniform(-1.0,1.0,3)
            sample[2] = np.random.uniform(0.0,1.0)
            sample /= np.linalg.norm(sample)
            scale = i / 16.0
            scale = 0.1 + 0.9 * scale * scale
            samples.append(sample * scale)
        self.ssao_samples = np.array(samples, dtype=np.float32)

    def create_quad(self):
        vertices = np.array([
            -1.0, -1.0, 0.0, 0.0,
             1.0, -1.0, 1.0, 0.0,
             1.0,  1.0, 1.0, 1.0,
             1.0,  1.0, 1.0, 1.0,
            -1.0,  1.0, 0.0, 1.0,
            -1.0, -1.0, 0.0, 0.0,
        ], dtype=np.float32)
        self.quad_vao = glGenVertexArrays(1)
        self.quad_vbo = glGenBuffers(1)
        glBindVertexArray(self.quad_vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.quad_vbo)
        glBufferData(GL_ARRAY_BUFFER, vertices.nbytes, vertices, GL_STATIC_DRAW)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, ctypes.c_void_p(8))
        glEnableVertexAttribArray(1)

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
        light_view = look_at(-self.dir_light * 10.0, [0,0,0], [0,1,0])
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
        glUniform3f(glGetUniformLocation(self.program, 'dirLightDir'), *self.dir_light)
        glUniform3f(glGetUniformLocation(self.program, 'pointLightPos'), *self.point_light)
        glUniform3f(glGetUniformLocation(self.program, 'viewPos'), eye[0], eye[1], eye[2])
        glUniform3f(glGetUniformLocation(self.program, 'dirLightColor'), *self.dir_color)
        glUniform3f(glGetUniformLocation(self.program, 'pointLightColor'), *self.point_color)
        glUniform1f(glGetUniformLocation(self.program, 'ambientStrength'), self.ambient)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.lightmap)
        glUniform1i(glGetUniformLocation(self.program, 'lightMap'), 0)
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.shadow_map)
        glUniform1i(glGetUniformLocation(self.program, 'shadowMap'), 1)
        glActiveTexture(GL_TEXTURE2)
        glBindTexture(GL_TEXTURE_2D, self.ssao_tex)
        glUniform1i(glGetUniformLocation(self.program, 'ssaoMap'), 2)
        glUniform2f(glGetUniformLocation(self.program, 'screenSize'), float(self.width), float(self.height))

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

    def draw_gbuffer_geometry(self, vbo):
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(24))
        glEnableVertexAttribArray(2)
        glDrawArrays(GL_TRIANGLES, 0, 36 if vbo == self.cube_vbo else 6)

    def render_gbuffer(self):
        glUseProgram(self.gbuffer_program)
        angle = np.radians(self.angle)
        c, s = np.cos(angle), np.sin(angle)
        model = np.array([
            [c, 0.0, s, 0.0],
            [0.0, 1.0, 0.0, 0.0],
            [-s, 0.0, c, 0.0],
            [0.0, 0.0, 0.0, 1.0]
        ], dtype=np.float32)
        view = look_at(self.camera_pos, [0,0,0], [0,1,0])
        proj = perspective(45, self.width/self.height, 0.1, 100.0)
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'model'),1,GL_TRUE,model.flatten())
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'view'),1,GL_TRUE,view.flatten())
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'projection'),1,GL_TRUE,proj.flatten())
        glViewport(0,0,self.width,self.height)
        glBindFramebuffer(GL_FRAMEBUFFER, self.g_fbo)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        self.draw_gbuffer_geometry(self.plane_vbo)
        self.draw_gbuffer_geometry(self.cube_vbo)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def render_ssao(self):
        glUseProgram(self.ssao_program)
        glBindFramebuffer(GL_FRAMEBUFFER, self.ssao_fbo)
        glClear(GL_COLOR_BUFFER_BIT)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.g_position)
        glUniform1i(glGetUniformLocation(self.ssao_program,'gPosition'),0)
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.g_normal)
        glUniform1i(glGetUniformLocation(self.ssao_program,'gNormal'),1)
        glActiveTexture(GL_TEXTURE2)
        glBindTexture(GL_TEXTURE_2D, self.noise_tex)
        glUniform1i(glGetUniformLocation(self.ssao_program,'texNoise'),2)
        proj = perspective(45, self.width/self.height, 0.1, 100.0)
        glUniformMatrix4fv(glGetUniformLocation(self.ssao_program,'projection'),1,GL_TRUE,proj.flatten())
        for i in range(16):
            loc = glGetUniformLocation(self.ssao_program, f'samples[{i}]')
            glUniform3f(loc, *self.ssao_samples[i])
        glBindVertexArray(self.quad_vao)
        glDrawArrays(GL_TRIANGLES, 0, 6)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        # blur pass
        glUseProgram(self.ssao_blur_program)
        glBindFramebuffer(GL_FRAMEBUFFER, self.ssao_fbo_blur)
        glClear(GL_COLOR_BUFFER_BIT)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.ssao_color)
        glUniform1i(glGetUniformLocation(self.ssao_blur_program, 'ssaoInput'), 0)
        glBindVertexArray(self.quad_vao)
        glDrawArrays(GL_TRIANGLES, 0, 6)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def render(self):
        self.render_depth_pass()
        if self.enable_ssao:
            self.render_gbuffer()
            self.render_ssao()
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
        light_view = look_at(-self.dir_light * 10.0, [0,0,0], [0,1,0])
        light_proj = perspective(90, 1.0, 1.0, 25.0)
        light_space = np.dot(light_proj, light_view)
        loc_model = glGetUniformLocation(self.depth_program, 'model')
        loc_light = glGetUniformLocation(self.depth_program, 'lightSpaceMatrix')
        glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.flatten())
        glUniformMatrix4fv(loc_light, 1, GL_TRUE, light_space.flatten())
        glViewport(0, 0, self.shadow_size, self.shadow_size)
        glBindFramebuffer(GL_FRAMEBUFFER, self.shadow_fbo)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonOffset(2.0, 4.0)
        glClear(GL_DEPTH_BUFFER_BIT)
        self.draw_depth_geometry(self.plane_vbo)
        self.draw_depth_geometry(self.cube_vbo)
        glDisable(GL_POLYGON_OFFSET_FILL)
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
    quality = 'low' if len(sys.argv) > 1 and sys.argv[1] == 'low' else 'high'
    engine = Engine(quality=quality)
    engine.run()
