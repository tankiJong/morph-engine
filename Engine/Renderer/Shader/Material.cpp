#include "Material.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Persistence/yaml.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"
//void bindFloat(uint loc, uint num, void* data) {
//  glUniform1fv(loc, num, (float*)data);
//}
//
//void MatProp<float>::bind(uint bindPoint) {
//  bindFloat(bindPoint, 1, &value);
//}
//
//void MatProp<vec2>::bind(uint bindPoint) {
//  bindFloat(bindPoint, 2, &value);
//}
//
//void MatProp<vec3>::bind(uint bindPoint) {
//  bindFloat(bindPoint, 3, &value);
//}
//
//void MatProp<Rgba>::bind(uint bindPoint) {
//  bindFloat(bindPoint, 4, value.normalized().data);
//}
//
//void MatProp<mat44>::bind(uint bindPoint) {
//  bindFloat(bindPoint, 16, value.data);
//}

MaterialProperty* MatProp<Texture>::clone() const {
  MatProp<Texture>* prop = new MatProp<Texture>(slot, value, name.c_str());
  prop->sampler = sampler;
  return prop;
}

void MaterialProperty::bind(uint bindPoint) {
  value.putGpu();
  glBindBufferBase(GL_UNIFORM_BUFFER, bindPoint, value.handle());
}

Material::~Material() {
  for(MaterialProperty*& prop: mProps) {
    SAFE_DELETE(prop);
  } 

  SAFE_DELETE(mShader);

}

void Material::shader(S<const Shader> shader) {
  mResShader = shader;
  delete mShader;
}

const Shader* Material::shader() const {
  return mShader == nullptr ? mResShader.get() : mShader;
}

Shader* Material::shader() {
  if (mShader == nullptr && mResShader != nullptr) {
    mShader = Resource<Shader>::clone(mResShader);
  }

  return mShader;
}

void Material::setTexture(uint slot, const Texture* tex, const Sampler* sampler, const char* nameid) {
  MatProp<Texture>*& prop = mTextures[slot];

  if(prop) {
    delete prop;
  }

  auto texture = new MatProp<Texture>(slot, tex, nameid);

  if(sampler) {
    texture->sampler = sampler;
  } else {
    texture->sampler = &Sampler::Default();
  }

  prop = texture;
}

void Material::setPropertyBlock(S<const PropertyBlockInfo> layout, void* data) {
  MaterialProperty*& prop = property(layout->name);

  if(prop) {
    delete prop;
  }

  prop = new MaterialProperty(layout->name.data(), layout, data);
}

MaterialProperty*& Material::property(std::string_view propName) {
  for (MaterialProperty*& prop : mProps) {
    if (prop->name == propName) return prop;
  }

  mProps.push_back(nullptr);

  return mProps.back();
}

const MaterialProperty* Material::property(std::string_view propName) const {
  for (const MaterialProperty* prop : mProps) {
    if (prop->name == propName) return prop;
  }

  return nullptr;
}

Material::Material(const Material& mat) {
  mResShader = mat.mResShader;
  mShader = mat.mShader;

  mProps.reserve(mat.mProps.size());
  for(const MaterialProperty* prop: mat.mProps) {
    mProps.push_back(prop == nullptr ? nullptr : prop->clone());
  }

  for(uint i = 0; i<NUM_TEXTURE_SLOT; i++) {
    mTextures[i] = mat.mTextures[i] == nullptr ? nullptr : static_cast<MatProp<Texture>*>(mat.mTextures[i]->clone());
  }
}

InstaMaterial::InstaMaterial(const Material& mat)
  : Material(mat) {}

