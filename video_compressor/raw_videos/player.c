#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {
    uint8_t fps;
    uint8_t width;
    uint8_t height;
    uint8_t grayscale;
} metadata;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: %s arquivo.bit\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("Erro abrindo arquivo");
        return 1;
    }

    metadata meta;
    fread(&meta, sizeof(metadata), 1, f);

    size_t pixels = meta.width * meta.height;
    size_t bytes_per_frame = pixels / 8;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "BIT Player",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        meta.width,
        meta.height,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        meta.width,
        meta.height
    );

    uint8_t *frame_data = malloc(bytes_per_frame);
    uint32_t *pixels_buffer = malloc(pixels * sizeof(uint32_t));

    int delay = 1000 / meta.fps;
    int running = 1;
    SDL_Event event;

    while (running && fread(frame_data, 1, bytes_per_frame, f) == bytes_per_frame) {

        size_t pixel_index = 0;

        for (size_t i = 0; i < bytes_per_frame; i++) {
            for (int bit = 7; bit >= 0; bit--) {

                uint8_t value = (frame_data[i] >> bit) & 1;

                if (value)
                    pixels_buffer[pixel_index] = 0xFFFFFFFF; // branco
                else
                    pixels_buffer[pixel_index] = 0x00000000; // preto

                pixel_index++;
            }
        }

        SDL_UpdateTexture(texture, NULL, pixels_buffer, meta.width * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        Uint32 start = SDL_GetTicks();
        while (SDL_GetTicks() - start < delay) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    running = 0;
            }
        }
    }

    free(frame_data);
    free(pixels_buffer);
    fclose(f);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
