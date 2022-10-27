#include "window.hpp"

int main()
{
  Window window;
  
  while (window.isOpen())
  {
    window.update();
    window.render();
  }

  return 0;
}
