
import numpy as np, ctypes, math
from OpenGL import GL

VERT = """#version 330 core
layout (location=0) in vec2 aPos;
layout (location=1) in vec3 aColor;
uniform mat4 uM; uniform mat4 uP;
out vec3 vC;
void main(){ vC=aColor; gl_Position = uP * uM * vec4(aPos,0.0,1.0); }
""";

FRAG = """#version 330 core
in vec3 vC; out vec4 fragColor;
void main(){ fragColor = vec4(vC,1.0); }
""";

def _compile(src, stage):
    sid = GL.glCreateShader(stage); GL.glShaderSource(sid, src); GL.glCompileShader(sid)
    if GL.glGetShaderiv(sid, GL.GL_COMPILE_STATUS) != GL.GL_TRUE:
        raise RuntimeError(GL.glGetShaderInfoLog(sid))
    return sid

def _link(vs, fs):
    v=_compile(vs, GL.GL_VERTEX_SHADER); f=_compile(fs, GL.GL_FRAGMENT_SHADER)
    p = GL.glCreateProgram(); GL.glAttachShader(p,v); GL.glAttachShader(p,f); GL.glLinkProgram(p)
    if GL.glGetProgramiv(p, GL.GL_LINK_STATUS) != GL.GL_TRUE:
        raise RuntimeError(GL.glGetProgramInfoLog(p))
    GL.glDetachShader(p,v); GL.glDetachShader(p,f); GL.glDeleteShader(v); GL.glDeleteShader(f); return p

class OpenGL2DRenderer:
    def __init__(self, api=None):
        self.api=api
        self.prog=_link(VERT, FRAG)
        self.uM=GL.glGetUniformLocation(self.prog,'uM'); self.uP=GL.glGetUniformLocation(self.prog,'uP')
        verts = np.array([
            -0.5,-0.5,  1,0,0,
             0.5,-0.5,  0,1,0,
             0.5, 0.5,  0,0,1,
            -0.5,-0.5,  1,0,0,
             0.5, 0.5,  0,0,1,
            -0.5, 0.5,  1,1,0,
        ], dtype=np.float32).reshape((-1,5))
        self.n = verts.shape[0]
        self.vao = GL.glGenVertexArrays(1); self.vbo=GL.glGenBuffers(1)
        GL.glBindVertexArray(self.vao); GL.glBindBuffer(GL.GL_ARRAY_BUFFER,self.vbo)
        GL.glBufferData(GL.GL_ARRAY_BUFFER, verts.nbytes, verts, GL.GL_STATIC_DRAW)
        stride = 5*4
        GL.glEnableVertexAttribArray(0); GL.glVertexAttribPointer(0,2,GL.GL_FLOAT,GL.GL_FALSE,stride,ctypes.c_void_p(0))
        GL.glEnableVertexAttribArray(1); GL.glVertexAttribPointer(1,3,GL.GL_FLOAT,GL.GL_FALSE,stride,ctypes.c_void_p(8))
        GL.glBindVertexArray(0); GL.glBindBuffer(GL.GL_ARRAY_BUFFER,0)
        self._t=0.0; self._wire=False

    def toggle_wireframe(self): self._wire=not self._wire

    def render(self, scene, camera, window, dt:float):
        self._t += dt
        GL.glViewport(0,0,int(window.width),int(window.height))
        GL.glDisable(GL.GL_DEPTH_TEST); GL.glDisable(GL.GL_CULL_FACE)
        GL.glClearColor(0.12,0.12,0.14,1.0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)

        w=float(window.width); h=float(window.height)
        r=w/h
        P=np.array([[1/r,0,0,0],[0,1,0,0],[0,0,-1,0],[0,0,0,1]],dtype=np.float32)
        c = math.cos(self._t); s = math.sin(self._t)
        M=np.array([[c,-s,0,0],[s,c,0,0],[0,0,1,0],[0,0,0,1]],dtype=np.float32)

        GL.glUseProgram(self.prog)
        GL.glUniformMatrix4fv(self.uP,1,GL.GL_FALSE,P.T.astype(np.float32))
        GL.glUniformMatrix4fv(self.uM,1,GL.GL_FALSE,M.T.astype(np.float32))
        GL.glBindVertexArray(self.vao)
        GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_LINE if self._wire else GL.GL_FILL)
        GL.glDrawArrays(GL.GL_TRIANGLES, 0, self.n)
        GL.glBindVertexArray(0)
        GL.glUseProgram(0)
