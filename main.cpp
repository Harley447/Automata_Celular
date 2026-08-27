#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>

class Cell {
  public:
  	bool alive;
    virtual ~Cell() {}

    virtual bool step() {
      return 0;
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
  	const int maxChildren=4;
    int age;

  	Prey(){ //Necesita la ubicacion en la grilla
  		age = 0;
      alive = true;
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
      alive = true;
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
            break;
          default:
            matrix[i] = new Predator();
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
    }

    void print() {
      for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
          std::cout << matrix[i*N+j]->getName();
        }
        std::cout << std::endl;
      }
    }

    // Se hizo secuencial para evitar definir normas extra :c
    void step() {
      int** matrixChecked = new Bool*[M*N]
      for (int i = 0; i < M; i++) {
        for (int j =0; j < N; j++) {
          int index = i * N + j;
          int type = matrix[index]->getName();
          
          matrixChecked[index] = true;
          // Indices calculados para luego revisarlos :D
          // En orden son Arriba, Abajo, Izquierda, Derecha
          int neighbours[4] = {(M*N + (i-1) * N) % (M*N) + j - 1,
            ((i+1) * N) % M*N + j - 1,
            i * N + (N + j - 1)%N,
            i * N + (j + 1) % N};
          // Muerte por hambre o vejez
          if (matrix[index]->step() and type != 0) {
            delete matrix[index];
            matrix[index] = new Cell();
          }
          switch (type) {
            // Logica de presa
            case 1:
              // Revisar si se puede mover
              int libres = 0;
              for (int neighbour: neighbours) {
                if (matrix[neighbour]->getName() == 0) {
                  libres++;
                }
              }

              break;
            // Logica de cazador
            case 2:
              break;
            // Celda vacia
            default:
              break;
          }
        }
      }
    }
};

int main() {
  Grid grid = Grid();
  grid.print();
  grid.clean();
  return 0;
}
