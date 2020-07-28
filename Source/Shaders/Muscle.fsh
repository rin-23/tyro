#version 400
in vec4 vColor;
in float vDiffusion;
out vec4 fFragColor;

layout(pixel_center_integer) in vec4 gl_FragCoord;


void main()
{   
    if (vDiffusion < 0.2) 
    {
        fFragColor = vec4(0.0, 0.0, 0.0, 1.0);
        discard;
    }
    else 
    {
        if (!gl_FrontFacing) 
        {
            //fFragColor = vec4(gl_FragCoord.x/1000.0, 0.0, 0.0, 1.0); //vColor; 
            fFragColor = vColor; 

        }
        else 
        {

            fFragColor = vec4(0.0, 0.0, 0.0, 1.0);
            discard; 
        }
    }
}

