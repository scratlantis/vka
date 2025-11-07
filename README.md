# Vka Vulkan Framework

High-level Vulkan framework, designed with the intent to allow for quick prototyping of interactive research applications.

07.11.25: Readme currently outdated, will update in the next couple of days.
## Description

Current Features:
- API initialization & window creation
- Management of unique resources (Buffers, Images ...)
- Caching of reusable resources (Pipelines, Renderpasses, Shaders, ...)
- Scene abstraction
- Shader submodules
- Some tools for quick gui construction
- Abstracted draw command, compute commands
- Implementation of multiple standard compute and draw operations
- Implementation of shader submodules for path tracing
- ... and some other stuff

Working on:
- Better VR integration

Current applications:
- minimal path tracer with participating medium and gui
- particle simulation (2D/3D/VR)

## Screenshots:

- minimal path tracer with participating medium and gui:
![Alt text](/media/screenshot_pt_with_gui_1.png?raw=true "path_tracer_with_gui")

- particle simulation (2D)
![Alt text](/media/particle_sim_01.png?raw=true "particle simulation (2D)")


## Dependencies

### Contained in repo:
- glfw
- ImGui
- nlohmann_json
- stb
- tiny_obj_loader
- vma

### Other:
- Vulkan 1.3
- CMake

### C++ Standard
- C++17

## Installing

* Install Latest Vulkan Version (https://vulkan.lunarg.com/)
* clone
```
git clone --recurse-submodules https://github.com/scratlantis/vka.git
```
* Build with CMake


## Authors
Sidney Hansen (Contact: scratlantis@protonmail.com)

## License

This project is licensed under the MIT License - see the LICENSE.md file for details
