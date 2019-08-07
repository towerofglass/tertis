#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <time.h>

SDL_Rect atlas[23] = {
    {0, 0, 24, 24}, // blocks
    {24, 0, 24, 24},
    {48, 0, 24, 24},
    {72, 0, 24, 24},
    {96, 0, 24, 24},
    {120, 0, 24, 24},
    {144, 0, 24, 24},
    {168, 0, 24, 24},
    {0, 24, 240, 24}, // press enter
    {0, 48, 12, 24}, // 0
    {12, 48, 12, 24}, // 1
    {24, 48, 12, 24}, // 2
    {36, 48, 12, 24}, // 3
    {48, 48, 12, 24}, // 4
    {60, 48, 12, 24}, // 5
    {72, 48, 12, 24}, // 6
    {84, 48, 12, 24}, // 7
    {96, 48, 12, 24}, // 8
    {108, 48, 12, 24}, // 9
    {144, 48, 72, 24}, // lines
    {0, 72, 72, 24}, // level
    {72, 72, 48, 24}, // next
    {120, 72, 120, 24}, // game over
};
SDL_Rect textdest[5] = {
    {48, 240, 240, 24}, // press enter
    {108, 192, 120, 24}, // game over
    {456, 48, 48, 24}, // next
    {408, 312, 72, 24}, // level
    {408, 384, 72, 24} // lines
};
int figures[7][4] = {
    {2, 3, 4, 5}, //O
    {3, 1, 5, 7}, //I
    {2, 0, 3, 5}, //S
    {2, 1, 3, 4}, //Z
    {2, 0, 4, 5}, //L
    {3, 1, 4, 5}, //J
    {2, 0, 3, 4} //T
};
int field[200] = {0};

struct piece {
    int x, y, spr;
} ci[4], ti[4], ni[4];
bool keys[5] = {false};

enum states {
    g_title, g_play
} cs;

bool checkmove() {
    for (int i = 0; i < 4; i++) {
        if ((ci[i].x < 0) || (ci[i].x > 9) || (ci[i].y > 19) || (field[ci[i].y * 10 + ci[i].x] != 0)) return false;
    }
    return true;
}

bool newpiece() {
    bool res = true;
    int r = rand() / (RAND_MAX + 1.0) * 7;
    for (int i = 0; i < 4; i++) {
        ci[i] = ni[i];
        ni[i].x = figures[r][i] % 2 + 4;
        ni[i].y = figures[r][i] / 2;
        ni[i].spr = r + 1;
        if (field[ci[i].y * 10 + ci[i].x] != 0) res = false;
    }
    return res;
}

