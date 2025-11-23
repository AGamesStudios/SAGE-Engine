#version 330 core
in vec2 vTexCoord;
in vec4 vColor;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int uUseTexture;
uniform vec4 uColor;

void main() {
    if (uUseTexture != 0) {
        FragColor = texture(uTexture, vTexCoord) * uColor * vColor;
    } else {
        FragColor = uColor * vColor;
    }
}
