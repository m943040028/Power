

#include "pch.h"
#include <iostream>
#include <string>

#include <Windows.h>
#include <powrprof.h>
#pragma comment(lib, "PowrProf.lib") 


#include <winuser.h>



using namespace winrt;
using namespace Windows::Foundation;

bool connected = false;
UINT connection_timer = 0;
UINT suspend_timer = 0;

UINT idle_time_ms = 5 * 60 * 1000;

VOID CALLBACK SuspendTimerProc(
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{
    KillTimer(NULL, suspend_timer);
    suspend_timer = 0;

    std::cout << "suspend timer fired" << std::endl;
    SetSuspendState(FALSE, TRUE, FALSE);
}

bool is_client_connected() {
    char buf[256];
    FILE* p = _popen("netstat -na | findstr 47998", "r");
    std::string s;
    for (size_t count = 0; (count = fread(buf, 1, sizeof(buf), p));)
        s += std::string(buf, buf + count);
    _pclose(p);

    if (s.size() == 0) {
        return false;
    }
    else {
        return true;
    }
}

VOID CALLBACK MoonlightConnectionChecker(
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{
    bool client_connected = is_client_connected();

    if (connected && !client_connected) {
        std::cout << "moonlight client disconnected" << std::endl;
        connected = false;
    }
    
    if (!connected && client_connected) {
        std::cout << "moonlight client connected" << std::endl;
        connected = true;
    }

    if (!connected) {
        if (suspend_timer == 0) {
            suspend_timer = SetTimer(NULL,
                0, idle_time_ms,
                (TIMERPROC)SuspendTimerProc);
            std::cout << "suspend timer scheduled" << std::endl;
        }
    }
    else {
        if (suspend_timer != 0) {
            KillTimer(NULL, suspend_timer);
            suspend_timer = 0;
            std::cout << "suspend timer canceled" << std::endl;
        }
    }
}

int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());

    UINT connection_checker_interval_ms = 3000;
    connection_timer = SetTimer(NULL,
        0, connection_checker_interval_ms,
        (TIMERPROC)MoonlightConnectionChecker);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
  
}

