#version 400
in vec4 aPosition;
in vec2 aTexcoord;

uniform mat4 uNDCTransform;

out vec2 vTexcoord;

void main()
{
    gl_Position = uNDCTransform * aPosition;
    vTexcoord = aTexcoord;
}