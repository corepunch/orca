#include <stdint.h>
/* X11 screenshot fixture only: report logical window size and 2x backing
 * scale. The capture controller allocates 780x1688 pixels; no PNG is resized. */
struct AXsize { uint32_t width, height; };
float axGetScaling(void) { return 2.0f; }
uint32_t axGetSize(struct AXsize *size)
{
    if (size) *size = (struct AXsize){390, 844};
    return 390u | (844u << 16);
}
