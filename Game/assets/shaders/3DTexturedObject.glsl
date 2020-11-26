#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in float a_TextureIndex;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_LightSpace;

out vec3 v_FragPosition;
out vec2 v_UV;
out vec3 v_Normal;
out float v_TextureIndex;
out vec4 v_FragPosLightSpace;

void main()
{
	v_FragPosLightSpace = u_LightSpace * u_Transform * vec4(a_Position, 1.0);
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
    v_FragPosition = vec3(u_Transform * vec4(a_Position, 1.0));
	v_UV = a_UV;
	v_Normal = a_Normal;
	v_TextureIndex = a_TextureIndex;

}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_color;

uniform vec3 u_LightPos;
uniform vec3 u_ViewPos;
uniform vec4 u_TintColor;
uniform sampler2D u_Textures[16];
uniform sampler2D u_ShadowMap;

in vec3 v_FragPosition;
in vec2 v_UV;
in vec3 v_Normal;
in float v_TextureIndex;
in vec4 v_FragPosLightSpace;

float shadowCalc(float dotLightNormal)
{
	// transform from [-1, 1] range to [0, 1] range
	vec3 pos = v_FragPosLightSpace.xyz * 0.5 + 0.5;
	if(pos.z > 1.0)
	{
		pos.z = 1.0;
	}
	float depth = texture(u_Textures[15], pos.xy).r;
	float bias = max(0.05 * (1.0 - dotLightNormal), 0.005);
	return (depth + bias) < pos.z ? 0.0 : 1.0;
}

void main()
{
	/*vec3 norm = normalize(v_Normal); // maybe not needed
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
*/
	vec3 color = texture(u_Textures[int(v_TextureIndex+0.1)], v_UV).rgb;
	vec3 normal = normalize(v_Normal);
	vec3 lightColor = vec3(1.0);
	// ambient
	vec3 ambient = 0.3 * color;
	// diffuse
	vec3 lightDir = normalize(u_LightPos);
	float dotLightNormal = dot(lightDir, normal);
	float diff = max(dotLightNormal, 0.0);
	vec3 diffuse = diff * lightColor;
	//specular
	vec3 viewDir = normalize(u_ViewPos - v_FragPosition);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 64);
	vec3 specular = spec * lightColor;

	// calculate shadow
	float shadow = shadowCalc(dotLightNormal);
	//vec3 lighting = (shadow * (diffuse + specular) + ambient) * color;
	vec3 lighting = shadow * color + ambient;

	o_color = vec4(lighting, 1.0);
	//o_color = vec4(vec3(shadow), 1.0);
	o_color = u_TintColor * vec4(((diffuse + specular) + ambient) * color, 1.0);
	o_color = u_TintColor * vec4(color,1.0);
}

