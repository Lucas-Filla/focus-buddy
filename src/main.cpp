#include "raylib.h"

int main(){
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);

    InitWindow(192, 192, "Focus Buddy");
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
        ClearBackground(BLANK);
        DrawRectangle(64, 96, 64, 64, ORANGE); //PLACEHOLDER PET
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

