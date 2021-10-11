#pragma once

#include <bx/math.h>
#include "surfaceComponent.hpp"
#include "batchRenderer.hpp"

namespace Engine {
    namespace Renderer {
        class BxBgfxRenderer {
        public:
            void setViewMtx(const bx::Vec3& eye, const bx::Vec3& at, const bx::Vec3& up = { 0, 1, 0 });
            
            void setProjMtx(float fov, float aspect, float near, float far);
            
            void setViewport(float x, float y, float w, float h);
            
            void prepare ();
            
            void add(Surface::Mesh model);
            
            void present();
            
        private:
            bool hasViewMtx, hasProjMtx, hasViewport = false;
            bool hasInit = false;
            
            float viewMtx[16];
            float projMtx[16];
            
            /// Use this to cache the result of viewMtx * projMtx
            float vpMtx[16]; // v * p
            float vpMtx_inv[16]; // inverse of vp
            
            struct {
                float x, y, w, h;
            } viewport;
            
            uint8_t viewId;
            
            BatchRenderer batchRenderCtx;
            bgfx::VertexLayout vertexLayout;
            
        private:
            void init();
        };
    }
}
