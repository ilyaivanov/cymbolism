#include "types.h"

inline i32 ClampI32(i32 val, i32 min, i32 max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}