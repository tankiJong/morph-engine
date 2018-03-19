#include "ShaderStage.hpp"
#include "Engine/Tool/Parser.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <optional>
#include "Engine/File/Path.hpp"
#include <sstream>
#include "Engine/File/FileUtils.hpp"
#include <set>
#include "glFunctions.hpp"

GLenum glShaderType[NUM_SHADER_TYPE] = {
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
};

inline bool isComment(const char* c) {
  return (*c == '/' && *(c + 1) && *(c + 1) == '/'); // comment start
}

inline bool isReturn(const char* c) {
  return *c == '\n' || (*c == '\r' && *(c + 1) && *(c + 1) == '\n');
}

inline bool isLineEnd(const char* c) {
  return isComment(c) || isReturn(c);
}

inline bool isTermination(const char* c) {
  return *c == '\0';
}

inline bool isWhitespace(const char* c) {
  return *c == ' ' || *c == '\t';
}

inline bool isInlineCommentBegin(const char* c) {
  return *c == '/' && *(c + 1) && *(c + 1) == '*';
}

inline bool isInlineCommentEnd(const char* c) {
  return *c == '*' && *(c + 1) && *(c + 1) == '/';
}

bool consumeWhitespace(const char*& c) {
  while(isWhitespace(c)) {
    ++c;
  }

  return true;
}

inline void consumeReturn(const char*& c) {
  EXPECTS(isReturn(c));
  (*c == '\r') ? c+=2 : c++;
}

bool consumeComment(const char*& c) {
  EXPECTS(isComment(c));

  while(!isReturn(c) && !isTermination(c)) {
    ++c;
  }
  if(isTermination(c)) {
    c++;
  } else {
    consumeReturn(c);
  }
  return true;
}

uint consumeInlineComment(const char*& c) {
  EXPECTS(isInlineCommentBegin(c));

  uint offset = 0;
  while(!isInlineCommentEnd(c)) {
    if(isTermination(c)) {
      ERROR_RECOVERABLE("Preprocessing shader file failed, get '\0', expect '*/' for inline comment");
      return offset;
    }
    if(isReturn(c)) {
      consumeReturn(c);
      offset++;
    } else {
      ++c;
    }
  }

  c += 2;

  return offset;
}

/*
* \return the line offset, eg 1 means new *c is on the next line
*/
uint passWhitespaceAndComment(const char*& c) {
  uint offset = 0;
  while(true) {
    if (isTermination(c)) return offset;
    consumeWhitespace(c);
    
    if (isComment(c)) {
      consumeComment(c);
      continue;
    }

    if (isInlineCommentBegin(c)) {
      uint lineoffset = 0;
      offset += consumeInlineComment(c);
      continue;
    }

    break;
  }

  return offset;
}

/*
* \return the line offset, eg 1 means new *c is on the next line
*/
uint passWhitespaceAndInlineComment(const char*& c) {
  uint offset = 0;
  while (true) {
    if (isTermination(c)) return offset;

    consumeWhitespace(c);

    if (isInlineCommentBegin(c)) {
      uint lineoffset = 0;
      offset += consumeInlineComment(c);
      continue;
    }

    break;
  }

  return offset;
}

/*
 * the input c is any actual code content
 * * if get // first, pass until \n, *c would be either the symbol after \n
 * * if get /* first return *c would be / (start of the inlineComment)
 * * if get \n first, *c would be the symbol after \n
 * * if get \0 first, *c would be \0
 * \return the line offset, eg 1 means new *c is on the next line
 */
uint passCodeTillLineEndOrInlineComment(const char*& c) {
  while(true) {
    //  //
    if (isComment(c)) {
      while(true) {
        if (isTermination(c)) return 0;
        if(isReturn(c)) {
          c++;
          return 1;
        }
        c++;
      }
    }

    //  /*
    if (isInlineCommentBegin(c)) return 0;


    //  \n
    if (isReturn(c)) {
      c++;
      return 1;
    }

    //  \0
    if (isTermination(c)) return 0;

    c++;
  }
}

