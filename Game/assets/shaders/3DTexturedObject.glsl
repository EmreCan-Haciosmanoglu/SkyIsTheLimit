#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TextureIndex;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_FragPosition;
out vec2 v_UV;
out vec3 v_Normal;
out float v_TextureIndex;

void main()
{
    v_FragPosition = vec3(u_Transform * vec4(a_Position,1.0));
	v_UV = a_UV;
	v_Normal = a_Normal;
	v_TextureIndex = a_TextureIndex;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec3 u_LightPos;
uniform vec4 u_TintColor;
uniform sampler2D u_Textures[32];

in vec3 v_FragPosition;
in vec2 v_UV;
in vec3 v_Normal;
in float v_TextureIndex;

void main()
{
	vec3 norm = normalize(v_Normal); // maybe not needed
	float elevation = u_LightPos.y;

	vec3 lightDir = normalize(u_LightPos);
	if(lightDir.y < 0)
		lightDir = vec3(0.0, 0.0, 0.0);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = (diff) * vec3(1.0, 1.0, 1.0);
	
	//float minAL = max(0.1, elevation);
	float AO = 0.2;

	elevation = degrees(sin(radians(elevation*90.0)))/90.0;
	elevation = max(0.1, elevation);
	
	vec4 result = vec4(min(diffuse.x, elevation) + AO, min(diffuse.y, elevation) + AO, min(diffuse.z, elevation) + AO, 1.0);

	color = result * u_TintColor * texture(u_Textures[int(v_TextureIndex+0.1)], v_UV);
	//color = vec4( v_UV.x, v_UV.y, 1.0f, 1.0f);
	//color = vec4(1.0- v_TextureIndex,1.0- v_TextureIndex,1.0- v_TextureIndex, 1.0);
}

