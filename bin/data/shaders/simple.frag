uniform sampler2D tex; // texture unit

void main()
{
    vec2 texCoords = gl_TexCoord[0].st;
    vec4 color = texture2D(tex, texCoords);
    gl_FragColor = color;
} 