int main() {
    //// initializing
    srand(time(NULL));
    cs = g_title;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Window* gWindow = SDL_CreateWindow("tertis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_SHOWN);
    if (!gWindow) {
        printf("error creating window: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    //    SDL_Renderer* gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);

    if (!gRenderer) {
        printf("error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(gWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    //// load image
    SDL_Texture* gTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load("tertis.png");
    if (loadedSurface == NULL) {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
    } else {
        gTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
        loadedSurface = NULL;
    }
    if (gTexture == NULL) {
        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    //// game code
    bool running = true;
    SDL_Event e;
    bool rotate = false;
    int dx = 0;
    unsigned int currtime = SDL_GetTicks();
    unsigned int lasttime = currtime;
    int delay = 800;
    newpiece();
    newpiece();
    int level = 0;
    int lines = 0;
    SDL_Rect dgts = {552, 312, 12, 24};

    //// main loop
    while (running) {
        currtime = SDL_GetTicks();

        //// input
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                if ((e.key.keysym.sym == SDLK_UP)&&(!keys[0])) {
                    rotate = true;
                    keys[0] = true;
                } else if ((e.key.keysym.sym == SDLK_LEFT)&&(!keys[1])) {
                    dx = -1;
                    keys[1] = true;
                } else if ((e.key.keysym.sym == SDLK_RIGHT)&&(!keys[2])) {
                    dx = 1;
                    keys[2] = true;
                } else if ((e.key.keysym.sym == SDLK_DOWN)&&(!keys[3])) {
                    keys[3] = true;
                    delay = 0;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                else if (e.key.keysym.sym == SDLK_RETURN) keys[4] = true;
            }
            if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == SDLK_UP) keys[0] = false;
                else if (e.key.keysym.sym == SDLK_LEFT) keys[1] = false;
                else if (e.key.keysym.sym == SDLK_RIGHT) keys[2] = false;
                else if (e.key.keysym.sym == SDLK_DOWN) keys[3] = false;
                else if (e.key.keysym.sym == SDLK_RETURN) keys[4] = false;
            }
        }

        //// update
        if (cs == g_play) {
            for (int i = 0; i < 4; i++) {
                ti[i] = ci[i];
                ci[i].x += dx;
            }
            dx = 0;
            if (rotate) {
                int x = ci[0].x - ci[1].y + ci[0].y;
                int y = ci[0].y + ci[1].x - ci[0].x;
                ci[1].x = x;
                ci[1].y = y;
                x = ci[0].x - ci[2].y + ci[0].y;
                y = ci[0].y + ci[2].x - ci[0].x;
                ci[2].x = x;
                ci[2].y = y;
                x = ci[0].x - ci[3].y + ci[0].y;
                y = ci[0].y + ci[3].x - ci[0].x;
                ci[3].x = x;
                ci[3].y = y;
            }
            rotate = false;
            if (!checkmove()) for (int i = 0; i < 4; i++) ci[i] = ti[i];

            if (currtime - lasttime > delay) {
                lasttime = currtime;
                for (int i = 0; i < 4; i++) {
                    ti[i] = ci[i];
                    ci[i].y += 1;
                }
                if (!checkmove()) {
                    for (int i = 0; i < 4; i++) field[ti[i].y * 10 + ti[i].x] = ti[i].spr;
                    if (!newpiece()) {
                        cs = g_title;
                    }
                    int k = 0;
                    for (int i = 19; i >= 0; i--) {
                        int count = 0;
                        for (int j = 0; j < 10; j++) {
                            if (field[i * 10 + j] != 0) count++;
                            field[(i + k) * 10 + j] = field[i * 10 + j];
                        }
                        if (count == 10) {
                            k++;
                            lines++;
                            if ((lines / 10 < 10)&&(lines / 10 != level)) level = lines / 10;
                        }
                    }
                    if (k > 0) {
                        for (int i = 0; i < 10; i++) {
                            field[i] = 0;
                        }
                    }
                    delay = 800 - level * 60;
                }
            }
        } else if (cs == g_title) {
            if (keys[4]) {
                cs = g_play;
                lines = 0;
                level = 0;
                delay = 800;
                for (int i = 0; i < 200; i++) field[i] = 0;
            }
        }

        //// draw
        SDL_RenderClear(gRenderer);
        for (int i = 0; i < 20; i++) { // vert glass
            SDL_Rect dest = {24, 48 + i * 24, 24, 24};
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
            dest.x = 48 + 240;
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
        }
        for (int i = 0; i < 12; i++) { // bottom glass
            SDL_Rect dest = {24 + i * 24, 528, 24, 24};
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
        }
        for (int i = 0; i < 7; i++) { // next
            SDL_Rect dest = {384, 72 + i * 24, 24, 24};
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
            dest.x = 384 + i * 24;
            dest.y = 216;
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
            dest.x = 552;
            dest.y = 72 + i * 24;
            SDL_RenderCopy(gRenderer, gTexture, &atlas[0], &dest);
        }
        SDL_RenderCopy(gRenderer, gTexture, &atlas[21], &textdest[2]); // text next
        SDL_RenderCopy(gRenderer, gTexture, &atlas[20], &textdest[3]); // text level
        SDL_RenderCopy(gRenderer, gTexture, &atlas[19], &textdest[4]); // text lines

        /// draw digits
        dgts.x = 552;
        dgts.y = 312;
        SDL_RenderCopy(gRenderer, gTexture, &atlas[level + 9], &dgts);
        dgts.y = 384;
        if (lines < 10) {
            dgts.x = 552;
            SDL_RenderCopy(gRenderer, gTexture, &atlas[lines + 9], &dgts);
        } else {
            int offset = 0;
            int t = lines;
            while (t > 10) {
                dgts.x = 552 - offset * 12;
                SDL_RenderCopy(gRenderer, gTexture, &atlas[t % 10 + 9], &dgts);
                t /= 10;
                offset++;
            }
            dgts.x = 552 - offset * 12;
            SDL_RenderCopy(gRenderer, gTexture, &atlas[t % 10 + 9], &dgts);

        }

        if (cs == g_play) {
            for (int i = 0; i < 200; i++) {
                if (field[i] != 0) {
                    SDL_Rect dest = {i % 10 * 24 + 48, i / 10 * 24 + 48, 24, 24};
                    SDL_RenderCopy(gRenderer, gTexture, &atlas[field[i]], &dest);
                }
            }
            for (int i = 0; i < 4; i++) {
                SDL_Rect dest = {ci[i].x * 24 + 48, ci[i].y * 24 + 48, 24, 24};
                SDL_RenderCopy(gRenderer, gTexture, &atlas[ci[i].spr], &dest);
                dest.x = ni[i].x * 24 + 360; //x-4
                dest.y = ni[i].y * 24 + 96;
                SDL_RenderCopy(gRenderer, gTexture, &atlas[ni[i].spr], &dest);
            }
        } else if (cs == g_title) {
            SDL_RenderCopy(gRenderer, gTexture, &atlas[8], &textdest[0]);
            if (lines > 0) SDL_RenderCopy(gRenderer, gTexture, &atlas[22], &textdest[1]);
        }

        SDL_RenderPresent(gRenderer);
    }

    //// clean up resources before exiting
    SDL_DestroyTexture(gTexture);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
