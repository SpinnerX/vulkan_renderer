#version 460

layout(binding = 0) uniform UniformBufferObject {
    // mat4 model;
    // mat4 view;
    // mat4 proj;
    mat4 MVP;
} ubo;

/*


Vertex Attribute Specifications through our defined types


// We also need to supply VkVertexInputBindingDescription, which may work as the following:
for VkVertexInputBindingDescription information 

std::vector<vertex_binding_description> binding_description; // example of the type and uses below

{ "Vertex", .Binding = 0, .sttride = sizeof(vertex), .InputRate = VK_VERTEX_INPUT_RATE_VERTEX}

// These are gonna be used for VkVertexInputAttributeDescription

std::vector<pipeline_vertex_attributes> vertex_attributes; // just writing this to give an example on it's usage below

{ "inPosition", location = 0, binding = 0, .offset = offsetof(vk::vertex, Position)}
{ "inColor", location = 1, binding = 0, .offset = offsetof(vk::vertex, Color) }
{ "inTexCoords", location = 1, binding = 0, .offset = offsetof(vk::vertex, inTexCoords) }



[ descriptor set layout defined as the following ]

VkDescriptorSetLayout - set layout to define descriptor sets and ddata they may be updated with

// With descriptor set layouts, this is to define the layouts of incoming uniforms and the data associated with it

{"ubo", 0, vk::descriptor_type::STORAGE_BUFFER, vk::shader_stage::VERTEX}



*/

// vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.MVP * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}