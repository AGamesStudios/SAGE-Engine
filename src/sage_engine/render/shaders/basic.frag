#version 330
uniform sampler2D u_tex[8];
in vec2 v_uv;
in float v_atlas;
in vec4 v_color;
out vec4 color;
void main() {
    int idx = int(v_atlas);
    color = texture(u_tex[idx], v_uv) * v_color;
}
