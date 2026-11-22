#include "raylib.h"
#include "platform/overlay.h"
#include "platform/focus_monitor.h"
#include "app/focus_rules.h"

#include <cstdio>
#include <string>
#include <vector>

constexpr int kWidth = 256;
constexpr int kHeight = 256;

constexpr int kFrameSize = 32;
constexpr int kScale = 6;

constexpr float kFocusPollSeconds = 1.0f;

constexpr const char* kScoldLines[] = {
    "Hey! Back to work!",
    "Are you fur real rn?",
    "No dih for you, Kian",
    "Go to bed.",
    "You fcking chud.",
    "LOCK TF IN!",
    "Holy chudlite..",
    "This is a-paw-ling.",
};
constexpr int kScoldLineCount = sizeof(kScoldLines) / sizeof(kScoldLines[0]);

constexpr const char* kPraiseLines[] = {
    "Good boy.",
    "Phew..",
};
constexpr int kPraiseLineCount = sizeof(kPraiseLines) / sizeof(kPraiseLines[0]);

constexpr float kBubbleSeconds = 5.0f;
constexpr float kPraiseSeconds = 3.0f;
constexpr float kBubbleOffsetX = -12.0f;

const Color kInk = { 42, 20, 3, 255 };

struct Animation {
    Texture2D sheet;
    int frameCount;
    std::vector<float> frameSeconds;
};

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

Texture2D loadSheet(const char* name) {
    Texture2D sheet = LoadTexture(TextFormat("%sassets/pets/orange_cat/%s.png", GetApplicationDirectory(), name));
    if (sheet.id != 0) SetTextureFilter(sheet, TEXTURE_FILTER_POINT);
    return sheet;
}

void drawSpeechBubble(const char* text) {
    constexpr int kFontSize = 20;
    constexpr int kPadding = 8;

    const float width = static_cast<float>(MeasureText(text, kFontSize) + kPadding * 2);
    const float height = static_cast<float>(kFontSize + kPadding * 2);
    const Rectangle box{ (kWidth - width) / 2.0f + kBubbleOffsetX, 8.0f, width, height };

    const float tailX = kWidth / 2.0f + kBubbleOffsetX;
    const float boxBottom = box.y + box.height;

    DrawRectangleRounded(box, 0.4f, 8, WHITE);
    DrawRectangleRoundedLinesEx(box, 0.4f, 8, 2.0f, kInk);
    DrawTriangle({ tailX - 8.0f, boxBottom - 1.0f }, { tailX, boxBottom + 12.0f }, { tailX + 8.0f, boxBottom - 1.0f }, kInk);
    DrawTriangle({ tailX - 5.0f, boxBottom - 2.0f }, { tailX, boxBottom + 8.0f }, { tailX + 5.0f, boxBottom - 2.0f }, WHITE);
    DrawText(text, static_cast<int>(box.x) + kPadding, static_cast<int>(box.y) + kPadding, kFontSize, kInk);
}

