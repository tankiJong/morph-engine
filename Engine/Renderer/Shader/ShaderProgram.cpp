#include "ShaderProgram.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Core/StringUtils.hpp"

static const char* defaultVertexShader 
= R"(#version 420 core

in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;         

layout(std140, binding = 2) uniform cameraBlock {
   mat4 PROJECTION;
   mat4 VIEW;
};

out vec2 passUV; 
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.
out vec3 passPos;
out vec4 ViewMat;

void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * VIEW * local_pos; 

   passColor = COLOR; // pass it on. 
   passUV = UV; 
   passPos = clip_pos.xyz;
   ViewMat = PROJECTION * VIEW * vec4(0);
   gl_Position = clip_pos; 
})";

static const char* defaultFragmentShader = 
R"(#version 420 core
layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 passColor; // NEW, passed color
in vec2 passUV; 
in vec3 passPos;
out vec4 outColor; 
  
void main() 
{
   vec4 diffuse = texture( gTexDiffuse, passUV );
   
   // multiply is component-wise
   // so this gets (diff.x * passColor.x, ..., diff.w * passColor.w)
   outColor = diffuse * passColor;  
})";

static const char* invalidFragmentShader =
R"(#version 420 core
out vec4 outColor; 
void main( void )
{
   outColor = vec4( 1, 0, 1, 1 ); 
})";

void addDefinesToStage(ShaderStage& stage, const char* defineArgs) {
  if (defineArgs == nullptr) return;
  auto defs = split(defineArgs, ";");
  for (auto& def : defs) {
    size_t sp = def.find('=');

    if (sp != std::string::npos) {
      stage.define(std::string(def.begin(), def.begin() + sp), std::string(def.begin() + sp + 1, def.end()));
    } else {
      stage.define(def);
    }
  }
}

eTextureSlot ShaderProgramInfo::texture(std::string_view name) const {
  for(uint i = 0; i<NUM_TEXTURE_SLOT; i++) {
    if (mTextureInfo[i].name == name) return (eTextureSlot)i;
  }

  return NUM_TEXTURE_SLOT;
}

const PropertyBlockInfoBinding* ShaderProgramInfo::find(const std::string& blockName) const {
  auto binding = mBlockInfo.find(blockName);
  if (binding == mBlockInfo.end()) return nullptr;

  return &(binding->second);
}

PropertyBlockInfoBinding& ShaderProgramInfo::at(std::string_view blockName) {
  return mBlockInfo[blockName.data()];
}

bool ShaderProgram::fromFile(const char* relativePath, const char* defineArgs) {
  addDefinesToStage(mStages[SHADER_TYPE_VERTEX], defineArgs);
  addDefinesToStage(mStages[SHADER_TYPE_FRAGMENT], defineArgs);
  bool success = true;
  if(strcmp(relativePath, "@default") == 0) {
    success = success && 
      mStages[SHADER_TYPE_VERTEX].setFromString(SHADER_TYPE_VERTEX, defaultVertexShader) &&
      mStages[SHADER_TYPE_FRAGMENT].setFromString(SHADER_TYPE_FRAGMENT, defaultFragmentShader);

  } else if (strcmp(relativePath, "@invalid") == 0) {
    success = success &&
      mStages[SHADER_TYPE_VERTEX].setFromString(SHADER_TYPE_VERTEX, defaultVertexShader) &&
      mStages[SHADER_TYPE_FRAGMENT].setFromString(SHADER_TYPE_FRAGMENT, invalidFragmentShader);

  } else {
    std::string vsFile = relativePath;
    vsFile += ".vert";

    std::string fsFile = relativePath;
    fsFile += ".frag";

    success = success &&
      mStages[SHADER_TYPE_VERTEX].setFromFile(SHADER_TYPE_VERTEX, vsFile.c_str()) &&
      mStages[SHADER_TYPE_FRAGMENT].setFromFile(SHADER_TYPE_FRAGMENT, fsFile.c_str());

  }

  return commit();
}

bool ShaderProgram::setStage(eShaderType stageType, const char* stageString, const char* defineArgs) {
  addDefinesToStage(mStages[stageType], defineArgs);

  return mStages[stageType].setFromString(stageType, stageString);
}

bool ShaderProgram::reload() {

  bool success = true;
  for(ShaderStage& stage: mStages) {
    success = success & stage.reload();
  }

  success = success && 0 != createAndLinkProgram(mStages[SHADER_TYPE_VERTEX].handle(), mStages[SHADER_TYPE_VERTEX].handle(), mProgId);

  return success;
}

void ShaderProgram::fillBlockProperty(PropertyBlockInfoBinding& block, uint progId, GLint index) {
  GLint numUniform;
  glGetActiveUniformBlockiv(progId, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniform);
  if(numUniform <= 0) {
    return;
  }

  GLint* indices = (GLint*)_alloca(numUniform);
  glGetActiveUniformBlockiv(progId, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);
  
  GLint* offsets = (GLint*)_alloca(numUniform);
  GLint* types = (GLint*)_alloca(numUniform);
  GLint* counts = (GLint*)_alloca(numUniform);
  glGetActiveUniformsiv(progId, numUniform, (GLuint*)indices, GL_UNIFORM_OFFSET, offsets);
  glGetActiveUniformsiv(progId, numUniform, (GLuint*)indices, GL_UNIFORM_TYPE, types);
  glGetActiveUniformsiv(progId, numUniform, (GLuint*)indices, GL_UNIFORM_SIZE, counts);

  for(GLint i = 0; i < numUniform; ++i) {
    char name[64];
    GLint len = 0;
    glGetActiveUniformName(progId, (GLuint)indices[i], sizeof(name), &len, name);
    property_info_t& prop = block[name];

    prop.name = name;
    prop.offset = offsets[i];

   // unfinished should get size here, while glSize unimplemented
   size_t totalSize = GLSize(types[i]) * counts[i]; 
   prop.size = (uint)totalSize;
  }
}

