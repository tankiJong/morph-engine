#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/File/FileSystem.hpp"
#include <fstream>
#include "Engine/Renderer/SpriteAnim.hpp"

enum eLineType {
  LINE_PASS = 0,
  LINE_POSITION,
  LINE_NORMAL,
  LINE_UV,
  LINE_FACE,
  NUM_LINE_TYPE,
};

eLineType asType(const char* c) {
  switch(*c) {
    case 'f': return LINE_FACE;
    case 'v': break;
    default: return LINE_PASS;
  }

  EXPECTS(*c == 'v');
  
  switch(*(c+1)) {
    case ' ': return LINE_POSITION;
    case 't': return LINE_UV;
    case 'n': return LINE_NORMAL;
  }

  ERROR_AND_DIE("unexpected character");
}
struct face {
  int positon = -1, uv = -1, normal = -1;
};

vec3 lineAsVec3(std::string_view line) {
  auto comps = split(line.data(), " ");

  EXPECTS(comps.size() == 4);

  return {
    parse<float>(comps[1]),
    parse<float>(comps[2]),
    parse<float>(comps[3]),
  };
}

vec2 lineAsVec2(std::string_view line) {
  auto comps = split(line.data(), " ");

  EXPECTS(comps.size() == 3);

  return {
    parse<float>(comps[1]),
    parse<float>(comps[2]),
  };
}

face chunkAsFace(std::string_view chunk) {
  auto comps = split(chunk.data(), "/");

  EXPECTS(comps.size() == 3);

  return {
    parse<int>(comps[0]),
    parse<int>(comps[1]),
    parse<int>(comps[2]),
  };
}
std::array<face, 4> lineAsFace(std::string_view line) {
  auto comps = split(line.data(), " ");

  EXPECTS(comps.size() == 5 || comps.size() == 4);

  return {
    chunkAsFace(comps[1]),
    chunkAsFace(comps[2]),
    chunkAsFace(comps[3]),
    comps.size() == 5 ? chunkAsFace(comps[4]) : face(),
  };
}

// void Mesher::obj(fs::path objFile) {
//   static std::vector<vec3> positions;
//   static std::vector<vec2> uvs;
//   static std::vector<vec3> normals;
//   static std::vector<std::array<face, 4>> faces;
//   positions.clear();
//   uvs.clear();
//   normals.clear();
//
//   positions.reserve(1000u);
//   uvs.reserve(1000u);
//   normals.reserve(1000u);
//
//   std::ifstream f = FileSystem::Get().asStream(objFile);
//
//   std::string line;
//   while (std::getline(f, line)) {
//     switch(asType(line.c_str())) {
//       case LINE_PASS: continue;
//
//       case LINE_POSITION: 
//         positions.push_back(lineAsVec3(line));
//       break;
//       case LINE_NORMAL: 
//         normals.push_back(lineAsVec3(line));
//       break;
//
//       case LINE_UV: 
//         uvs.push_back(lineAsVec3(line).xy()); 
//       break;
//
//       case LINE_FACE: 
//         faces.push_back(lineAsFace(line));
//       break;
//
//       default:
//         ERROR_AND_DIE("should not reach here");
//     }
//   }
//
//   for(std::array<face, 4>& face: faces) {
//     std::array<uint, 6> content4 = { 0,1,2,0,2,3 };
//     std::array<uint, 6> content3 = {0,1,2};
//     span<uint> indices = content4;
//     if (face[3].positon == -1) indices = content3;
//     
//     for(auto i: indices) {
//       uv(uvs[face[i].uv-1]);
//       normal(normals[face[i].normal-1] * vec3(-1.f, 1.f, 1.f));
//       vertex3f(positions[face[i].positon-1] * vec3(-1.f, 1.f, 1.f));
//     }
// //
// //    if(size == 4) {
// //      quad();
// //    } else {
// //      triangle();
// //    }
//   }
//
//   f.close();
// }
