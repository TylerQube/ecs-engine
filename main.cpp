#include <iostream>
#include "platform/openGLPlatform/OpenGLPlatform.h"

int
main ()
{
  std::cout << "Hello, CMake!" << std::endl;

  OpenGLPlatform platform("Hello Window!", 800, 400);
  std::cout << "Starting render loop" << std::endl;
  platform.run();
  return 0;
}