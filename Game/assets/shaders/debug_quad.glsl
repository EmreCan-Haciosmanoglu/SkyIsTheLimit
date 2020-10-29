#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TextureIndex;

uniform mat4 u_ViewProjection;

out vec2 TexCoords;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    //gl_Position = vec4(a_Position, 1.0);
    TexCoords = a_UV;
}

#type fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
    float depthValue = texture(depthMap, TexCoords).r;
    color = vec4(vec3(depthValue), 1.0); //ortho
    //color = vec4(1.0);
}