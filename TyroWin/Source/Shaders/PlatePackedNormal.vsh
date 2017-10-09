attribute vec4 aPosition; //local space
attribute float aPackedNormal; //local space
attribute float aJoint1;
attribute float aJoint2;
attribute float aWeight1;
attribute float aWeight2;

uniform mat4 uMVPMatrix;
uniform mat3 uNMatrix;
uniform lowp vec4 uColor;
uniform mat4 uSkinMatrix[16];

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
    
    vec4 p1 = uSkinMatrix[int(aJoint1)] * aPosition;
    vec4 p2 = uSkinMatrix[int(aJoint2)] * aPosition;
    vec4 skinnedPos = vec4(p1.xyz * aWeight1 + p2.xyz * aWeight2, 1.0);
//    vec4 skinnedPos = aPosition;
    gl_Position = uMVPMatrix * skinnedPos;
    
    vec4 n1 = uSkinMatrix[int(aJoint1)] * vec4(unPackedNormal, 0.0);
    vec4 n2 = uSkinMatrix[int(aJoint2)] * vec4(unPackedNormal, 0.0);
    vec3 skinnedNorm = n1.xyz * aWeight1 + n2.xyz * aWeight2;
//    vec3 skinnedNorm = unPackedNormal;
    vec3 eyeNormal = normalize(uNMatrix * skinnedNorm);
    float diffuse = abs(dot(eyeNormal, cLightDir));
    
    vColor.rgb = max(0.1 * uColor.rgb, diffuse * uColor.rgb);
    vColor.a = uColor.a;
}


