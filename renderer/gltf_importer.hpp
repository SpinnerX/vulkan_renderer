#pragma once
#include <string>
#include <renderer/mesh.hpp>

/**
 * @note This is going to represent our importer for gltf
 * @note This should a mesh or some asset handler as this may be done async to other running threads
 * @note As of currently our rendering object is represented as a vk::mesh although this would be handled a bit differently
 * @note NOTE: Preferred to name this as atlas::asset_handle
*/
class gltf_importer {
public:
    gltf_importer(const std::string& p_filename);

    vk::mesh get_asset_handler() const { return m_mesh_handler; }

private:
    vk::mesh m_mesh_handler;
};