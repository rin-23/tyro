#version 400
in vec4 aPosition;
in vec3 aColor;

uniform mat4 uMVPMatrix;
out vec4 vColor;

void main()
{
    gl_Position = uMVPMatrix * aPosition;
    vColor.rgb = aColor;
    vColor.a = 1.0;
}