void ShaderProgram::genInfo() {
  mInfo.clear();

  glUseProgram(mProgId);

  // uniforms
  GLint count;
  glGetProgramiv(mProgId, GL_ACTIVE_UNIFORMS, &count);

  constexpr GLsizei MAX_NAME_LEN = 128;
  char name[MAX_NAME_LEN];

  for(GLint ui = 0; ui < count; ++ui) {
    GLsizei nameLen;
    GLsizei size;
    GLenum type = GL_NONE;

    glGetActiveUniform(mProgId, ui, MAX_NAME_LEN, &nameLen, &size, &type, name);

    if(type != GL_NONE) {
      GLuint location = glGetUniformLocation(mProgId, name);
      if(location != -1) {
        switch(type) {
          case GL_SAMPLER_1D:
          case GL_SAMPLER_2D:
          case GL_SAMPLER_3D:
          case GL_SAMPLER_2D_SHADOW:
          case GL_SAMPLER_CUBE: {
            // sampler and textures share locations in GL
            uint binding = uint(-1);
            glGetUniformuiv(mProgId, location, &binding);
            ENSURES(binding < NUM_TEXTURE_SLOT);
            shader_bind_info_t& info = mInfo.at((eTextureSlot)binding);
            info.location = location;
            info.name = name;
          }
          break;

          default:
            ERROR_RECOVERABLE(Stringf("%s: Global uniform not supported", name));
          break;
        }
      }
    }
  }

  // uniform blocks, aka ubo
  glGetProgramiv(mProgId, GL_ACTIVE_UNIFORM_BLOCKS, &count);

  GLsizei len = 0;

  for(GLint bi = 0; bi < count; ++bi) {
    len = 0;
    glGetActiveUniformBlockName(mProgId, bi, MAX_NAME_LEN, &len, name);

    if(len > 0) {
      PropertyBlockInfoBinding& block = mInfo.at(name);
      
      block.bindInfo.name = name;
      GLint binding = -1;
      glGetActiveUniformBlockiv(mProgId, bi, GL_UNIFORM_BLOCK_BINDING, &binding);
      ENSURES(binding != -1);
      block.bindInfo.location = (uint)binding;

      GLint size = 0u;
      glGetActiveUniformBlockiv(mProgId, bi, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
      block.totalSize = (size_t)size;


    }
  }


  
}

uint ShaderProgram::createAndLinkProgram(uint vs, uint fs, uint& handle) {
  // credate the program handle - how you will reference
  // this program within OpenGL, like a texture handle
  uint programId = handle;
  if(programId == 0) {
    programId = glCreateProgram();
  }
  EXPECTS(programId != 0);

  // Attach the shaders you want to use
  glAttachShader(programId, vs);
  glAttachShader(programId, fs);

  GL_CHECK_ERROR();
  // Link the program (create the GPU program)
  glLinkProgram(programId);
  GL_CHECK_ERROR();

  // Check for link errors - usually a result
  // of incompatibility between stages.
  GLint linkStatus;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
  GL_CHECK_ERROR();

  if (linkStatus == GL_FALSE) {
    logProgramError(programId);
    glDeleteProgram(programId);
    programId = 0;
  }

  // no longer need the shaders, you can detach them if you want
  // (not necessary)
  // glDetachShader(programId, vs);
  // glDetachShader(programId, fs);
  GL_CHECK_ERROR();

  vs = 0;
  fs = 0;
  handle = programId;
  return programId;

}

void ShaderProgram::logProgramError(uint programId) {
  // figure out how large the buffer needs to be
  GLint length = 0;
  glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);

  // make a buffer, and copy the log to it. 
  char* buffer = new char[length + 1];
  glGetProgramInfoLog(programId, length, &length, buffer);

  // Print it out (may want to do some additional formatting)
  buffer[length] = NULL;
  DebuggerPrintf("class", buffer);

  ERROR_RECOVERABLE(Stringf("Linking Shader error, %s", buffer));

//  DEBUGBREAK;

  // free up the memory we used. 
  delete []buffer;

}

bool ShaderProgram::commit() {
  for(ShaderStage& stage: mStages) {
    if(stage.status() != STAGE_READY) {
      stage.compile();
    }
  }
  uint vs = mStages[SHADER_TYPE_VERTEX].handle();
  uint fs = mStages[SHADER_TYPE_FRAGMENT].handle();
  createAndLinkProgram(vs, fs, mProgId);

  glDeleteShader(vs);
  glDeleteShader(fs);
  GL_CHECK_ERROR();

  return mProgId != 0;
}
