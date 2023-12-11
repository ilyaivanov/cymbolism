#include "types.h"

inline void MoveBytesRight(char *ptr, int length) 
{
    for (int i = length - 1; i > 0; i--) {
        ptr[i] = ptr[i - 1];
    }
}
inline void MoveBytesLeft(char *ptr, int length) 
{
    for (int i = 0; i < length - 1; i++) {
        ptr[i] = ptr[i + 1];
    }
}

//TODO: need to extract this filtering code away from here
void InitBuffer(StringBuffer *buffer, char* text, i32 sourceSize)
{
    buffer->capacity = sourceSize * 2;
    buffer->text = malloc(buffer->capacity);
    i32 targetIndex = 0;
    for (int sourceIndex = 0; sourceIndex < sourceSize; sourceIndex++)
    {
        char ch = *(text + sourceIndex);
        if (ch != '\r')
        {
            *(buffer->text + targetIndex) = ch;
            targetIndex++;
        }
    }
    buffer->length = targetIndex;
    // CopyMemory(buffer->text, text, buffer->length);
}

void InsertCharAt(StringBuffer *buffer, i32 at, i32 ch)
{
    if(buffer->length >= buffer->capacity)
    {
        char *currentStr = buffer->text;
        buffer->capacity *= 2;
        buffer->text = malloc(buffer->capacity);
        CopyMemory(buffer->text, currentStr, buffer->length);
        free(currentStr);
    }

    buffer->length += 1;
    MoveBytesRight(buffer->text + at, buffer->length - at);
    *(buffer->text + at) = ch;
}

void RemoveCharAt(StringBuffer *buffer, i32 at)
{
    MoveBytesLeft(buffer->text + at, buffer->length - at);
    buffer->length--;
}