#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TextureIndex;

uniform mat4 ligthSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = ligthSpaceMatrix * model * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

void main()
{
}