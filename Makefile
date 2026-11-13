BUILD_DIR := build

all:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_GAME=ON
	cmake --build $(BUILD_DIR)

run: all
	@if [ -x ./$(BUILD_DIR)/chroniques_de_la_faille ]; then ./$(BUILD_DIR)/chroniques_de_la_faille; else ./$(BUILD_DIR)/chroniques_de_la_faille_terminal; fi

terminal:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_GAME=OFF
	cmake --build $(BUILD_DIR) --target chroniques_de_la_faille_terminal

run-terminal: terminal
	./$(BUILD_DIR)/chroniques_de_la_faille_terminal

test:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_GAME=OFF
	cmake --build $(BUILD_DIR) --target roguelite_tests
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR) chroniques_de_la_faille chroniques_de_la_faille_terminal

.PHONY: all run terminal run-terminal test clean
