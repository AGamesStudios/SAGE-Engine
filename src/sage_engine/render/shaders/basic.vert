#version 330
in vec2 in_vert;
in vec2 in_pos;
in vec2 in_scale;
in float in_rot;
in float in_atlas;
in vec4 in_uv;
in float in_blend;
in vec4 in_color;
in float in_depth;
uniform mat3 u_viewProj;
out vec2 v_uv;
out float v_atlas;
out vec4 v_color;
void main() {
    vec2 scaled = in_vert * in_scale;
    float c = cos(in_rot);
    float s = sin(in_rot);
    vec2 pos = in_pos + vec2(c * scaled.x - s * scaled.y, s * scaled.x + c * scaled.y);
    gl_Position = vec4((u_viewProj * vec3(pos, 1.0)).xy, in_depth, 1.0);
    v_uv = mix(in_uv.xy, in_uv.zw, in_vert + vec2(0.5));
    v_atlas = in_atlas;
    v_color = in_color;
}
