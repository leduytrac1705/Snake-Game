#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <deque>

using namespace std;

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color red = {255, 0, 0, 255};

float cellSize = 30;
float cellCount = 25;
float offset = 75;

double lastUpdatedTime = 0;

bool EventTriggered(double interval) {
    double currentTime = GetTime();
    if ((currentTime - lastUpdatedTime) >= interval) {
        lastUpdatedTime = currentTime;
        return true;
    }
    else {
        return false;
    }
};

bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); ++i) {
        if (Vector2Equals(element, deque[i])) {
            return true;
        } 
    }
    return false;
};

bool WaitForStart() {
    const char *message = "Press SPACE to start";
    const int fontSize = 40;

    while (WindowShouldClose() == false) {
        if (IsKeyPressed(KEY_SPACE)) {
            return true;
        }

        BeginDrawing();
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{offset - 5, offset - 5, cellSize * cellCount + 10, cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(message, (GetScreenWidth() - MeasureText(message, fontSize)) / 2,
                 (GetScreenHeight() - fontSize) / 2, fontSize, darkGreen);
        EndDrawing();
    }

    return false;
};

class Snake {
    public:
    deque<Vector2> body = {Vector2{7, 9}, Vector2{6, 9}, Vector2{5, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw() {
        for(unsigned int i = 0; i < body.size(); ++i) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, cellSize, cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true) {
            addSegment = false;
        }
        else {
            body.pop_back();
        }
    }

    void Reset() {
        body = {Vector2{7, 9}, Vector2{6, 9}, Vector2{5, 9}};
        direction = {1, 0};
    }
};

class Food {

    public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody) {
        Image image = LoadImage("graphic/apple.png");
        ImageResize(&image, cellSize, cellSize);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GetRandomPosition(snakeBody);
    }

    ~Food() {
        UnloadTexture(texture);
    }

    void Draw() {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GetRandomPosition(deque<Vector2> snakeBody) {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody) == true) {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game {
    public:
    Food food = Food(snake.body);
    Snake snake = Snake();
    bool running = false;
    bool gameOver = false;
    bool quitRequested = false;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    /*Game() {
        InitAudioDevice();
        eatSound = LoadSound("sound/eat.mp3");
        wallSound = LoadSound("sound/wall.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }*/

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        if (running) {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position = food.GetRandomPosition(snake.body);
            snake.addSegment = true;
            score++;
            //PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1 || snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();
        }
    }

    void CheckCollisionWithTail() {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }

    void GameOver() {
        gameOver = true;
        running = false;
        const char* message = "Game Over! Press SPACE to restart";
        int fontSize = 40;
        while (WindowShouldClose() == false) {
            if (IsKeyPressed(KEY_SPACE)) {
                gameOver = false;
                break;
            }
            BeginDrawing();
            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{offset - 5, offset - 5, cellSize * cellCount + 10, cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            DrawText(TextFormat("Highest Score: %i", score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
            DrawText(message, (GetScreenWidth() - MeasureText(message, fontSize)) / 2,
                    (GetScreenHeight() - fontSize) / 2, fontSize, darkGreen);
            EndDrawing();
        }
        if (gameOver == false) {
            snake.Reset();
            food.position = food.GetRandomPosition(snake.body);
            score = 0;
        }
        else {
            quitRequested = true;
            return;
        }
        //PlaySound(wallSound);
    }

};

int main () {

    cout << "Starting the game..." << endl;
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    if (WaitForStart() == false) {
        CloseWindow();
        return 0;
    }

    Game game = Game();

    while (WindowShouldClose() == false && game.quitRequested == false) {
        BeginDrawing();
        if (EventTriggered(0.2)) {
            game.Update();
        }

        if (game.quitRequested) {
            break;
        }

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.running = true;
        }

        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{offset - 5, offset - 5, cellSize * cellCount + 10, cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}