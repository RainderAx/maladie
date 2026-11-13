#include "../include/systems/Game.hpp"
#include "../include/core/Terminal.hpp"
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    } catch (const Terminal::EndOfInput&) {
        std::cout << "\nFin de l'entrée. La partie se termine proprement.\n";
    }
    return 0;
}
