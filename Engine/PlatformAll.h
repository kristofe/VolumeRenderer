#ifndef _PLATFORM_ALL
#define _PLATFORM_ALL

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
#include <direct.h>
#define _GetCurrentDir _getcwd
#define _ChangeDir _chdir
#else
#include <unistd.h>
#define _GetCurrentDir getcwd
#define _ChangeDir chdir
#endif

#include <dirent.h>
#include <string>
#include <vector>

std::string GetCurrentDir()
{
  std::string s;
  char cCurrentPath[FILENAME_MAX];

  if (!_GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
  {
    s += errno;
    return s;
  }

  //printf ("The current working directory is %s", cCurrentPath);
  s = cCurrentPath;

  return s;
}

std::vector<std::string> ListAllDirectories(std::string& directory)
{
  std::vector<std::string> d;
  DIR *dir = opendir(directory.c_str());
  
  struct dirent *entry = readdir(dir);
  
  while (entry != NULL)
  {
    if (entry->d_type == DT_DIR)
    {
      if(!(
           (strcmp(entry->d_name, ".") == 0)
            ||
           (strcmp(entry->d_name, "..") == 0)
           )
         )
      {
        d.push_back(entry->d_name);
      }
      
    }
    
    entry = readdir(dir);
  }
  
  closedir(dir);
  
  return d;
}

void ChangeParentDir(std::string targetParent)
{
  const int MAX_LEVELS_TO_SEARCH = 3;
  int levels = 0;
  //Try to descend until targetParent is visible
  std::string currDir = GetCurrentDir();
  std::cout << "Searching for " << targetParent
            << " parent directory.. starting with "
            << currDir
            << std::endl;
  
  std::vector<std::string> folders;
  do
  {
    folders = ListAllDirectories(currDir);
    //New C++11 range based for loop
    for(std::string& dir : folders)
    {
      if(dir.compare(targetParent) == 0)
      {
        std::cout << "Found " << targetParent
                  << ".  Switching to " << dir << std::endl;
        _ChangeDir(currDir.c_str());
        return;
      }
    }
    currDir += "/..";
    
    levels++;
  }while( levels < MAX_LEVELS_TO_SEARCH);
  
  std::cout << "Failed to find " << targetParent << std::endl;
}

#endif //_PLATFORM_ALL