bool consumeVersion (const char*& c, uint& version) {
  constexpr size_t VERSION_KEY_LEN = 7;
  passWhitespaceAndComment(c);

//  DEBUGBREAK;
  // #
  if (*c != '#') return false;
  c++;

  consumeWhitespace(c);

  // version
  if (strncmp(c, "version", VERSION_KEY_LEN)) return false;
  c += VERSION_KEY_LEN;

  consumeWhitespace(c);

  if (!isdigit(*c)) return false;

  version = (uint)std::strtoul(c, nullptr, 0);
  while (!isReturn(c)) {
    c++;
  };
  return true;
}


/**
 * \brief assumption: the input *c is an actuall symbol not part of the whitspace or comment, so bascially it should be actual code content
 * if return false, c will be the thing according to @passCodeTillLineEndOrInlineComment
 * if return true,  c will be the symbol after the end of line
 */
bool tryInclude(const char*& c, uint& out_lineOffset, std::string* includePath = nullptr) {
  constexpr uint INCLUDE_KEY_LEN = 7;

  const char* pathStart = nullptr;

  out_lineOffset = 0;
  // #
  if (*c != '#') {
    out_lineOffset += passCodeTillLineEndOrInlineComment(c);
    return false;
  };
  c++;

  // include
  if (strncmp(c, "include", INCLUDE_KEY_LEN)) {
    out_lineOffset += passCodeTillLineEndOrInlineComment(c);
    return false;
  }
  c += INCLUDE_KEY_LEN;

  out_lineOffset += passWhitespaceAndInlineComment(c);
  if (isTermination(c)) return false;

  // < or "
  char quoteBegin = *c;
  char quoteEnd;

  switch(quoteBegin) {
    case '"':
      quoteEnd = '"';
      break;
    case '<':
      quoteEnd = '>';
      break;
    default: {
      out_lineOffset += passCodeTillLineEndOrInlineComment(c);
      return false;
    }
  }

  c++;

  pathStart = c;

  while(*c != quoteEnd) {
    if (isTermination(c)) {
      return false; // change line or EOF before end of the include
    }

    if(isReturn(c)) {
      out_lineOffset++;
      consumeReturn(c);
      return false;
    }

    c++;
  }

  ENSURES(*c == quoteEnd);


  if(includePath) {
    *includePath = std::string(pathStart, c);
  }

  c++;
  return true;
};

void logShaderError(GLuint shaderId) {
  // figure out how large the buffer needs to be
  GLint length = 0;
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

  // make a buffer, and copy the log to it. 
  char* buffer = new char[length + 1];
  glGetShaderInfoLog(shaderId, length, &length, buffer);

  // Print it out (may want to do some additional formatting)
  buffer[length] = NULL;
  DebuggerPrintf("class", buffer);

  ERROR_RECOVERABLE(Stringf("Compile Shader error, %s", buffer));

  //  DEBUGBREAK;

  // free up the memory we used. 
  delete[]buffer;

}

ShaderStage::ShaderStage() {}
ShaderStage::ShaderStage(eShaderType type) : mType(type) {}
ShaderStage::ShaderStage(eShaderType type, const char* source): mPath(source) {
  std::string file(fileToBuffer(source).as<char*>());
  setFromString(type, file);
}

bool ShaderStage::setFromString(eShaderType type, std::string source) {
  mType = type;
  if (!parse(source)) {
    mStatus = STAGE_PREPROCESS_FAILED;
    return false;
  }
  mStatus = STAGE_PREPROCESSED;
}

bool ShaderStage::setFromFile(eShaderType type, const char* path) {
  mPath = path;
  std::string file(fileToBuffer(path).as<char*>());
  return setFromString(type, file);
}

int ShaderStage::compile() {
  mHandle = compile(mShaderString.data(), mType);
  if (mHandle == NULL) mStatus = STAGE_COMPILE_FAILED;
  else mStatus = STAGE_READY;

  return mHandle;
}

ShaderStage& ShaderStage::define(const std::string& k, std::string v) {
  for(auto& def: mDefineDirectives) {
    if(def.first == k) {
      def.second = v;
      return *this;
    }
  }

  mDefineDirectives.push_back(std::make_pair(k, v));

  return *this;
}

