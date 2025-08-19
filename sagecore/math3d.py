
import numpy as np, math
def normalize(v):
    v = np.array(v, dtype=np.float32)
    n = float(np.linalg.norm(v))
    return v if n < 1e-8 else (v / n)
def compose_trs(pos, euler_deg, scl):
    px,py,pz = [float(x) for x in pos]
    import numpy as np, math
    rx,ry,rz = [math.radians(float(a)) for a in euler_deg]
    sx,sy,sz = [float(x) for x in scl]
    cx, sxn = math.cos(rx), math.sin(rx)
    cy, syn = math.cos(ry), math.sin(ry)
    cz, szn = math.cos(rz), math.sin(rz)
    Rz = np.array([[cz,-szn,0,0],[szn,cz,0,0],[0,0,1,0],[0,0,0,1]], dtype=np.float32)
    Ry = np.array([[cy,0,syn,0],[0,1,0,0],[-syn,0,cy,0],[0,0,0,1]], dtype=np.float32)
    Rx = np.array([[1,0,0,0],[0,cx,-sxn,0],[0,sxn,cx,0],[0,0,0,1]], dtype=np.float32)
    S  = np.diag([sx,sy,sz,1]).astype(np.float32)
    T  = np.eye(4,dtype=np.float32); T[0,3]=px; T[1,3]=py; T[2,3]=pz
    return T @ (Rz @ (Ry @ (Rx @ S)))
