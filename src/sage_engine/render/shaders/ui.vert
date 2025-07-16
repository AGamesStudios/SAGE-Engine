#version 330
in vec2 in_vert;
in vec2 in_pos;
in vec2 in_scale;
// 'in_rot' stores radius for UI widgets
in float in_radius;
in float in_atlas;
in vec4 in_uv;
in float in_blend;
in vec4 in_color;
in float in_depth;
uniform mat3 u_viewProj;
out vec2 v_uv;
out float v_atlas;
out vec4 v_color;
out vec2 v_local;
out float v_radius;
void main() {
    v_local = in_vert + vec2(0.5);
    vec2 pos = in_pos + in_vert * in_scale;
    gl_Position = vec4((u_viewProj * vec3(pos, 1.0)).xy, in_depth, 1.0);
    v_uv = mix(in_uv.xy, in_uv.zw, v_local);
    v_atlas = in_atlas;
    v_color = in_color;
    v_radius = in_radius;
}
