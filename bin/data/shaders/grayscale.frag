uniform sampler2D tex; // texture unit

void main()
{
    vec2 texCoords = gl_TexCoord[0].st;
    vec4 color = texture2D(tex, texCoords);
    color.rgb = vec3(color.r*0.31 + color.g*0.59 + color.b*0.11);
    gl_FragColor = colora;
} 
