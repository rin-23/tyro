attribute vec4 aPosition; //local space
attribute vec3 aNormal; //local space

uniform mat4 uMVPMatrix;
uniform mat3 uNMatrix;
uniform lowp vec4 uColor;

const float c_zero = 0.0;

uniform mat4 uMVMatrix;
uniform vec4 uLightPos; //camera space

varying lowp vec4 vColor;

void main()
{
    gl_Position = uMVPMatrix * aPosition;
    vec3 eyeNormal = normalize(uNMatrix * aNormal);

    vec4 eyePosition = uMVMatrix * aPosition;
    vec3 lightDirection = normalize(vec3(uLightPos - eyePosition));
    float diffuse = max(c_zero, dot(eyeNormal, lightDirection));
    vColor.rgb = (0.1 + diffuse) * uColor.rgb;
    vColor.a = uColor.a;
}
