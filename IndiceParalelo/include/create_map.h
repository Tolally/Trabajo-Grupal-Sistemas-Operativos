#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <regex>
#include <exception>

namespace fs = std::filesystem;

// limpia título (ej. "002 - Frankenstein..." -> "Frankenstein...")
std::string limpiarTitulo(const std::string& name);

// extrae prefijo numérico (o -1 si no)
long extraccionNumeroPrefijo(const std::string& s);

// Genera el mapa (archivo out) a partir de los archivos en dir.
// Si loteSize > 0 también devuelve en memoria los lotes (vector de vectores de directory_entry).
std::vector<std::vector<fs::directory_entry>> create_map(const std::string& dir,
                                                         const std::string& out,
                                                         int loteSize = 6);


// Carga el MAPA-LIBROS (formato "id; title") y devuelve un mapa title -> id
std::unordered_map<std::string,std::string> cargarMapaLibros(const std::string& mapaPath);