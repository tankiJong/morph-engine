# morph Engine 
Morph Engine is a morden C++ Game Engine in development.

# Recent update
## Renderer
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
