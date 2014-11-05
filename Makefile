game_of_life: game_of_life.cpp
	$(CXX) $$(pkg-config --cflags --libs sdl2) game_of_life.cpp -o game_of_life
