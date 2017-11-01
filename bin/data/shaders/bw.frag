uniform sampler2D tex; // texture unit

void main()
{
    vec2 texCoords = gl_TexCoord[0].st;
    vec4 color = texture2D(tex, texCoords);
    color.rgb = vec3(color.r + color.g + color.b)/3.0;
    if (color.r<0.2 || color.r>0.9) color.r = 0.0; else color.r = 1.0;
    if (color.g<0.2 || color.g>0.9) color.g = 0.0; else color.g = 1.0;
    if (color.b<0.2 || color.b>0.9) color.b = 0.0; else color.b = 1.0;
    gl_FragColor = color;
} 
