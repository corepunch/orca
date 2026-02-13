# Implementing a Simple Text Input in WinAPI

This document explains how to implement a simple text input control in WinAPI without using existing solutions like standard EDIT controls. We'll build everything from scratch to understand the underlying mechanisms.

## Overview

A text input control in Windows requires handling several aspects:
- Window procedure (WinProc) to process messages
- Keyboard input handling (WM_CHAR, WM_KEYDOWN, WM_KEYUP)
- Text buffer management
- Caret (cursor) positioning and rendering
- Text rendering and display
- Focus management
- Selection and clipboard support (optional)

## The Window Procedure (WinProc)

The window procedure is the heart of any Windows control. It processes all messages sent to the window. Here's a complete implementation of a WinProc for a simple text input:

```c
#include <windows.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold our text input state
typedef struct {
    char* buffer;           // Text buffer
    size_t bufferSize;      // Maximum size of buffer
    size_t textLength;      // Current length of text
    size_t caretPos;        // Current caret position (0-based index)
    BOOL hasFocus;          // Whether control has focus
    HFONT hFont;            // Font handle
    int charWidth;          // Average character width
    int charHeight;         // Character height
} TextInputData;

// Window procedure for our text input control
LRESULT CALLBACK TextInputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TextInputData* pData = (TextInputData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg)
    {
        case WM_CREATE:
        {
            // Allocate and initialize our data structure
            pData = (TextInputData*)calloc(1, sizeof(TextInputData));
            if (!pData)
                return -1;
            
            pData->bufferSize = 256;  // Default buffer size
            pData->buffer = (char*)calloc(pData->bufferSize, sizeof(char));
            if (!pData->buffer) {
                free(pData);
                return -1;
            }
            
            pData->textLength = 0;
            pData->caretPos = 0;
            pData->hasFocus = FALSE;
            
            // Create default font
            pData->hFont = CreateFont(
                16,                         // Height
                0,                          // Width
                0,                          // Escapement
                0,                          // Orientation
                FW_NORMAL,                  // Weight
                FALSE,                      // Italic
                FALSE,                      // Underline
                FALSE,                      // StrikeOut
                ANSI_CHARSET,               // CharSet
                OUT_DEFAULT_PRECIS,         // OutPrecision
                CLIP_DEFAULT_PRECIS,        // ClipPrecision
                DEFAULT_QUALITY,            // Quality
                DEFAULT_PITCH | FF_DONTCARE,// PitchAndFamily
                "Courier New"               // FaceName
            );
            
            // Get font metrics
            HDC hdc = GetDC(hwnd);
            HFONT hOldFont = (HFONT)SelectObject(hdc, pData->hFont);
            TEXTMETRIC tm;
            GetTextMetrics(hdc, &tm);
            pData->charWidth = tm.tmAveCharWidth;
            pData->charHeight = tm.tmHeight;
            SelectObject(hdc, hOldFont);
            ReleaseDC(hwnd, hdc);
            
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pData);
            return 0;
        }
        
        case WM_DESTROY:
        {
            if (pData) {
                if (pData->hasFocus) {
                    DestroyCaret();
                }
                if (pData->buffer) {
                    free(pData->buffer);
                }
                if (pData->hFont) {
                    DeleteObject(pData->hFont);
                }
                free(pData);
            }
            return 0;
        }
        
        case WM_SETFOCUS:
        {
            pData->hasFocus = TRUE;
            
            // Create and show the caret
            CreateCaret(hwnd, NULL, 2, pData->charHeight);
            SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
            ShowCaret(hwnd);
            
            // Invalidate to redraw
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_KILLFOCUS:
        {
            pData->hasFocus = FALSE;
            
            // Hide and destroy the caret
            HideCaret(hwnd);
            DestroyCaret();
            
            // Invalidate to redraw
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_CHAR:
        {
            if (!pData->hasFocus)
                return 0;
            
            char ch = (char)wParam;
            
            // Handle special characters
            switch (ch)
            {
                case 0x08:  // Backspace
                {
                    if (pData->caretPos > 0) {
                        // Remove character before caret
                        memmove(&pData->buffer[pData->caretPos - 1],
                               &pData->buffer[pData->caretPos],
                               pData->textLength - pData->caretPos + 1);
                        pData->caretPos--;
                        pData->textLength--;
                        
                        // Update caret position
                        SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                
                case 0x0D:  // Enter
                {
                    // In a single-line edit, Enter typically does nothing
                    // or signals completion. We'll do nothing here.
                    break;
                }
                
                case 0x1B:  // Escape
                {
                    // Clear focus
                    SetFocus(GetParent(hwnd));
                    break;
                }
                
                default:
                {
                    // Regular printable character
                    if (ch >= 32 && pData->textLength < pData->bufferSize - 1)
                    {
                        // Insert character at caret position
                        memmove(&pData->buffer[pData->caretPos + 1],
                               &pData->buffer[pData->caretPos],
                               pData->textLength - pData->caretPos + 1);
                        pData->buffer[pData->caretPos] = ch;
                        pData->caretPos++;
                        pData->textLength++;
                        
                        // Update caret position
                        SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
            }
            return 0;
        }
        
        case WM_KEYDOWN:
        {
            if (!pData->hasFocus)
                return 0;
            
            switch (wParam)
            {
                case VK_LEFT:
                {
                    // Move caret left
                    if (pData->caretPos > 0) {
                        pData->caretPos--;
                        SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                    }
                    break;
                }
                
                case VK_RIGHT:
                {
                    // Move caret right
                    if (pData->caretPos < pData->textLength) {
                        pData->caretPos++;
                        SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                    }
                    break;
                }
                
                case VK_HOME:
                {
                    // Move caret to beginning
                    pData->caretPos = 0;
                    SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                    break;
                }
                
                case VK_END:
                {
                    // Move caret to end
                    pData->caretPos = pData->textLength;
                    SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                    break;
                }
                
                case VK_DELETE:
                {
                    // Delete character at caret position
                    if (pData->caretPos < pData->textLength) {
                        memmove(&pData->buffer[pData->caretPos],
                               &pData->buffer[pData->caretPos + 1],
                               pData->textLength - pData->caretPos);
                        pData->textLength--;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
            }
            return 0;
        }
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Draw background
            HBRUSH hBrush;
            if (pData->hasFocus) {
                hBrush = CreateSolidBrush(RGB(255, 255, 255));  // White background
            } else {
                hBrush = CreateSolidBrush(RGB(240, 240, 240));  // Light gray background
            }
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            
            // Draw border
            HPEN hPen;
            if (pData->hasFocus) {
                hPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 215));  // Blue border
            } else {
                hPen = CreatePen(PS_SOLID, 1, RGB(160, 160, 160));  // Gray border
            }
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
            
            // Draw text
            if (pData->textLength > 0) {
                HFONT hOldFont = (HFONT)SelectObject(hdc, pData->hFont);
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(0, 0, 0));
                
                RECT textRect = { 5, 5, rc.right - 5, rc.bottom - 5 };
                DrawText(hdc, pData->buffer, (int)pData->textLength, &textRect, 
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(hdc, hOldFont);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        {
            // Set focus when clicked
            SetFocus(hwnd);
            
            // Calculate click position and move caret
            int x = LOWORD(lParam);
            int clickPos = (x - 5) / pData->charWidth;
            if (clickPos < 0) clickPos = 0;
            if (clickPos > (int)pData->textLength) clickPos = (int)pData->textLength;
            
            pData->caretPos = (size_t)clickPos;
            if (pData->hasFocus) {
                SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
            }
            
            return 0;
        }
        
        case WM_GETTEXT:
        {
            size_t cchMax = (size_t)wParam;
            char* pszText = (char*)lParam;
            
            if (pszText && cchMax > 0) {
                size_t copyLen = (pData->textLength < cchMax - 1) ? 
                                 pData->textLength : cchMax - 1;
                memcpy(pszText, pData->buffer, copyLen);
                pszText[copyLen] = '\0';
                return (LRESULT)copyLen;
            }
            return 0;
        }
        
        case WM_SETTEXT:
        {
            const char* pszText = (const char*)lParam;
            if (pszText) {
                size_t newLen = strlen(pszText);
                if (newLen < pData->bufferSize) {
                    strcpy(pData->buffer, pszText);
                    pData->textLength = newLen;
                    pData->caretPos = newLen;
                    
                    if (pData->hasFocus) {
                        SetCaretPos(5 + pData->caretPos * pData->charWidth, 5);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    return TRUE;
                }
            }
            return FALSE;
        }
        
        case WM_GETTEXTLENGTH:
        {
            return (LRESULT)pData->textLength;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
```

