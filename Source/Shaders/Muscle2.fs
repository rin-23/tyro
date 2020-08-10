#version 400

#define FLT_MAX 3.402823466e+30 // changed +38 to 30 to avoid overflow?




uniform mat4 uMVPMatrix;
uniform mat4 uPMatrix;
uniform mat4 uMVMatrix;
uniform mat3 uNMatrix;
uniform vec4 uColor;
uniform vec4 uViewport;

 
uniform samplerBuffer uVerSampler;
uniform samplerBuffer uNormSampler;
// uniform samplerBuffer uGradSampler;

in vec4 vColor;
in float vDiffusion;
in vec3 vPosition;
in vec3 vNormal;
flat in int vTetId;

out vec4 fFragColor;

const float c_zero = 0.0;
const float c_one = 1.0;
const vec3 cLightDir = vec3(c_zero, c_zero, c_one);

// layout(pixel_center_integer) in vec4 gl_FragCoord;
// layout(pixel_center_integer) in vec4 gl_FragCoord;

vec3 glslproject(in vec3  obj, in mat4 mvp, in vec4  viewport)
{
    vec4 tmp = vec4(obj,1.0);
    tmp = mvp * tmp;
    // tmp = proj * tmp;

    tmp = tmp / tmp[3];
    tmp = 0.5 * tmp  + vec4(0.5);
    tmp[0] = tmp[0] * viewport[2] + viewport[0];
    tmp[1] = tmp[1] * viewport[3] + viewport[1];

    return vec3(tmp);
}


float func(in vec3 v0, in vec3 p, in float maxlength) 
{
    return length(v0 - p)/maxlength;    
}

vec3 calcNormal( in vec3 v0, in vec3 p, in float maxlength ) // for function f(p)
{
    const float eps = 0.0001; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3(func(v0,p+h.xyy,maxlength) - func(v0, p-h.xyy,maxlength),
                           func(v0,p+h.yxy,maxlength) - func(v0, p-h.yxy,maxlength),
                           func(v0,p+h.yyx,maxlength) - func(v0, p-h.yyx,maxlength)));
}

float volume(in mat4 V)
{
    // VOLUME Compute volumes of the tet defined over vertices V
    vec3 a = V[0].xyz;
    vec3 b = V[1].xyz;
    vec3 c = V[2].xyz;
    vec3 d = V[3].xyz;
    
    float v = dot((a-d),cross(b-d,c-d))/6;
    return v;
}

vec4 barycoord(in vec3 p3, in mat4 V) 
{
    vec4 a = V[0];
    vec4 b = V[1];
    vec4 c = V[2];
    vec4 d = V[3];
    vec4 p = vec4(p3,0.0); 

    mat4 T = mat4(p,b,c,d);
    float A1 = volume(T);

    T = mat4(p,c,a,d);
    float A2 = volume(T);

    T = mat4(p,b,d,a);
    float A3 = volume(T);

    T = mat4(p,b,c,a);
    float A4 = volume(T);
    
    float A = volume(V);

    vec4 B = vec4(A1/A, A2/A, A3/A, A4/A);
    return B;
}

// returns 1 if we hit an isovalue, in which case isopoint will contatin the coordinate
// return zero otherwise
int raymarch(in  vec3  entrypoint, 
             in  vec3  exitpoint, 
             in  vec3  v0, 
             in float maxlength,
             in float isovalue,
             out vec3  isopoint) 
{
    vec3 r = normalize(exitpoint - entrypoint);
    
    const int numSteps = 200;
    float stepSize = length(entrypoint - exitpoint)/numSteps;
    
    for (int i = 0; i < numSteps; i++) 
    {
        isopoint = entrypoint + stepSize*i*r;
        if (func(v0, isopoint, maxlength) < isovalue) 
            return 1;
    }
    return 0;
}

void 
fetchTetData(in int vTetId, out mat4 VO, out mat4 VC, out mat4 N, out vec4 D) 
{
    // fetch 4 verticies in object space for the current tet
    VO[0] = texelFetch(uVerSampler, 4*vTetId+0);
    VO[1] = texelFetch(uVerSampler, 4*vTetId+1);
    VO[2] = texelFetch(uVerSampler, 4*vTetId+2);
    VO[3] = texelFetch(uVerSampler, 4*vTetId+3);
    
    // convert 4 verticies in object space to screen space
    VC[0] = uMVMatrix * vec4(VO[0].xyz,1.0);
    VC[1] = uMVMatrix * vec4(VO[1].xyz,1.0);
    VC[2] = uMVMatrix * vec4(VO[2].xyz,1.0);
    VC[3] = uMVMatrix * vec4(VO[3].xyz,1.0);

    // fetch 4 normals for the current tet
    N[0] = texelFetch(uNormSampler, 4*vTetId+0);
    N[1] = texelFetch(uNormSampler, 4*vTetId+1);
    N[2] = texelFetch(uNormSampler, 4*vTetId+2);
    N[3] = texelFetch(uNormSampler, 4*vTetId+3);

    // fetch 4 diffusion values for the current tet
    // D = texelFetch(uDiffusedValuesSampler, vTetId);
}

