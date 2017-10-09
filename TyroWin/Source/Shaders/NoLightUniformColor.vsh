attribute vec4 aPosition;

uniform mat4 uMVPMatrix;
uniform float uPointSize;

void main()
{
    gl_PointSize = uPointSize;
    gl_Position = uMVPMatrix * aPosition;
}


