#include "Vertex.hpp"

DefineVertexType(vertex_pcu_t) {
  attribute("POSITION", MP_FLOAT, 3, 0, true);
  attribute("COLOR",    MP_FLOAT, 3, 3, false);
  attribute("UV",       MP_FLOAT, 2, 5, true);
}