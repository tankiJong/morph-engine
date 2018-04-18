# Morph Engine 
Morph Engine is a morden C++ Game Engine in development.

# Recent update
## Renderer
* Engine support lighting now, up to 8. Also see [lighting.glsl](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/inc/light.glsl) & [default_lit.shader](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/default_lit.shader)(which uses [default.vert](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/progs/default.vert) and [default.frag](https://github.com/tankiJong/morph-engine/blob/master/Content/shader/progs/default.frag)).
* Mikkt integration @ Mesher
* Mesher support `SurfacePatch`
* support different vertex types, related concept: **VertexAttribute**, **VertexLayout**
* Mesher(aka MeshBuilder), support loading mesh from simple obj file
* introduce several rendering concepts: **Mesh**, **Shader**
* support multiple texture binding
* support simple custom Uniform buffer binding
* Migrating to modern rendering pipline

## Math
* Transform, Eular angle, matrix related utils
* support perspective camera
* Math library refactoring

## Other
* render text use metadata and distance field
* Json, Yaml parser integrated
* Virtual file system
* Simple clock&time util for profiler
* Dev Console
* Xml tool (feature keep growing)
