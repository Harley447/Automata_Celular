#include <iostream>

class Cell {
  public:
    int state;
    Creature creature;
};

class Creature {
  public:
    int age;
    int die() {
      return 0;
    }
    int move() {
      return 0;
    }
};

// Clase de presas
class Prey: public Creature {
  public:
    int reproduction() {
      return 0;
    }
};

// Clases de los depredadores
class Predator: public Creature {
  public:
    int energy;
  };

int main() {
  int M = 10;
  int N = 10;

  Cell* grid = new Cell[M*N]();

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      int pos = i * N + j;
      std::cout << grid[pos].state;
    }
    std::cout << std::endl;
  }
 
  return 0;
}
