attribute vec4 aPosition; //local space
attribute float aJoint1;
attribute float aJoint2;
attribute float aWeight1;
attribute float aWeight2;

uniform mat4 uMVPMatrix;
uniform mat4 uSkinMatrix[16];

void main()
{
    vec4 p1 = uSkinMatrix[int(aJoint1)] * aPosition;
    vec4 p2 = uSkinMatrix[int(aJoint2)] * aPosition;
    vec4 skinnedPos = vec4(p1.xyz * aWeight1 + p2.xyz * aWeight2, 1.0);
    gl_Position = uMVPMatrix * skinnedPos;
}


