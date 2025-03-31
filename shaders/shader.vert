// Triangle shader from tutorial 15 
// layout(location = 0) out vec3 fragColor;

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

// void main() {
//     gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
//     fragColor = colors[gl_VertexIndex];
// }

#version 460

// struct VertexData
// {
// 	float x, y, z;
// 	float u, v;
// };

// layout (binding = 0) readonly buffer Vertices { VertexData data[]; } in_Vertices;

// layout (binding = 1) readonly uniform UniformBuffer { mat4 mvp; } ubo;

// void main()
// {
//     VertexData vd = in_Vertices.data[gl_VertexIndex];

//     vec3 pos = vec3(vd.x, vd.y, vd.z);
//     gl_Position = ubo.mvp * vec4(pos, 1.0);
// }

// tutorial 17 shader code
// struct VertexData
// {
// 	float x, y, z;
// 	float u, v;
// };

// layout (binding = 0) readonly buffer Vertices { VertexData data[]; } in_Vertices;

// layout (binding = 1) readonly uniform UniformBuffer { mat4 WVP; } ubo;


// void main() 
// {
// 	VertexData vtx = in_Vertices.data[gl_VertexIndex];

// 	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

// 	gl_Position = ubo.WVP * vec4(pos, 1.0);
// }

// tutorial 18 code
// struct VertexData
// {
// 	float x, y, z;
// 	float u, v;
// };

// layout (binding = 0) readonly buffer Vertices { VertexData data[]; } in_Vertices;

// layout (binding = 1) readonly uniform UniformBuffer { mat4 WVP; } ubo;

// layout (location = 0) out vec2 texCoord;

// void main() 
// {
// 	VertexData vtx = in_Vertices.data[gl_VertexIndex];

// 	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

// 	gl_Position = ubo.WVP * vec4(pos, 1.0);
// 	texCoord = vec2(vtx.x, vtx.y);
// }

struct VertexData
{
	float x, y, z;
	float u, v;
};

layout (binding = 0) readonly buffer Vertices { VertexData data[]; } in_Vertices;

layout (binding = 1) readonly uniform UniformBuffer { mat4 WVP; } ubo;

layout(location = 0) out vec2 texCoord;

void main() {
	VertexData vtx = in_Vertices.data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = ubo.WVP * vec4(pos, 1.0);

	texCoord = vec2(vtx.u, vtx.v);
}