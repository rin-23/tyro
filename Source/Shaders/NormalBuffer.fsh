varying mediump vec3 vNormal;

void main()
{
    gl_FragColor = vec4((normalize(vNormal) + 1.0) * 0.5, 1);
}

