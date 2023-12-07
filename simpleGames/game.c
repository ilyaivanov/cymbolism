#include <math.h>
#include <stdio.h>
#include "types.h"

#define BACKGROUND_COLOR_GREY 0x11

Color bg = {BACKGROUND_COLOR_GREY / 0xff, BACKGROUND_COLOR_GREY / 0xff, BACKGROUND_COLOR_GREY / 0xff, 0xff / 0xff};

float GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

inline i32 ColorToHex(Color color)
{
    return (u8)(color.a * 0xff) << 24 |
           (u8)(color.r * 0xff) << 16 |
           (u8)(color.g * 0xff) << 8 |
           (u8)(color.b * 0xff) << 0;
}

inline Color MultColor(Color c, float scalar)
{
    return (Color){c.r * scalar, c.g * scalar, c.b * scalar, c.a * scalar};
}

inline Color AddColor(Color c1, Color c2)
{
    return (Color){c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a};
}

inline V2f Vector2Add(V2f v1, V2f v2)
{
    return (V2f){v1.x + v2.x, v1.y + v2.y};
}
inline Color InterpolateColor(Color c1, Color c2, float t)
{
    return AddColor(MultColor(c1, 1 - t), MultColor(c2, t));
}

void DrawCircleFilled(MyBitmap *canvas, V2f center, float r, Color color)
{
    i32 colorHex = ColorToHex(color);
    float glowDistance = 2;
    for (int x = center.x - r - glowDistance; x < center.x + r + glowDistance; x++)
    {
        for (int y = center.y - r - glowDistance; y < center.y + r + glowDistance; y++)
        {
            float d = GetDistance(center.x, center.y, x, y);

            if (d <= r)
            {
                *(canvas->pixels + y * canvas->width + x) = colorHex;
            }
            else if (d <= r + glowDistance)
            {
                float outlineDelta = ((r + glowDistance) - d) / glowDistance;
                *(canvas->pixels + y * canvas->width + x) = ColorToHex(InterpolateColor(bg, color, outlineDelta));
            }
        }
    }
}

void DrawCircleOutline(MyBitmap *canvas, V2f center, float r, Color color)
{
    i32 colorHex = ColorToHex(color);
    float glowDistance = 4;
    for (int x = center.x - r - glowDistance; x < center.x + r + glowDistance; x++)
    {
        for (int y = center.y - r - glowDistance; y < center.y + r + glowDistance; y++)
        {
            float d = GetDistance(center.x, center.y, x, y);

            if (d >= r && d <= r + glowDistance)
            {
                float outlineDelta = ((r + glowDistance) - d) / glowDistance;
                *(canvas->pixels + y * canvas->width + x) = ColorToHex(InterpolateColor(bg, color, outlineDelta));
            }

            if (d >= r - glowDistance && d <= r)
            {
                float outlineDelta = (d - (r - glowDistance)) / glowDistance;
                *(canvas->pixels + y * canvas->width + x) = ColorToHex(InterpolateColor(bg, color, outlineDelta));
            }
        }
    }
}

#define PI 3.14159265

// radians per second
f32 slowAngularSpeed = PI / 4;
f32 normalAngularSpeed = PI / 2;
f32 fastAngularSpeed = PI;


V2f RotateAround(const V2f *center, float r, float radians)
{
    return (V2f) {center->x + cos(radians) * r, center->y + sin(radians) * r};
}


void UpdateAndDrawApp(AppState *state, MyInput * input)
{
    Color red = {1, 0.2, 0.2, 1};
    Color green = {0.2, 1, 0.2, 1};
    Color strange = {0.5, 1, 1, 1};

    V2f center = {state->canvas.width / 2, state->canvas.height / 3};

    float r = 20;
    float speed = normalAngularSpeed;
    if(input->isPressed['W'])
    {
        speed = fastAngularSpeed;
        r = 20 * 0.7;
    }
    else if(input->isPressed['S'])
    {
        speed = slowAngularSpeed;
        r = 20 * 1.2;
    }

    for(int i = 0; i < state->enemiesCount; i++){

    }

    float currentDegrees = state->degrees;
    state->degrees += speed / 60;
    float nextDegrees = state->degrees;

    if((i32)(nextDegrees / (2 * PI)) > (i32)(currentDegrees / (2 * PI))){
        state->circlesComplete += 1;

        if(state->circlesComplete == 1)
        {
            RotatingEnemy *enemy = &state->enemies[state->enemiesCount++];
            enemy->center = center;
            enemy->positionRadians = 0;
            enemy->rotationSpeed = PI;
        }
    }


    float degree = state->degrees;


    V2f cPos = RotateAround(&center, 300, degree);

    DrawCircleOutline(&state->canvas, center, 300, strange);

    DrawCircleOutline(&state->canvas, Vector2Add(center, (V2f){0, 300 + 70}), 300, strange);
    
    DrawCircleFilled(&state->canvas, cPos, r, green);


    char buff[20];
    sprintf(buff, "%d", state->circlesComplete);
    DrawTextureCentered(&state->canvas, GetGlyphBitmap(&state->fonts.regular, buff[0]), center.x, center.y - 10, ColorToHex(strange));
}

