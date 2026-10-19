/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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
