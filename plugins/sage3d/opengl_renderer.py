
import numpy as np, ctypes
from OpenGL import GL
from sagecore.math3d import normalize, compose_trs

def _compile_shader(src, stage):
    sid = GL.glCreateShader(stage); GL.glShaderSource(sid, src); GL.glCompileShader(sid)
    if GL.glGetShaderiv(sid, GL.GL_COMPILE_STATUS) != GL.GL_TRUE:
        log = GL.glGetShaderInfoLog(sid).decode('utf-8','ignore')
        raise RuntimeError(f'Shader compile error:\n{log}')
    return sid

def _link_program(vs_src, fs_src):
    vs = _compile_shader(vs_src, GL.GL_VERTEX_SHADER)
    fs = _compile_shader(fs_src, GL.GL_FRAGMENT_SHADER)
    pid = GL.glCreateProgram(); GL.glAttachShader(pid, vs); GL.glAttachShader(pid, fs)
    GL.glLinkProgram(pid)
    GL.glDetachShader(pid, vs); GL.glDetachShader(pid, fs)
    GL.glDeleteShader(vs); GL.glDeleteShader(fs)
    if GL.glGetProgramiv(pid, GL.GL_LINK_STATUS) != GL.GL_TRUE:
        log = GL.glGetProgramInfoLog(pid).decode('utf-8','ignore')
        raise RuntimeError(f'Program link error:\n{log}')
    return pid

VERT_SRC = """#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
uniform mat4 uM; uniform mat4 uV; uniform mat4 uP; uniform mat3 uN;
out vec3 vN;
void main(){ vN = normalize(uN * aNormal); gl_Position = uP * uV * uM * vec4(aPos,1.0); }
""";

FRAG_SRC = """#version 330 core
in vec3 vN; out vec4 fragColor;
uniform vec3 uLightDir; uniform vec3 uAmbient; uniform vec3 uLightColor;
uniform vec3 uAlbedo; uniform int uShade; // 0=unlit, 1=lambert
void main(){
    if(uShade==0){ fragColor = vec4(uAlbedo,1.0); return; }
    vec3 N = normalize(vN);
    float ndl = max(dot(N, normalize(uLightDir)), 0.0);
    vec3 color = uAmbient + uLightColor * ndl;
    fragColor = vec4(color * uAlbedo, 1.0);
}
""";

AXIS_VERT = """#version 330 core
layout (location=0) in vec3 aPos; layout (location=2) in vec3 aColor;
uniform mat4 uM; uniform mat4 uV; uniform mat4 uP;
out vec3 vC; void main(){ vC=aColor; gl_Position = uP*uV*uM*vec4(aPos,1.0); }
""";
AXIS_FRAG = """#version 330 core
in vec3 vC; out vec4 fragColor; void main(){ fragColor = vec4(vC,1.0); }
""";

def _make_unit_cube():
    s=0.5
    v000=(-s,-s,-s); v001=(-s,-s,s); v010=(-s,s,-s); v011=(-s,s,s)
    v100=(s,-s,-s);  v101=(s,-s,s);  v110=(s,s,-s);  v111=(s,s,s)
    def face(a,b,c,d, n): return [(*a,*n),(*b,*n),(*c,*n),(*a,*n),(*c,*n),(*d,*n)]
    tris=[]; tris+=face(v000,v001,v011,v010,(0,-1,0)); tris+=face(v100,v110,v111,v101,(0,1,0))
    tris+=face(v000,v010,v110,v100,(0,0,-1)); tris+=face(v001,v101,v111,v011,(0,0,1))
    tris+=face(v000,v100,v101,v001,(-1,0,0)); tris+=face(v010,v011,v111,v110,(1,0,0))
    arr=np.array(tris,dtype=np.float32).reshape((-1,6)); return arr

def _make_axes():
    lines = np.array([
        -10,0,0, 1,0,0,   10,0,0, 1,0,0,
        0,-10,0, 0,1,0,   0,10,0, 0,1,0,
        0,0,-10, 0,0,1,   0,0,10, 0,0,1,
    ], dtype=np.float32).reshape((-1,6))
    return lines

class _MeshGL:
    __slots__=('vao','vbo','n','stride')
    def __init__(self, arr: np.ndarray, has_color=False):
        self.n=int(arr.shape[0])
        self.stride=6*4
        self.vao=GL.glGenVertexArrays(1); self.vbo=GL.glGenBuffers(1)
        GL.glBindVertexArray(self.vao); GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self.vbo)
        GL.glBufferData(GL.GL_ARRAY_BUFFER, arr.nbytes, arr, GL.GL_STATIC_DRAW)
        GL.glEnableVertexAttribArray(0); GL.glVertexAttribPointer(0,3,GL.GL_FLOAT,GL.GL_FALSE,self.stride,ctypes.c_void_p(0))
        if has_color:
            GL.glEnableVertexAttribArray(2); GL.glVertexAttribPointer(2,3,GL.GL_FLOAT,GL.GL_FALSE,self.stride,ctypes.c_void_p(12))
        else:
            GL.glEnableVertexAttribArray(1); GL.glVertexAttribPointer(1,3,GL.GL_FLOAT,GL.GL_FALSE,self.stride,ctypes.c_void_p(12))
        GL.glBindVertexArray(0); GL.glBindBuffer(GL.GL_ARRAY_BUFFER,0)

