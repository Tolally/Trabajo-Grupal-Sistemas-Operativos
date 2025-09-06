#include "../include/matematicas.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 4) {
        cerr << "Uso: multi <rutaA> <rutaB> <sep>\n";
        return 1;
    }
    string rutaA = argv[1];
    string rutaB = argv[2];
    string sep = argv[3];
    if (sep.empty()) sep = ",";

    vector<vector<double>> A, B, C;
    string err;
    if (!leerMatriz(rutaA, sep, A, err)) { cerr << err << "\n"; return 1; }
    if (!leerMatriz(rutaB, sep, B, err)) { cerr << err << "\n"; return 1; }
    if (!multiplicarNxN(A, B, C, err)) { cerr << err << "\n"; return 1; }
    imprimirMatriz(C, sep);
    return 0;
}

