uniform sampler2D tex; // texture unit

void main()
{
    vec2 texCoords = gl_TexCoord[0].st;
    vec4 color = texture2D(tex, texCoords);
    color -= texture2D(tex, texCoords - 0.0001)*2.7;
    color += texture2D(tex, texCoords + 0.0001)*2.7;
    color.rgb = vec3(color.r + color.g + color.b)/3.0;
    gl_FragColor = color;
}
