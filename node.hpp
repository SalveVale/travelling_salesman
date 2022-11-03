#pragma once

#include <SFML/Graphics.hpp>

class Node {
public:
  Node(int x, int y)
  {
    this->circle.setRadius(this->radius);
    this->xpos = x;
    this->ypos = y;
    
    this->x = x;
    this->y = y;
  }
  // ~Node() {}
  
  sf::CircleShape getCircle()
  {
    if (this->isAnimating)
    {
      this->circle.setPosition(sf::Vector2f(this->xpos, this->ypos));
      this->xpos -= 2;
      this->ypos -= 2;
      
      this->radius *= 1.5;
      this->circle.setRadius(this->radius);
      if (this->radius >= 7)
      {
        this->isAnimating = false;
      }
    }
    return this->circle;
  }
  
  int getx() { return this->x; }
  int gety() { return this->y; }

private:
  sf::CircleShape circle; 
  bool isAnimating = true;
  float radius = 1.f;
  int xpos;
  int ypos;
  
  int x;
  int y;
};
