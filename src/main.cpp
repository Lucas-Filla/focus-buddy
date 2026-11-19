#include "raylib.h"
#include "platform/overlay.h"

constexpr int kWidth = 192;
constexpr int kHeight = 192;

int main() {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(kWidth, kHeight, "Focus Buddy renderer");
    SetTargetFPS(30);

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);

    if (!overlay::create(kWidth, kHeight)) {
        UnloadRenderTexture(canvas);
        CloseWindow();
        return 1;
    }

    while (!overlay::shouldClose()) {
        BeginTextureMode(canvas);
        ClearBackground(BLANK);
        DrawRectangle(64, 96, 64, 64, ORANGE);
        EndTextureMode();

        Image frame = LoadImageFromTexture(canvas.texture);
        ImageFlipVertical(&frame);
        overlay::present(static_cast<const unsigned char*>(frame.data), frame.width, frame.height);
        UnloadImage(frame);

        BeginDrawing();
        EndDrawing();
    }

    overlay::destroy();
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}