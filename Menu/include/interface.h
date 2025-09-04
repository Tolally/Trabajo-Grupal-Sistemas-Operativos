#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace std;

// Adelantos para tipos usados
struct Argumentos;
struct UsuarioMP;

// Muestra y gestiona el menú principal. Retorna la opción elegida.
int mostrarMenuPrincipal(const string& username, const string& perfil);

// Muestra la pantalla de palíndromo, permitiendo ingresar texto y validar o cancelar.
void pantallaPalindromo();

// Muestra la pantalla para calcular f(x) = x^2 + 2x + 8 y permite volver.
void pantallaFuncionCuadratica();

// Muestra la interfaz de conteo de texto. Si rutaArchivo no es vacío, intenta usarlo como fuente.
void pantallaConteoTexto(const string& rutaArchivo);
