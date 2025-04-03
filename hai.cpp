#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <ctime>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = 20;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* headUpTexture = nullptr;
SDL_Texture* headDownTexture = nullptr;
SDL_Texture* bodyStraightTexture = nullptr;
SDL_Texture* bodyTurnTexture = nullptr;
SDL_Texture* tailTexture = nullptr;
SDL_Texture* tristanaTexture = nullptr;
SDL_Texture* pizzaTexture = nullptr;
SDL_Texture* beerTexture = nullptr;
Mix_Music* backgroundMusic = nullptr;
TTF_Font* font = nullptr;
SDL_Color textColor = { 255, 255, 255, 255 };

int score = 0;
bool running = true;
bool inMenu = true;
int speed = 100;

struct Point {
    int x, y;
};

std::vector<Point> snake = { {10, 10}, {9, 10}, {8, 10} };
Point pizza = { 5, 5 };
std::vector<Point> beer;
int dx = 1, dy = 0;

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    TTF_Init();

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    headUpTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("head_up.png"));
    headDownTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("head_down.png"));
    bodyStraightTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("body_straight.png"));
    bodyTurnTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("body_turn.png"));
    tailTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("tail.png"));
    tristanaTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("Tristana_51.png"));
    pizzaTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("pizza.png"));
    beerTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("beer.png"));

    backgroundMusic = Mix_LoadMUS("newjeans.wav");
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }

    font = TTF_OpenFont("ShinyCrystal-Yq3z4.ttf", 24);
}

void spawnPizza() {
    pizza.x = rand() % (SCREEN_WIDTH / CELL_SIZE);
    pizza.y = rand() % (SCREEN_HEIGHT / CELL_SIZE);
}

void spawnBeer() {
    Point obs = { rand() % (SCREEN_WIDTH / CELL_SIZE), rand() % (SCREEN_HEIGHT / CELL_SIZE) };
    beer.push_back(obs);
}

void moveSnake() {
    Point newHead = { snake[0].x + dx, snake[0].y + dy };

    if (newHead.x < 0) newHead.x = SCREEN_WIDTH / CELL_SIZE - 1;
    if (newHead.x >= SCREEN_WIDTH / CELL_SIZE) newHead.x = 0;
    if (newHead.y < 0) newHead.y = SCREEN_HEIGHT / CELL_SIZE - 1;
    if (newHead.y >= SCREEN_HEIGHT / CELL_SIZE) newHead.y = 0;

    for (size_t i = 1; i < snake.size(); i++) {
        if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
            running = false;
            return;
        }
    }

    for (const auto& obs : beer) {
        if (newHead.x == obs.x && newHead.y == obs.y) {
            running = false;
            return;
        }
    }

    snake.insert(snake.begin(), newHead);
    if (newHead.x == pizza.x && newHead.y == pizza.y) {
        score += 10;
        speed = std::max(30, speed - 5);
        spawnPizza();
        spawnBeer();
    }
    else {
        snake.pop_back();
    }
}

void render() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tristanaTexture, NULL, NULL);

    for (size_t i = 0; i < snake.size(); i++) {
        SDL_Rect rect = { snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
        if (i == 0) {
            SDL_RenderCopy(renderer, (dy == -1) ? headUpTexture : headDownTexture, NULL, &rect);
        }
        else if (i == snake.size() - 1) {
            SDL_RenderCopy(renderer, tailTexture, NULL, &rect);
        }
        else {
            SDL_RenderCopy(renderer, bodyStraightTexture, NULL, &rect);
        }
    }

    SDL_Rect pizzaRect = { pizza.x * CELL_SIZE, pizza.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_RenderCopy(renderer, pizzaTexture, NULL, &pizzaRect);

    for (const auto& obs : beer) {
        SDL_Rect obsRect = { obs.x * CELL_SIZE, obs.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
        SDL_RenderCopy(renderer, beerTexture, NULL, &obsRect);
    }

    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = { 10, 10, scoreSurface->w, scoreSurface->h };
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_RenderPresent(renderer);
}

void renderMenu() {
    SDL_RenderClear(renderer);
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Press SPACE to Start", textColor);
    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect messageRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, surface->w, surface->h };
    SDL_RenderCopy(renderer, message, NULL, &messageRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(message);
    SDL_RenderPresent(renderer);
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_UP: if (dy == 0) { dx = 0; dy = -1; } break;
            case SDLK_DOWN: if (dy == 0) { dx = 0; dy = 1; } break;
            case SDLK_LEFT: if (dx == 0) { dx = -1; dy = 0; } break;
            case SDLK_RIGHT: if (dx == 0) { dx = 1; dy = 0; } break;
            case SDLK_SPACE: inMenu = false; break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));
    initSDL();
    spawnPizza();

    while (running) {
        if (inMenu) {
            renderMenu();
            handleInput();
        }
        else {
            handleInput();
            moveSnake();
            render();
            SDL_Delay(speed);
        }
    }

    SDL_DestroyTexture(pizzaTexture);
    SDL_DestroyTexture(beerTexture);
    return 0;
}