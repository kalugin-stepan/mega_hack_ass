#ifndef UNICODE
#define UNICODE
#endif 

#define WM_NEW_IMAGE WM_USER+1
#define WM_SCREEN_SIZE_AVALIBLE WM_USER+2

#define REMOTE_CONTROL
// #define VIDEO_STREAM

#include <iostream>
#include <boost/asio.hpp>
#include <windows.h>
#include <windowsx.h>
#include <objidl.h>
#include <gdiplus.h>
#include <gdiplus/gdiplusheaders.h>
#include <gdiplus/gdiplusgraphics.h>
#include <shlwapi.h>
#include <thread>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

const uint8_t on_connection_message = 2;
const unsigned PPS = 5;

#ifdef VIDEO_STREAM

class VideoStreamHandler {
    const uint8_t on_connection_message = 2;
    const char* jpg_start = "\xff\xd8";
    const size_t jpg_start_size = strlen(jpg_start);
    const char* jpg_end = "\xff\xd9";
    const size_t jpg_end_size = strlen(jpg_end);
    size_t package_size;
    boost::system::error_code ec;
    asio::io_context context;
    tcp::socket client;
    tcp::endpoint remote_address;
    int remote_screen_size[2];
    char* buffer = nullptr;
    size_t buffer_i = 0;
    bool _connected = false;
    HWND win = nullptr;
    Gdiplus::Image* img = nullptr;
public:
    VideoStreamHandler(size_t package_size = 5000): client(context) {
        this->package_size = package_size;
    }
    VideoStreamHandler(tcp::endpoint remote_address,
        size_t package_size = 5000): VideoStreamHandler(package_size) {
        connect(remote_address);
    }
    ~VideoStreamHandler() {
        if (buffer != nullptr) delete[] buffer;
    }
    bool connect(tcp::endpoint remote_address) {
        this->remote_address = remote_address;
        return connect();
    }
    bool connect() {
        client.connect(remote_address, ec);
        client.write_some(asio::buffer(&on_connection_message, 1), ec);
        if (ec) {
            std::cerr << ec.message() << std::endl;
            return false;
        }
        _connected = true;
        return true;
    }
    bool connected() {
        return _connected;
    }
    int get_remote_screen_width() {
        return remote_screen_size[0];
    }
    int get_remote_screen_height() {
        return remote_screen_size[1];
    }

    void set_window(HWND win) {
        this->win = win;
    }

    Gdiplus::Image* get_current_image() {
        return img;
    }

    void run() {
        if (!_connected || win == nullptr) return;
        start:
        client.read_some(asio::buffer(remote_screen_size, sizeof(int)*2), ec);
        if (ec) {
            std::cerr << ec.message() << std::endl;
            return;
        }
        buffer = new char[remote_screen_size[0]*remote_screen_size[1]*3];
        SendMessage(win, WM_SCREEN_SIZE_AVALIBLE, NULL, NULL);
        size_t start;
        client.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 2000 });
        for (;;) {
            size_t cur_size = client.read_some(asio::buffer(buffer+buffer_i, package_size), ec);
            if (ec) {
                if (ec.value() == asio::error::basic_errors::timed_out) {
                    client.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 0 });
                    goto start;
                }
                std::cerr << ec.message() << std::endl;
                break;
            }
            if (buffer_i == 0) {
                start = find(buffer, cur_size, jpg_start, jpg_start_size);
                if (start != -1) {
                    buffer_i += cur_size;
                }
                continue;
            }
            size_t end = find(buffer+buffer_i, cur_size, jpg_end, jpg_end_size);
            if (end != -1) {
                end += buffer_i;
                buffer_i += cur_size;
                
                IStream* memstream = SHCreateMemStream((const BYTE*)buffer, end - start + 2);

                if (memstream != NULL) {
                    img = Gdiplus::Image::FromStream(memstream);
                    SendMessage(win, WM_NEW_IMAGE, NULL, NULL);
                    delete memstream;
                }

                memcpy(buffer, buffer + end + 2, buffer_i - end - 2);
                start = find(buffer, buffer_i, jpg_start, jpg_start_size);
                if (start == -1) buffer_i = 0;
                else buffer_i -= end + 2;
                continue;
            }
            buffer_i += cur_size;
        }
    }
    void run_thread() {
        std::thread(VideoStreamHandler::run, this).detach();
    }
