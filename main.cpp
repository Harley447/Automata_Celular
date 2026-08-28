#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>

class Cell {
  public:
    virtual ~Cell() {}
    virtual bool canBreed() {
      return false;
    };

    virtual bool step() {
      return false;
    };
 
    virtual int getName() {
      return 0;
    };
};
// Clase de presas
class Prey: public Cell {
  public:
    const int umbralBreeding=3;
  	const int maxAge=30;
  	//const int maxChildren=4;
    int age;

  	Prey(){ //Necesita la ubicacion en la grilla
  		age = 0;
	  }

  	bool increaseAge(){
  		age++;

      if (age >= maxAge) {
        return 0;
      } else {
        return 1;
      }
    }

    bool canBreed(){
    	return age >= umbralBreeding;
	  }
	
    bool step(){
		  return increaseAge();
	  }

	  int getName(){
		  return 1;
	  }
};

// Clases de los depredadores
class Predator: public Cell {
  public:
  	const int umbralBreeding = 10;
  	const int maxAge = 80;
  	const int maxChildren = 2;
    const int startEnergy = 4;
    int age;
    int energy;    

    Predator(){ //Necesita la ubicacion en la grilla
  		age = 0;
      energy = startEnergy;
	  }

  	bool increaseAge() {
  		age++;
  		if(age>maxAge) {
  			return 0;
		  } else {
        return 1;
      }
    }

    bool increaseHunger() {
      energy--;
      if(energy<=0) {
        return 0;
      } else {
        return 1;
      }
    }

    bool canBreed(){
    	return age >= umbralBreeding;
	  }

	  bool step(){
		  return increaseAge() * increaseHunger();
	  }

	  int getName(){
		  return 2;
	  }
};

class Grid {
  private:
    int M, N;
    Cell** matrix;
  public:
    int totalPrey = 0; int totalPredator = 0;

    Grid(int width = 10, int height = 10) {
      M = width;
      N = height;
      matrix = new Cell*[M*N];

      for (int i = 0; i < M*N; i++) {
        switch (rand()%3) {
          case 0:
            matrix[i] = new Cell();
            break;
          case 1:
            matrix[i] = new Prey();
            totalPrey++;
            break;
          default:
            matrix[i] = new Predator();
            totalPredator++;
            break;
        }
      }
    }

    void clean() {
      // Limpiamos la memoria luego de acabar
      for (int i = 0; i < M*N; i++) {
        delete matrix[i];
      }
      delete[] matrix;
      totalPrey = 0;
      totalPredator = 0;
    }

    void print() {
      std::cout << "Total depredadores:" << totalPredator << std::endl;
      std::cout << "Total presas:" << totalPrey << std::endl;


      for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
          std::cout << matrix[i*N+j]->getName();
        }
        std::cout << std::endl;
      }
    }

    // Se hizo secuencial para evitar definir normas extra :c
    void step() {
      bool* matrixChecked = new bool[M*N]();
      for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
          int index = i * N + j;
          
          if (matrixChecked[index]) {
            continue;
          }

          matrixChecked[index] = true;

          int type = matrix[index]->getName();
          
          // Indices calculados para luego revisarlos :D
          // En orden son Arriba, Abajo, Izquierda, Derecha
          int neighbours[4] = {
            ((i - 1 + M) % M) * N + j,
            ((i + 1) % M) * N + j,
            i * N + ((N + j - 1) % N),
            i * N + ((j + 1) % N)};

          // Muerte por hambre o vejez
          if (!matrix[index]->step() and type != 0) {
            delete matrix[index];
            matrix[index] = new Cell();
            type = 0;
            switch (type) {
              case 1: {
                totalPrey--;
                break; 
              }
              default: {
                totalPredator++;
                break;
              }
            }
          }

          switch (type) {
            // Logica de presa
            case 1: {
              // Revisar si se puede mover y si puede tener coito :D
              int empty = 0;
              int emptyNeighbours[4];

              bool thereIsCouple = false;
              for (int neighbour: neighbours) {
                switch (matrix[neighbour]->getName()) {
                  case 0:
                    emptyNeighbours[empty] = neighbour; 
                    empty++;
                    break;
                  case 1:
                    thereIsCouple = true;
                    break;
                  default:
                    break;
                }
              }

              if (empty > 0) {
                int chosenIndex = emptyNeighbours[rand() % empty];
                delete matrix[chosenIndex];
                matrix[chosenIndex] = matrix[index];
                matrixChecked[chosenIndex] = true;
                if (thereIsCouple &&  matrix[index]->canBreed()) {
                  matrix[index] = new Prey();
                  totalPrey++;
                } else {
                  matrix[index] = new Cell();
                }
              }
              break;
            }
            // Logica de cazador
            case 2: {
              // Revisar si se puede mover y si puede tener coito :D
              int empty = 0;
              int emptyNeighbours[4];

              bool thereIsCouple = false;
              int food = 0;
              int foodNeighbours[4];
              for (int neighbour: neighbours) {
                switch (matrix[neighbour]->getName()) {
                  case 0:
                    emptyNeighbours[empty] = neighbour; 
                    empty++;
                    break;
                  case 1:
                    foodNeighbours[food] = neighbour;
                    food++;
                    break;
                  default:
                    thereIsCouple = true;
                    break;
                }
              }
              
              if (food + empty > 0) {
                int chosenIndex;
                if (food > 0) {
                  chosenIndex = foodNeighbours[rand()%food];
                  totalPrey--;
                } else {
                  thereIsCouple = false;
                  chosenIndex = emptyNeighbours[rand()%empty];
                }
                delete matrix[chosenIndex];
                matrix[chosenIndex] = matrix[index];
                matrixChecked[chosenIndex] = true;
                if (thereIsCouple &&  matrix[index]->canBreed()) {
                  matrix[index] = new Predator();
                  totalPredator++;
                } else {
                  matrix[index] = new Cell();
                }
              }
              break;
            }
            // Celda vacia
            default: {
              break;
            }
          }
        }
      }
      delete[] matrixChecked;
    }
};

int main() {
  return 0;
}
