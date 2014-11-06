#include <iostream>
#include <memory>
#include <cstdint>

#include "SDL2/SDL.h"

typedef std::unique_ptr<uint32_t[]> cells;

const int SCALE = 25;
const int WIDTH = 30;
const int HEIGHT = 30;
const int BOARD_SIZE = WIDTH * HEIGHT;
const size_t BOARD_SIZE_BYTES = BOARD_SIZE * sizeof(uint32_t);
const uint32_t OFF = 0x00000000;
const uint32_t ON  = 0xFFFFFFFF;

int mod(int a, int b) {
    int ret = a % b;
    if (ret < 0) {
        ret += b;
    }
    return ret;
}

void log(const std::string& message) {
    std::cout << "[game_of_life] " << message << std::endl;
}

uint32_t get_cell(const cells& generation, int x, int y) {
    return generation[y * WIDTH + x];
}

void set_cell(cells& generation, int x, int y, uint32_t value) {
    generation[y * WIDTH + x] = value;
}

void draw(cells& current_generation, int x, int y) {
    int scaled_x = x/SCALE;
    int scaled_y = y/SCALE;
    uint32_t cell_value = get_cell(current_generation, scaled_x, scaled_y);
    uint32_t next_state = cell_value ? OFF : ON;
    set_cell(current_generation, scaled_x, scaled_y, next_state);
}

size_t neighbors(const cells& current_generation, int x, int y) {
    size_t count = 0;

    // Check cell on the right
    if (current_generation[mod((x+1), WIDTH) + mod((y+0), HEIGHT) * WIDTH])
        count++;

    // Check cell on bottom right
    if (current_generation[mod((x+1), WIDTH) + mod((y+1), HEIGHT) * WIDTH])
        count++;

    // Check cell on the bottom
    if (current_generation[mod((x+0), WIDTH) + mod((y+1), HEIGHT) * WIDTH])
        count++;

    // Check cell on bottom left
    if (current_generation[mod((x-1), WIDTH) + mod((y+1), HEIGHT) * WIDTH])
        count++;

    // Check cell on the left
    if (current_generation[mod((x-1), WIDTH) + mod((y+0), HEIGHT) * WIDTH])
        count++;

    // Check cell on the top left
    if (current_generation[mod((x-1), WIDTH) + mod((y-1), HEIGHT) * WIDTH])
        count++;

    // Check cell on the top
    if (current_generation[mod((x+0), WIDTH) + mod((y-1), HEIGHT) * WIDTH])
        count++;

    // Check cell on the top right
    if (current_generation[mod((x+1), WIDTH) + mod((y-1), HEIGHT) * WIDTH])
        count++;

    return count;
}


void tick(cells& current_generation, cells& next_generation) {
    memcpy(next_generation.get(), current_generation.get(), BOARD_SIZE_BYTES);
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            uint32_t current_cell = current_generation[y * WIDTH + x];
            int live_neighbors = neighbors(current_generation, x, y);
            if (current_cell && !((live_neighbors == 2) || (live_neighbors == 3))) {
                next_generation[y * WIDTH + x] = OFF;
            }
            if (!current_cell && live_neighbors == 3) {
                next_generation[y * WIDTH + x] = ON;
            }
        }
    }
    std::swap(current_generation, next_generation);
}

int main(int argc, char* argv[]) {
    log("initializing video");
    SDL_Init(SDL_INIT_VIDEO);

    log("creating window");
    SDL_Window *window = SDL_CreateWindow(
        "Game of Life",
        100,
        100,
        WIDTH * SCALE,
        HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    log("creating renderer");
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    log("clearing renderer");
    SDL_RenderClear(renderer);

    log("creating base texture");
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    log("creating boards");
    std::unique_ptr<uint32_t[]> current_generation(new uint32_t[BOARD_SIZE]);
    std::unique_ptr<uint32_t[]> next_generation(new uint32_t[BOARD_SIZE]);

    memset(current_generation.get(), 0, BOARD_SIZE_BYTES);
    memset(next_generation.get(), 0, BOARD_SIZE_BYTES);

    bool quit = false;
    bool drawing = false;

    SDL_Event event;

    while (!quit) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                draw(current_generation, event.button.x, event.button.y);
                drawing = true;
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON(1)) {
                    draw(current_generation, event.motion.x, event.motion.y);
                    drawing = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                drawing = false;
                break;
            case SDL_FINGERMOTION:
                break;
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_q)
                    quit = true;
                else if (event.key.keysym.sym == SDLK_SPACE) {
                    log("computing next generation");
                    tick(current_generation, next_generation);
                }
                drawing = true;
                break;
            case SDL_KEYUP:
                drawing = false;
                break;
            default:
                break;
        }

        if (drawing) {
            SDL_UpdateTexture(texture, NULL, current_generation.get(), WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            drawing = false;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
