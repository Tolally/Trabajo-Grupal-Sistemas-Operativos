#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <cctype>
#include <unistd.h>
#include <cctype>
#include <string>


namespace fs = std::filesystem;

// Tipo público para índice: palabra -> (archivo -> frecuencia)
using Index = std::map<std::string, std::map<std::string,int>>;

// Procesa un lote (lista de directory_entry) y devuelve el índice parcial (no guarda a disco)
Index crearIndiceInvertidoPorLote(const std::vector<fs::directory_entry>& lote);

// Guarda un índice en disco (formato simple). Devuelve true si tuvo éxito.
bool guardarIndice(const std::string& nombreArchivo, const Index& indice);

// Crear índice a partir de una carpeta (uso histórico). Devuelve true si se creó correctamente.
bool crearIndiceInvertido(const std::string& nombreArchivo, const std::string& carpetaLibros);