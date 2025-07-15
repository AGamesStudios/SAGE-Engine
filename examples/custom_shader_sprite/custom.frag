#version 330
uniform sampler2D u_tex;
in vec2 v_uv;
in vec4 v_color;
out vec4 color;
void main() {
    vec4 t = texture(u_tex, v_uv) * v_color;
    float g = dot(t.rgb, vec3(0.299, 0.587, 0.114));
    color = vec4(g, g, g, t.a);
}
