#ifndef COLOR_H
#define COLOR_H
#include "utils.h"

namespace renderlib{

struct Color
{
public:
  Color();

private:
  DISALLOW_COPY_AND_ASSIGN(Color);

public:
  float r,g,b,a;
};

}

#endif // COLOR_H
