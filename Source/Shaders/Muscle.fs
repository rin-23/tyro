#version 400

#define FLT_MAX 3.402823466e+30 // changed +38 to 30 to avoid overflow?


uniform mat4 uMVMatrix;
uniform mat3 uNMatrix;
uniform vec4 uColor;

in vec4 vColor;
in float vDiffusion;
in vec3 vPosition;
in vec3 vNormal;


out vec4 fFragColor;

const float c_zero = 0.0;
const float c_one = 1.0;
const vec3 cLightDir = vec3(c_zero, c_zero, c_one);

layout(pixel_center_integer) in vec4 gl_FragCoord;

const mat4 V =  mat4( 0.,   0.,  0., -1., // 0. column
                      1.,   0.,  0., -1., // 1. column
                     0.5,   1.,  0., -1.,
                     0.5,  0.5,  1., -1.);

const mat4 N =  mat4(  0.872,    0.436,   0.218,  -1.0, //0 1 3
                      -0.872,    0.436,   0.218,  -1.0,
                         0.0,   -0.894,   0.447,  -1.0,
                         0.0,      0.0,    -1.0,  -1.0);

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

void main()
{   
    if (vDiffusion < 0.2) 
    {
        fFragColor = vec4(0.0, 0.0, 0.0, 1.0);
        discard;
    }
    else 
    {
    //    if (!gl_FrontFacing) 

        vec3 e = vec3(0.,0.,0.); // eye origin in eye space
        vec3 entrypoint = vec3(uMVMatrix * vec4(vPosition,1.0)); // entry point into tet in eye space
        vec3 r = normalize(entrypoint - e); // ray  vector from the camera to  the entry point
        vec4 lambdavec = vec4(FLT_MAX);     // the interstion of the ray with the face i is e+lambda[i]*r 

        //compute max depth of the tet
        vec3 center = vec3(0.25*(V[0] + V[1] + V[2] + V[3]));
        center = vec3(uMVMatrix  * vec4(center, 1));
        vec3 v0 = vec3(uMVMatrix  * vec4(V[0].xyz, 1));
        float tl = length(center - v0);
        
        for (int i=0; i < 4; i++) 
        {
            vec3 v = vec3(uMVMatrix*vec4(V[3-i].xyz,1.0));
            vec3 n =  normalize(uNMatrix*N[i].xyz);

            float denom = dot(v - e, n);
            float numerator = dot(r, n);
            
            if (numerator > 0.) // face is not visible to a potential exit point
                lambdavec[i] = denom/numerator;
        }

        // find closest exit point
        float lambda = min(min(min(lambdavec[0], lambdavec[1]), lambdavec[2]), lambdavec[3]);
        vec3 exitpoint = e + lambda*r;
        
        // float l = length(entrypoint-exitpoint);
        // float diff = l/tl;
        // fFragColor = vColor; 
        // // fFragColor = vec4(l, 0.0, 0.0, 1.0); 
        // // diff = length(exitpoint-center) / tl;
        // if (diff < 100000)  
        //     discard; 

        // vec3 u = exitpoint-entrypoint;
        // // vec3 nn = vec3(0.0,  0.0, -1.0);
        // float l = length(u);
        // float diff = l/tl;
        // // fFragColor = vColor; //vec4(diff, 0.0, 0.0, 1.0); 
        // fFragColor = vec4(diff, 0.0, 0.0, 1.0); 
        // // if (l  < 0.5)  
            // discard; 


        vec3 isopoint;
        int rm = raymarch(entrypoint, 
                          exitpoint, 
                          v0, 
                          tl,
                          1.0,
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
            // vDiffusion = aDiffusion;
            // vPosition = aPosition.xyz;
        }
        
            // fFragColor = vec4(0.0, 1.0, 0.0, 1.0);
    
    }
}

