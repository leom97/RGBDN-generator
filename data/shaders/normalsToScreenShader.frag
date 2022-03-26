#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D normalsTexture;

void main()
{             
    vec3 col = texture(normalsTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
}