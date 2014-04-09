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
  std::string currentShader = "";
  std::string currentShaderName = "";
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
       std::cout << line << std::endl;
       currentShaderName = line.substr(delimeter.size(), line.size());
       currentShaderName = trim(currentShaderName);

     }
     else{
       currentShader += line;
       currentShader += "\n";
     }
  }
  _shaders[currentShaderName] = currentShader;

  using namespace std;
  for(map<string, string>::iterator it = _shaders.begin();
      it != _shaders.end(); ++it)
  {
      std::cout << it->first <<"\n";// " " << it->second << "\n";
  }
}



std::string ShaderSource::getShader(const std::string id)
{
  if(_shaders.find(id) == _shaders.end()){
      return "//////\n";
  }
  return _shaders[id];
}




} //namespace renderlib
