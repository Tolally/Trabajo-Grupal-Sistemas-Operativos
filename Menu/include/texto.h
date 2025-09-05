#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <stdexcept>

using namespace std;

// Resultado de conteo sobre texto: vocales, consonantes, especiales y palabras.
struct ConteoTexto {
    long vocales = 0;
    long consonantes = 0;
    long especiales = 0;
    long palabras = 0;
};

// Devuelve true si el texto es palíndromo (ignorando espacios, tildes y mayúsculas/minúsculas y signos)
// false en caso contrario.
bool esPalindromo(const string& texto);

// Calcula conteo de vocales, consonantes, especiales y palabras sobre el texto indicado.
ConteoTexto contarTexto(const string& texto);

// Lee archivo de texto completo a un string. Devuelve true/false según éxito y deja contenido en out.
bool leerArchivoTexto(const string& ruta, string& outContenido, string& outError);

