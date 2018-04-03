#include "Vertex.hpp"

DefineVertexType(vertex_pcu_t) {
  define("POSITION", MP_FLOAT, 3, 0, true);
  define("COLOR",    MP_UBYTE, 3, 12, false);
  define("UV",       MP_FLOAT, 2, 16, true);
}

template<>
vertex_pcu_t Vertex::as() {
  return {
    position,
    color,
    uv
  };
}