void
findExitPoint(in mat4 VO, in mat4 N, in vec3 entrypoint, out vec3 exitpoint) 
{   
    vec3 e = vec3(0.); // eye origin in eye space
    // vec3 entrypoint = vec3(uMVMatrix * vec4(vPosition,1.0)); // entry point into tet in eye space   
    vec3 r = normalize(entrypoint - e); // ray  vector from the camera to  the entry point
    vec4 lambdavec = vec4(FLT_MAX);     // the interstion of the ray with the face i is e+lambda[i]*r 

    for (int i=0; i < 4; i++) 
    {
        vec3 v = vec3(uMVMatrix*vec4(VO[3-i].xyz,1.0));
        vec3 n =  normalize(uNMatrix*N[i].xyz);

        float denom = dot(v - e, n);
        float numerator = dot(r, n);
        
        if (numerator > 0.) // face is not visible to a potential exit point
            lambdavec[i] = denom/numerator;
    }

    // find closest exit point
    float lambda = min(min(min(lambdavec[0], lambdavec[1]), lambdavec[2]), lambdavec[3]);
    exitpoint = e + lambda*r;
}

void main()
{    
    //    if (!gl_FrontFacing) 

    mat4 V,VC;
    mat4 N;
    vec4 D;
    
    fetchTetData(vTetId, V, VC, N, D);

    // vec3 e = vec3(0.,0.,0.); // eye origin in eye space
    vec3 entrypoint = vec3(uMVMatrix * vec4(vPosition,1.0)); // entry point into tet in eye space
    
    vec3 exitpoint;
    findExitPoint(V, N, entrypoint, exitpoint);

    // vec3 r = normalize(entrypoint - e); // ray  vector from the camera to  the entry point
    // vec4 lambdavec = vec4(FLT_MAX);     // the interstion of the ray with the face i is e+lambda[i]*r 

    
    
    // for (int i=0; i < 4; i++) 
    // {
    //     vec3 v = vec3(uMVMatrix*vec4(V[3-i].xyz,1.0));
    //     vec3 n =  normalize(uNMatrix*N[i].xyz);

    //     float denom = dot(v - e, n);
    //     float numerator = dot(r, n);
        
    //     if (numerator > 0.) // face is not visible to a potential exit point
    //         lambdavec[i] = denom/numerator;
    // }

    // // find closest exit point
    // float lambda = min(min(min(lambdavec[0], lambdavec[1]), lambdavec[2]), lambdavec[3]);
    // vec3 exitpoint = e + lambda*r;
    
    //compute max depth of the tet
    vec3 center = vec3(0.25*(V[0] + V[1] + V[2] + V[3]));
    center = vec3(uMVMatrix  * vec4(center, 1));
    vec3 v0 = vec3(uMVMatrix  * vec4(V[0].xyz, 1));
    float tl = 2*length(center - v0);

    // float l = length(entrypoint-exitpoint);
    // float diff = l/tl;
    // fFragColor = vColor; 
    // // fFragColor = vec4(l, 0.0, 0.0, 1.0); 
    // // diff = length(exitpoint-center) / tl;
    // if (diff < 100000)  
    //     discard; 

    vec3 u = exitpoint-entrypoint;
    // // vec3 nn = vec3(0.0,  0.0, -1.0);
    float l = length(u);
    float diff = l/(2*tl);
    // fFragColor = vColor; //vec4(diff, 0.0, 0.0, 1.0); 
    // fFragColor = vec4(diff, 0.0, 0.0, 1.0); 
    // if (diff > 1.1)  
    //     discard; 

#if 1
    vec3 isopoint;
    int rm = raymarch(entrypoint, 
                        exitpoint, 
                        v0, 
                        tl,
                        0.5,
                        isopoint);

    

    // fFragColor = vColor;
    if (rm == 0) 
    {
        discard; 
    }
    else 
    {
        
        vec3 eyeNormal;
        if (length(isopoint-entrypoint) < 0.01) 
        {
            eyeNormal = normalize(uNMatrix * normalize(vNormal));
        }
        else 
        {
            eyeNormal =  calcNormal(v0, isopoint, tl );
        }
        
        float intensity = max(c_zero, dot(eyeNormal, cLightDir));
        fFragColor.rgb = max(0.3 * uColor.rgb, intensity * uColor.rgb);
        fFragColor.a = uColor.a;

        // overwrite depth
        vec3 isopointscreen = glslproject(isopoint, uPMatrix, uViewport);
        gl_FragDepth = isopointscreen.z;

        // vDiffusion = aDiffusion;
        // vPosition = aPosition.xyz;
    }

    // vec4(vPosition,1.0)
    // vec3 entrypointscreen = glslproject(vPosition, uMVPMatrix, uViewport);

    
    // if (abs(gl_FragCoord.x - entrypointscreen.x) > 0.01)
    //     discard;
    
    // if (abs(gl_FragCoord.y - entrypointscreen.y) > 0.01)
    //     discard;
    
    // if (abs(gl_FragDepth - entrypointscreen.z) > 0.02)
    //     discard;

        // fFragColor = vec4(0.0, 1.0, 0.0, 1.0);
#else
    if (vTetId == 0)
        fFragColor = diff*vec4(1.0,0.0,0.0,1.0);
    else if (vTetId == 1)
        fFragColor = diff*vec4(0.0,1.0,0.0,1.0);
    else if (vTetId == 2)
        fFragColor = diff*vec4(0.0,1.0,1.0,1.0);
    else 
        fFragColor = diff*vec4(1.0,1.0,1.0,1.0);

#endif
    
}
