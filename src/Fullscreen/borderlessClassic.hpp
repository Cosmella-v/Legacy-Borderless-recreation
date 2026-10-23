
#pragma once
#ifdef GEODE_IS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Geode/cocos/robtop/glfw/glfw3.h>

namespace Cosmella::FakeFullScreen {
     struct lastWindowStateStruct {
        bool valid = false;
        bool wasMaximized = false;
        WINDOWPLACEMENT placement{};
        LONG_PTR style = 0;
        LONG_PTR exStyle = 0;
    }; inline lastWindowStateStruct g_state{};

    inline HWND getHwnd() {
        HWND result = WindowFromDC(wglGetCurrentDC()); // excuse me? why are you sometimes returning null
        if (result) return result;
        // fallback, idk why thats null
        EnumWindows([](HWND hwnd, LPARAM param) -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);

            if (pid != GetCurrentProcessId())
                return TRUE;
            if (!IsWindowVisible(hwnd))
                return TRUE;
            if (GetWindow(hwnd, GW_OWNER))
                return TRUE;

            *reinterpret_cast<HWND*>(param) = hwnd;
            return FALSE;
        }, reinterpret_cast<LPARAM>(&result));
        log::debug("res? {}", !result);
        return result;
    }

    inline bool isActive() {
        return g_state.valid;
    }

    inline bool setFakeFullscreen(bool enabled) {
        if (enabled == g_state.valid)
            return true;

        HWND hwnd = getHwnd();
        if (!hwnd)
            return false;

        if (enabled) {
            lastWindowStateStruct state{};
            state.style = GetWindowLongPtrW(hwnd, GWL_STYLE);
            state.exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);

            state.placement.length = sizeof(WINDOWPLACEMENT);
            if (!GetWindowPlacement(hwnd, &state.placement))
                return false;

            state.wasMaximized = IsZoomed(hwnd) != FALSE;

            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

            MONITORINFO mi{};
            mi.cbSize = sizeof(mi);

            if (!GetMonitorInfoW(monitor, &mi))
                return false;

            g_state = state;
            g_state.valid = true;

            LONG_PTR style = state.style;
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_BORDER |
                       WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX |
                       WS_MAXIMIZEBOX);
            style |= WS_POPUP;

            SetWindowLongPtrW(hwnd, GWL_STYLE, style);
            SetWindowLongPtrW(
                hwnd, GWL_EXSTYLE,
                state.exStyle & ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
            );

            if (!SetWindowPos(
                    hwnd, HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left + 1,
                    mi.rcMonitor.bottom - mi.rcMonitor.top + 1,
                    SWP_FRAMECHANGED | SWP_NOOWNERZORDER)) {
                SetWindowLongPtrW(hwnd, GWL_STYLE, state.style);
                SetWindowLongPtrW(hwnd, GWL_EXSTYLE, state.exStyle);
                g_state = {};
                return false;
            }

            return true;
        }

        SetWindowLongPtrW(hwnd, GWL_STYLE, g_state.style);
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, g_state.exStyle);

        SetWindowPlacement(hwnd, &g_state.placement);

        SetWindowPos(
            hwnd, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );

        g_state = {};
        return true;
    }
}
#endif