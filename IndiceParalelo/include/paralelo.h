#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <limits>

using namespace std;

static unsigned int obtenerThreadsHardware();

int pedirNumeroThreads(unsigned int maxThreads);

void invertidoParalelo();

int main(int argc, char** argv);
