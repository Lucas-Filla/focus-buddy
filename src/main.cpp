#include "raylib.h"
#include "platform/overlay.h"
#include "platform/focus_monitor.h"

#include <cstdio>

constexpr int kWidth = 256;
constexpr int kHeight = 256;

constexpr int   kFrameSize = 32;
constexpr int   kFrameCount = 4;
constexpr int   kScale = 6;
constexpr float kFrameSeconds[kFrameCount] = { 0.5f, 0.5f, 0.3f, 0.5f };

struct RareFrame {
    int   index;
    int   minWait;
    int   maxWait;
    float elapsed;
    float waitTime;
};

bool shouldSkip(RareFrame& rare) {
    if (rare.elapsed < rare.waitTime) return true;
    rare.elapsed = 0.0f;
    rare.waitTime = static_cast<float>(GetRandomValue(rare.minWait, rare.maxWait));
    return false;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(kWidth, kHeight, "Focus Buddy renderer");
    SetTargetFPS(30);
    SetTraceLogLevel(LOG_WARNING);

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);

    Texture2D idleSheet = LoadTexture(TextFormat("%sassets/pets/orange_cat/idle.png", GetApplicationDirectory()));
    if (idleSheet.id == 0) {
        UnloadRenderTexture(canvas);
        CloseWindow();
        return 1;
    }
    SetTextureFilter(idleSheet, TEXTURE_FILTER_POINT);

    int   frame = 0;
    float frameTimer = 0.0f;

    float focusPollTimer = 0.0f;

    RareFrame tailFlick{ 1, 3, 5, 0.0f, 2.0f };
    RareFrame blink{ 2, 3, 6, 0.0f, 4.0f };

    if (!overlay::create(kWidth, kHeight)) {
        UnloadTexture(idleSheet);
        UnloadRenderTexture(canvas);
        CloseWindow();
        return 1;
    }

    while (!overlay::shouldClose()) {
        const float dt = GetFrameTime();
        frameTimer += dt;
        tailFlick.elapsed += dt;
        blink.elapsed += dt;

        if (frameTimer >= kFrameSeconds[frame]) {
            frameTimer -= kFrameSeconds[frame];
            frame = (frame + 1) % kFrameCount;

            if (frame == tailFlick.index && shouldSkip(tailFlick)) frame = (frame + 1) % kFrameCount;
            if (frame == blink.index && shouldSkip(blink))     frame = (frame + 1) % kFrameCount;
        }

        focusPollTimer += dt;
        if (focusPollTimer >= 1.0f) {
            focusPollTimer -= 1.0f;
            std::printf("FOCUS: \"%s\" | idle %.0f s\n",
                focus::activeWindowTitle().c_str(), focus::idleSeconds());
        }

        BeginTextureMode(canvas);
        ClearBackground(BLANK);
        Rectangle source{ static_cast<float>(frame * kFrameSize), 0.0f,
                          static_cast<float>(kFrameSize), static_cast<float>(kFrameSize) };
        const float drawSize = static_cast<float>(kFrameSize * kScale);
        Rectangle dest{ (kWidth - drawSize) / 2.0f, kHeight - drawSize, drawSize, drawSize };
        DrawTexturePro(idleSheet, source, dest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
        EndTextureMode();

        Image frameImage = LoadImageFromTexture(canvas.texture);
        ImageFlipVertical(&frameImage);
        overlay::present(static_cast<const unsigned char*>(frameImage.data), frameImage.width, frameImage.height);
        UnloadImage(frameImage);

        BeginDrawing();
        EndDrawing();
    }

    overlay::destroy();
    UnloadTexture(idleSheet);
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}