#include "include/systems/Game.hpp"
#include "include/ui/RaylibGame.hpp"
#include "include/core/Terminal.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--terminal") {
        try {
            Game game;
            game.run();
        } catch (const Terminal::EndOfInput&) {
            std::cout << "\nFin de l'entrée. La partie se termine proprement.\n";
        }
        return 0;
    }
    return runRaylibGame();
}
