#pragma once

#include <SFML/Graphics.hpp>

class Node {
public:
  Node()
  {
    this->circle.setRadius(10);
    this->circle.setPosition(sf::Vector2f(100, 300));
  }
  // ~Node() {}
  
  sf::CircleShape getCircle()
  {
    return this->circle;
  }
private:
  sf::CircleShape circle; 
};
