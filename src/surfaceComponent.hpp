#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <bgfx/bgfx.h>

typedef uint32_t ABGRColor_t;

namespace Engine {
    namespace Surface {

#pragma pack(push, 1)
        typedef struct Vertex {
            float x, y, z;
            ABGRColor_t color; // ABGRColor_t is a typedef for uint32_t. I have this defined in another header file
        } Vertex;
#pragma pack(pop)
        
        inline static void getVertexLayout (bgfx::VertexLayout * vertexLayout) {
            vertexLayout->begin();
            vertexLayout->add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
            vertexLayout->add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
            vertexLayout->end();
        }
        
        typedef std::vector<Vertex>   VertexBuffer;
        typedef std::vector<uint16_t> VertexBufferIndices;
        
        typedef std::vector<Vertex> VertexBuffer;
        typedef std::vector<uint16_t> VertexBufferIndices;
        
        typedef std::shared_ptr<VertexBufferIndices> VertexBufferIndices_ptr;
        typedef std::shared_ptr<VertexBuffer> VertexBuffer_ptr;

        typedef struct Mesh {
            VertexBuffer vertices;
            VertexBufferIndices vertexIndices;
        } Mesh;

    } // namespace Surface
} // namespace Engine