class OpenGLRenderer:
    def __init__(self, api=None):
        self.api=api
        self.prog=_link_program(VERT_SRC, FRAG_SRC)
        self.uM=GL.glGetUniformLocation(self.prog,'uM'); self.uV=GL.glGetUniformLocation(self.prog,'uV')
        self.uP=GL.glGetUniformLocation(self.prog,'uP'); self.uN=GL.glGetUniformLocation(self.prog,'uN')
        self.uLightDir=GL.glGetUniformLocation(self.prog,'uLightDir'); self.uAmbient=GL.glGetUniformLocation(self.prog,'uAmbient')
        self.uLightColor=GL.glGetUniformLocation(self.prog,'uLightColor'); self.uAlbedo=GL.glGetUniformLocation(self.prog,'uAlbedo')
        self.uShade=GL.glGetUniformLocation(self.prog,'uShade')
        self._cube=_MeshGL(_make_unit_cube(), has_color=False)
        self.axes_prog=_link_program(AXIS_VERT, AXIS_FRAG)
        self.axes_um=GL.glGetUniformLocation(self.axes_prog,'uM')
        self.axes_uv=GL.glGetUniformLocation(self.axes_prog,'uV')
        self.axes_up=GL.glGetUniformLocation(self.axes_prog,'uP')
        self._axes=_MeshGL(_make_axes(), has_color=True)
        self.draw_axes=True

        self.wire=False; self.light_dir=normalize(np.array([-0.5,-1.0,-0.3],dtype=np.float32))
        self.ambient=np.array([0.25,0.25,0.28],dtype=np.float32); self.light_color=np.array([1.0,1.0,1.0],dtype=np.float32)
        self.shade='unlit'

    def toggle_axes(self): self.draw_axes = not self.draw_axes
    def set_wireframe(self, on:bool): self.wire=bool(on)
    def set_shade(self, mode:str): self.shade='lambert' if mode!='unlit' else 'unlit'

    def debug_pre(self, scene=None, camera=None, window=None, **_):
        if not self.draw_axes: return
        V=camera.get_view(); P=camera.get_proj(max(1.0, window.width/max(1,window.height)))
        GL.glUseProgram(self.axes_prog)
        GL.glUniformMatrix4fv(self.axes_uv,1,GL.GL_FALSE,np.ascontiguousarray(V.T,dtype=np.float32))
        GL.glUniformMatrix4fv(self.axes_up,1,GL.GL_FALSE,np.ascontiguousarray(P.T,dtype=np.float32))
        M=np.eye(4,dtype=np.float32)
        GL.glUniformMatrix4fv(self.axes_um,1,GL.GL_FALSE,np.ascontiguousarray(M.T,dtype=np.float32))
        GL.glBindVertexArray(self._axes.vao)
        GL.glLineWidth(2.0); GL.glDrawArrays(GL.GL_LINES, 0, self._axes.n); GL.glBindVertexArray(0)
        GL.glUseProgram(0)

    def render(self, scene, camera, window, dt:float):
        GL.glViewport(0,0,int(window.width),int(window.height))
        GL.glEnable(GL.GL_DEPTH_TEST); GL.glFrontFace(GL.GL_CCW); GL.glDisable(GL.GL_CULL_FACE)
        GL.glClearColor(0.18,0.19,0.22,1.0); GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

        V=camera.get_view(); P=camera.get_proj(max(1.0, window.width/max(1,window.height)))
        GL.glUseProgram(self.prog)
        GL.glUniformMatrix4fv(self.uV,1,GL.GL_FALSE,np.ascontiguousarray(V.T,dtype=np.float32))
        GL.glUniformMatrix4fv(self.uP,1,GL.GL_FALSE,np.ascontiguousarray(P.T,dtype=np.float32))
        GL.glUniform3f(self.uLightDir, float(self.light_dir[0]), float(self.light_dir[1]), float(self.light_dir[2]))
        GL.glUniform3f(self.uAmbient, float(self.ambient[0]), float(self.ambient[1]), float(self.ambient[2]))
        GL.glUniform3f(self.uLightColor, float(self.light_color[0]), float(self.light_color[1]), float(self.light_color[2]))
        GL.glUniform1i(self.uShade, 1 if self.shade=='lambert' else 0)

        for obj, mesh in getattr(scene,'get_drawlist',lambda:[])():
            size=float(mesh.get('size',1.0))
            pos=obj.get('params',{}).get('position',[0,0,0])
            rot=obj.get('params',{}).get('rotation_euler',[0,0,0])
            scl=[size,size,size]
            M=compose_trs(pos,rot,scl); MV=V@M; N=np.linalg.inv(MV[:3,:3]).T
            GL.glUniformMatrix4fv(self.uM,1,GL.GL_FALSE,np.ascontiguousarray(M.T,dtype=np.float32))
            GL.glUniformMatrix3fv(self.uN,1,GL.GL_FALSE,np.ascontiguousarray(N.T,dtype=np.float32))
            GL.glUniform3f(self.uAlbedo,0.85,0.85,0.95)
            GL.glBindVertexArray(self._cube.vao)
            GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_LINE if self.wire else GL.GL_FILL)
            GL.glDrawArrays(GL.GL_TRIANGLES, 0, self._cube.n)
            GL.glBindVertexArray(0)
        GL.glUseProgram(0)
