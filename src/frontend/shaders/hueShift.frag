#version 450

layout(std140, binding=0) uniform ubuf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float uHue;
};

layout(binding = 1) uniform sampler2D source;

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

vec3 hueShift(vec3 color, float hue) {
    const vec3 k = vec3(0.57735, 0.57735, 0.57735);
    float cosAngle = cos(hue);
    return vec3(
        color * cosAngle + cross(k, color) * sin(hue) + k * dot(k, color) * (1.0 - cosAngle)
    );
}

void main() {
    vec4 srcColor = texture(source, qt_TexCoord0);
    vec3 shiftedColor = hueShift(srcColor.rgb, uHue);
    fragColor = vec4(shiftedColor, srcColor.a);
}
