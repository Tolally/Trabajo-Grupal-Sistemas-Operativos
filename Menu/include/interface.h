#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <limits>
#include <filesystem>
#include <sys/stat.h>

using namespace std;

// Muestra y gestiona el menú principal. Retorna la opción elegida.
int mostrarMenuPrincipal(const string& username, const string& perfil);

// Muestra la pantalla de palíndromo, permitiendo ingresar texto y validar o cancelar.
void pantallaPalindromo();

// Muestra la pantalla para calcular f(x) = x^2 + 2x + 8 y permite volver.
void pantallaFuncionCuadratica();

// Muestra la interfaz de conteo de texto. Si rutaArchivo no es vacío, intenta usarlo como fuente.
void pantallaConteoTexto(const string& rutaArchivo);

// Interfaz para crear índice invertido: pide archivo .idx, carpeta libros y ejecuta programa externo.
void pantallaCrearIndiceInvertido();

// Interfaz para crear índice invertido paralelo: parecido a lo anterior pero llama al ejecutable de la variable de entorno INDICE_INVET_PARALELO.
void pantallaCrearIndiceInvertidoParalelo();

// Muestra la interfaz de llamar a multiplicador de matrices. Retorna verdadero o falso si se desea o no multiplicar
void multiplicarMatrices();

// Verifica que la ruta de la matriz ingresada existe.
bool verificarRutaMatriz(string& rutaMatriz);

// Limpia la consola usando el comando del sistema según plataforma.
void limpiarConsola();