## Registering the Window Class

Before you can create instances of your text input control, you need to register the window class:

```c
BOOL RegisterTextInputClass(HINSTANCE hInstance)
{
    WNDCLASSEX wc = { 0 };
    
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = TextInputWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);  // Text cursor
    wc.hbrBackground = NULL;  // We handle painting ourselves
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "CustomTextInput";
    wc.hIconSm = NULL;
    
    return RegisterClassEx(&wc) != 0;
}
```

## Creating an Instance

Once the class is registered, you can create instances of your text input control:

```c
HWND CreateTextInput(HWND hParent, HINSTANCE hInstance, int x, int y, int width, int height, int id)
{
    return CreateWindowEx(
        WS_EX_CLIENTEDGE,           // Extended style (3D border)
        "CustomTextInput",           // Class name
        "",                          // Initial text
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,  // Style
        x, y, width, height,         // Position and size
        hParent,                     // Parent window
        (HMENU)(UINT_PTR)id,        // Control ID
        hInstance,                   // Instance handle
        NULL                         // Creation parameters
    );
}
```

## Complete Example Application

Here's a complete example that demonstrates the text input control:

```c
#include <windows.h>
#include <stdio.h>

// Forward declarations
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define IDC_TEXTINPUT1 1001
#define IDC_TEXTINPUT2 1002
#define IDC_BUTTON_GET 1003

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    // Register the main window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "MainWindow";
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Register our custom text input class
    if (!RegisterTextInputClass(hInstance)) {
        MessageBox(NULL, "TextInput Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Create the main window
    HWND hwndMain = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "MainWindow",
        "Custom Text Input Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwndMain == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndInput1, hwndInput2, hwndButton;
    
    switch (msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
            
            // Create labels
            CreateWindow("STATIC", "Username:", WS_CHILD | WS_VISIBLE,
                        20, 20, 100, 20, hwnd, NULL, hInstance, NULL);
            
            CreateWindow("STATIC", "Password:", WS_CHILD | WS_VISIBLE,
                        20, 60, 100, 20, hwnd, NULL, hInstance, NULL);
            
            // Create text inputs
            hwndInput1 = CreateTextInput(hwnd, hInstance, 120, 15, 300, 30, IDC_TEXTINPUT1);
            hwndInput2 = CreateTextInput(hwnd, hInstance, 120, 55, 300, 30, IDC_TEXTINPUT2);
            
            // Create button
            hwndButton = CreateWindow("BUTTON", "Get Text",
                                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     120, 100, 100, 30,
                                     hwnd, (HMENU)IDC_BUTTON_GET, hInstance, NULL);
            
            return 0;
        }
        
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDC_BUTTON_GET && HIWORD(wParam) == BN_CLICKED) {
                char buffer1[256], buffer2[256];
                
                // Get text from both inputs
                GetWindowText(hwndInput1, buffer1, sizeof(buffer1));
                GetWindowText(hwndInput2, buffer2, sizeof(buffer2));
                
                // Display them
                char message[512];
                sprintf(message, "Username: %s\nPassword: %s", buffer1, buffer2);
                MessageBox(hwnd, message, "Input Values", MB_OK | MB_ICONINFORMATION);
            }
            return 0;
        }
        
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            return 0;
        }
        
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
```

