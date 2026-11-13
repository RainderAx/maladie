#include "include/systems/Game.hpp"
#include "include/ui/RaylibGame.hpp"
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--terminal") {
        Game game;
        game.run();
        return 0;
    }
    return runRaylibGame();
}
