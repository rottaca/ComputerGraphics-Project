#version 330

uniform sampler2D diffuseTex;
uniform sampler2D heightTex;
uniform vec3 heightColor;

in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 outputColor;

void main()
{
    if (gl_FragCoord.x < 640) outputColor = texture(diffuseTex, fragTexCoord);
    else outputColor = texture(heightTex, fragTexCoord).x * vec4(heightColor, 1.0f);
}
