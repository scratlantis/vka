# vka
High-level Vulkan framework, designed with the intent to allow for quick prototyping of interactive research applications.
Work in progress...

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

Current applications:
    - minimal path tracer with participating medium
    - minimal path tracer with participating medium and gui

Screenshots:
minimal path tracer with participating medium and gui:
![Alt text](/media/screenshot_pt_with_gui_1.png?raw=true "path_tracer_with_gui")