#version 400

in vec2 vTexcoord;

uniform sampler2D uSampler;
out vec4 fFragColor;

void main()
{
    fFragColor = vec4(texture2D(uSampler, vTexcoord).rgb, 1.0);
}