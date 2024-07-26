# The Pirate Bay

Final project for Computer Graphics course at PoliMi.
Voxel-based pirate game using Vulkan. Sail the high seas, explore a procedurally generated island, and hoist yer flag. 

## Tech Stack

- C++
- Vulkan API
- Custom shaders (GLSL)
- External Perlin Noise library for procedural generation

## Features

- Vulkan renderer with low-level mesh operations
- Procedural voxel island generation using Perlin Noise
- Custom shaders for sea, sky, islands, and more
- 3D models for palm trees and other objects
- Dynamic flag physics
- Text rendering for UI

### Game Logic

- Movement with basic physics
- Spawning cubes (voxels) with/without gravity 
- Variable-height jumping
- Collision detection

### Advanced Rendering

- Instanced rendering via storage buffers
- Procedural dynamic skyboxky shader with day/night cycle
- Multi-texture blending based on noise
- Vertex shader height mapping

