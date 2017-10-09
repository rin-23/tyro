attribute vec4 aPosition; //local space
attribute float aPackedNormal; //local space

uniform mat4 uMVPMatrix;
uniform mat3 uNMatrix;
uniform lowp vec4 uColor;

const float c_zero = 0.0;
const float c_one = 1.0;
const float c_two = 2.0;
const vec3 packVal = vec3(c_one, 256.0, 65536.0);
const vec3 c_one_vec3 = vec3(c_one, c_one, c_one);
const vec3 c_two_vec3 = vec3(c_two, c_two, c_two);
const vec3 cLightDir = vec3(c_zero,c_zero,c_one);

varying lowp vec4 vColor;

void main()
{
    vec3 unPackedNormal = fract(aPackedNormal * packVal) * c_two_vec3 - c_one_vec3;
    vec3 eyeNormal = normalize(uNMatrix * unPackedNormal);
    float diffuse = abs(dot(eyeNormal, cLightDir));

    vColor.rgb = max(0.1 * uColor.rgb, diffuse * uColor.rgb);
    vColor.a = uColor.a;
    gl_Position = uMVPMatrix * aPosition;
}


