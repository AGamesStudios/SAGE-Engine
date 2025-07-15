@group(1) @binding(0) var textures : array<texture_2d<f32>>;
@group(1) @binding(1) var samp : sampler;

@fragment
fn fs_main(@location(0) uv: vec2<f32>, @location(1) tex_id: u32) -> @location(0) vec4<f32> {
    return textureSample(textures[tex_id], samp, uv);
}
