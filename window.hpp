#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

#include "node.hpp"

class Window {
public:
  Window()
  {
    this->initVariables();
    this->initWindow();
  }
  ~Window()
  {
    delete this->window;
  }

  const bool isOpen() const
  {
    return this->window->isOpen();
  }
  
  void update()
  {
    switch (this->state)
    {
      case build:
        this->pollEventsBuild();
        this->updateMouse();
        this->updateUI();
        break;
      case solving:
        this->pollEventsSolving();
        break;
    }
  }
  
  void render()
  {
    this->window->clear();
    
    this->drawNodes();
    this->drawUI();
    
    this->window->display();
  }
private:
  //state engine
  enum states{
    build,
    solving
  } state = build;
  
  //window
  sf::RenderWindow *window;
  sf::VideoMode videoMode;
  sf::Event event;
  
  sf::Vector2i mousePosView;
  sf::Vector2f mousePosWindow;
  
  //ui
  sf::Font font;
  sf::Text solveText;
  sf::RectangleShape solveBox;
  
  //logic
  std::vector<Node> nodes;
  
  //functions
  //state engine
  void setState(states newState)
  {
    if (this->state == build && newState == solving)
    {
      this->state = solving;
    }
  }  
  
  void initVariables()
  {
    this->window = nullptr;
    
    if (!font.loadFromFile("resources/Roboto/Roboto-Regular.ttf")) std::cout << "Failed to load font from file";
    this->solveText.setFont(this->font);
    this->solveText.setCharacterSize(15);
    this->solveText.setPosition(10, 10);
    this->solveText.setString("Solve");
    
    this->solveBox.setPosition(sf::Vector2f(12, 10));
    this->solveBox.setSize(sf::Vector2f(50, 20));
    this->solveBox.setFillColor(sf::Color(30, 30, 30, 255));
  }
  
  void initWindow()
  {
    this->videoMode.height = 1000;
    this->videoMode.width = 1500;
    
    this->window = new sf::RenderWindow(this->videoMode, "Travelling Salesman");
    this->window->setFramerateLimit(60);
  }
  
  void pollEventsBuild()
  {
    while (this->window->pollEvent(this->event))
    {
      switch (this->event.type)
      {
        case sf::Event::Closed:
          this->window->close();
          break;
        case sf::Event::KeyPressed:
          if (this->event.key.code == sf::Keyboard::Escape)
            this->window->close();
          if (this->event.key.code == sf::Keyboard::A)
            this->spawnNode();
          break;
        default: break;
      }
    }
  }
  
  void pollEventsSolving()
  {
    while (this->window->pollEvent(this->event))
    {
      switch (this->event.type)
      {
        case sf::Event::Closed:
          this->window->close();
          break;
        case sf::Event::KeyPressed:
          if (this->event.key.code == sf::Keyboard::Escape)
            this->window->close();
          break;
        default: break;
      }
    }
  }
  
  void updateMouse()
  {
    this->mousePosView = sf::Mouse::getPosition(*this->window);
    this->mousePosWindow = this->window->mapPixelToCoords(this->mousePosView);
  }
  
  void spawnNode()
  {
    Node *newNode = new Node(this->mousePosWindow.x, this->mousePosWindow.y);
    this->nodes.push_back(*newNode);
  }
  
  void updateUI()
  {
    if (this->solveBox.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->solveBox.setFillColor(sf::Color(100, 100, 100, 255));
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->solveBox.setFillColor(sf::Color(150, 150, 150, 255));
        this->setState(solving);
      }
    }
    else
    {
      this->solveBox.setFillColor(sf::Color(30, 30, 30, 255));
    }
  }
  
  void drawNodes()
  {
    for (int i=0; i<this->nodes.size(); i++)
    {
      this->window->draw(this->nodes[i].getCircle());
    }
  }
  
  void drawUI()
  {
    this->window->draw(this->solveBox);
    this->window->draw(this->solveText);
  }
};
