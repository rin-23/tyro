#version 400
in vec4 aPosition; //local space
in vec3 aNormal; //local space
in float aDiffusion;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat3 uNMatrix;
uniform vec4 uColor;

const float c_zero = 0.0;
const float c_one = 1.0;
const vec3 cLightDir = vec3(c_zero, c_zero, c_one);

out vec4 vColor;
out vec3 vPosition;
out vec3 vNormal;
out float vDiffusion;


void main()
{
    gl_Position = uMVPMatrix * aPosition;
    
    vec3 eyeNormal = normalize(uNMatrix * aNormal);
    float intensity = max(c_zero, dot(eyeNormal, cLightDir));
    vColor.rgb = max(0.3 * uColor.rgb, intensity * uColor.rgb);
    vColor.a = uColor.a;
    vDiffusion = aDiffusion;
    vPosition = aPosition.xyz;
    vNormal = aNormal;
}
