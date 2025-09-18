#include <iostream>
#include "Game/Game.hpp"

int
main ()
{
  std::cout << "Hello, project!" << std::endl;

  auto game = std::make_unique<Game>();
  game->init();
  game->run();

  return 0;
}