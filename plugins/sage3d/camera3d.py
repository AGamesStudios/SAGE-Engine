
import numpy as np, math
class Camera3D:
    def __init__(self, fov=60.0, near=0.1, far=1000.0):
        self.pos=np.array([0.0,0.0,-3.0],dtype=np.float32); self.rot=np.array([0.0,0.0,0.0],dtype=np.float32)
        self.fov=float(fov); self.near=float(near); self.far=float(far); self.move_speed=3.0; self.look_speed=0.15
        self._last_cursor=None
    def _basis(self):
        pitch=math.radians(float(self.rot[0])); yaw=math.radians(float(self.rot[1]))
        cp,sp=math.cos(pitch),math.sin(pitch); cy,sy=math.cos(yaw),math.sin(yaw)
        fwd=np.array([sy*cp, sp, cy*cp],dtype=np.float32); right=np.array([cy,0.0,-sy],dtype=np.float32)
        up=np.cross(fwd,right)
        def n(v): m=float(np.linalg.norm(v)); return v if m<1e-8 else (v/m)
        return n(fwd),n(right),n(up)
    def get_view(self):
        fwd,right,up=self._basis(); p=self.pos.astype(np.float32)
        R=np.eye(4,dtype=np.float32); R[0,0:3]=right; R[1,0:3]=up; R[2,0:3]=-fwd
        T=np.eye(4,dtype=np.float32); T[0:3,3]=-p
        return R@T
    def get_proj(self, aspect: float = 16/9):
        f = 1.0 / math.tan(math.radians(self.fov) * 0.5)
        n, fa = float(self.near), float(self.far)
        P = np.zeros((4,4), dtype=np.float32)
        P[0,0] = f / max(1e-6, float(aspect)); P[1,1] = f
        P[2,2] = -(fa + n) / (fa - n); P[2,3] = -(2.0 * fa * n) / (fa - n); P[3,2] = -1.0
        return P
    def handle_input(self, window, dt: float):
        cur = window.get_cursor_pos()
        if self._last_cursor is not None:
            dx = float(cur[0]) - float(self._last_cursor[0])
            dy = float(cur[1]) - float(self._last_cursor[1])
            self.rot[1] -= dx * self.look_speed * 0.1
            self.rot[0] -= dy * self.look_speed * 0.1
            self.rot[0]=max(-89.9,min(89.9,float(self.rot[0])))
        self._last_cursor=cur
        forw,right,_ = self._basis()
        import numpy as np
        m=np.zeros(3,dtype=np.float32)
        try:
            if window.is_key_down(87): m+=forw
            if window.is_key_down(83): m-=forw
            if window.is_key_down(68): m+=right
            if window.is_key_down(65): m-=right
        except Exception:
            pass
        n=float(np.linalg.norm(m))
        if n>1e-6: self.pos += (m/n)*(self.move_speed*float(dt))
