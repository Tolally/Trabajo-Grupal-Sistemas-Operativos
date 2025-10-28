#pragma once
#include <iostream>
#include <thread>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <limits>
#include <atomic>

namespace fs = std::filesystem;

using namespace std;

static unsigned int obtenerThreadsHardware();

int pedirNumeroThreads(unsigned int maxThreads);

static void procesarLote(const vector<fs::directory_entry>& lote, int loteId, int threadId);

void invertidoParalelo();

int main(int argc, char** argv);
