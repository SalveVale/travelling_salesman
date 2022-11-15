#pragma once

#include <SFML/Graphics.hpp>

#include <math.h>

class Node {
public:
  Node(int x, int y, int index)
  {
    this->circle.setRadius(this->radius);
    this->xpos = x;
    this->ypos = y;
    
    this->x = x;
    this->y = y;
    
    this->index = index;
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
  
  int getIndex() { return this->index; }
  
  void setDesirability(float distance, float desirabilityModifier, float pharamoneStrength)
  {
    this->distance = (1/distance) * 1000;
    this->desirability = pow((this->distance), desirabilityModifier) * pharamoneStrength;
  }
  
  float updateDesirability(float power)
  {
    this->desirability = pow(this->distance, power);
    return this->desirability;
  }
  
  float getDesirability() { return this->desirability; }
  
  void decrementIndex() { this->index--; }
  
  void setIndex(int newIndex) { this->index = newIndex; }
  
private:
  sf::CircleShape circle; 
  bool isAnimating = true;
  float radius = 1.f;
  int xpos;
  int ypos;
  
  int x;
  int y;
  
  int index;
  
  float distance;
  
  float desirability;
};