owner<Material*> Material::fromYaml(const fs::path& file) {
  auto vfs = FileSystem::Get();

  std::optional<Blob> f = vfs.asBuffer(file);

  if (!f) {
    return nullptr;
  }

  yaml::Node node = yaml::Load(f->as<const char*>());

  Material* mat = new Material();

  if (node["name"]) mat->name = node["name"].as<std::string>();

  auto props = node["properties"];
  if(props) {
    EXPECTS(props.IsMap());

    for(auto prop: props) {
      if(!prop.first.IsSequence()) {
        S<PropertyBlockInfo> propInfo;

        propInfo.reset(new PropertyBlockInfo());

        propInfo->name = prop.first.as<std::string>();

        if(prop.second.IsMap()) {           // property block
          uint offset = 0u;
          Blob data(1024u);
          for(auto attr: prop.second) {
            EXPECTS(attr.first.IsSequence()); // bug in cpp yaml
            std::string name = attr.first[0].as<std::string>();
            std::string type = attr.first[1].as<std::string>();
            property_info_t& info = (*propInfo)[name];
            info.name = name;
            info.size = typeSizeOf(type);

            // I should check for behaviors of [10.f], 10.f
            info.count = attr.second.IsSequence() ? attr.second.size() : 1;
        
            info.offset = offset;
            offset += info.size * info.count;

            if(type == "float") {
              if(attr.second.IsSequence()) {
                std::vector<float> val = attr.second.as<std::vector<float>>();
                data.set(val.data(), info.count * sizeof(float), info.offset);
              } else {
                // float val = attr,second.as<float>(); // does not work! probably bug in cpp-yaml, sad.
                float val = parse<float>(attr.second.as<std::string>());
                EXPECTS(info.count == 1);
                data.set(&val, info.count * sizeof(float), info.offset);
              }
            } else if (type == "vec4") {
              vec4 val = parse<vec4>(attr.second.as<std::string>());
              EXPECTS(info.count == 1);
              data.set(&val, info.count * sizeof(float), info.offset);
            } else {
              ERROR_AND_DIE("unsupported property type for material");
            }

          }

          propInfo->totalSize = offset;
          mat->setPropertyBlock(propInfo, data);

        } else if(prop.second.IsSequence()) { // texture
          uint slot = prop.second[0].as<uint>();
          std::string source = prop.second[1].as<std::string>();

          const Sampler* sampler = nullptr;
          if(prop.second[2]) {
            sampler = prop.second[2].as<const Sampler*>();
          }

          S<const Texture> tex = Resource<Texture>::get(source);
          if(!tex) {
            bool defined = Resource<Texture>::define(source);
            ENSURES(defined);

            tex = Resource<Texture>::get(source);
          }

          mat->setTexture(slot, tex.get(), sampler, propInfo->name.c_str());

        } else {
          ERROR_AND_DIE("unsupported material meta data");
        }
      } else {
        ERROR_AND_DIE("unsupported material meta data");
      }


    }

  }
  
  auto shader = node["shader"];

  if(shader["name"]) {
    S<const Shader> resShader = Resource<Shader>::get(shader["name"].as<std::string>());
    if(resShader != nullptr) {
      mat->mResShader = resShader;

      uint i = 0;
      for(auto pass: shader["pass"]) {
        render_state rs = pass.as<render_state>();

        if (rs != mat->mResShader->pass(i).state()) {
          mat->shader()->pass(i)->state() = rs;
        }

        i++;
      }
    }
  } else {
    EXPECTS(mat->mShader == nullptr);
    mat->mShader = shader.as<Shader*>();
  }

  return mat;
}

template<>
ResDef<Material> Resource<Material>::load(const std::string& file) {
  EXPECTS(fs::path(file).extension() == ".mat");

  Material* mat = Material::fromYaml(file);

  return { mat->name, mat };
}

#define VAL_MAP(str, val) if(v == str) { rhs = val; return true; }

namespace YAML {
  template<>
  struct convert<const Sampler*> {
    static bool decode(const Node& node, const Sampler*& rhs) {
      std::string v = node.as<std::string>();

      VAL_MAP("LINEAR", &Sampler::Linear());
      VAL_MAP("POINT", &Sampler::Point());

      return false;
    }
  };
}