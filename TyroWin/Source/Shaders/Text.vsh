attribute vec4 aPosition;
attribute vec2 aTexcoord;

varying highp vec2 vTexcoord;

uniform mat4 uNDCTransform;

void main()
{
    gl_Position = uNDCTransform * aPosition;
    vTexcoord = aTexcoord;
}