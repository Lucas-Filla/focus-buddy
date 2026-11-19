#include "raylib.h"
#include "platform/overlay.h"

const Color KEY_COLOR = { 255, 0, 255, 255 };   // magenta = invisible

int main(){
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST); 
    InitWindow(192, 192, "Focus Buddy");
    makeColorKeyTransparent(GetWindowHandle(), KEY_COLOR.r, KEY_COLOR.g, KEY_COLOR.b);
    SetTargetFPS(30);

    Vector2 grabOffset{};
    bool dragging = false;

    while(!WindowShouldClose()){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            dragging = true;
            grabOffset = GetMousePosition();
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            dragging = false;
        }
        if(dragging){
            Vector2 windowPos = GetWindowPosition();
            Vector2 mouse = GetMousePosition();
            SetWindowPosition((int)(windowPos.x + mouse.x - grabOffset.x),
                              (int)(windowPos.y + mouse.y - grabOffset.y));
        }

        BeginDrawing();
        ClearBackground(KEY_COLOR);
        DrawRectangle(64, 96, 64, 64, ORANGE); //PLACEHOLDER PET
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

