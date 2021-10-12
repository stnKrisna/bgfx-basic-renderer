<p align="center">
  <img src="https://github.com/stnKrisna/bgfx-basic-renderer/blob/master/screenshot/Screenshot%202021-10-12%20at%203.40.53%20PM.png" width="456" height="367">
</p>

# Basic BGFX renderer
A bare bone BGFX renderer with SDL2

## Capabilities
- Render cube with vertex color
- Super basic vertex & fragment shader
- Super basic batch rendering
- Easy to use API

## To-do
- Material support
- Lighting support

## How to use
```C++
// Create a new renderer instance
Engine::Renderer::BxBgfxRenderer renderer;

// Do some other stuff here

// Set view matrix & projection matrix
renderer.setViewMtx(...);
renderer.setProjMtx(...);
renderer.setViewport(...);

// Prepare renderer
renderer.prepare();

// Add you stuff
renderer.add(...);

// Done!
renderer.present();
```

## Example usage
See full source code here: https://github.com/stnKrisna/bgfx-basic-renderer/blob/master/src/main.cpp
```C++
// 0. Create object
Engine::Surface::Mesh myObject;

myObject.vertices = {
    {-1.0f,  1.0f,  1.0f, 0xFFFFFFFF },
    ...
    { 1.0f, -1.0f, -1.0f, 0xFFFFFFFF },
};

myObject.vertexIndices = {
    0, 2, 1,
    ...
    6, 7, 3,
};

// 1. Create camera
bx::Vec3 cameraPos(5, 5, -2.5);
bx::Quaternion cameraRot(-0.4254518, 0.4254518, -0.237339, 0.762661);

bx::Vec3 up(0, 1, 0);
bx::Vec3 forward(0, 0, 1);

// 2. Create a new renderer instance
Engine::Renderer::BxBgfxRenderer renderer;

// 3. Set view matrix
bx::Vec3 at = bx::add(cameraPos, bx::mul(forward, cameraRot));
renderer.setViewMtx(cameraPos, at, bx::mul(up, cameraRot));

// 4. Set projection matrix
renderer.setProjMtx(90, 1, 0.01f, 1000.0f);
renderer.setViewport(0, 0, 800, 600);

// 5. Prepare
renderer.prepare();

// 6. Add you stuff
renderer.add(myObject);

// 7. ...
renderer.present();

// 8. Profit!
bgfx::frame();
```
