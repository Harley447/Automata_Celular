#include <iostream>
#include <cstdlib>
#include <cstring>
#include "Entidades.h"
using namespace std;

int main ()
{
    int N, MAX_ITER, iter=0, vecinos=0;
    cout << "Ingrese el tamaño de la matriz NxN: ";
    cin >> N;
    
    int* Mundo =
        (int*)malloc((N + 2) * (N + 2) * sizeof(int));

    int* copiaMundo =
        (int*)malloc((N + 2) * (N + 2) * sizeof(int));
    
    // Inicializar toda la matriz
    for (int i = 0; i < (N + 2) * (N + 2); i++)
    {
        Mundo[i] = -1;
        copiaMundo[i] = -1;
    }

    // Inicializar únicamente el mundo real
    for (int i = 1; i <= N; i++)
    {
        for (int j = 1; j <= N; j++)
        {
            //Mundo[i * (N + 2) + j] = rand() % 3;
//            int random=rand()%3;
//            if(random==2){
//            	Predator pred;
//            	Mundo[i * (N + 2) + j] =pred.getName();
//        	}
//            if(random==1){
//            	Prey prey;
//            	Mundo[i * (N + 2) + j] =prey.getName();
//       		 }
//            if(random==0){
//            	Mundo[i * (N + 2) + j] =0;
//			}
			if(i==2 && j==2){
				Prey prey;
            	Mundo[i * (N + 2) + j] =prey.getName();
			}
            else{
            	Mundo[i * (N + 2) + j] =0;
			}
        }
    }

    for (int i = 1; i < N + 1; i++)
    {
        for (int j = 1; j < N + 1; j++)
        {
            cout << Mundo[i * (N + 2) + j] << "\t";
        }

        cout << endl;
    }
    cout << "Ingrese el número de iteraciones: ";
    cin >> MAX_ITER;
    while (iter < MAX_ITER)
    {
        // --------------------------------------
        // COPIAR EL MUNDO ACTUAL
        // --------------------------------------

        memcpy(copiaMundo, Mundo, (N + 2) * (N + 2) * sizeof(int));


        for (int i = 1; i <= N; i++)
        {
            for (int j = 1; j <= N; j++)
            {
                int posicionActual = i * (N + 2) + j;
                

                int animal = Mundo[posicionActual];
	
				//Instanciar Creature existente en la posición actual, creature.move()
				// Para obtener que hay en la celda hace falta arreglar Location, que le da
				// Coordenadas a todas los animales
				
                if (animal == 1 || animal == 2)
                {
                    // Arreglos para almacenar
                    // las posiciones libres
                    int posicionesI[8];
                    int posicionesJ[8];

                    int libres = 0;


                    for (int di = -1; di <= 1; di++)
                    {
                        for (int dj = -1; dj <= 1; dj++)
                        {
                            // No contar la propia celda
                            if (di == 0 && dj == 0)
                                continue;

                            int ni = i + di;
                            int nj = j + dj;

                            int posicionVecino =
                                ni * (N + 2) + nj;

                            if (Mundo[posicionVecino] == 0)
                            {
                                posicionesI[libres] = ni;
                                posicionesJ[libres] = nj;

                                libres++;
                            }
                        }
                    }


                    if (libres > 0)
                    {
                        // Elegir aleatoriamente una
                        int elegido = rand() % libres;

                        int nuevaI = posicionesI[elegido];
                        int nuevaJ = posicionesJ[elegido];

                        copiaMundo[
                            nuevaI * (N + 2) + nuevaJ
                        ] = animal;


                        copiaMundo[
                            i * (N + 2) + j
                        ] = 0;
                    }
                }
            }
        }

        memcpy(Mundo, copiaMundo, (N + 2) * (N + 2) * sizeof(int));

        iter++;

        cout << "\n============================";
        cout << "\nEPOCA: " << iter;
        cout << "\n============================\n";

        for (int i = 1; i <= N; i++)
        {
            for (int j = 1; j <= N; j++)
            {
                cout << Mundo[i * (N + 2) + j] << "\t";
            }

            cout << endl;
        }
    }


    free(Mundo);
    free(copiaMundo);
    


    return 0;
}
