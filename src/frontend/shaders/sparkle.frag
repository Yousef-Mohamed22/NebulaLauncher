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

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 starColor;
    float intensity; 
} ubuf;

void main()
{
    vec2 uv = qt_TexCoord0 - vec2(0.5);    
    float d = length(uv);

    float aa = fwidth(d);

    float core = ubuf.intensity * 0.005 / (d + 0.001);

    vec2 rays = ubuf.intensity / (abs(uv) + 0.001);
    float starEffect = rays.x * rays.y;

    float mask = 1.0 - smoothstep(0.2 - aa, 0.5 + aa, d);

    float finalIntensity = (core + starEffect) * mask;

    fragColor = ubuf.starColor *
                finalIntensity *
                ubuf.qt_Opacity;
}

