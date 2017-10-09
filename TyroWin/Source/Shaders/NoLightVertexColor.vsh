attribute vec4 aPosition; //local space
attribute vec4 aColor; //local space

uniform mat4 uMVPMatrix;

varying lowp vec4 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uMVPMatrix * aPosition;
}
