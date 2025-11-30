#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits>

using namespace std;

void limpiarConsola();

string buscar(const string& palabra);

void iniciarBuscador();

int main();