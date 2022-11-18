#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>

#include "node.hpp"

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 1000;

const float desirabilityChance = 0.01;

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
      case buttonClicked:
        this->pollEventsButtonClicked();
        break;
      case tweakAnt:
        this->pollEventsBuild();
        this->updateMouse();
        this->updateUITweak();
        this->pickNodesProximity();
        // this->updateLinksTweak();
        break;
      case solving:
        this->pollEventsSolving();
        this->solve();
        this->shuffleNodes();
        this->increaseSolveStep();
        break;
      case solvingAnt:
        this->pollEventsSolving();
        // this->printPharamones();
        this->solveAnt();
        this->decrementPharamones();
        this->generateLinksPharamone();
        this->increaseSolveStep();
        break;
      case solved:
        this->pollEventsBasic();
        this->updateMouse();
        this->updateResetButton();
        break;
    }
  }
  
  void render()
  {
    this->window->clear(this->colBG);
    
    this->drawLinks();
    this->drawNodes();
    if (this->state == tweakAnt)
    {
      this->drawUITweak();
    } else {
      this->drawUI();
    }
    
    this->window->display();
  }
private:
  //state engine
  enum states{
    build,
    buttonClicked,
    tweakAnt,
    solving,
    solvingAnt,
    solved
  } state = build;
  
  //window
  sf::RenderWindow *window;
  sf::VideoMode videoMode;
  sf::Event event;
  sf::Color colBG = sf::Color(20, 20, 26, 255);
  
  sf::Vector2i mousePosView;
  sf::Vector2f mousePosWindow;
  
  //ui
  sf::Color colButton = sf::Color(40, 35, 30, 255);
  sf::Color colButtonHighlight = sf::Color(110, 105, 100, 255);
  sf::Color colButtonActive = sf::Color(160, 155, 150, 255);
  
  sf::RectangleShape UIShader;
  
  sf::Font font;

  sf::Text solveText;
  sf::RectangleShape solveButton;
  sf::Text solveAntText;
  sf::RectangleShape solveAntButton;
  
  sf::Text resetText;
  sf::RectangleShape resetButton;
  
  sf::RectangleShape genSliderOutline;
  sf::RectangleShape genSliderBox;
  sf::Text genSliderNumText;
  sf::Text genSliderDescriptionText;
  sf::RectangleShape generateButton;
  
  sf::Text numNodesText;
  sf::Text numNodesVal;
  sf::Text totalPossibleSolutionsText;
  sf::Text totalPossibleSolutionsVal;
  sf::Text searchedSolutionsText;
  sf::Text searchedSolutionsVal;
  sf::Text bestPathText;
  sf::Text bestPathVal;
  
  int numNodes = 0;
  
  //tweakUI
  sf::RectangleShape tweakButton;
  sf::Text tweakButtonText;
  
  sf::RectangleShape powerSliderOutline;
  sf::RectangleShape powerSliderBox;
  sf::Text powerSliderDescriptionText;
  
  //logic
  float desirabilityModifier = 4.9;
  
  std::vector<Node> nodes;
  std::vector<sf::RectangleShape> links;
  std::vector<Node> visitedNodesInOrder;
  std::vector<std::vector<float>> pharamones;
  
  long totalSolutions;
  int solveStep = 0;
  int shuffleIndex;
  int randomTweakIndex;
  
  int genSliderNum = 10;
  
  float prevBestDistance = 9999999999.f;
  
  bool isNodes = false;
  
  //functions
  //state engine
  void setState(states newState)
  {
    if (this->state == build && newState == buttonClicked)
    {
      this->state = buttonClicked;
    }
    else if (this->state == buttonClicked && newState == build)
    {
      this->state = build;
    }
    else if (this->state == build && newState == tweakAnt)
    {
      this->colBG = sf::Color(26, 20, 20, 255);
      this->UIShader.setSize(sf::Vector2f(100, 200));
      
      std::random_device rd;
      std::default_random_engine eng(rd());
      std::uniform_int_distribution<int> distr(0, this->nodes.size());
      this->randomTweakIndex = distr(eng);
      
      this->state = tweakAnt;
    }
    else if (this->state == tweakAnt && newState == solvingAnt)
    {
      this->window->setFramerateLimit(30);
      this->initPharamones();
      this->colBG = sf::Color(20, 20, 26, 255);
      this->UIShader.setSize(sf::Vector2f(200, 100));
      this->state = solvingAnt;
    }
    else if (this->state == build && newState == solving)
    {
      if (this->nodes.size() > 1)
      {
        this->window->setFramerateLimit(165);
        this->shuffleIndex = this->nodes.size()-1;
        this->state = solving;
      }
      else
      {
        std::cout << "There must be at least 2 nodes to solve\n";
      }
    }
    else if (this->state == build && newState == solvingAnt)
    {
      this->initPharamones();
      this->window->setFramerateLimit(3);
      this->state = solvingAnt;
    }
    else if (this->state == solving && newState == solved)
    {
      this->colBG = sf::Color(20, 26, 20, 255);
      this->window->setFramerateLimit(60);
      this->state = solved;
    }
    else if (this->state == solvingAnt && newState == solved)
    {
      this->colBG = sf::Color(20, 26, 20, 255);
      this->window->setFramerateLimit(60);
      this->state = solved;
    }
    else if (this->state == solved && newState == build)
    {
      this->links.clear();
      this->resetUI();
      this->state = build;
    }
  }  
  
  void initVariables()
  {
    this->window = nullptr;
    
    //ui
    this->UIShader.setSize(sf::Vector2f(300, 170));
    this->UIShader.setFillColor(sf::Color(12, 12, 12, 150));
    
    if (!font.loadFromFile("resources/Roboto/Roboto-Regular.ttf")) std::cout << "Failed to load font from file";
    this->solveText.setFont(this->font);
    this->solveText.setCharacterSize(15);
    this->solveText.setPosition(15, 140);
    this->solveText.setString("Solve");
    
    this->solveButton.setPosition(sf::Vector2f(9, 140));
    this->solveButton.setSize(sf::Vector2f(50, 20));
    this->solveButton.setFillColor(this->colButton);
    
    this->solveAntText.setFont(this->font);
    this->solveAntText.setCharacterSize(15);
    this->solveAntText.setPosition(80, 140);
    this->solveAntText.setString("Ant");
    
    this->solveAntButton.setPosition(sf::Vector2f(69, 140));
    this->solveAntButton.setSize(sf::Vector2f(50, 20));
    this->solveAntButton.setFillColor(this->colButton);

    this->resetText.setFont(this->font);
    this->resetText.setCharacterSize(15);
    this->resetText.setPosition(15, 170);
    this->resetText.setString("Reset");
    
    this->resetButton.setPosition(sf::Vector2f(9, 170));
    this->resetButton.setSize(sf::Vector2f(50, 20));
    this->resetButton.setFillColor(this->colButton);
    
    this->genSliderOutline.setPosition(sf::Vector2f(12, 10));
    this->genSliderOutline.setSize(sf::Vector2f(200, 15));
    this->genSliderOutline.setFillColor(sf::Color(25, 25, 25, 255));
    this->genSliderOutline.setOutlineColor(sf::Color::White);
    this->genSliderOutline.setOutlineThickness(1);
    
    this->genSliderBox.setPosition(sf::Vector2f(30, 12));
    this->genSliderBox.setSize(sf::Vector2f(10, 10));
    this->genSliderBox.setFillColor(sf::Color::White);
    
    this->genSliderNumText.setFont(this->font);
    this->genSliderNumText.setCharacterSize(15);
    this->genSliderNumText.setPosition(200, 30);
    this->genSliderNumText.setString("10");

    this->genSliderDescriptionText.setFont(this->font);
    this->genSliderDescriptionText.setCharacterSize(15);
    this->genSliderDescriptionText.setPosition(10, 30);
    this->genSliderDescriptionText.setString("Generate random nodes: ");
    
    this->generateButton.setPosition(sf::Vector2f(9, 30));
    this->generateButton.setSize(sf::Vector2f(173, 20));
    this->generateButton.setFillColor(this->colButton);
    
    this->numNodesText.setFont(this->font);
    this->numNodesText.setCharacterSize(15);
    this->numNodesText.setPosition(sf::Vector2f(10, 50));
    this->numNodesText.setString("Number of nodes:");
    this->numNodesVal.setFont(this->font);
    this->numNodesVal.setCharacterSize(15);
    this->numNodesVal.setPosition(sf::Vector2f(200, 50));
    
    this->totalPossibleSolutionsText.setFont(this->font);
    this->totalPossibleSolutionsText.setCharacterSize(15);
    this->totalPossibleSolutionsText.setPosition(sf::Vector2f(10, 70));
    this->totalPossibleSolutionsText.setString("Total Possible Solutions:");
    this->totalPossibleSolutionsVal.setFont(this->font);
    this->totalPossibleSolutionsVal.setCharacterSize(15);
    this->totalPossibleSolutionsVal.setPosition(sf::Vector2f(200, 70));
    
    this->searchedSolutionsText.setFont(this->font);
    this->searchedSolutionsText.setCharacterSize(15);
    this->searchedSolutionsText.setPosition(sf::Vector2f(10, 90));
    this->searchedSolutionsText.setString("Searched solutions:");
    this->searchedSolutionsVal.setFont(this->font);
    this->searchedSolutionsVal.setCharacterSize(15);
    this->searchedSolutionsVal.setPosition(sf::Vector2f(200, 90));
    
    this->bestPathText.setFont(this->font);
    this->bestPathText.setCharacterSize(15);
    this->bestPathText.setPosition(sf::Vector2f(10, 110));
    this->bestPathText.setString("Lenght of best path:");
    this->bestPathVal.setFont(this->font);
    this->bestPathVal.setCharacterSize(15);
    this->bestPathVal.setPosition(sf::Vector2f(200, 110));
    
    //tweakUI
    this->tweakButton.setPosition(sf::Vector2f(150, 140));
    this->tweakButton.setSize(sf::Vector2f(80, 20));
    this->tweakButton.setFillColor(colButton);
    
    this->tweakButtonText.setFont(this->font);
    this->tweakButtonText.setCharacterSize(15);
    this->tweakButtonText.setPosition(sf::Vector2f(200, 140));
    this->tweakButtonText.setString("Tweak");
    
    this->powerSliderOutline.setPosition(sf::Vector2f(12, 20));
    this->powerSliderOutline.setSize(sf::Vector2f(200, 15));
    this->powerSliderOutline.setFillColor(sf::Color(25, 25, 25, 255));
    this->powerSliderOutline.setOutlineColor(sf::Color::White);
    this->powerSliderOutline.setOutlineThickness(1);
    
    this->powerSliderBox.setPosition(sf::Vector2f(55, 22));
    this->powerSliderBox.setSize(sf::Vector2f(10, 10));
    this->powerSliderBox.setFillColor(sf::Color::White);
    
    this->powerSliderDescriptionText.setFont(this->font);
    this->powerSliderDescriptionText.setCharacterSize(15);
    this->powerSliderDescriptionText.setPosition(sf::Vector2f(10, 40));
    this->powerSliderDescriptionText.setString("Proximity bias");
  }
  
  void initWindow()
  {
    this->videoMode.width = WINDOW_WIDTH;
    this->videoMode.height = WINDOW_HEIGHT;
    
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
  
  void pollEventsButtonClicked()
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
    
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
      this->setState(build);
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
          if (this->event.key.code == sf::Keyboard::Space)
            this->setState(solved);
          break;
        default: break;
      }
    }
  }
  
  void pollEventsBasic()
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
    Node *newNode = new Node(this->mousePosWindow.x, this->mousePosWindow.y, this->numNodes);
    this->nodes.push_back(*newNode);
    
    this->numNodes++;
    if (this->numNodes > 3) this->isNodes = true;
    this->updatePossibleSolutionsText();
  }
  
  void updateResetButton()
  {
    if (this->resetButton.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->resetButton.setFillColor(this->colButtonHighlight);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->resetButton.setFillColor(this->colButtonActive);
        this->setState(build);
      }
    }
    else
    {
      this->resetButton.setFillColor(this->colButton);
    }
  }
  
  void updateUI()
  {
    if (this->solveButton.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->solveButton.setFillColor(this->colButtonHighlight);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->solveButton.setFillColor(this->colButtonActive);
        this->setState(solving);
      }
    }
    else
    {
      this->solveButton.setFillColor(this->colButton);
    }
    
    if (this->solveAntButton.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->solveAntButton.setFillColor(this->colButtonHighlight);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->solveAntButton.setFillColor(this->colButtonActive);
        this->setState(solvingAnt);
      }
    }
    else
    {
      this->solveAntButton.setFillColor(this->colButton);
    }
    
    if (this->genSliderOutline.getGlobalBounds().contains(this->mousePosWindow) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
      if (this->mousePosWindow.x > 18 && this->mousePosWindow.x < 204)
      {
        this->genSliderBox.setPosition(sf::Vector2f(this->mousePosWindow.x - 5, 12));
        this->genSliderNum = (this->mousePosWindow.x - 12) / 2;
        this->genSliderNumText.setString(std::to_string(this->genSliderNum));
      }
    }
    
    if (this->generateButton.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->generateButton.setFillColor(this->colButtonHighlight);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->generateButton.setFillColor(this->colButtonActive);
        this->generateNodes();
        this->setState(buttonClicked);
      }
    }
    else
    {
      this->generateButton.setFillColor(this->colButton);
    }
    
    if (this->tweakButton.getGlobalBounds().contains(this->mousePosWindow))
    {
      this->tweakButton.setFillColor(this->colButtonHighlight);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        this->tweakButton.setFillColor(this->colButtonActive);
        this->setState(tweakAnt);
      }
    }
    else
    {
      this->tweakButton.setFillColor(this->colButton);
    }
  }
  
  void updateUITweak()
  {
    if (this->powerSliderOutline.getGlobalBounds().contains(this->mousePosWindow) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
      if (this->mousePosWindow.x > 18 && this->mousePosWindow.x < 204)
      {
        this->powerSliderBox.setPosition(sf::Vector2f(this->mousePosWindow.x - 5, 20));
        this->desirabilityModifier = (this->mousePosWindow.x - 18) / 10;
      }
    }
  }
  
  void resetUI()
  {
    this->colBG = sf::Color(20, 20, 26, 255);
    
    this->resetButton.setFillColor(this->colButton);
    
    this->bestPathVal.setString("");
    this->searchedSolutionsVal.setString("");
  }
  
  void generateNodes()
  {
    srand(time(NULL));
    for (int i=0; i<this->genSliderNum; i++)
    {
      int x = rand() % WINDOW_WIDTH + 1;
      int y = rand() % WINDOW_HEIGHT + 1;
      Node *newNode = new Node(x, y, this->numNodes);
      this->nodes.push_back(*newNode);
      this->numNodes++;
    }
    if (this->numNodes > 3) this->isNodes = true;
    this->updatePossibleSolutionsText();
    this->setState(build);
  }
  
  void updatePossibleSolutionsText()
  {
    if (this->numNodes < 2)
    {
      this->totalSolutions = 0;
    }
    else if (this->numNodes < 3)
    {
      this->totalSolutions = 1;
    }
    else
    {
      long factorial = 1;
      for (int i=1; i<=(this->numNodes-1); i++)
      {
        factorial *= i;
      }
      this->totalSolutions = factorial / 2;
    }

    this->numNodesVal.setString(std::to_string(this->numNodes));
    this->totalPossibleSolutionsVal.setString(std::to_string(this->totalSolutions));
  }
  
  void solve()
  {
    float totalDistance = 0;

    for (int i=0; i<this->nodes.size(); i++)
    {
      Node currentNode = this->nodes[i];
      int currentx = currentNode.getx();
      int currenty = currentNode.gety();
      
      Node nextNode = this->nodes[i+1];
      int nextx = nextNode.getx();
      int nexty = nextNode.gety();
      
      int xlen = abs(currentx - nextx);
      int ylen = abs(currenty - nexty);
      float hypotenuse = this->findHypotenuse(xlen, ylen);
      totalDistance += hypotenuse;
    }
    
    if (totalDistance < this->prevBestDistance)
    {
      this->prevBestDistance = totalDistance;
      
      this->bestPathVal.setString(std::to_string(this->prevBestDistance));
      
      this->generateLinks();
    }
    
    // this->solveStep++;
    // this->searchedSolutionsVal.setString(std::to_string(this->solveStep));
    // if (this->solveStep >= this->totalSolutions)
    // {
    //   this->solveStep = 0;
    //   this->setState(solved);
    // }
  }
  
  void generateLinksPharamone()
  {
    this->links.clear();
    
    for (int i=0; i<this->nodes.size(); i++)
    {
      for (int j=0; j<this->nodes.size(); j++)
      {
        Node currentNode = this->nodes[i];
        int currentx = currentNode.getx();
        int currenty = currentNode.gety();
        
        sf::RectangleShape link;
        link.setPosition(sf::Vector2f(currentx+1, currenty));
        link.setFillColor(sf::Color(255, 255, 255, 10 * this->pharamones[i][j]));
        
        if (j == this->nodes.size()-1)
        {
          Node firstNode = this->nodes[0];
          int firstx = firstNode.getx();
          int firsty = firstNode.gety();
        
          float xlen, ylen, hypotenuse;
        
          if (currentx < firstx)
          {
            xlen = firstx - currentx;
            ylen = firsty - currenty;
            hypotenuse = this->findHypotenuse(xlen, ylen);
            link.setSize(sf::Vector2f(hypotenuse, 2));
          }
          else
          {
            xlen = currentx - firstx;
            ylen = currenty - firsty;
            hypotenuse = this->findHypotenuse(xlen, ylen);
            link.setSize(sf::Vector2f(-hypotenuse, 2));
          }

          float angle = this->findAngle(xlen, ylen);
        
          link.setRotation(angle);

          this->links.push_back(link);
          break;
        }
      
        Node nextNode = this->nodes[j+1];
        int nextx = nextNode.getx();
        int nexty = nextNode.gety();
      
        float xlen, ylen, hypotenuse;
      
        if (currentx < nextx)
        {
          xlen = nextx - currentx;
          ylen = nexty - currenty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(hypotenuse, 2));
        }
        else
        {
          xlen = currentx - nextx;
          ylen = currenty - nexty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(-hypotenuse, 2));
        }

        float angle = this->findAngle(xlen, ylen);
            
        link.setRotation(angle);
      
        this->links.push_back(link);
      }
    }
  }
  
  void generateLinksDesirability(int startingIndex)
  {
    this->links.clear();
    
    Node startingNode = this->nodes[startingIndex];
    int startingx = startingNode.getx();
    int startingy = startingNode.gety();
    for (int i=0; i<this->nodes.size(); i++)
    {
      if (i != startingIndex)
      {
        Node nextNode = this->nodes[i];
        int nextx = nextNode.getx();
        int nexty = nextNode.gety();
        
        sf::RectangleShape link;
        link.setPosition(sf::Vector2f(startingx+1, startingy));
        // float desirability = pow(nextNode.getDesirability()*10, 3.5);
        float desirability = nextNode.getDesirability();
        link.setFillColor(sf::Color(255, 255, 255, desirability));
        
        int xlen, ylen;
        float hypotenuse;
        if (startingx < nextx)
        {
          xlen = nextx - startingx;
          ylen = nexty - startingy;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(hypotenuse, 2));
        } else {
          xlen = startingx - nextx;
          ylen = startingy - nexty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(-hypotenuse, 2));
        }
        
        float angle = this->findAngle(xlen, ylen);
        
        link.setRotation(angle);
        
        this->links.push_back(link);
      }
    }
  }
  
  // void updateLinksTweak()
  // {
  //   for (int i=0; i<this->links.size(); i++)
  //   {
  //     float desirability = this->nodes[i].updateDesirability(this->desirabilityModifier);
  //     this->links[i].setFillColor(sf::Color(255, 255, 255, desirability));
  //   }
  // }
  
  void generateLinks()
  {
    this->links.clear();
    // std::vector<sf::RectangleShape> currentLinks;
    
    // int transparencyColor = 15;

    for (int i=0; i<this->nodes.size(); i++)
    {
      Node currentNode = this->nodes[i];
      int currentx = currentNode.getx();
      int currenty = currentNode.gety();
      
      sf::RectangleShape link;
      link.setPosition(sf::Vector2f(currentx+1, currenty));
      link.setFillColor(sf::Color(255, 255, 255, 80));
      // transparencyColor += 10;

      if (i == this->nodes.size()-1)
      {
        Node firstNode = this->nodes[0];
        int firstx = firstNode.getx();
        int firsty = firstNode.gety();
        
        float xlen, ylen, hypotenuse;
        
        if (currentx < firstx)
        {
          xlen = firstx - currentx;
          ylen = firsty - currenty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(hypotenuse, 2));
        }
        else
        {
          xlen = currentx - firstx;
          ylen = currenty - firsty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(-hypotenuse, 2));
        }

        float angle = this->findAngle(xlen, ylen);
        
        link.setRotation(angle);

        this->links.push_back(link);
        break;
      }
      
      Node nextNode = this->nodes[i+1];
      int nextx = nextNode.getx();
      int nexty = nextNode.gety();
      
      float xlen, ylen, hypotenuse;
      
      if (currentx < nextx)
      {
        xlen = nextx - currentx;
        ylen = nexty - currenty;
        hypotenuse = this->findHypotenuse(xlen, ylen);
        link.setSize(sf::Vector2f(hypotenuse, 2));
      }
      else
      {
        xlen = currentx - nextx;
        ylen = currenty - nexty;
        hypotenuse = this->findHypotenuse(xlen, ylen);
        link.setSize(sf::Vector2f(-hypotenuse, 2));
      }

      float angle = this->findAngle(xlen, ylen);
            
      link.setRotation(angle);
      
      this->links.push_back(link);
    }
  }
  
  float findAngle(float adjacent, float opposite)
  {
    float radians = atan(opposite / adjacent);
    return (radians * (180 / 3.14159265358979));
  }
  
  float findHypotenuse(int xlen, int ylen)
  {
    return sqrt((xlen * xlen) + (ylen * ylen));
  }
  
  void shuffleNodes()
  {
    std::random_shuffle(this->nodes.begin(), this->nodes.end());
    
    
    // if (this->shuffleIndex < 1)
    // {
    //   this->shuffleIndex = this->nodes.size()-1;
    // }
    
    // Node temp = this->nodes[this->shuffleIndex];
    // this->nodes[this->shuffleIndex] = this->nodes[this->shuffleIndex-1];
    // this->nodes[this->shuffleIndex-1] = temp;
    // shuffleIndex--;
    // for (int i=0; i<this->nodes.size(); i++)
    // {
    //   std::cout << this->nodes[i].getIndex();
    // }
    // std::cout << std::endl;

    
    // int arrayIndeces[this->nodes.size()];
    // std::vector<int> arrayIndeces;
    // for (int i=0; i<this->nodes.size(); i++)
    // {
    //   // arrayIndeces[i] = this->nodes[i].getIndex();
    //   arrayIndeces.push_back(this->nodes[i].getIndex());
    // }
    // std::next_permutation(arrayIndeces.begin(), arrayIndeces.end());
    
    // std::vector<Node> tempVec = this->nodes;
    // for(int i=0; i<this->nodes.size(); i++)
    // {
    //   tempVec[i] = this->nodes[arrayIndeces[i]];
    //   std::cout << arrayIndeces[i];
    // }
    // std::cout << std::endl;
    // this->nodes = tempVec;
  }
  
  void pickNodesProximity()
  {
    Node firstNode = this->nodes[this->randomTweakIndex];
    int firstx = firstNode.getx();
    int firsty = firstNode.gety();
  
    for (int j=0; j<this->nodes.size()-1; j++)
    {
      if (j != this->randomTweakIndex)
      {
        Node *nextNode = &this->nodes[j];
        int nextx = nextNode->getx();
        int nexty = nextNode->gety();

        int xlen, ylen;
        float distance;
    
        xlen = abs(firstx - nextx);
        ylen = abs(firsty - nexty);
        distance = this->findHypotenuse(xlen, ylen);

        nextNode->setDesirability(distance, desirabilityModifier, 1);
      }
    }
    this->generateLinksDesirability(this->randomTweakIndex);
  }
  
  void solveAnt()
  {
    std::vector<std::vector<Node>> antPaths;
    // 5 ants determine the desirability of nodes based on the distance from their current node
    for (int i=0; i<5; i++)
    {
      std::random_device rd;
      std::default_random_engine eng(rd());
      std::uniform_int_distribution<int> distr(0, this->nodes.size()-1);
      int randomIndex = distr(eng);

      Node firstNode = this->nodes[randomIndex];
      int firstx = firstNode.getx();
      int firsty = firstNode.gety();
    
      for (int j=0; j<this->nodes.size(); j++)
      {
        if (j != randomIndex)
        {
          Node *nextNode = &this->nodes[j];
          int nextx = nextNode->getx();
          int nexty = nextNode->gety();

          int xlen, ylen;
          float distance;
      
          xlen = abs(firstx - nextx);
          ylen = abs(firsty - nexty);
          distance = this->findHypotenuse(xlen, ylen);

          // nextNode->setDesirability(distance, desirabilityModifier, this->getPharamones(firstNode.getIndex(), nextNode->getIndex()));
          // nextNode->setDesirability(distance, desirabilityModifier, 1);
          nextNode->setDesirability(distance, desirabilityModifier, this->pharamones[firstNode.getOtherIndex()][nextNode->getOtherIndex()]);
          // std::cout << this->nodes[j].getDesirability() << ", ";
        }
      }
      // std::cout << std::endl;
    
      // this->generateLinksDesirability(randomIndex);
    
      // this->setState(solved);
      // return;
    
      std::vector<Node> unvisitedNodes = this->nodes;
      unvisitedNodes.erase(unvisitedNodes.begin());
      
      for (int j=0; j<unvisitedNodes.size()-1; j++)
      {
        unvisitedNodes[j].decrementIndex();
      }
      
      std::vector<Node> currentPath;
      currentPath.push_back(firstNode);
      
      // the ants then randomely select a node to travel to based on its desirability until all nodes are explored
      do {
        float bestDesirability = 0.f;
        Node *bestNode;
        for (int k=1; k<unvisitedNodes.size(); k++)
        {
          std::random_device rd2;
          std::default_random_engine eng2(rd2());
          std::uniform_int_distribution<int> distr2(1, 2);
          int randomModifier = distr2(eng2);

          // float total = desirabilityChance * randomModifier * unvisitedNodes[k].getDesirability();
          float total = randomModifier * unvisitedNodes[k].getDesirability();
          // float total = unvisitedNodes[k].getDesirability();
          if (total > bestDesirability)
          {
            bestDesirability = total;
            bestNode = &unvisitedNodes[k];
          }
        }
        currentPath.push_back(*bestNode);
      
        for (int l=bestNode->getIndex()+1; l<unvisitedNodes.size(); l++)
        {
          unvisitedNodes[l].decrementIndex();
        }

        unvisitedNodes.erase(unvisitedNodes.begin() + bestNode->getIndex());
      } while (unvisitedNodes.size() > 0);
      
      // for (int j=0; j<currentPath.size(); j++)
      // {
      //   currentPath[j].setIndex(j);
      // }
      
      antPaths.push_back(currentPath);
    }
    
    // the paths the ants took are then scored based on the total distance the ants traveled
    std::vector<float> scoredPaths;
    for (int i=0; i<antPaths.size(); i++)
    {
      float totalDistance;
      for (int j=0; j<antPaths[i].size(); j++)
      {
        Node currentNode = antPaths[i][j];
        int currentx = currentNode.getx();
        int currenty = currentNode.gety();
        
        float distance;
        if (j == antPaths[i].size()-1)
        {
          Node firstNode = antPaths[i][0];
          int firstx = firstNode.getx();
          int firsty = firstNode.gety();
          
          int xlen = abs(currentx - firstx);
          int ylen = abs(currenty - firsty);
          distance = this->findHypotenuse(xlen, ylen);
        } else {
          Node nextNode = antPaths[i][j+1];
          int nextx = nextNode.getx();
          int nexty = nextNode.gety();
          
          int xlen = abs(currentx - nextx);
          int ylen = abs(currenty - nexty);
          distance = this->findHypotenuse(xlen, ylen);
        }
        
        totalDistance += distance;
      }
      scoredPaths.push_back(totalDistance);
    }

    // the vector of paths is sorted from shortest path taken to longest
    std::sort(scoredPaths.begin(), scoredPaths.end());
    
    // this->generateLinksBestAntTest(antPaths[0]);
    
    float strength = 25.f;
    for (int i=0; i<antPaths.size(); i++)
    {
      for (int j=0; j<antPaths[i].size(); j++)
      {
        if (j < antPaths[i].size()-1)
        {
          // this->pharamones[antPaths[i][j].getIndex()][antPaths[i][j+1].getIndex()] = strength;
          this->setPharamones(antPaths[i][j].getIndex(), antPaths[i][j+1].getIndex(), strength);
        } else {
          // this->pharamones[antPaths[i][j].getIndex()][antPaths[i][0].getIndex()] = strength;
          this->setPharamones(antPaths[i][j].getIndex(), antPaths[i][0].getIndex(), strength);
        }
      }
      strength /= 2;
    }
    
    // this->decrementPharamones();
    
    // this->generateLinksPharamone();

    // this->solveStep++;
    // this->searchedSolutionsVal.setString(std::to_string(this->solveStep));
    // if (this->solveStep >= this->totalSolutions)
    // {
    //   this->solveStep = 0;
    //   this->setState(solved);
    // }
  } 
  
  void generateLinksBestAntTest(std::vector<Node> path)
  {
    this->links.clear();

    for (int i=0; i<path.size(); i++)
    {
      Node currentNode = path[i];
      int currentx = currentNode.getx();
      int currenty = currentNode.gety();
      
      sf::RectangleShape link;
      link.setPosition(sf::Vector2f(currentx+1, currenty));
      link.setFillColor(sf::Color(255, 255, 255, 80));

      if (i == path.size()-1)
      {
        Node firstNode = path[0];
        int firstx = firstNode.getx();
        int firsty = firstNode.gety();
        
        float xlen, ylen, hypotenuse;
        
        if (currentx < firstx)
        {
          xlen = firstx - currentx;
          ylen = firsty - currenty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(hypotenuse, 2));
        }
        else
        {
          xlen = currentx - firstx;
          ylen = currenty - firsty;
          hypotenuse = this->findHypotenuse(xlen, ylen);
          link.setSize(sf::Vector2f(-hypotenuse, 2));
        }

        float angle = this->findAngle(xlen, ylen);
        
        link.setRotation(angle);

        this->links.push_back(link);
        break;
      }
      
      Node nextNode = path[i+1];
      int nextx = nextNode.getx();
      int nexty = nextNode.gety();
      
      float xlen, ylen, hypotenuse;
      
      if (currentx < nextx)
      {
        xlen = nextx - currentx;
        ylen = nexty - currenty;
        hypotenuse = this->findHypotenuse(xlen, ylen);
        link.setSize(sf::Vector2f(hypotenuse, 2));
      }
      else
      {
        xlen = currentx - nextx;
        ylen = currenty - nexty;
        hypotenuse = this->findHypotenuse(xlen, ylen);
        link.setSize(sf::Vector2f(-hypotenuse, 2));
      }

      float angle = this->findAngle(xlen, ylen);
            
      link.setRotation(angle);
      
      this->links.push_back(link);
    }
  }
  
  void increaseSolveStep()
  {
    this->solveStep++;
    this->searchedSolutionsVal.setString(std::to_string(this->solveStep));
    if (this->solveStep >= this->totalSolutions)
    {
      this->solveStep = 0;
      this->setState(solved);
    }
  }
  
  void initPharamones()
  {
    for (int i=0; i<this->numNodes; i++)
    {
      std::vector<float> currentPharamones;
      for (int j=0; j<this->numNodes; j++)
      {
        currentPharamones.push_back(0.01);
      }
      this->pharamones.push_back(currentPharamones);
    }
  }
  
  void decrementPharamones()
  {
    for (int i=0; i<this->numNodes; i++)
    {
      for (int j=0; j<this->numNodes; j++)
      {
        if (this->pharamones[i][j] < 0.015)
        {
          this->pharamones[i][j] = 0;
        } else {
          this->pharamones[i][j] /= 2;
        }
      }
    }
  }
  
  // float getPharamones(int i, int j)
  // {
  //   if (i < j)
  //   {
  //     return this->pharamones[i][j];
  //   } else {
  //     return this->pharamones[j][i];
  //   }
  // }
  
  void setPharamones(int i, int j, float value)
  {
    this->pharamones[i][j] = value;
    this->pharamones[j][i] = value;
  }
  
  // void chooseNextNode(std::vector<Node> unvisitedNodes)
  // {
  //   srand(time(NULL));
  //   float bestDesirability = 0;
  //   Node *bestNode;
  //   for (int i=0; i<unvisitedNodes.size(); i++)
  //   {
  //     int randomModifier = rand() % 10;
  //     float total = randomModifier * unvisitedNodes[i].getDesirability();
  //     if (total > bestDesirability)
  //     {
  //       bestDesirability = total;
  //       bestNode = &unvisitedNodes[i];
  //     }
  //   }
  //   this->nodes.push_back(*bestNode);
  //   unvisitedNodes.erase(unvisitedNodes.begin() + bestNode->getIndex());
  //   std::cout << unvisitedNodes.size() << " " << this->nodes.size() << std::endl;
    
  //   if (unvisitedNodes.size() > 1) this->chooseNextNode(unvisitedNodes);
  // }
  
  void drawNodes()
  {
    for (int i=0; i<this->nodes.size(); i++)
    {
      this->window->draw(this->nodes[i].getCircle());
    }
  }
  
  void drawLinks()
  {
    for (int i=0; i<this->links.size(); i++)
    {
      this->window->draw(this->links[i]);
    }
  }
  
  void drawUI()
  {
    this->window->draw(this->UIShader);
    this->window->draw(this->solveButton);
    this->window->draw(this->solveText);
    this->window->draw(this->solveAntButton);
    this->window->draw(this->solveAntText);
    this->window->draw(this->genSliderOutline);
    this->window->draw(this->genSliderBox);
    this->window->draw(this->generateButton);
    this->window->draw(this->genSliderNumText);
    this->window->draw(this->genSliderDescriptionText);
    this->window->draw(this->numNodesText);
    this->window->draw(this->numNodesVal);
    this->window->draw(this->totalPossibleSolutionsText);
    this->window->draw(this->totalPossibleSolutionsVal);
    this->window->draw(this->searchedSolutionsText);
    this->window->draw(this->searchedSolutionsVal);
    this->window->draw(this->bestPathText);
    this->window->draw(this->bestPathVal);
    this->window->draw(this->resetButton);
    this->window->draw(this->resetText);
    
    if (this->isNodes)
    {
      this->window->draw(this->tweakButton);
      this->window->draw(this->tweakButtonText);
    }
  }
  
  void drawUITweak()
  {
    this->window->draw(this->UIShader);
    this->window->draw(this->powerSliderOutline);
    this->window->draw(this->powerSliderBox);
    this->window->draw(this->powerSliderDescriptionText);
  }
  
  void printPharamones()
  {
    for (int i=0; i<this->numNodes; i++)
    {
      for (int j=0; j<this->numNodes; j++)
      {
        std::cout << this->pharamones[i][j] << ", ";
      }
      std::cout << std::endl;
    }  
    std::cout << std::endl;
  }
};
