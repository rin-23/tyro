attribute vec4 aPosition;
attribute float aPackedNormal; //local space

uniform mat4 uMVPMatrix;
uniform mat3 uNMatrix;

const float c_zero = 0.0;
const float c_one = 1.0;
const float c_two = 2.0;
const vec3 packVal = vec3(c_one, 256.0, 65536.0);
const vec3 c_one_vec3 = vec3(c_one, c_one, c_one);
const vec3 c_two_vec3 = vec3(c_two, c_two, c_two);

varying mediump vec3 vNormal;

void main()
{
    vec3 normal = fract(aPackedNormal * packVal) * c_two_vec3 - c_one_vec3;
    vNormal = normalize(uNMatrix * normal);
    gl_Position = uMVPMatrix * aPosition;
}