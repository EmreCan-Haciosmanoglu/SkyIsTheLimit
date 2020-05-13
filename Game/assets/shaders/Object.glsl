#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_UV;
out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
	v_UV = a_UV;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
    v_FragPos = vec3(u_Transform * vec4(a_Position,1.0));
	v_Normal = a_Normal;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform vec3 u_LightPos;

in vec2 v_UV;
in vec3 v_Normal;
in vec3 v_FragPos;

void main()
{
	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(u_LightPos - v_FragPos);

	float diff = max(dot(norm, lightDir), 0.0);

	vec3 diffuse = (diff) * vec3(1.0, 1.0, 1.0);

	vec3 result = vec3(diffuse.x + 0.1, diffuse.y + 0.1, diffuse.z + 0.1);// * vec3(v_Color);

	color = texture(u_Texture, v_UV );
}