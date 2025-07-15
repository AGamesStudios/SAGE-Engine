[[block]] struct Globals { mvp : mat4x4<f32>; };
@group(0) @binding(0) var<uniform> globals : Globals;
@group(1) @binding(0) var textures : array<texture_2d<f32>>;
@group(1) @binding(1) var samp : sampler;

struct Instance {
    pos_uv : vec4<f32>;
    tex_id : u32;
};

@vertex
fn vs_main(@location(0) pos: vec2<f32>, @location(1) inst: Instance) ->
    (@builtin(position) vec4<f32>, @location(0) vec2<f32>, @location(1) u32) {
    let world = globals.mvp * vec4<f32>(pos * inst.pos_uv.zw + inst.pos_uv.xy, 0.0, 1.0);
    return (world, pos, inst.tex_id);
}
