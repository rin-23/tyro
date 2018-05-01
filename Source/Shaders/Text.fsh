#version 400

in vec2 vTexcoord;

uniform sampler2D uSampler;
uniform vec4 uColor;
out vec4 fFragColor;

void main()
{
    fFragColor = vec4(1, 1, 1, texture2D(uSampler, vTexcoord).a) * uColor;
}