#include "../include/matematicas.h"

// Calcula f(x) = x*x + 2x + 8 y devuelve el resultado en double.
double calcularFx(double x) {
    return x * x + 2.0 * x + 8.0;
}

// Lee una matriz desde archivo con separador 'sep' (string). Valida formato rectangular.
static bool leerMatriz(const string& ruta, const string& sep, vector<vector<double>>& M, string& err) {
    ifstream f(ruta);
    if (!f.is_open()) { err = "No se pudo abrir archivo: " + ruta; return false; }
    string linea; size_t cols = 0; size_t fila = 0;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        vector<double> row;
        size_t start = 0;
        while (start <= linea.size()) {
            size_t pos = linea.find(sep, start);
            string token = (pos == string::npos) ? linea.substr(start) : linea.substr(start, pos - start);
            if (!token.empty()) {
                try { row.push_back(stod(token)); }
                catch (...) { err = "Valor no numérico en fila " + to_string(fila+1); return false; }
            } else {
                // token vacío -> tratar como 0 (o error). Optamos por error para ser estrictos.
                err = "Elemento vacío en fila " + to_string(fila+1);
                return false;
            }
            if (pos == string::npos) break;
            start = pos + sep.size();
        }
        if (row.empty()) continue;
        if (cols == 0) cols = row.size();
        if (row.size() != cols) { err = "Formato no rectangular en fila " + to_string(fila+1); return false; }
        M.push_back(move(row));
        fila++;
    }
    if (M.empty()) { err = "Matriz vacía: " + ruta; return false; }
    return true;
}

// Multiplica A (n x n) por B (n x n) y deja resultado en C.
static bool multiplicarNxN(const vector<vector<double>>& A, const vector<vector<double>>& B, vector<vector<double>>& C, string& err) {
    size_t n = A.size();
    if (n == 0) { err = "A vacía"; return false; }
    if (A.size() != A[0].size()) { err = "A no es NxN"; return false; }
    if (B.size() != B[0].size()) { err = "B no es NxN"; return false; }
    if (A[0].size() != B.size()) { err = "Dimensiones incompatibles"; return false; }

    C.assign(n, vector<double>(n, 0.0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t k = 0; k < n; ++k) {
            double aik = A[i][k];
            for (size_t j = 0; j < n; ++j) {
                C[i][j] += aik * B[k][j];
            }
        }
    }
    return true;
}

// Imprime matriz con separador sep.
static void imprimirMatriz(const vector<vector<double>>& M, const string& sep) {
    cout.setf(ios::fixed); cout.precision(6);
    for (size_t i = 0; i < M.size(); ++i) {
        for (size_t j = 0; j < M[i].size(); ++j) {
            if (j) cout << sep;
            cout << M[i][j];
        }
        cout << "\n";
    }
}

// Punto de entrada del multiplicador de matrices.
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