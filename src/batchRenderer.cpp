#include "batchRenderer.hpp"

#include "shader/default.fs.h"
#include "shader/default.vs.h"

using namespace Engine::Renderer;

static bgfx::ShaderHandle vs;
static bgfx::ShaderHandle fs;
static bgfx::ProgramHandle shaderProgram;

void BatchRenderer::add(bgfx::ViewId viewId, Surface::Mesh model, uint64_t state) { 
    BatchData * batchData = getBatchData(1, 1); // We can hard code the the shader ID & the texture ID for now since we're not using it yet
    
    // Check if we have enough room in the buffer. If not, flush the buffer to the GPU before adding stuff
    if (model.vertices.size() + batchData->transientVertexBuffer.elementCount >= 0xFFFF
        || model.vertexIndices.size() + batchData->transientIndexBuffer.elementCount >= 0xFFFF) {
        flush();
    }
    
    // Make sure the batch renderer has been initialized
    init();
    
    // Create new batch command
    BatchCommand newCommand;
    
    newCommand.vertexStartIndex = batchData->transientVertexBuffer.elementCount;
    newCommand.vertexCount = model.vertices.size();
    
    newCommand.indexStartIndex = batchData->transientIndexBuffer.elementCount;
    newCommand.indexCount = model.vertexIndices.size();
    
    newCommand.viewId = viewId;
    newCommand.state = state;
    
    newCommand.shaderProgram = shaderProgram;
    
    // Copy vertex + indices to transient buffer
    for (size_t i = 0; i < newCommand.vertexCount; ++i) {
        ++batchData->transientVertexBuffer.elementCount;
        std::memcpy(&batchData->transientVertexBuffer.content[newCommand.vertexStartIndex + i], &model.vertices[i], sizeof(Surface::Vertex));
    }
    
    int vertexIndexOffset = 0;
    if (batchData->batchCommands.size() >= 1)
        vertexIndexOffset = canBatch(&batchData->batchCommands.front(), &newCommand) ? newCommand.vertexStartIndex : 0;
    
    for (size_t i = 0; i < newCommand.indexCount; ++i) {
        batchData->transientIndexBuffer.content[batchData->transientIndexBuffer.elementCount + i] = model.vertexIndices[i] + vertexIndexOffset;
    }
    
    // Push batch command
    batchData->batchCommands.push(newCommand);
}

static bgfx::RendererType::Enum rendererType = bgfx::RendererType::Noop;
static uint64_t vertexWindingDirection = 0;
void BatchRenderer::flush() { 
    bool isBatch = false;
    uint16_t vertexStart  = 0;
    uint16_t vertexCount  = 0;
    
    uint16_t indiciesStart = 0;
    uint16_t indiciesCount = 0;
    
    // Set the vertex winding to CCW if we must
    if (rendererType == bgfx::RendererType::Noop) {
        rendererType = bgfx::getRendererType();
        
        switch (rendererType) {
            case bgfx::RendererType::Metal:
                vertexWindingDirection = BGFX_STATE_FRONT_CCW;
                break;
                
            default:
                break;
        }
    }
    
    for (auto& batch : batchGroup) {
        if (batch.second.transientVertexBuffer.elementCount == 0)
            continue;
        
        while (!batch.second.batchCommands.empty()) {
            BatchCommand cmd = batch.second.batchCommands.front();
            BatchCommand nextCmd;
            bool canBatchCmd, hasNextCmd = false;
            
            batch.second.batchCommands.pop();
            
            if (batch.second.batchCommands.size() >= 1) {
                nextCmd = batch.second.batchCommands.front();
                hasNextCmd = true;
            }
            
            canBatchCmd = hasNextCmd && canBatch(&cmd, &nextCmd);
            if (!isBatch) {
                isBatch = true;
                vertexStart = cmd.vertexStartIndex;
                indiciesStart = cmd.indexStartIndex;
                
                indiciesCount = vertexCount = 0;
            }
            
            vertexCount   += cmd.vertexCount;
            indiciesCount += cmd.indexCount;
            
            // We can no longer batch the command. Send to GPU
            if (!canBatchCmd) {
                bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(batch.second.transientVertexBuffer.content.data() + vertexStart, sizeof(Surface::Vertex) * vertexCount), *vertexLayout);
                bgfx::IndexBufferHandle  ibh = bgfx::createIndexBuffer( bgfx::makeRef(batch.second.transientIndexBuffer.content.data() + indiciesStart,  sizeof(uint16_t) * indiciesCount ));
                
                bgfx::UniformHandle cameraHandle = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4);

                bgfx::setVertexBuffer(0, vbh);
                bgfx::setIndexBuffer(ibh);
                bgfx::setState(0
                               | BGFX_STATE_DEFAULT
                               | vertexWindingDirection);
            
                bgfx::submit(cmd.viewId, cmd.shaderProgram);
                
                bgfx::destroy(vbh);
                bgfx::destroy(ibh);
                bgfx::destroy(cameraHandle);

                isBatch = false;
            }
        }
    }
    
    // Done!
    resetBuffers();
    hasInit = false;
}

bool BatchRenderer::canBatch(Engine::Renderer::BatchRenderer::BatchCommand *a, Engine::Renderer::BatchRenderer::BatchCommand *b) { 
    return (a->state == b->state);
}

void BatchRenderer::init() { 
    if (hasInit)
        return;
    
    if (shaderProgram.idx == 0) {
        vs = bgfx::createShader( bgfx::makeRef(default_vs, sizeof(default_vs)) );
        fs = bgfx::createShader( bgfx::makeRef(default_fs, sizeof(default_fs)) );
        shaderProgram = bgfx::createProgram(vs, fs, true);
    }
    
    hasInit = true;
}
