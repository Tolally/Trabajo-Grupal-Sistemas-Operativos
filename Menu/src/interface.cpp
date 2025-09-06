#include "../include/interface.h"
#include "../include/texto.h"
#include "../include/matematicas.h"

namespace {
    // Lee un entero de forma segura desde stdin con validación básica.
    int leerEnteroSeguro() {
        int v;
        while (true) {
            if (cin >> v) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return v; }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Entrada inválida. Intente nuevamente: ";
        }
    }
}

// Muestra y gestiona el menú principal. Retorna la opción elegida.
int mostrarMenuPrincipal(const string& username, const string& perfil) {
    cout << "==============================\n";
    cout << "       MENÚ PRINCIPAL\n";
    cout << "==============================\n";
    cout << "User: " << username << " (" << perfil << ")\n";
    cout << "\n0) Salir\n";
    cout << "1) Admin Users (En construcción)\n";
    cout << "2) Multi Matrices NxN\n";
    cout << "3) Juego\n";
    cout << "4) ¿Es palíndromo?\n";
    cout << "5) Calcula f(x) = x^2 + 2x + 8\n";
    cout << "6) Conteo sobre texto\n";
    cout << "\nOpción : ";
    return leerEnteroSeguro();
}

// Muestra la pantalla de palíndromo, permitiendo ingresar texto y validar o cancelar.
void pantallaPalindromo() {
    limpiarConsola();
    cout << "=== ¿Es palíndromo? ===\n";
    cout << "Ingrese un texto: ";
    string texto; getline(cin, texto);
    cout << "\n1) Validar     2) Cancelar\n";
    cout << "Opción : ";
    int op = 0; 
    while (true) {
        if (cin >> op) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); break; }
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Entrada inválida. Intente nuevamente: ";
    }
    limpiarConsola();
    if (op == 1) {
        bool ok = esPalindromo(texto);
        cout << (ok ? "El texto SÍ es palíndromo." : "El texto NO es palíndromo.") << "\n";
        cout << "\n(Enter para volver)";
        cin.get();
    } else {
        cout << "Operación cancelada.\n";
    }
}

// Muestra la pantalla para calcular f(x) = x^2 + 2x + 8 y permite volver.
void pantallaFuncionCuadratica() {
    limpiarConsola();
    cout << "=== f(x) = x^2 + 2x + 8 ===\n";
    cout << "Indique X (número real): ";
    double x;
    while (!(cin >> x)) { 
        cin.clear(); 
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        cout << "Entrada inválida, intente nuevamente: "; 
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    double y = calcularFx(x);
    cout.setf(ios::fixed); cout.precision(4);
    cout << "\nResultado: f(" << x << ") = " << (x*x) << " + " << (2.0*x) << " + 8 = " << y << "\n";
    cout << "\n(Enter para VOLVER)";
    cin.get();
}

// Muestra la interfaz de conteo de texto. Si rutaArchivo no es vacío, intenta usarlo como fuente.
void pantallaConteoTexto(const string& rutaArchivo) {
    limpiarConsola();
    cout << "=== Conteo sobre texto ===\n";
    string contenido, err;
    if (!rutaArchivo.empty()) {
        if (!leerArchivoTexto(rutaArchivo, contenido, err)) {
            cout << "No se pudo leer el archivo indicado (-f). Detalle: " << err << "\n";
        }
    }
    if (contenido.empty()) {
        cout << "Ingrese texto (una línea). Si desea, presione Enter para línea vacía y volver.\n";
        cout << "> ";
        getline(cin, contenido);
    }
    if (contenido.empty()) {
        cout << "(Sin contenido para analizar)\n";
        cout << "\n(Enter para volver)";
        cin.get();
        return;
    }
    auto c = contarTexto(contenido);
    cout << "\nConteo:\n";
    cout << "- vocales: " << c.vocales << "\n";
    cout << "- consonantes: " << c.consonantes << "\n";
    cout << "- caracteres especiales: " << c.especiales << "\n";
    cout << "- palabras: " << c.palabras << "\n";
    cout << "\n(Enter para volver)";
    cin.get();
}

void limpiarConsola(){
	#ifdef _WIN32
		(void)system("cls");
	#else
		(void)system("clear");
	#endif
}
