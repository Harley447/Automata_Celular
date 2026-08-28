#pragma once
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <any>

class Location{
	public:
		int row;
		int column; // Coordenadas
		Location(int row, int column){
			this->column=column;
			this->row=row;
		}
		int getRow(){
			return row;
		}
		int getCol(){
			return column;
		}
};
class Creature {

  	public:
  	bool alive;
    int age;
    int name;
    //Location location;
    Creature(){
    	//this->location=location();
    	alive=true;
	}	
//	void setLocation(Location newLocation){
//		
//	}
    int die() {
      alive=false;
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
  
  //ASumiendo que Cell son todas las celdas
class Cell {
  public:
    int state;
    int depth, width;
    std::vector<std::vector<std::any>> field;
    Cell(int size_grilla){
    	this->depth=size_grilla;
    	this->width=size_grilla;
    	field.resize(depth, std::vector<std::any>(width));
    	 
	}
	void clear()
    {
        for(int row = 0; row < depth; row++) {
            for(int col = 0; col < width; col++) {
                field[row][col] = nullptr;
            }
        }
    }
    void clear(Location location)
    {
        field[location.getRow()][location.getCol()] = nullptr;
    }
    void place(Creature animal, int row, int col)
    {
        place(animal, Location (row, col));
    }
    void place(Creature animal, Location location)
    {
        field[location.getRow()][location.getCol()] = animal;
    }
    std::any getObjectAt(int row, int col) {
    return field[row][col];
	}
	std::any getObjectAt(Location location) {
    return getObjectAt(location.getRow(), location.getCol());
	}
};



//int main() {
//  int M = 10;
//  int N = 10;
//
//  //Cell grid = new Cell[M*N]{N};
//  Cell grid(N);
//
//  for (int i = 0; i < M; i++) {
//    for (int j = 0; j < N; j++) {
//      int pos = i * N + j;
//      //std::cout << grid[pos].state;
//    }
//    std::cout << std::endl;
//  }
////free(grid);
//  return 0;
//}
