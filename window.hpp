#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>

#include "node.hpp"

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 1000;

const float desirabilityModifier = 0.15;
const float desirabilityChance = 0.05;

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
      case solving:
        this->pollEventsSolving();
        this->solve();
        this->shuffleNodes();
        break;
      case solvingAnt:
        this->pollEventsSolving();
        this->solveAnt();
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
    this->drawUI();
    
    this->window->display();
  }
private:
  //state engine
  enum states{
    build,
    buttonClicked,
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
  sf::Color colButton = sf::Color(40, 40, 30, 255);
  sf::Color colButtonHighlight = sf::Color(110, 110, 100, 255);
  sf::Color colButtonActive = sf::Color(160, 160, 150, 255);
  
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
  
  //logic
  std::vector<Node> nodes;
  std::vector<sf::RectangleShape> links;
  std::vector<Node> visitedNodesInOrder;
  
  long totalSolutions;
  int solveStep = 0;
  int shuffleIndex;
  
  int genSliderNum = 10;
  
  float prevBestDistance = 9999999999.f;
  
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
      this->window->setFramerateLimit(10);
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
    
    this->genSliderBox.setPosition(sf::Vector2f(12, 11));
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
      this->genSliderBox.setPosition(sf::Vector2f(this->mousePosWindow.x - 5, 11));
      this->genSliderNum = (this->mousePosWindow.x - 12) / 2;
      this->genSliderNumText.setString(std::to_string(this->genSliderNum));
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
    
    this->solveStep++;
    this->searchedSolutionsVal.setString(std::to_string(this->solveStep));
    if (this->solveStep >= this->totalSolutions)
    {
      this->solveStep = 0;
      this->setState(solved);
    }
  }
  
  void generateLinks()
  {
    this->links.clear();
    // std::vector<sf::RectangleShape> currentLinks;

    for (int i=0; i<this->nodes.size(); i++)
    {
      Node currentNode = this->nodes[i];
      int currentx = currentNode.getx();
      int currenty = currentNode.gety();
      
      sf::RectangleShape link;
      link.setPosition(sf::Vector2f(currentx+1, currenty));
      link.setFillColor(sf::Color(255, 255, 255, 80));

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
  
  void solveAnt()
  {
    srand(time(NULL));
    int randomIndex = rand() % this->nodes.size();
    Node firstNode = this->nodes[randomIndex];
    int firstx = firstNode.getx();
    int firsty = firstNode.gety();
    
    std::vector<Node> unvisitedNodes;

    for (int i=0; i<this->nodes.size()-1; i++)
    {
      if (i != randomIndex)
      {
        Node nextNode = this->nodes[i];
        int nextx = nextNode.getx();
        int nexty = nextNode.gety();

        int xlen, ylen;
        float hypotenuse;
      
        xlen = abs(firstx - nextx);
        ylen = abs(firsty - nexty);
        hypotenuse = this->findHypotenuse(xlen, ylen);

        // if (currentx < nextx)
        // {
        //   xlen = nextx - currentx;
        //   ylen = nexty - currenty;
        // }
        // else
        // {
        //   xlen = currentx - nextx;
        //   ylen = currenty - nexty;
        // }
        // hypotenuse = this->findHypotenuse(xlen, ylen);
      
        nextNode.setDesirability(hypotenuse, desirabilityModifier);
      }
    }
    
    unvisitedNodes = this->nodes;
    this->nodes.clear();
    // this->chooseNextNode(unvisitedNodes);
    this->nodes.push_back(firstNode);
    
    do {
      srand(time(NULL));
      float bestDesirability = 100.f;
      Node *bestNode;
      for (int i=1; i<unvisitedNodes.size(); i++)
      {
        int randomModifier = rand() % 10;
        float total = desirabilityChance * randomModifier * unvisitedNodes[i].getDesirability();
        if (total < bestDesirability)
        {
          bestDesirability = total;
          bestNode = &unvisitedNodes[i];
        }
      }
      this->nodes.push_back(*bestNode);
      
      for (int i=bestNode->getIndex()+1; i<unvisitedNodes.size(); i++)
      {
        unvisitedNodes[i].decrementIndex();
      }

      unvisitedNodes.erase(unvisitedNodes.begin() + bestNode->getIndex());
    } while (unvisitedNodes.size() > 1);
    
    for (int i=0; i<this->nodes.size(); i++)
    {
      this->nodes[i].setIndex(i);
    }

    this->generateLinks();
        
    this->solveStep++;
    this->searchedSolutionsVal.setString(std::to_string(this->solveStep));
    if (this->solveStep >= this->totalSolutions)
    {
      this->solveStep = 0;
      this->setState(solved);
    }
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
  }
};
