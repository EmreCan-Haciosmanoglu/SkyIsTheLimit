#type vertex
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 ligthSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = ligthSpaceMatrix * model * vec4(aPos, 1.0);
}

#type fragment
#version 330 core

void main()
{
}