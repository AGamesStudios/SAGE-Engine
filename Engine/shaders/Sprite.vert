#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 vTexCoord;
out vec4 vColor;

uniform mat3 uProjection;
uniform mat3 uTransform;
uniform vec4 uTexRect; // xy = offset, zw = scale

void main() {
    vec3 pos = uProjection * uTransform * vec3(aPos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    vTexCoord = aTexCoord * uTexRect.zw + uTexRect.xy;
    vColor = aColor;
}