bool ShaderStage::parse(std::string& source) {
  auto parsed = parseDirectives(source);

  if (!parsed) return false;
  std::string head, body;
  uint lineOfVersion;
  std::tie(head, body, lineOfVersion) = parsed.value();

  std::set<Path> included;
  included.insert(mPath);

  bool success = parseBody(mPath, body, lineOfVersion, included);

  mShaderString = head;
  mShaderString.append(body);

  return success;
}

std::optional<std::tuple<std::string, std::string, uint>> ShaderStage::parseDirectives(std::string& source) {
  const char* begin = source.data();
  if(!consumeVersion(begin, mVersion)) {
    ERROR_RECOVERABLE("Invalid shader without verison at the begin of the file");
    return {};
  }

  passWhitespaceAndComment(begin);
  ENSURES(isReturn(begin) || isTermination(begin));

  uint line = 1;
  for(const char* i = source.data(); i != begin; ++i) {
    if (*i == '\n') line++;
  }

  size_t len = begin - source.data() + 1;
  std::string first(source.data(), len); // from begin to the end of #version line, include \n
  std::string rest(source.data() + len, source.length() - len); // the rest
  
  if (mDefineDirectives.empty()) {
    return std::make_tuple(first, rest, line);
  }


  for(const auto& def: mDefineDirectives) {
    first += "#define " + def.first;
    if(!def.second.empty()) {
      first += " " + def.second;
    }
    first += "\n";
  }

  first += makelineDirective(mVersion, line, mPath.c_str());

  return std::make_tuple(first, rest, line);
}


bool ShaderStage::parseBody(const Path& currentFile, std::string& body, uint currentLine, std::set<Path>& includedFiles) {
  std::istringstream input(body);

  std::stringstream output;

  Path workingDir = fs::current_path();

  Path absDir = fs::absolute(currentFile);;
  fs::current_path(absDir.parent_path());
  output << makelineDirective(mVersion, currentLine, currentFile);

  const char* c = body.data();
  while(!isTermination(c)) {
    const char* parseStart = c;

    currentLine += passWhitespaceAndComment(c);

    if(parseStart != c) {
      output << std::string(parseStart, c);
    }

    parseStart = c;
    std::string includePath;
    uint lineoffset = 0;
    bool isInclude = tryInclude(c, lineoffset, &includePath);
    currentLine += lineoffset;
    if(isInclude) {
      if(includedFiles.count(includePath) == 0) {
//        Path absPath = fs::canonical(includePath);
        Blob f = fileToBuffer(includePath.c_str());

        if(f.size() == 0) {
          ERROR_RECOVERABLE("Include file with empty content, possibly file does not exist")
          return false;
        }

        std::string includeFile(f.as<char*>());
        includedFiles.insert(includePath);
        bool success = parseBody(includePath, includeFile, 0, includedFiles);

        //        if(!success) return false;

        output << includeFile << '\n';
        output << makelineDirective(mVersion, currentLine, currentFile);
      }
    } else {
      output << std::string(parseStart, c);
    }
  }

  body = std::move(output.str());

  fs::current_path(workingDir);
  return true;
}

std::string ShaderStage::makelineDirective(uint version, uint line, const Path& filename) {
  // glsl 330 and up, #line means next line, before means current line
  if(version >= 330) {
    line++;
  }
  return Stringf("#line %u \"%s\"\n", line, filename.filename().string().c_str());
}

int ShaderStage::compile(const char* shaderStr, eShaderType type) {
  EXPECTS(shaderStr != nullptr);
  GLenum shaderType = glShaderType[type];


  // Create a shader
  GLuint shaderId = glCreateShader(shaderType);
  EXPECTS(shaderId != NULL);

  // Bind source to it, and compile
  // You can add multiple strings to a shader – they will 
  // be concatenated together to form the actual source object.
  GLint shaderLength = (GLint)strlen(shaderStr);

  glShaderSource(shaderId, 1, (GLchar**)&shaderStr, &shaderLength);
  glCompileShader(shaderId);

  // Check status
  GLint status;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    mStatus = STAGE_COMPILE_FAILED;
    logShaderError(shaderId);
    glDeleteShader(shaderId);
    shaderId = NULL;
  }

  return shaderId;
}