private:
    ptrdiff_t find(const char* data, const size_t data_size, const char* target, const size_t target_size) {
        for (size_t i = 0; i < data_size - target_size + 1; i++) {
            for (size_t j = 0; j < target_size; j++) {
                if (data[i + j] != target[j]) goto end;
            }
            return i;
        end: continue;
        }
        return -1;
    }
};

#endif

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef REMOTE_CONTROL
boost::system::error_code ec;
asio::io_context context;
tcp::endpoint remote_address(address::from_string("192.168.1.100"), 6000);
tcp::socket client(context);
#endif

#ifdef VIDEO_STREAM
VideoStreamHandler vsh;
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    setlocale(LC_ALL, "russian");

#ifdef VIDEO_STREAM
    if (!vsh.connect(remote_address)) {
        std::cerr << "Failed to connect to video server" << std::endl;
        return 1;
    }
#endif

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &input, NULL);
    if (status != Gdiplus::Ok) {
        std::cerr << "Failed to init GDI+" << std::endl;
        return status;
    }

    const wchar_t CLASS_NAME[]  = L"Hack ass client";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Lansky gay",
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,

        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,    
        NULL,
        hInstance,
        NULL
        );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

#ifdef VIDEO_STREAM
    vsh.set_window(hwnd);
    vsh.run_thread();
#endif

#ifdef REMOTE_CONTROL
    connect:
    client.connect(remote_address, ec);
    client.write_some(asio::buffer(&on_connection_message, 1), ec);
    if (ec) {
        std::cerr << ec.message() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        goto connect;
    }
#endif

    MSG msg = { };

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

std::chrono::system_clock::time_point last_mousemove_time = std::chrono::system_clock::now();;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            
            HDC hdc = BeginPaint(hwnd, &ps);

#ifdef VIDEO_STREAM
            Gdiplus::Image* img = vsh.get_current_image();

            if (img != nullptr) {
                Gdiplus::Graphics graphics(hdc);

                graphics.DrawImage(img, 0, 0);

                delete img;
            }
            else {
                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            }
#else
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
#endif

            EndPaint(hwnd, &ps);
            return TRUE;
        }
    case WM_NEW_IMAGE:
        {
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            return TRUE;
        }
#ifdef REMOTE_CONTROL
    case WM_KEYDOWN:
        {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = wParam;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }

            return TRUE;
        }
    case WM_KEYUP:
        {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = wParam;
            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }

            return TRUE;
        }
    case WM_MOUSEMOVE:
        {
            auto cur_time = std::chrono::system_clock::now();
            auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - last_mousemove_time).count();
            if (dur < 1000/PPS) return 0;
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dx = GET_X_LPARAM(lParam)*2;
            input.mi.dy = GET_Y_LPARAM(lParam)*2;
            input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }

            return TRUE;
        }
    case WM_LBUTTONDOWN:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }
        }
    case WM_LBUTTONUP:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }
        }
    case WM_RBUTTONDOWN:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }
        }
    case WM_RBUTTONUP:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }
        }
    case WM_MOUSEWHEEL:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = GET_WHEEL_DELTA_WPARAM(wParam);

            client.write_some(asio::buffer(&input, sizeof(input)), ec);
            if (ec) {
                std::cerr << ec.message() << std::endl;
            }

            return TRUE;
        }
#endif

#ifdef VIDEO_STREAM
    case WM_SCREEN_SIZE_AVALIBLE:
        {
            SetWindowPos(hwnd, 0, 0, 0,
                vsh.get_remote_screen_width()/2,
                vsh.get_remote_screen_height()/2,   
                SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            return TRUE;
        }
#endif
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}