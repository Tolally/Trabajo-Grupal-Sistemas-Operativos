#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

// Carga el mapa de libros (id -> nombre)
bool cargarMapaLibros(const string& rutaMapa);

// Carga el índice invertido desde archivo .idx
bool cargarIndiceInvertido(const string& rutaIndice);

// Busca una palabra y devuelve JSON con TOP K resultados
string buscarPalabra(const string& palabra, int topK);

// Inicia el servidor del motor
void iniciarServidorMotor();

int main();