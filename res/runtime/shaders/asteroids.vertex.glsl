// Vertex Shader version 4.30
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

out vec3 vs_position_modelspace;
out vec2 vs_texcoord_modelspace;
out vec3 vs_normal_modelspace;

out mat4 vs_modelMatrix;
out mat4 vs_viewMatrix;
out mat4 vs_projectionMatrix;
out mat3 vs_tbnMatrix;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

void share()
{
	vs_position_modelspace = position;
	vs_texcoord_modelspace = texcoord;
	vs_normal_modelspace = normal;
	
	vs_modelMatrix = m;
	vs_viewMatrix = v;
	vs_projectionMatrix = p;
	
	vec3 normal_cameraspace = normalize((v * m * vec4(vs_normal_modelspace, 0.0)).xyz);
	vec3 tangent_cameraspace = normalize((v * m * vec4(tangent, 0.0)).xyz);
	
	// Gramm-Schmidt Process
	tangent_cameraspace = normalize(tangent_cameraspace - dot(tangent_cameraspace, normal_cameraspace) * normal_cameraspace);
	
	vec3 bitangent_cameraspace = cross(tangent_cameraspace, normal_cameraspace);
	
	vs_tbnMatrix = transpose(mat3(tangent_cameraspace, bitangent_cameraspace, normal_cameraspace));
}

void main()
{
	share();
	gl_Position = (p * v * m) * vec4(position, 1.0);
}