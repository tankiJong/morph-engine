#include "ShaderPass.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Persistence/yaml.hpp"
/* implemented version:
x name: shader/example
program:
  path:
  define: defines;seperated;by;semicolons
cull: back|front|none
fill: solid|wire
frontface: ccw|cw
blend: false
blend:
  color:
    op: add|sub|rev_sub|min|max
    src: one|zero|src_color|inv_src_color|src_alpha|inv_src_alpha|dest_color|inv_dest_color|dest_alpha|inv_dest_alpha|constant|inv_constant
    dest: zero
  depth:
    write: true|false
    test: less|never|equal|lequal|greater|gequal|not|always
 */
#define VAL_MAP(str, val) if(v == str) { rhs = val; return true; }

namespace YAML {
  template<>
  struct convert<eCullMode> {
    static bool decode(const Node& node, eCullMode& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("back", CULL_BACK);
      VAL_MAP("front", CULL_FRONT);
      VAL_MAP("none", CULL_NONE);

      return false;
    }
  };

  template<>
  struct convert<eFillMode> {
    static bool decode(const Node& node, eFillMode& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("solid", FILL_SOLID);
      VAL_MAP("wire", FILL_WIRE);

      return false;
    }
  };

  template<>
  struct convert<eWindOrder> {
    static bool decode(const Node& node, eWindOrder& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("ccw", WIND_COUNTER_CLOCKWISE);
      VAL_MAP("cw", WIND_CLOCKWISE);

      return false;
    }
  };

  template<>
  struct convert<eBlendOp> {
    static bool decode(const Node& node, eBlendOp& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("add", BLEND_OP_ADD);
      VAL_MAP("sub", BLEND_OP_SUB);
      VAL_MAP("rev_sub", BLEND_OP_REV_SUB);
      VAL_MAP("min", BLEND_OP_MIN);
      VAL_MAP("max", BLEND_OP_MAX);

      return false;
    }
  };

  template<>
  struct convert<eFlag> {
    static bool decode(const Node& node, eFlag& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("true", FLAG_TRUE);
      VAL_MAP("false", FLAG_FALSE);

      return false;
    }
  };

  template<>
  struct convert<eBlendFactor> {
    static bool decode(const Node& node, eBlendFactor& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("one", BLEND_F_ONE);
      VAL_MAP("zero", BLEND_F_ZERO);
      VAL_MAP("src_alpha", BLEND_F_SRC_ALPHA);
      VAL_MAP("dest_alpha", BLEND_F_DST_ALPHA);
      VAL_MAP("inv_src_alpha", BLEND_F_INV_SRC_ALPHA);
      VAL_MAP("inv_dest_alpha", BLEND_F_INV_DST_ALPHA);
      return false;
    }
  };

  template<>
  struct convert<eCompare> {
    static bool decode(const Node& node, eCompare& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("never", COMPARE_NEVER);
      VAL_MAP("less", COMPARE_LESS);
      VAL_MAP("lequal", COMPARE_LEQUAL);
      VAL_MAP("greater", COMPARE_GREATER);
      VAL_MAP("gequal", COMPARE_GEQUAL);
      VAL_MAP("equal", COMPARE_EQUAL);
      VAL_MAP("not", COMPARE_NOT_EQUAL);
      VAL_MAP("always", COMPARE_ALWAYS);

      return false;
    }
  };

  bool convert<ShaderPass>::decode(const Node& node, ShaderPass& rhs) {
    ShaderPass* shaderPass = &rhs;

    ShaderProgram* prog = new ShaderProgram();

    if(node["program"]["path"]) {
      prog->fromFile(
                     node["program"]["path"].as<std::string>().c_str(), node["program"]["define"] ? node["program"]["define"].as<std::string>().c_str():nullptr);
    } else {
      std::string vert = node["program"]["vert"].as<std::string>();
      std::string frag = node["program"]["frag"].as<std::string>();

      prog->setStage(SHADER_TYPE_VERTEX, vert.c_str(), node["program"]["define"] ? node["program"]["define"].as<std::string>().c_str():nullptr);
      prog->setStage(SHADER_TYPE_FRAGMENT, frag.c_str(), node["program"]["define"] ? node["program"]["define"].as<std::string>().c_str():nullptr);
      prog->commit();
    }
    prog->genInfo();
    shaderPass->prog() = prog;
    shaderPass->state() = node.as<render_state>();


    if (node["layer"]) {
      std::string layer = node["layer"].as<std::string>();
      if (layer == "opaque") {
        shaderPass->layer = SHADER_LAYER_OPAQUE;
      } else if (layer == "alpha") {
        shaderPass->layer = SHADER_LAYER_ALPHA;
      } else {
        shaderPass->layer = parse<uint>(layer);
      }
    }

    if (node["sort"]) {
      shaderPass->sort = node["sort"].as<uint>();
    }
    return true;
  }

  bool convert<render_state>::decode(const Node& node, render_state& rs) {
    if (node["cull"]) rs.cullMode = node["cull"].as<eCullMode>();
    if (node["fill"]) rs.fillMode = node["fill"].as<eFillMode>();
    if (node["frontface"]) rs.frontFace = node["frontface"].as<eWindOrder>();

    if (node["blend"])
      if (node["blend"].IsMap()) {
        if (node["blend"]["color"]) {
          auto color = node["blend"]["color"];
          if (color["op"]) rs.colorBlendOp = color["op"].as<eBlendOp>();
          if (color["src"]) rs.colorSrcFactor = color["src"].as<eBlendFactor>();
          if (color["dest"]) rs.colorDstFactor = color["dest"].as<eBlendFactor>();
        }
        if (node["blend"]["alpha"]) {
          auto alpha = node["blend"]["alpha"];
          if (alpha["op"]) rs.alphaBlendOp = alpha["op"].as<eBlendOp>();
          if (alpha["src"]) rs.alphaSrcFactor = alpha["src"].as<eBlendFactor>();
          if (alpha["dest"]) rs.alphaDstFactor = alpha["dest"].as<eBlendFactor>();
        }
      }

    if (node["depth"]) {
      auto depth = node["depth"];
      if (depth["write"]) rs.isWriteDepth = depth["write"].as<eFlag>();
      if (depth["test"]) rs.depthMode = depth["test"].as<eCompare>();
    }
    return true;
  }
}

ShaderPass::~ShaderPass() {
  SAFE_DELETE(mProg);
}

ShaderPass::ShaderPass(ShaderPass&& from) {
  mProg = from.mProg;
  from.mProg = nullptr;
  mState = from.mState;
  layer = from.layer;
  sort = from.sort;
}

ShaderPass& ShaderPass::operator=(ShaderPass&& from) {
  mProg = from.mProg;
  from.mProg = nullptr;
  mState = from.mState;
  layer = from.layer;
  sort = from.sort;

  return *this;
}
