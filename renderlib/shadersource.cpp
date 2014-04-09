#include "shadersource.h"
#include <string>
#include <iostream>
#include <fstream>

namespace renderlib{

ShaderSource::ShaderSource()
{
}

void ShaderSource::parseFile(const std::string filename, const std::string delimeter)
{
  std::string currentShader;
  std::string currentShaderName;
  std::ifstream  fin(filename);
  std::string    line;
  while(std::getline(fin, line))
  {
     if(line.substr(0,delimeter.size()) == delimeter) //Line starts with delimeter
     {
       if(currentShaderName.size() > 0) {
         _shaders[currentShaderName] = currentShader;
         currentShader = "";
       }
       currentShaderName = line.substr(delimeter.size() - 1, line.size());
     }
     else{
       currentShader += line;
       currentShader += "\n";
     }
  }
  _shaders[currentShaderName] = currentShader;
}

} //namespace renderlib
