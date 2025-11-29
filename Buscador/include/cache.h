#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

static string consultarMotor(const string& palabra);

void iniciarServidorCache();

int main();