#pragma once

#include <SFML/Graphics.hpp>

class Node {
public:
  Node(int x, int y)
  {
    this->circle.setRadius(this->radius);
    this->circle.setPosition(sf::Vector2f(x-10, y-10));
  }
  // ~Node() {}
  
  sf::CircleShape getCircle()
  {
    if (this->isAnimating)
    {
      this->radius *= 1.5;
      this->circle.setRadius(this->radius);
      if (this->radius >= 10)
      {
        this->isAnimating = false;
      }
    }
    return this->circle;
  }

private:
  sf::CircleShape circle; 
  bool isAnimating = true;
  float radius = 1;
};
