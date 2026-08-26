#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>

using namespace std;

int main ()
{
    int N;
    cout << "Ingrese el tamaño de la matriz NxN: ";
    cin >> N;
    
    int Mundo[N+2][N+2];
  
    for (int i = 1; i < N+1; i++)
    {
        for (int j = 1; j < N+1; j++)
        {
            Mundo[i][j] = rand() % 3;
        }
    }
    for (int i = 1; i < N+1; i++)
    {
        for (int j = 1; j < N+1; j++)
        {
            cout << Mundo[i][j] << "\t";
        }
        cout << endl;
    }
    return 0;
}
