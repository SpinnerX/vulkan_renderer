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
//   // out_Color = vec4( 0.0, 0.4, 1.0, 1.0 ); 
//   out_Color = texture(texSampler, uv);

// }
// layout(location = 0) in vec2 uv;

// layout(location = 0) out vec4 out_Color;

// layout(binding = 2) uniform sampler2D texSampler;

layout (binding = 1) uniform sampler2D texSampler;
layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

void main() 
{
    // vec4 color = vec4(0.0, 0.0, 1.0, 1.0);
    // vec4 color = vec4(0.5, 0.5, 0.5, 1.0);
    // out_Color = texture(texSampler, uv) * color;
    outColor = texture(texSampler, fragTexCoords);
}