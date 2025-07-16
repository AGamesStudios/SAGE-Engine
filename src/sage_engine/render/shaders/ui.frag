#version 330
uniform sampler2D u_tex[8];
in vec2 v_uv;
in float v_atlas;
in vec4 v_color;
in vec2 v_local;
in float v_radius;
out vec4 color;
void main() {
    int idx = int(v_atlas);
    vec4 base = texture(u_tex[idx], v_uv) * v_color;
    float r = v_radius;
    if (r > 0.0) {
        vec2 dist = min(v_local, 1.0 - v_local);
        float a = clamp(min(dist.x, dist.y) / r, 0.0, 1.0);
        base.a *= a;
    }
    color = base;
}
