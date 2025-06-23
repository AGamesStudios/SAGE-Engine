import sys
import ctypes
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    from OpenGL.GL import *
    from OpenGL.GLUT import *
    from OpenGL.GLU import *
except ImportError as e:
    print("PyOpenGL is required to run this example. Install it via pip.")
    sys.exit(1)

import numpy as np
try:
    from numba import njit
    import scipy  # Numba's BLAS support requires SciPy
    NUMBA_AVAILABLE = True
except Exception:
    NUMBA_AVAILABLE = False
    def njit(*args, **kwargs):
        def wrapper(func):
            return func
        if args and callable(args[0]):
            return args[0]
        return wrapper

def optional_njit(*args, **kwargs):
    def decorator(func):
        return njit(*args, **kwargs)(func) if NUMBA_AVAILABLE else func
    if args and callable(args[0]):
        return decorator(args[0])
    return decorator

@optional_njit(cache=True, fastmath=True)
def perspective(fov, aspect, near, far):
    f = 1.0 / np.tan(np.radians(fov) / 2.0)
    proj = np.zeros((4, 4), dtype=np.float32)
    proj[0, 0] = f / aspect
    proj[1, 1] = f
    proj[2, 2] = (far + near) / (near - far)
    proj[2, 3] = (2.0 * far * near) / (near - far)
    proj[3, 2] = -1.0
    return proj

@optional_njit(cache=True, fastmath=True)
def ortho(left, right, bottom, top, near, far):
    mat = np.zeros((4,4), dtype=np.float32)
    mat[0,0] = 2.0/(right-left)
    mat[1,1] = 2.0/(top-bottom)
    mat[2,2] = -2.0/(far-near)
    mat[3,3] = 1.0
    mat[0,3] = -(right+left)/(right-left)
    mat[1,3] = -(top+bottom)/(top-bottom)
    mat[2,3] = -(far+near)/(far-near)
    return mat

@optional_njit(cache=True, fastmath=True)
def _look_at(eye, target, up):
    f = target - eye
    norm = np.sqrt(np.sum(f * f))
    if norm != 0.0:
        f /= norm
    u = up.copy()
    norm = np.sqrt(np.sum(u * u))
    if norm != 0.0:
        u /= norm
    s = np.cross(f, u)
    norm = np.sqrt(np.sum(s * s))
    if norm != 0.0:
        s /= norm
    u = np.cross(s, f)
    view = np.identity(4, dtype=np.float32)
    view[0, :3] = s
    view[1, :3] = u
    view[2, :3] = -f
    view[0, 3] = -(s[0]*eye[0] + s[1]*eye[1] + s[2]*eye[2])
    view[1, 3] = -(u[0]*eye[0] + u[1]*eye[1] + u[2]*eye[2])
    view[2, 3] = f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2]
    return view

def look_at(eye, target, up):
    eye = np.asarray(eye, dtype=np.float32)
    target = np.asarray(target, dtype=np.float32)
    up = np.asarray(up, dtype=np.float32)
    return _look_at(eye, target, up)

@optional_njit(cache=True)
def generate_ssao_samples(n):
    samples = np.empty((n, 3), dtype=np.float32)
    for i in range(n):
        sample = np.random.uniform(-1.0, 1.0, 3)
        sample[2] = np.random.uniform(0.0, 1.0)
        norm = np.sqrt(np.sum(sample * sample))
        if norm != 0.0:
            sample /= norm
        scale = i / n
        scale = 0.1 + 0.9 * scale * scale
        samples[i] = sample * scale
    return samples

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
uniform float metallic;
uniform float roughness;
uniform sampler2D lightMap;
uniform sampler2D shadowMap;
uniform sampler2D ssaoMap;
uniform vec2 screenSize;
uniform int shadowSamples;
uniform float evsmExponent;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

float chebyshev_upper(float mean, float m2, float t){
    float variance = max(m2 - mean * mean, 0.00002);
    float d = t - mean;
    return clamp(variance / (variance + d * d), 0.0, 1.0);
}

float evsm_eval(vec4 moments, float depth){
    float pos = chebyshev_upper(moments.x, moments.z, exp(evsmExponent * depth));
    float neg = chebyshev_upper(moments.y, moments.w, exp(-evsmExponent * depth));
    return min(pos, neg);
}

