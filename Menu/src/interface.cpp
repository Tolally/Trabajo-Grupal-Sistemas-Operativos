#include "../include/interface.h"
#include "../include/texto.h"
#include "../include/matematicas.h"
#include <iostream>


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
    if (perfil == "ADMIN")
        cout << "1) Admin Users\n";
    cout << "2) Multi Matrices NxN\n";
    cout << "3) Juego\n";
    cout << "4) ¿Es palíndromo?\n";
    cout << "5) Calcula f(x) = x^2 + 2x + 8\n";
    cout << "6) Conteo sobre texto\n";
    cout << "7) Crea índice invertido\n";
    cout << "8) Crea índice invertido paralelo\n";
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

// Interfaz para crear índice invertido: pide archivo .idx, carpeta libros y ejecuta programa externo.
void pantallaCrearIndiceInvertido() {
    std::string nombreArchivo, pathCarpeta;
    namespace fs = std::filesystem;

    // --- 3a) Pedir nombre de archivo .idx ---
    while (true) {
        std::cout << "Ingrese nombre del archivo a crear (debe terminar en .idx, o '0' para cancelar): ";
        std::cin >> nombreArchivo;

        if (nombreArchivo == "0") {
            std::cout << "Operación cancelada por el usuario.\n";
            return;
        }

        if (nombreArchivo.size() >= 4 && nombreArchivo.substr(nombreArchivo.size() - 4) == ".idx") {
            break;
        }

        std::cerr << "Error: el archivo debe tener extensión .idx\n";
    }

    // --- 3b) Pedir path de carpeta ---
    while (true) {
        std::cout << "Ingrese el path de la carpeta con los libros (o '0' para cancelar): ";
        std::cin >> pathCarpeta;

        if (pathCarpeta == "0") {
            std::cout << "Operación cancelada por el usuario.\n";
            return;
        }

        if (fs::exists(pathCarpeta) && fs::is_directory(pathCarpeta)) {
            break;
        }

        std::cerr << "Error: la carpeta no existe o no es válida\n";
    }

    // --- 2) Variable de entorno ---
    const char* progPath = std::getenv("CREATE_INDEX");
    if (!progPath) {
        std::cerr << "Error: Variable de entorno CREATE_INDEX no definida.\n";
        return;
    }
    
    // --- 3c) Ejecutar programa externo ---
    std::string comando = std::string(progPath) + " " + nombreArchivo + " " + pathCarpeta;
    std::cout << "Creando índice invertido...\n";
    int resultado = std::system(comando.c_str());

    if (resultado != 0) {
        std::cout << "Error al ejecutar el programa externo.\n";
    }
}

// Interfaz para crear índice invertido paralelo: pide archivo .idx, carpeta de libros y ejecuta el programa externo indicado en INDICE_INVET_PARALELO.
void pantallaCrearIndiceInvertidoParalelo() {
    std::string nombreArchivo, pathCarpeta;
    namespace fs = std::filesystem;

    while (true) {
        std::cout << "Ingrese nombre del archivo a crear (debe terminar en .idx, o '0' para cancelar): ";
        std::cin >> nombreArchivo;
        if (nombreArchivo == "0") { std::cout << "Operación cancelada por el usuario.\n"; return; }
        if (nombreArchivo.size() >= 4 && nombreArchivo.substr(nombreArchivo.size() - 4) == ".idx") break;
        std::cerr << "Error: el archivo debe tener extensión .idx\n";
    }

    while (true) {
        std::cout << "Ingrese el path de la carpeta con los libros (o '0' para cancelar): ";
        std::cin >> pathCarpeta;
        if (pathCarpeta == "0") { std::cout << "Operación cancelada por el usuario.\n"; return; }
        if (fs::exists(pathCarpeta) && fs::is_directory(pathCarpeta)) break;
        std::cerr << "Error: la carpeta no existe o no es válida\n";
    }

    const char* progPath = std::getenv("INDICE_INVET_PARALELO");
    if (!progPath) {
        std::cerr << "Error: Variable de entorno INDICE_INVET_PARALELO no definida.\n";
        return;
    }

    std::string comando = std::string(progPath) + " " + nombreArchivo + " " + pathCarpeta;
    std::cout << "Creando índice invertido paralelo...\n";
    int resultado = std::system(comando.c_str());
    if (resultado != 0) {
        std::cout << "Error al ejecutar el programa externo.\n";
    }
}


