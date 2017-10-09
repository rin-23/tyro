varying highp vec2 vTexcoord;

uniform sampler2D uSampler;
uniform lowp vec4 uColor;

void main()
{
    gl_FragColor = vec4(1, 1, 1, texture2D(uSampler, vTexcoord).a) * uColor;
}