#include <raylib.h>
#include <vector>
#include <cstdlib>
#include <ctime>

const int screenWidth = 800;
const int screenHeight = 600;
const float gravity = 0.5f;
const float friction = 0.99f;
const float maxVelocity = 10.0f;

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;

    Ball(float x, float y, float r, Color c) : position({x, y}), velocity({0, 0}), radius(r), color(c) {}

    void Draw() {
        DrawCircleV(position, radius, color);
    }

    void Update() {
        position.x += velocity.x;
        position.y += velocity.y;

        velocity.y += gravity; // Apply gravity

        // Apply friction
        velocity.x *= friction;
        velocity.y *= friction;

        // Limit velocity
        if (Vector2Length(velocity) > maxVelocity) {
            velocity = Vector2Scale(Vector2Normalize(velocity), maxVelocity);
        }

        // Screen boundaries
        if (position.y + radius >= screenHeight) {
            position.y = screenHeight - radius;
            velocity.y *= -0.8f; // Bounce
        }
        if (position.y - radius <= 0) {
            position.y = radius;
            velocity.y *= -0.8f;
        }
        if (position.x + radius >= screenWidth) {
            position.x = screenWidth - radius;
            velocity.x *= -0.8f;
        }
        if (position.x - radius <= 0) {
            position.x = radius;
            velocity.x *= -0.8f;
        }
    }

    void ApplyForce(Vector2 force) {
        velocity.x += force.x;
        velocity.y += force.y;
    }
};

struct Star {
    Vector2 position;
    float radius;
    Color color;

    Star(float x, float y, float r, Color c) : position({x, y}), radius(r), color(c) {}

    void Draw() {
        DrawCircleV(position, radius, color);
    }
};

struct Obstacle {
    Rectangle rect;
    Vector2 velocity;
    Color color;

    Obstacle(float x, float y, float width, float height, Color c) : rect({x, y, width, height}), velocity({0, 0}), color(c) {}

    void Draw() {
        DrawRectangleRec(rect, color);
    }

    void Update() {
        rect.x += velocity.x;
        rect.y += velocity.y;

        // Screen boundaries
        if (rect.y + rect.height >= screenHeight || rect.y <= 0) {
            velocity.y *= -1;
        }
        if (rect.x + rect.width >= screenWidth || rect.x <= 0) {
            velocity.x *= -1;
        }
    }
};

std::vector<Obstacle> GenerateObstacles(int count) {
    std::vector<Obstacle> obstacles;
    for (int i = 0; i < count; i++) {
        float x = GetRandomValue(0, screenWidth - 50);
        float y = GetRandomValue(0, screenHeight - 50);
        float width = GetRandomValue(50, 100);
        float height = GetRandomValue(20, 50);
        Obstacle obs(x, y, width, height, DARKGRAY);
        obs.velocity = {GetRandomValue(-3, 3), GetRandomValue(-3, 3)};
        obstacles.push_back(obs);
    }
    return obstacles;
}

std::vector<Star> GenerateStars(int count) {
    std::vector<Star> stars;
    for (int i = 0; i < count; i++) {
        float x = GetRandomValue(50, screenWidth - 50);
        float y = GetRandomValue(50, screenHeight - 50);
        stars.emplace_back(x, y, 10, GOLD);
    }
    return stars;
}

void InitializeLevel(Ball &player, std::vector<Obstacle> &obstacles, std::vector<Star> &stars, int level) {
    // Reset player position and velocity
    player.position = {screenWidth / 2, screenHeight / 2};
    player.velocity = {0, 0};

    // Generate new obstacles and stars
    obstacles = GenerateObstacles(5 + level);  // Increase the number of obstacles with each level
    stars = GenerateStars(10 + level);  // Increase the number of stars with each level

    // Debugging output
    TraceLog(LOG_INFO, "Initializing Level: %i", level);
    TraceLog(LOG_INFO, "Number of Obstacles: %i", obstacles.size());
    TraceLog(LOG_INFO, "Number of Stars: %i", stars.size());
}

int main() {
    InitWindow(screenWidth, screenHeight, "Physics Game");

    Ball player(screenWidth / 2, screenHeight / 2, 20, BLUE);
    std::vector<Obstacle> obstacles;
    std::vector<Star> stars;
    int level = 0;
    InitializeLevel(player, obstacles, stars, level);

    int score = 0;

    // Load sound effects
    InitAudioDevice();
    Sound starSound = LoadSound("star.wav");
    Sound bounceSound = LoadSound("bounce.wav");

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // Player controls
        if (IsKeyDown(KEY_RIGHT)) player.ApplyForce({1.0f, 0.0f});
        if (IsKeyDown(KEY_LEFT)) player.ApplyForce({-1.0f, 0.0f});
        if (IsKeyDown(KEY_UP)) player.ApplyForce({0.0f, -1.0f});
        if (IsKeyDown(KEY_DOWN)) player.ApplyForce({0.0f, 1.0f});

        // Update
        player.Update();
        for (auto& obstacle : obstacles) {
            obstacle.Update();
        }

        // Collision detection with obstacles
        for (auto& obstacle : obstacles) {
            if (CheckCollisionCircleRec(player.position, player.radius, obstacle.rect)) {
                player.velocity.x *= -1;
                player.velocity.y *= -1;
                PlaySound(bounceSound);
            }
        }

        // Collision detection with stars
        for (size_t i = 0; i < stars.size(); i++) {
            if (CheckCollisionCircles(player.position, player.radius, stars[i].position, stars[i].radius)) {
                stars.erase(stars.begin() + i);
                score++;
                PlaySound(starSound);
            }
        }

        // Check if all stars are collected
        if (stars.empty()) {
            level++;
            InitializeLevel(player, obstacles, stars, level);
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        player.Draw();
        for (auto& obstacle : obstacles) {
            obstacle.Draw();
        }
        for (auto& star : stars) {
            star.Draw();
        }

        DrawText(TextFormat("Score: %i", score), 10, 10, 20, BLACK);
        DrawText(TextFormat("Level: %i", level), 10, 40, 20, BLACK);

        EndDrawing();
    }

    // Unload sound effects
    UnloadSound(starSound);
    UnloadSound(bounceSound);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}
