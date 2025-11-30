#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm> 
#include <queue>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

static string consultarMotor(const string& palabra);

static string agregarTiemposAlJSON(const string& json, long tiempoCache_us, long tiempoMotor_us, bool esHit);

static void insertarEnCache(const string& palabra, const string& resultado);

void iniciarServidorCache();

int main();