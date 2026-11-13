#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Terminal {
inline const std::string cyan = "\033[36m";
inline const std::string purple = "\033[35m";
inline const std::string red = "\033[31m";
inline const std::string green = "\033[32m";
inline const std::string yellow = "\033[33m";
inline const std::string reset = "\033[0m";

class EndOfInput final : public std::runtime_error {
public:
    EndOfInput() : std::runtime_error("Fin de l'entrée standard.") {}
};

inline void title(const std::string& text) {
    std::cout << "\n" << purple
              << "╔══════════════════════════════════════════════════════════════╗\n║"
              << std::setw(34 + static_cast<int>(text.size()) / 2) << text
              << std::setw(30 - static_cast<int>(text.size()) / 2) << "║\n"
              << "╚══════════════════════════════════════════════════════════════╝"
              << reset << "\n";
}

inline int ask(int min, int max) {
    while (true) {
        std::string line;
        try {
            if (!std::getline(std::cin, line)) throw EndOfInput{};
        } catch (const std::ios_base::failure&) {
            if (std::cin.bad() || (std::cin.eof() && line.empty())) throw EndOfInput{};
        }

        std::istringstream input(line);
        int choice = 0;
        if (input >> choice && choice >= min && choice <= max) return choice;
        std::cout << "Choix invalide. Recommencez : ";
    }
}
}
