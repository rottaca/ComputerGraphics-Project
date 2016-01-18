#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 matMVP;

out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    fragTexCoord = texCoord;
    fragNormal = mat3(matNormal) * normal;
    gl_Position = matMVP * position;
}
