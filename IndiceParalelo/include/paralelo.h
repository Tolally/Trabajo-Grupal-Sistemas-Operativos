#pragma once
#include "../include/create_index_paralel.h"
#include <iostream>
#include <thread>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <limits>
#include <atomic>
#include <chrono>

namespace fs = std::filesystem;

using namespace std;

static unsigned int obtenerThreadsHardware();

int pedirNumeroThreads(unsigned int maxThreads);

long long procesarLote(const vector<fs::directory_entry>& lote,
                       int loteId,
                       int threadId,
                       const fs::path& carpetaIndices,
                       string &out);

void mergeIndex (Index &dest, const Index &src);

void invertidoParalelo();

int main(int argc, char** argv);