## Key Concepts Explained

### 1. **Message Handling**

The window procedure processes various messages:
- `WM_CREATE`: Initialize the control data structure
- `WM_DESTROY`: Clean up resources
- `WM_SETFOCUS`/`WM_KILLFOCUS`: Manage caret visibility
- `WM_CHAR`: Handle character input
- `WM_KEYDOWN`: Handle special keys (arrows, Delete, etc.)
- `WM_PAINT`: Render the control
- `WM_LBUTTONDOWN`: Handle mouse clicks for caret positioning
- `WM_GETTEXT`/`WM_SETTEXT`: Standard text messages

### 2. **Caret Management**

The caret (blinking cursor) is managed through Windows API:
- `CreateCaret()`: Creates the caret when focus is gained
- `SetCaretPos()`: Positions the caret
- `ShowCaret()`: Makes the caret visible
- `HideCaret()`: Hides the caret
- `DestroyCaret()`: Destroys the caret when focus is lost

### 3. **Text Buffer**

We maintain our own text buffer:
- Dynamic allocation allows flexibility
- Null-terminated for compatibility with standard C string functions
- Separate tracking of buffer size and text length

### 4. **Keyboard Input Processing**

Two key messages handle keyboard input:
- `WM_CHAR`: Receives translated character codes (handles Shift, Caps Lock, etc.)
- `WM_KEYDOWN`: Receives virtual key codes (for navigation keys)

### 5. **Painting**

The `WM_PAINT` handler:
- Draws background (different color when focused)
- Draws border (different style when focused)
- Renders text using `DrawText()`
- Uses double-buffering implicitly through `BeginPaint`/`EndPaint`

### 6. **Focus Management**

Focus determines:
- Whether keyboard input is processed
- Whether caret is visible
- Visual appearance (border color, background)

## Extensions and Improvements

This basic implementation can be extended with:

1. **Text Selection**: Track selection start/end, highlight selected text
2. **Clipboard Support**: Handle Ctrl+C, Ctrl+V, Ctrl+X
3. **Undo/Redo**: Maintain command history
4. **Multi-line Support**: Handle line breaks and scrolling
5. **Horizontal Scrolling**: Show text longer than control width
6. **Password Mode**: Display bullets instead of characters
7. **Input Validation**: Restrict character types (numbers only, etc.)
8. **Placeholder Text**: Show hint when empty
9. **Max Length**: Enforce maximum text length
10. **Custom Fonts and Colors**: Expose as properties

## Performance Considerations

- **Avoid frequent allocations**: Pre-allocate buffer with reasonable size
- **Minimize redraws**: Only invalidate when text actually changes
- **Use appropriate fonts**: Monospace fonts simplify caret positioning
- **Buffer management**: Consider reallocating buffer if text grows beyond initial size

## Conclusion

This implementation demonstrates the fundamental concepts of creating a custom text input control in WinAPI. While production controls have many more features, this provides a solid foundation for understanding how Windows processes keyboard input, manages focus, and renders text-based controls.

The window procedure pattern shown here applies to all custom controls in Windows, making it a valuable reference for any WinAPI GUI development.
