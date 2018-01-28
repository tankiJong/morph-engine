#include "FileUtils.hpp"
#include <cstdio>
#include <cstdlib>

Blob fileToBuffer(const char * nameWithFullPath) {
  FILE* fp = nullptr;
  fopen_s(&fp, nameWithFullPath, "r");
  if(fp == nullptr) {
    return Blob();
  }
  size_t size = 0U;
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  char* buffer = new char[size+1];
  size_t read = fread(buffer, 1, size, fp);
  buffer[read] = 0;
  fclose(fp);
  return Blob(buffer, size+1);
}