int main() {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(kWidth, kHeight, "Focus Buddy renderer");
    SetTargetFPS(30);
    SetTraceLogLevel(LOG_WARNING);

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);

    Animation idle{ loadSheet("idle"), 4,{ 0.5f, 0.5f, 0.3f, 0.5f } };
    Animation agitated{ loadSheet("agitated"), 4,{ 0.3f, 0.3f, 0.3f, 0.3f } };
    if (idle.sheet.id == 0 || agitated.sheet.id == 0) {
        UnloadTexture(idle.sheet);
        UnloadTexture(agitated.sheet);
        UnloadRenderTexture(canvas);
        CloseWindow();
        return 1;
    }

    Animation* current = &idle;
    int   frame = 0;
    float frameTimer = 0.0f;

    RareFrame tailFlick{ 1, 3, 5, 0.0f, 2.0f };
    RareFrame blink{ 2, 3, 6, 0.0f, 4.0f };

    float focusPollTimer = 0.0f;
    FocusRules rules(FocusRulesConfig{
        .allowKeywords = { "Visual Studio", "Pixelorama", "Google Docs",
                              "CS 4040", "CS 3200", "CS 3620", "AI 3100", "CHEM 1510" },
        .blockKeywords = { "YouTube", "Reddit", "Netflix", "Instagram", "TikTok" },
        .ignoreKeywords = {},
        .graceSeconds = 10.0,
        .idleLimitSeconds = 300.0,
        });
    FocusState lastState = rules.state();
    int   bubbleLine = 0;
    float bubbleTimer = 0.0f;
    int   praiseLine = 0;
    float praiseTimer = 0.0f;

    if (!overlay::create(kWidth, kHeight)) {
        UnloadTexture(idle.sheet);
        UnloadTexture(agitated.sheet);
        UnloadRenderTexture(canvas);
        CloseWindow();
        return 1;
    }

    while (!overlay::shouldClose()) {
        const float dt = GetFrameTime();

        focusPollTimer += dt;
        if (focusPollTimer >= kFocusPollSeconds) {
            focusPollTimer -= kFocusPollSeconds;
            const std::string title = focus::activeWindowTitle();
            const FocusState state = rules.update(title, focus::idleSeconds(), kFocusPollSeconds);
            if (state != lastState) {
                std::printf("STATE: %s -> %s  (\"%s\")\n", toString(lastState), toString(state), title.c_str());
                const FocusState previous = lastState;
                lastState = state;

                Animation* wanted = (state == FocusState::Distracted) ? &agitated : &idle;
                if (wanted != current) {
                    current = wanted;
                    frame = 0;
                    frameTimer = 0.0f;
                }
                if (state == FocusState::Distracted) {
                    bubbleLine = GetRandomValue(0, kScoldLineCount - 1);
                    bubbleTimer = 0.0f;
                    praiseTimer = 0.0f;
                }
                if (previous == FocusState::Distracted && state != FocusState::Idle) {
                    praiseLine = GetRandomValue(0, kPraiseLineCount - 1);
                    praiseTimer = kPraiseSeconds;
                }
            }
        }

        frameTimer += dt;
        tailFlick.elapsed += dt;
        blink.elapsed += dt;

        if (frameTimer >= current->frameSeconds[frame]) {
            frameTimer -= current->frameSeconds[frame];
            frame = (frame + 1) % current->frameCount;

            if (current == &idle) {
                if (frame == tailFlick.index && shouldSkip(tailFlick)) frame = (frame + 1) % current->frameCount;
                if (frame == blink.index && shouldSkip(blink))         frame = (frame + 1) % current->frameCount;
            }
        }

        if (current == &agitated) {
            bubbleTimer += dt;
            if (bubbleTimer >= kBubbleSeconds) {
                bubbleTimer -= kBubbleSeconds;
                bubbleLine = (bubbleLine + 1) % kScoldLineCount;
            }
        }

        if (praiseTimer > 0.0f) {
            praiseTimer -= dt;
        }

        BeginTextureMode(canvas);
        ClearBackground(BLANK);
        Rectangle source{ static_cast<float>(frame * kFrameSize), 0.0f,
                          static_cast<float>(kFrameSize), static_cast<float>(kFrameSize) };
        const float drawSize = static_cast<float>(kFrameSize * kScale);
        Rectangle dest{ (kWidth - drawSize) / 2.0f, kHeight - drawSize, drawSize, drawSize };
        DrawTexturePro(current->sheet, source, dest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
        if (current == &agitated) {
            drawSpeechBubble(kScoldLines[bubbleLine]);
        }
        else if (praiseTimer > 0.0f) {
            drawSpeechBubble(kPraiseLines[praiseLine]);
        }
        EndTextureMode();

        Image frameImage = LoadImageFromTexture(canvas.texture);
        ImageFlipVertical(&frameImage);
        overlay::present(static_cast<const unsigned char*>(frameImage.data), frameImage.width, frameImage.height);
        UnloadImage(frameImage);

        BeginDrawing();
        EndDrawing();
    }

    overlay::destroy();
    UnloadTexture(idle.sheet);
    UnloadTexture(agitated.sheet);
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}