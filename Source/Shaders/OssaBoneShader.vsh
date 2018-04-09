//#version 100

#define NUM_FRAGMENTS 10
#define USE_POINT_LIGHT 0


attribute vec4 aPosition; //local space
attribute float aPackedNormal; //local space
attribute float aBoneID;

uniform mat4 uModelViewProjectionMatrix[NUM_FRAGMENTS];
uniform lowp mat3 uNormalMatrix[NUM_FRAGMENTS];
uniform lowp vec3 uColor[NUM_FRAGMENTS];

const float c_zero = 0.0;
const float c_one = 1.0;
const float c_two = 2.0;
const vec3 packVal = vec3(c_one, 256.0, 65536.0);
const vec3 c_one_vec3 = vec3(c_one, c_one, c_one);
const vec3 c_two_vec3 = vec3(c_two, c_two, c_two);


#if USE_POINT_LIGHT
uniform mat4 uModelViewMatrix[NUM_FRAGMENTS];
uniform vec4 uLightPosition; //camera space
#else
const vec3 uLightDir = vec3(c_zero,c_zero,c_one);
#endif

varying lowp vec3 vColor;

void main()
{
    int boneID = int(aBoneID);
    lowp vec3 unPackedNormal = fract(aPackedNormal * packVal) * c_two_vec3 - c_one_vec3;
    lowp vec3 eyeNormal = normalize(uNormalMatrix[boneID] * unPackedNormal);

#if USE_POINT_LIGHT
    vec4 eyePosition = uModelViewMatrix[boneID] * aPosition;
    vec3 lightDirection = normalize(vec3(uLightPosition - eyePosition));
    float diffuse = max(c_zero, dot(eyeNormal, lightDirection));
#else
    lowp float diffuse = max(c_zero, dot(eyeNormal, uLightDir));
#endif
    vColor = max(0.3 * uColor[boneID], diffuse * uColor[boneID]);
    gl_Position = uModelViewProjectionMatrix[boneID] * aPosition;
}