void multiplicarMatrices(){
    limpiarConsola();
    string Nstr, sep, rutaMatrizA, rutaMatrizB;
    cout << "=== Multiplicador de Matrices ===\n";
    cout << "Para ejecutar esta aplicación, debe ingresar: \n";
    cout << "Dimensión Matriz Cuadrada NxN (N, o 0 para cancelar): ";
    getline(cin, Nstr);
    if (Nstr == "0") {
        cout << "Operación cancelada.\n";
        return;
    }
    int N = stoi(Nstr);

    cout << "Separador del archivo de matrices (o 0 para cancelar): ";

    getline(cin, sep);
    if (sep == "0") {
        cout << "Operación cancelada.\n";
        return;
    }

    while (true) {
        cout << "Ruta Matriz 1 (o 0 para cancelar): ";
        getline(cin, rutaMatrizA);
        if (rutaMatrizA == "0") {
            cout << "Operación cancelada.\n";
            return;
        }
        if (!esMatrizDeNxN(rutaMatrizA, N, sep))
            cout <<"Matriz no válida o no es de " << N << "x" << N << ", por favor ingresar nuevamente\n";
        else break;
    }
    cout << "Primera matriz cargada con éxito \n\n";
    while (true) {
        cout << "Ruta Matriz 2 (o 0 para cancelar): ";
        getline(cin, rutaMatrizB);
        if (rutaMatrizB == "0") {
            cout << "Operación cancelada.\n";
            return;
        }
        if (!esMatrizDeNxN(rutaMatrizB, N, sep))
            cout <<"Matriz no válida o no es de " << N << "x" << N << ", por favor ingresar nuevamente\n";
        else break;
    }
    cout << "Segunda matriz cargada con éxito \n";
    while (true) {
    cout << "\n1) Multiplicar     2) Cancelar\n";
    cout << "Opción : ";
    int op;
    if (cin >> op) {

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (op) {
            case 1: {
                const char* rutaMulti = getenv("MULTI_M");
                string multiplicacion = string(rutaMulti) + " " + rutaMatrizA + " " + rutaMatrizB + " " + sep;
                limpiarConsola();
                system(multiplicacion.c_str());
                return;
            }
            case 2:
                cout << "Operación cancelada.\n";
                return;
            default:
                cout << "Opción inválida. Intente nuevamente.\n";
                break;
        }
    } else {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Entrada inválida. Intente nuevamente.\n";
    }
}
}

bool verificarRutaMatriz(string& rutaMatriz){
    struct stat buffer;
    return (stat(rutaMatriz.c_str(), &buffer) == 0);
}

// Función para mostrar la pantalla del juego en el menú
void pantallaJuego() {
    limpiarConsola();
    cout << "=== JUEGO MULTIPLAYER ===" << endl;
    cout << "1) Iniciar Servidor" << endl;
    cout << "2) Conectar como Cliente" << endl;
    cout << "3) Volver al Menú Principal" << endl;
    cout << "\nOpción: ";
    
    int opcion;
    cin >> opcion;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    switch (opcion) {
        case 1:
            iniciarServidorJuego();
            break;
        case 2:
            iniciarClienteJuego();
            break;
        case 3:
            // Simplemente volver
            break;
        default:
            cout << "Opción inválida." << endl;
            cout << "\n(Enter para volver)";
            cin.get();
            break;
    }
}

// Función para iniciar el servidor del juego
void iniciarServidorJuego() {
    limpiarConsola();
    cout << "=== INICIANDO SERVIDOR DEL JUEGO ===" << endl;
    
    // Cargar configuración desde .env
    const char* port_env = getenv("GAME_PORT");
    const char* board_env = getenv("GAME_BOARD_SIZE");
    const char* dice_env = getenv("DICE_SIDES");
    
    int port = port_env ? stoi(port_env) : 8080;
    int board_size = board_env ? stoi(board_env) : 50;
    int dice_sides = dice_env ? stoi(dice_env) : 6;
    
    cout << "Configuración:" << endl;
    cout << " - Puerto: " << port << endl;
    cout << " - Tamaño del tablero: " << board_size << endl;
    cout << " - Caras del dado: " << dice_sides << endl;
    cout << "\nEl servidor se está iniciando..." << endl;
    cout << "Presiona Ctrl+C para detener el servidor" << endl;
    cout << "=====================================" << endl;
    
    // Ejecutar el servidor en un proceso separado
    const char* server_bin = getenv("GAME_SERVER");
    if (!server_bin) {
        server_bin = "bin/server";
    }
    
    int result = system(server_bin);
    if (result != 0) {
        cout << "Error al iniciar el servidor. Asegúrate de que el binario 'server' existe." << endl;
    }
    
    cout << "\n(Enter para volver)";
    cin.get();
}

// Función para iniciar el cliente del juego
void iniciarClienteJuego() {
    limpiarConsola();
    cout << "=== CONECTAR COMO CLIENTE ===" << endl;
    
    string host, port_str, username;
    
    cout << "Host del servidor [127.0.0.1]: ";
    getline(cin, host);
    if (host.empty()) host = "127.0.0.1";
    
    cout << "Puerto [9000]: ";
    getline(cin, port_str);
    if (port_str.empty()) port_str = "9000";
    
    cout << "Nombre de usuario: ";
    getline(cin, username);
    if (username.empty()) username = "Jugador";
    
    cout << "\nConectando a " << host << ":" << port_str << " como " << username << "..." << endl;
    cout << "=====================================" << endl;
    
    // Construir comando para ejecutar el cliente
    string comando = "bin/client " + host + " " + port_str + " " + username;
    
    int result = system(comando.c_str());
    if (result != 0) {
        cout << "Error al conectar. Verifica que el servidor esté ejecutándose." << endl;
    }
    
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
