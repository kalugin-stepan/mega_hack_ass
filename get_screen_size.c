#include <stdint.h>
#include <windows.h>

void get_screen_size(int32_t* width, int32_t* height) {
    RECT rect;
    HWND win = GetDesktopWindow();
    GetWindowRect(win, &rect);

    *width = rect.right;
    *height = rect.bottom;
}