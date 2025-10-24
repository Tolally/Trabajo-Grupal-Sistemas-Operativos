#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>

using namespace std;

// Calcula f(x) = x*x + 2x + 8 y devuelve el resultado en double.
double calcularFx(double x);

// Lee una matriz desde archivo con separador 'sep' (string). Valida formato rectangular.
bool leerMatriz(const string& ruta, const string& sep, vector<vector<double>>& M, string& err);

// Verifica que la matriz es de orden NxN
bool esMatrizDeNxN(const string& ruta, int N, const string& sep);

// Multiplica A (n x n) por B (n x n) y deja resultado en C.
bool multiplicarNxN(
    const vector<vector<double>>& A,
    const vector<vector<double>>& B,
    vector<vector<double>>& C,
    string& err);

// Imprime matriz con separador sep.
void imprimirMatriz(const vector<vector<double>>& M, const string& sep);
