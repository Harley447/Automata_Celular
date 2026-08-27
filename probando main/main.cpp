#include <iostream>
#include <cstdlib>
#include <cstring>
class Creature {

  	public:
  	bool alive;
    int age;
    Creature(){
    	alive=true;
	}
	

    int die() {
      return 0;
    }
    void move();
    int getName();

};
// Clase de presas
class Prey: public Creature {
  public:
	const int umbral_breeding=3;
  	const int max_age=30;
  	const int max_hijos=4;
  	Prey(){ //Necesita la ubicacion en la grilla
  		age=0;
	  }
  	void aumentar_Edad(){
  		age++;
  		if(age>max_age){
  			die();
		  }
	}
    void reproduction() { //Se reproduce aleatoriamente si el residuo es 0
      int probabilidad=rand() % 5;
      if(probabilidad%2==0){
      	//Requiere conocer todas las 8 celdas contiguas para decidir donde generar en nuevo Prey
      	// int? free_cell= xyzw
      	//Prey child(free_cell)
	  }
    }
    bool canBreed(){
    	return age >= umbral_breeding;
	}
	void move(){
		aumentar_Edad();
	}
	int getName(){
		return 1;
	}
};

// Clases de los depredadores
class Predator: public Creature {
  public:
  	const int umbral_breeding=10;
  	const int max_age=80;
  	const int max_hijos=2;
    int energy=4;
      	
	Predator(){ //Necesita la ubicacion en la grilla
  		age=0;
	  }
  	void aumentar_Edad(){
  		age++;
  		if(age>max_age){
  			die();
		  }
	}
	void aumentar_Hambre(){
  		energy--;
  		if(energy<=0){
  			die();
		  }
	}
    void reproduction() { //Se reproduce aleatoriamente si el residuo es 0
      int probabilidad=rand() % 8;
      if(probabilidad%3==0){
      	//Requiere conocer todas las 8 celdas contiguas para decidir donde generar en nuevo Prey
      	// int? free_cell= xyzw
      	//Predator child(free_cell)
	  }
    }
    bool canBreed(){
    	return age >= umbral_breeding;
	}
	void move(){
		aumentar_Edad();
	}
	int getName(){
		return 2;
	}
  };
class Cell {
  public:
    int state;
    //vector<Creature> animales; 
    // Hace falta un vector para guardar a creature
    Creature creature;
    Cell(int size_grilla){
    	for(int i=0;i<size_grilla;i++)
    	
    	animales.push_back(Creature()); 
	}
};



int main() {
  int M = 10;
  int N = 10;

  Cell* grid = new Cell[M*N](M*N);

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      int pos = i * N + j;
      std::cout << grid[pos].state;
    }
    std::cout << std::endl;
  }
	free(grid);
  return 0;
}
