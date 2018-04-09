#version 400
in vec3 aPosition;

uniform mat4 uMVPMatrix;
uniform float uPointSize;

void main()
{
    gl_PointSize = uPointSize;
    gl_Position = uMVPMatrix * vec4(aPosition,1.0);
}


