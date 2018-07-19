# Morph Engine 
Morph Engine is a morden C++ Game Engine in development.

# Recent update
## Renderer
* all font rendering are now based on [distance field font](https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf) textures.
* support shader/material reloading throw Resource<>::reload()
* Post effect: support bloom, fog effect.(After effect framework is still not finalized yet)
* lights have an option to cast shadow(hard coded PCF shadow).(shadow map is in efficient for now because the engine do not have texture view concept yet)
* Fully support forward rendering path
* Material support property blocks(shader reflaction)
* `Material` support.([data format](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/mat.format))
* `Mesh` support mutiple sub mesh now
* Multiple passes shader support
* Introducing `RenderScene`, `RenderTask`, `Renderable`, and a built-in Forward rendering path.
* Engine support lighting now, up to 8. Also see [lighting.glsl](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/inc/light.glsl) & [default_lit.shader](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/default/default_lit.shader)(which uses [default.vert](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/progs/default.vert) and [default.frag](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/progs/default.frag))
* Mikkt integration @ Mesher
* Mesher support `SurfacePatch`
* support different vertex types, related concept: **VertexAttribute**, **VertexLayout**
* Mesher(aka MeshBuilder), support loading mesh from simple obj file
* introduce several rendering concepts: **Mesh**, **Shader**
* support multiple texture binding
* support simple custom Uniform buffer binding
* glsl support `#include`, `#define`
* Migrating to modern rendering pipline

## Math
* Transform hierarchy(no cache support, would be slow if the hierarchy is deep)
* Transform, Eular angle, matrix related utils
* support perspective camera
* Math library refactoring

## debug/profile
* Threaded Logger
* Instrimental profiler and visual layout
* Debug draw system(2D/3D text, some 2D/3D primitives)
* Dev Console

## Other
* better developing experience: the engine resource folder would get automatically registered so that re-run the project can see all changes immediately.
* Engine [built-in resource folder](https://github.com/tankiJong/morph-engine/tree/master/Content)
* file system can traverse the whole mounted file tree and call the call back function provided.
* render text use metadata and distance field
* Json, Yaml parser integrated
* Virtual file system
* Simple clock&time util for profiler
* Xml tool (feature keep growing)
