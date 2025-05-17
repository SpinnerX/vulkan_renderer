#version 460

// layout(location = 0) in vec3 fragColor;

// layout(location = 0) out vec4 outColor;

// void main() {
//     outColor = vec4(fragColor, 1.0);
// }

// tutorial 17 code
// layout(location = 0) out vec4 out_Color;

// void main() {
//   out_Color = vec4( 0.0, 0.4, 1.0, 1.0 ); 
// }

//tutorial 18 code

// layout(location = 0) in vec2 uv;

// layout(location = 0) out vec4 out_Color;

// layout(binding = 2) uniform sampler2D texSampler;

// void main() {
//   // out_Color = vec4( 1.0, 1.0, 1.0, 1.0 ); 
//   out_Color = texture(texSampler, uv);

// }
// layout(location = 0) in vec2 uv;

// layout(location = 0) out vec4 out_Color;

// layout(binding = 2) uniform sampler2D texSampler;

layout (binding = 1) uniform sampler2D texSampler;
layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec3 fragNormals;
layout (location = 2) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform UniformBuffer {
	mat4 MVP;
	float delta_time;
    vec2 mouse_pos;
} ubo;

//aaaaa
void main()
{
    vec3 color = vec3(1.0, 1.0, 1.0);

    vec4 tex = texture(texSampler, fragTexCoords);
    float fragVal = length(tex);
    vec3 normal = normalize(fragNormals);
    vec3 sunDirection = vec3(cos(ubo.delta_time), sin(ubo.delta_time), 0);
    vec3 pcol = (0.25 + 0.9 * color * dot(normalize(sunDirection), normal));
    outColor = tex * vec4(pcol, 1.0);
}

