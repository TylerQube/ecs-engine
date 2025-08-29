#include <iostream>
#include "Engine/Engine.hpp"

int
main ()
{
  std::cout << "Hello, project!" << std::endl;

  auto engine = std::make_unique<Engine>();
  engine->init();
  engine->run();

  return 0;
}