float shadow_calculation(vec4 fragPosLightSpace, float bias) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;
    float currentDepth = projCoords.z - bias;
    float angle = rand(projCoords.xy) * 6.2831853;
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    vec2 poissonDisk[16] = vec2[](
        vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
        vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
        vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
        vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
        vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
        vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
    );
    for(int i=0;i<shadowSamples;i++){
        vec2 offset = rot * poissonDisk[i] * texelSize * 2.0;
        vec4 moments = texture(shadowMap, projCoords.xy + offset);
        shadow += evsm_eval(moments, currentDepth);
    }
    shadow /= float(shadowSamples);
    return 1.0 - shadow;
}

const float PI = 3.14159265359;

vec3 fresnel_schlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_ggx(vec3 N, vec3 H, float rough){
    float a = rough * rough;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float geometry_schlick_ggx(float NdotV, float rough){
    float r = rough + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float rough){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometry_schlick_ggx(NdotV, rough);
    float ggx2 = geometry_schlick_ggx(NdotL, rough);
    return ggx1 * ggx2;
}

void main() {
    vec3 baked = texture(lightMap, TexCoord).rgb;

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 F0 = mix(vec3(0.04), objectColor, metallic);

    vec3 Lo = vec3(0.0);

    vec3 L = normalize(-dirLightDir);
    vec3 H = normalize(V + L);
    float NDF = distribution_ggx(N, H, roughness);
    float G   = geometry_smith(N, V, L, roughness);
    vec3 F    = fresnel_schlick(max(dot(H, V), 0.0), F0);
    vec3 spec = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = dirLightColor;
    Lo += (kD * objectColor / PI + spec) * radiance * NdotL;

    L = normalize(pointLightPos - FragPos);
    H = normalize(V + L);
    float dist = length(pointLightPos - FragPos);
    float attenuation = 1.0 / (dist * dist);
    NDF = distribution_ggx(N, H, roughness);
    G   = geometry_smith(N, V, L, roughness);
    F   = fresnel_schlick(max(dot(H, V), 0.0), F0);
    spec = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001);
    kS = F;
    kD = (1.0 - kS) * (1.0 - metallic);
    NdotL = max(dot(N, L), 0.0);
    radiance = pointLightColor * attenuation;
    Lo += (kD * objectColor / PI + spec) * radiance * NdotL;

    float bias = max(0.001 * (1.0 - dot(N, normalize(-dirLightDir))), 0.0005);
    float shadow = shadow_calculation(FragPosLightSpace, bias);

    float ao = texture(ssaoMap, gl_FragCoord.xy / screenSize).r;
    vec3 ambient = ambientStrength * baked * ao * objectColor;
    vec3 result = (ambient + (1.0 - shadow) * Lo) * baked;
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
    occlusion = clamp(occlusion, 0.0, 1.0);
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

# advanced shaders for the shadow depth map
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
layout(location = 0) out vec4 Moments;
uniform float near_plane;
uniform float far_plane;
uniform float evsmExponent;

float linearizeDepth(float depth){
    float z = depth * 2.0 - 1.0;
    return (2.0 * near_plane * far_plane) /
           (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() {
    float linear = linearizeDepth(gl_FragCoord.z);
    float pos = exp(evsmExponent * linear);
    float neg = exp(-evsmExponent * linear);
    Moments = vec4(pos, neg, pos * pos, neg * neg);
}
"""

def compile_shader(source, shader_type):
    shader = glCreateShader(shader_type)
    glShaderSource(shader, source)
    glCompileShader(shader)
    if glGetShaderiv(shader, GL_COMPILE_STATUS) != GL_TRUE:
        msg = glGetShaderInfoLog(shader)
        logger.error("Shader compilation failed:\n%s", msg.decode())
        raise RuntimeError(msg)
    return shader

# OpenGL debug output support for easier troubleshooting
DEBUGPROC = ctypes.CFUNCTYPE(None, GLuint, GLuint, GLuint, GLuint, GLsizei,
                             ctypes.c_char_p, ctypes.c_void_p)

def _gl_debug_callback(source, type_, id_, severity, length, message, userParam):
    msg = ctypes.string_at(message, length).decode('utf-8', 'ignore')
    logger.debug("GL DEBUG: %s", msg)

def enable_debug_output():
    if 'glDebugMessageCallback' in globals():
        glEnable(GL_DEBUG_OUTPUT)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)
        glDebugMessageCallback(DEBUGPROC(_gl_debug_callback), None)

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
    def __init__(self, width=800, height=600, quality='high', enable_debug=False):
        self.width = width
        self.height = height
        self.low_quality = quality == 'low'
        self.enable_debug = enable_debug
        self.camera_angle = 0.0
        self.camera_radius = 6.0
        self.camera_height = 3.0
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
        self.shadow_depth = None
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
        # exponent for the exponential variance shadow map. 80 caused overflow
        # with the scene's far plane so use a smaller value for stable shadows
        self.evsm_exponent = 3.0

    def init_gl(self):
        try:
            glutInit(sys.argv)
        except Exception as e:
            print("Failed to initialize GLUT. Ensure an OpenGL context is available:", e)
            raise
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
        glutInitWindowSize(self.width, self.height)
        glutCreateWindow(b"PyOpenGL Scene")
        if self.enable_debug:
            enable_debug_output()
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
        glutTimerFunc(int(1000/60), self.update, 0)
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
            GL_RGBA32F,
            self.shadow_size,
            self.shadow_size,
            0,
            GL_RGBA,
            GL_FLOAT,
            None,
        )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)
        border = (GLfloat * 4)(1.0, 1.0, 1.0, 1.0)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border)
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.shadow_map, 0
        )

        self.shadow_depth = glGenRenderbuffers(1)
        glBindRenderbuffer(GL_RENDERBUFFER, self.shadow_depth)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, self.shadow_size, self.shadow_size)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self.shadow_depth)

        glDrawBuffer(GL_COLOR_ATTACHMENT0)
        glReadBuffer(GL_COLOR_ATTACHMENT0)
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

        self.ssao_samples = generate_ssao_samples(16)

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

    def apply_common_uniforms(self):
        eye = np.array([
            np.cos(np.radians(self.camera_angle)) * self.camera_radius,
            self.camera_height,
            np.sin(np.radians(self.camera_angle)) * self.camera_radius
        ], dtype=np.float32)
        view = look_at(eye, [0, 0, 0], [0, 1, 0])
        projection = perspective(45, self.width / self.height, 0.1, 100.0)
        light_view = look_at(-self.dir_light * 10.0, [0,0,0], [0,1,0])
        light_proj = ortho(-10.0, 10.0, -10.0, 10.0, 1.0, 25.0)
        light_space = np.dot(light_proj, light_view)
        loc_view = glGetUniformLocation(self.program, 'view')
        loc_proj = glGetUniformLocation(self.program, 'projection')
        loc_lightspace = glGetUniformLocation(self.program, 'lightSpaceMatrix')
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
        glUniform1i(glGetUniformLocation(self.program, 'shadowSamples'), 8 if self.low_quality else 16)
        glUniform1f(glGetUniformLocation(self.program, 'evsmExponent'), self.evsm_exponent)

    def draw_geometry(self, vbo, color, model, metallic=0.0, roughness=0.5):
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(24))
        glEnableVertexAttribArray(2)
        glUniformMatrix4fv(glGetUniformLocation(self.program, 'model'), 1, GL_TRUE, model.flatten())
        glUniform3f(glGetUniformLocation(self.program, 'objectColor'), *color)
        glUniform1f(glGetUniformLocation(self.program, 'metallic'), metallic)
        glUniform1f(glGetUniformLocation(self.program, 'roughness'), roughness)
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
        cube_model = np.identity(4, dtype=np.float32)
        plane_model = np.identity(4, dtype=np.float32)
        eye = np.array([
            np.cos(np.radians(self.camera_angle)) * self.camera_radius,
            self.camera_height,
            np.sin(np.radians(self.camera_angle)) * self.camera_radius
        ], dtype=np.float32)
        view = look_at(eye, [0,0,0], [0,1,0])
        proj = perspective(45, self.width/self.height, 0.1, 100.0)
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'view'),1,GL_TRUE,view.flatten())
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'projection'),1,GL_TRUE,proj.flatten())
        glViewport(0,0,self.width,self.height)
        glBindFramebuffer(GL_FRAMEBUFFER, self.g_fbo)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'model'),1,GL_TRUE,plane_model.flatten())
        self.draw_gbuffer_geometry(self.plane_vbo)
        glUniformMatrix4fv(glGetUniformLocation(self.gbuffer_program,'model'),1,GL_TRUE,cube_model.flatten())
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
        glUseProgram(self.program)
        self.apply_common_uniforms()
        plane_model = np.identity(4, dtype=np.float32)
        cube_model = np.identity(4, dtype=np.float32)
        self.draw_geometry(self.plane_vbo, (0.8, 0.8, 0.8), plane_model, metallic=0.0, roughness=1.0)
        self.draw_geometry(self.cube_vbo, (0.4, 0.2, 0.8), cube_model, metallic=0.3, roughness=0.2)
        glutSwapBuffers()

    def render_depth_pass(self):
        glUseProgram(self.depth_program)
        cube_model = np.identity(4, dtype=np.float32)
        plane_model = np.identity(4, dtype=np.float32)
        light_view = look_at(-self.dir_light * 10.0, [0,0,0], [0,1,0])
        light_proj = ortho(-10.0, 10.0, -10.0, 10.0, 1.0, 25.0)
        light_space = np.dot(light_proj, light_view)
        loc_model = glGetUniformLocation(self.depth_program, 'model')
        loc_light = glGetUniformLocation(self.depth_program, 'lightSpaceMatrix')
        glUniformMatrix4fv(loc_light, 1, GL_TRUE, light_space.flatten())
        glUniform1f(glGetUniformLocation(self.depth_program,'near_plane'), 1.0)
        glUniform1f(glGetUniformLocation(self.depth_program,'far_plane'), 25.0)
        glUniform1f(glGetUniformLocation(self.depth_program,'evsmExponent'), self.evsm_exponent)
        glViewport(0, 0, self.shadow_size, self.shadow_size)
        glBindFramebuffer(GL_FRAMEBUFFER, self.shadow_fbo)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonOffset(2.0, 4.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUniformMatrix4fv(loc_model, 1, GL_TRUE, plane_model.flatten())
        self.draw_depth_geometry(self.plane_vbo)
        glUniformMatrix4fv(loc_model, 1, GL_TRUE, cube_model.flatten())
        self.draw_depth_geometry(self.cube_vbo)
        glDisable(GL_POLYGON_OFFSET_FILL)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def update(self, value=0):
        self.camera_angle += 0.1
        glutPostRedisplay()
        glutTimerFunc(int(1000/60), self.update, 0)

    def reshape(self, w, h):
        self.width = max(1, w)
        self.height = max(1, h)
        glViewport(0, 0, self.width, self.height)
        if self.enable_ssao:
            glDeleteFramebuffers(1, [self.g_fbo])
            glDeleteTextures(1, [self.g_position])
            glDeleteTextures(1, [self.g_normal])
            glDeleteRenderbuffers(1, [self.g_depth])
            glDeleteFramebuffers(1, [self.ssao_fbo])
            glDeleteFramebuffers(1, [self.ssao_fbo_blur])
            glDeleteTextures(1, [self.ssao_color])
            glDeleteTextures(1, [self.ssao_tex])
            glDeleteTextures(1, [self.noise_tex])
            self.create_gbuffer()
            self.create_ssao_resources()

    def set_quality(self, quality):
        """Switch between 'low' and 'high' quality at runtime."""
        if quality not in ('low', 'high'):
            raise ValueError("quality must be 'low' or 'high'")
        self.low_quality = quality == 'low'
        self.shadow_size = 512 if self.low_quality else 1024
        self.enable_ssao = not self.low_quality
        glDeleteFramebuffers(1, [self.shadow_fbo])
        glDeleteTextures(1, [self.shadow_map])
        glDeleteRenderbuffers(1, [self.shadow_depth])
        self.create_shadow_buffer()
        self.reshape(self.width, self.height)

    def run(self):
        self.init_gl()
        glutMainLoop()

def main(argv=None):
    if argv is None:
        argv = sys.argv
    quality = 'low' if 'low' in argv[1:] else 'high'
    enable_debug = 'debug' in argv[1:]
    engine = Engine(quality=quality, enable_debug=enable_debug)
    engine.run()

if __name__ == '__main__':
    main()
