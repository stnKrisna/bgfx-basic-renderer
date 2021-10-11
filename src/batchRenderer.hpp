#pragma once

#include "surfaceComponent.hpp"
#include <bgfx/bgfx.h>
#include <queue>
#include <cstdint>
#include <map>
#include <array>
#include <vector>

template <class _T, size_t _Size>
struct Buffer_t {
    std::array<_T, _Size> content = {};
    size_t elementCount = 0;
};

namespace Engine {
    namespace Renderer {
        class BatchRenderer {
        public:
            /// Add model to the buffer
            void add (bgfx::ViewId viewId, Surface::Mesh model, uint64_t state);
            
            /// Flush buffer to GPU
            void flush ();
            
            inline void prepare (bgfx::VertexLayout * _vertexLayout) {
                if (vertexLayout != nullptr)
                    return;

                vertexLayout = _vertexLayout;
                        
                resetBuffers();
            }
            
        private:
            typedef struct BatchCommand {
                uint16_t vertexStartIndex;
                uint16_t vertexCount;
                uint16_t indexStartIndex;
                uint16_t indexCount;
                bgfx::ViewId viewId;
                uint64_t state;
                bgfx::ProgramHandle shaderProgram;
            } BatchCommand;
            
            typedef struct BatchData {
                Buffer_t<Surface::Vertex, 0xFFFF> transientVertexBuffer;
                Buffer_t<uint16_t, 0xFFFF>        transientIndexBuffer;
                std::queue<BatchCommand>          batchCommands;
            } BatchData;
            
            bgfx::VertexLayout * vertexLayout = nullptr;
            
            std::map<unsigned int, BatchData> batchGroup;
            
            bool hasInit = false;
            
        private:
            bool canBatch(BatchCommand * a, BatchCommand * b);

            void init ();

            /// Fetch existing batch data from the batch group or create a new group and return the batch data
            inline BatchData * getBatchData (uint16_t shaderIdx, uint16_t textureIdx) {
                unsigned int batchId = shaderIdx * textureIdx;
                const auto& batchMapEntry = batchGroup.find(batchId);
                        
                if (batchMapEntry == batchGroup.end()) {
                    BatchData newBatchData;
                            
                    if(batchGroup.insert_or_assign(batchId, std::move(newBatchData)).second == false)
                        throw std::runtime_error("Failed to insert batch data");
                            
                    return &batchGroup.find(batchId)->second;
                }
                        
                return &batchMapEntry->second;
            }

            /// Reset all buffer
            inline void resetBuffers () {
                for (auto& batch : batchGroup) {
                    batch.second.transientVertexBuffer.elementCount = 0;
                    batch.second.transientIndexBuffer.elementCount = 0;
                }
            }
        };
    }
}
