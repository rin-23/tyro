#version 400
in vec4 aPosition;
in vec2 aTexcoord;

out vec2 vTexcoord;

void main()
{
    gl_Position = aPosition;
    vTexcoord = aTexcoord;
}