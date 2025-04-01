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
Mix_Music* backgroundMusic = nullptr;
TTF_Font* font = nullptr;
SDL_Texture* foodTexture = nullptr;
SDL_Texture* obstacleTexture = nullptr;

SDL_Color textColor = { 255, 255, 255, 255 };

int score = 0;
bool running = true;
bool inMenu = true;
int speed = 150;

struct Point {
    int x, y;
};

std::vector<Point> snake = { {10, 10}, {9, 10}, {8, 10} };
Point food = { 5, 5 };
std::vector<Point> obstacles;
int dx = 1, dy = 0;

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    TTF_Init();

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    headUpTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\head_up.png"));
    headDownTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\head_down.png"));
    bodyStraightTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\body_straight.png"));
    bodyTurnTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\body_turn.png"));
    tailTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\tail.png"));
    tristanaTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img\\back.png"));
    foodTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img/food.png"));
    obstacleTexture = SDL_CreateTextureFromSurface(renderer, IMG_Load("img/obstacle.png"));


    backgroundMusic = Mix_LoadMUS("img\\newjeans.wav");
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }

    font = TTF_OpenFont("img\\ShinyCrystal-Yq3z4.ttf", 24);
}

void spawnFood() {
    int maxAttempts = 100; // Giới hạn số lần thử để tránh vòng lặp vô tận
    int attempts = 0;
    bool valid;

    do {
        valid = true;
        food.x = rand() % (SCREEN_WIDTH / CELL_SIZE);
        food.y = rand() % (SCREEN_HEIGHT / CELL_SIZE);

        // Kiểm tra xem mồi có trùng rắn không
        for (const auto& segment : snake) {
            if (segment.x == food.x && segment.y == food.y) {
                valid = false;
                break;
            }
        }

        // Kiểm tra xem mồi có trùng chướng ngại vật không
        for (const auto& obs : obstacles) {
            if (obs.x == food.x && obs.y == food.y) {
                valid = false;
                break;
            }
        }

        attempts++;
        if (attempts >= maxAttempts) {
            valid = true; // Nếu thử quá nhiều lần, dừng kiểm tra để tránh vòng lặp vô tận
        }

    } while (!valid);
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

    for (const auto& obs : obstacles) {
        if (newHead.x == obs.x && newHead.y == obs.y) {
            running = false;
            return;
        }
    }

    snake.insert(snake.begin(), newHead);
    if (newHead.x == food.x && newHead.y == food.y) {
        score += 10;
        speed = std::max(100, speed - 5);  // Giảm delay để tăng tốc độ
        spawnFood();
        spawnObstacle();
    }
    else {
        snake.pop_back();
    }
}

void renderMenu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Surface* surface = TTF_RenderText_Solid(font, "Press ENTER to Start", textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstRect = { SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 2 - surface->h / 2, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void renderScore() {
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstRect = { 10, 10, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tristanaTexture, NULL, NULL);
    SDL_Rect foodRect = { food.x * CELL_SIZE, food.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_RenderCopy(renderer, foodTexture, NULL, &foodRect);


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

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect foodRect = { food.x * CELL_SIZE, food.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    for (const auto& obs : obstacles) {
        SDL_Rect obsRect = { obs.x * CELL_SIZE, obs.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
        SDL_RenderCopy(renderer, obstacleTexture, NULL, &obsRect);
    }
    

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (const auto& obs : obstacles) {
        SDL_Rect obsRect = { obs.x * CELL_SIZE, obs.y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &obsRect);
    }

    renderScore();
    SDL_RenderPresent(renderer);
}

void handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        else if (e.type == SDL_KEYDOWN) {
            if (inMenu) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    inMenu = false;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            else {
                switch (e.key.keysym.sym) {
                case SDLK_UP: if (dy == 0) { dx = 0; dy = -1; } break;
                case SDLK_DOWN: if (dy == 0) { dx = 0; dy = 1; } break;
                case SDLK_LEFT: if (dx == 0) { dx = -1; dy = 0; } break;
                case SDLK_RIGHT: if (dx == 0) { dx = 1; dy = 0; } break;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));
    initSDL();
    spawnFood();

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

    return 0;
}
