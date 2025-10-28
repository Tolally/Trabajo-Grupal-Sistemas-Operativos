#pragma once
#include <string>

// Mensajes del protocolo (formatos)
static const char SEP = '|';

inline std::string make_msg(const std::string& type) {
    return type;
}

inline std::string make_msg2(const std::string& type, const std::string& a) {
    return type + std::string(1, SEP) + a;
}

inline std::string make_msg3(const std::string& type, const std::string& a, const std::string& b) {
    return type + std::string(1, SEP) + a + std::string(1, SEP) + b;
}

inline std::string make_msg4(const std::string& type, const std::string& a, const std::string& b, const std::string& c) {
    return type + std::string(1, SEP) + a + std::string(1, SEP) + b + std::string(1, SEP) + c;
}