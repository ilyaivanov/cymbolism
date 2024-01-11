#ifndef INPUT_C
#define INPUT_C

#include "math.c"

typedef struct UserInput
{
    u8 keyboardState[256];
    u8 keysPressedhisFrame[256];

    WPARAM charEventsThisFrame[32];
    u32 charEventsThisFrameCount;


    float mouseX;
    float mouseY;
    float zDeltaThisFrame;
} UserInput;

#endif