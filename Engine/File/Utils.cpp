#include "Utils.hpp"
#include <cstdlib>
#include <fstream>
#include "Path.hpp"
#include <filesystem>
namespace sfs {
  using namespace std::experimental::filesystem;
}

Blob fileToBuffer(const char* nameWithFullPath) {
  std::ifstream file(nameWithFullPath, std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();

  file.seekg(0, std::ios::beg);

  char* buffer = new char[(uint)size + 1];

  if(file.read(buffer, size)) {
    buffer[size] = 0;
    return Blob(buffer, (uint)size+1);
  } else {
    return Blob();
  }
//  size_t size = 0U;
//  fseek(fp, 0L, SEEK_END);
//  size = ftell(fp);
//  fseek(fp, 0L, SEEK_SET);
//  size_t read = fread(buffer, 1, size, fp);
//  buffer[read] = 0;
//  fclose(fp);
//  return Blob(buffer, size+1);
}

fs::path fs::workingPath() {
  return sfs::current_path();
}

fs::path fs::absolute(const path& path, const Path& base) {
  return sfs::absolute(path, base);
}

bool fs::isDirectory(const path& path) {
  return is_directory(path);
}
