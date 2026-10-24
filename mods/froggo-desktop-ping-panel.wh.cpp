// ==WindhawkMod==
// @id              froggo-desktop-ping-panel
// @name            Froggo Desktop Ping Panel
// @description     A draggable desktop widget that monitors up to 10 IPv4, IPv6, or hostname targets with ICMP and TCP fallback.
// @version         0.1.11
// @author          Sapito Sucio
// @github          https://github.com/SapitoSucio
// @include         windhawk.exe
// @compilerOptions -D_WIN32_WINNT=0x0A00 -DWIN32_LEAN_AND_MEAN -DNOMINMAX -lws2_32 -liphlpapi -lgdiplus -ldwmapi -lole32 -lshell32 -lgdi32 -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Froggo Desktop Ping Panel

![Running desktop widget](https://raw.githubusercontent.com/SapitoSucio/windhawk-mods/d3ffc73d36d4672b5a679b0c61a1ac7845b78690/media/froggo-desktop-ping-panel/screenshot.png)

A compact desktop widget inspired by the Rainmeter Ping skin.

Each enabled row normally displays a right-aligned latency value and a colored
status dot. Hovering a row replaces the latency with the destination name.

The mod supports:

- Up to 10 destinations.
- IPv4, IPv6, and hostnames.
- ICMP first, followed by TCP fallback when ICMP fails.
- Global defaults with per-destination overrides.
- Desktop-attached mode by default, or an always-on-top mode.
- Automatic hiding over fullscreen apps in always-on-top mode.
- Dragging with saved multi-monitor position and an optional position lock.
- Optional Ctrl-drag override while the position lock is enabled.
- Optional click-through mode. Hold Ctrl to interact temporarily.
- Transparent or styled backgrounds, borders, typography, and spacing.

## Installation

Create a new mod in Windhawk, replace the editor contents with this file,
compile it, and exit editing mode.

## Notes

- TCP latency measures the time needed to establish a TCP connection. It is not
  identical to ICMP latency, but it is useful when a host blocks ICMP.
- Values lower than 250 ms for the update interval are clamped to 250 ms.
- DNS resolution uses the Windows resolver and is refreshed periodically.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- General:
  - intervalMs: 1000
    $name: Global update interval
    $description: Milliseconds between checks. Minimum 250.
  - timeoutMs: 3000
    $name: Global timeout
    $description: Timeout in milliseconds for ICMP and TCP attempts.
  - tcpPort: 443
    $name: Global TCP fallback port
    $description: Used only when ICMP fails.
  - warningThresholdMs: 80
    $name: Warning threshold
    $description: Values at or above this threshold use the warning color.
  - badThresholdMs: 150
    $name: Bad threshold
    $description: Values at or above this threshold use the bad color.
  - goodColor: "#67E480"
    $name: Good color
  - warningColor: "#FFD45C"
    $name: Warning color
  - badColor: "#FF6464"
    $name: Bad color
  - timeoutText: "TIMEOUT"
    $name: Timeout text
  - timeoutColor: "#8C8C8C"
    $name: Timeout color
  $name: Global ping defaults

- Panel:
  - mode: desktop
    $name: Window mode
    $options:
      - desktop: Attached to desktop
      - topmost: Always on top
  - lockPosition: false
    $name: Lock position
    $description: Prevents normal dragging while keeping hover behavior active.
  - ctrlDragOverride: true
    $name: Ctrl overrides position lock
    $description: When Lock position is enabled, hold Ctrl while dragging to move the panel temporarily.
  - clickThrough: false
    $name: Click through
    $description: Sends mouse input to windows below the widget. Hold Ctrl to interact with the widget temporarily.
  - fontName: "Segoe UI"
    $name: Font
  - fontSize: 14
    $name: Font size
  - fontWeight: 600
    $name: Font weight
    $description: Use 400 for regular or 700 for bold.
  - textColor: "#FFFFFF"
    $name: Text color
  - textVerticalOffset: 2
    $name: Text vertical offset
    $description: Moves text vertically without changing its rendering. Positive values move it down; negative values move it up.
  - dotSize: 8
    $name: Dot diameter
  - textDotGap: 7
    $name: Text-to-dot gap
  - rowGap: 2
    $name: Vertical row gap
    $description: Extra pixels between entries. Use a negative value to tighten the rows.
  - paddingX: 8
    $name: Horizontal padding
  - paddingY: 4
    $name: Vertical padding
  - backgroundColor: "#000000"
    $name: Background color
  - backgroundOpacity: 0
    $name: Background opacity
    $description: Percentage from 0 to 100. Default is fully transparent.
  - cornerRadius: 8
    $name: Corner radius
  - borderEnabled: false
    $name: Draw border
  - borderColor: "#FFFFFF"
    $name: Border color
  - borderOpacity: 35
    $name: Border opacity
    $description: Percentage from 0 to 100.
  - borderThickness: 1
    $name: Border thickness
  $name: Panel appearance and behavior

- Destinations:
  - targets:
    - - enabled: true
        $name: Enabled
      - name: "Google"
        $name: Name
      - address: "8.8.8.8"
        $name: IPv4, IPv6, or hostname
      - overrideInterval: false
        $name: Use custom update interval
      - intervalMs: 1000
        $name: Custom update interval
      - overrideTcpPort: false
        $name: Use custom TCP port
      - tcpPort: 443
        $name: Custom TCP fallback port
      - overrideStatus: false
        $name: Use custom thresholds and colors
      - warningThresholdMs: 80
        $name: Custom warning threshold
      - badThresholdMs: 150
        $name: Custom bad threshold
      - goodColor: "#67E480"
        $name: Custom good color
      - warningColor: "#FFD45C"
        $name: Custom warning color
      - badColor: "#FF6464"
        $name: Custom bad color
      - timeoutText: "TIMEOUT"
        $name: Custom timeout text
      - timeoutColor: "#8C8C8C"
        $name: Custom timeout color
    - - enabled: true
        $name: Enabled
      - name: "Cloudflare"
        $name: Name
      - address: "1.1.1.1"
        $name: IPv4, IPv6, or hostname
      - overrideInterval: false
        $name: Use custom update interval
      - intervalMs: 1000
        $name: Custom update interval
      - overrideTcpPort: false
        $name: Use custom TCP port
      - tcpPort: 443
        $name: Custom TCP fallback port
      - overrideStatus: false
        $name: Use custom thresholds and colors
      - warningThresholdMs: 80
        $name: Custom warning threshold
      - badThresholdMs: 150
        $name: Custom bad threshold
      - goodColor: "#67E480"
        $name: Custom good color
      - warningColor: "#FFD45C"
        $name: Custom warning color
      - badColor: "#FF6464"
        $name: Custom bad color
      - timeoutText: "TIMEOUT"
        $name: Custom timeout text
      - timeoutColor: "#8C8C8C"
        $name: Custom timeout color
    - - enabled: true
        $name: Enabled
      - name: "Pi4"
        $name: Name
      - address: "192.168.1.195"
        $name: IPv4, IPv6, or hostname
      - overrideInterval: false
        $name: Use custom update interval
      - intervalMs: 1000
        $name: Custom update interval
      - overrideTcpPort: false
        $name: Use custom TCP port
      - tcpPort: 443
        $name: Custom TCP fallback port
      - overrideStatus: false
        $name: Use custom thresholds and colors
      - warningThresholdMs: 80
        $name: Custom warning threshold
      - badThresholdMs: 150
        $name: Custom bad threshold
      - goodColor: "#67E480"
        $name: Custom good color
      - warningColor: "#FFD45C"
        $name: Custom warning color
      - badColor: "#FF6464"
        $name: Custom bad color
      - timeoutText: "TIMEOUT"
        $name: Custom timeout text
      - timeoutColor: "#8C8C8C"
        $name: Custom timeout color
    $name: Ping destinations
    $description: Add, remove, and reorder entries. Only the first 10 are used.
  $name: Destinations
*/
// ==/WindhawkModSettings==

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cwctype>
#include <string>

struct Argb {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum class LatencyState {
    Pending,
    Good,
    Warning,
    Bad,
    Timeout,
};

struct RectI {
    int left;
    int top;
    int right;
    int bottom;
};

static int HexDigit(wchar_t ch) {
    if (ch >= L'0' && ch <= L'9') {
        return ch - L'0';
    }

    ch = static_cast<wchar_t>(towupper(ch));
    if (ch >= L'A' && ch <= L'F') {
        return 10 + ch - L'A';
    }

    return -1;
}

static Argb ParseHexColor(const std::wstring& text, Argb fallback) {
    std::wstring value = text;
    if (!value.empty() && value.front() == L'#') {
        value.erase(value.begin());
    }

    if (value.size() != 6 && value.size() != 8) {
        return fallback;
    }

    uint32_t packed = 0;
    for (wchar_t ch : value) {
        int digit = HexDigit(ch);
        if (digit < 0) {
            return fallback;
        }
        packed = (packed << 4) | static_cast<uint32_t>(digit);
    }

    if (value.size() == 6) {
        return {
            255,
            static_cast<uint8_t>((packed >> 16) & 0xFF),
            static_cast<uint8_t>((packed >> 8) & 0xFF),
            static_cast<uint8_t>(packed & 0xFF),
        };
    }

    return {
        static_cast<uint8_t>((packed >> 24) & 0xFF),
        static_cast<uint8_t>((packed >> 16) & 0xFF),
        static_cast<uint8_t>((packed >> 8) & 0xFF),
        static_cast<uint8_t>(packed & 0xFF),
    };
}

static LatencyState ClassifyLatency(bool success,
                                    int latencyMs,
                                    int warningThresholdMs,
                                    int badThresholdMs) {
    if (!success) {
        return LatencyState::Timeout;
    }

    warningThresholdMs = std::max(0, warningThresholdMs);
    badThresholdMs = std::max(warningThresholdMs + 1, badThresholdMs);

    if (latencyMs < warningThresholdMs) {
        return LatencyState::Good;
    }
    if (latencyMs < badThresholdMs) {
        return LatencyState::Warning;
    }
    return LatencyState::Bad;
}

static std::wstring BuildDisplayText(bool hovered,
                                     const std::wstring& name,
                                     LatencyState state,
                                     int latencyMs,
                                     const std::wstring& timeoutText) {
    if (hovered) {
        return name;
    }
    if (state == LatencyState::Timeout) {
        return timeoutText;
    }
    if (state == LatencyState::Pending) {
        return L"...";
    }
    return std::to_wstring(std::max(0, latencyMs));
}

static RectI ClampRectToBounds(RectI rect, RectI bounds) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int boundsWidth = bounds.right - bounds.left;
    int boundsHeight = bounds.bottom - bounds.top;

    if (width >= boundsWidth) {
        rect.left = bounds.left;
    } else {
        rect.left = std::clamp(rect.left, bounds.left, bounds.right - width);
    }

    if (height >= boundsHeight) {
        rect.top = bounds.top;
    } else {
        rect.top = std::clamp(rect.top, bounds.top, bounds.bottom - height);
    }

    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
    return rect;
}

static int CalculateRowStride(int rowHeight, int rowGap) {
    return std::max(1, rowHeight + rowGap);
}

static float CalculateTextTop(float rowTop, int verticalOffset) {
    return rowTop + static_cast<float>(verticalOffset);
}

static int CalculatePanelHeight(int paddingY,
                                int rows,
                                int rowHeight,
                                int rowGap) {
    if (rows <= 0) {
        return 1;
    }

    int stride = CalculateRowStride(rowHeight, rowGap);
    return std::max(1, paddingY * 2 + rowHeight + (rows - 1) * stride);
}

static bool ShouldStartDrag(bool lockPosition,
                            bool ctrlDragOverride,
                            bool ctrlPressed) {
    return !lockPosition || (ctrlDragOverride && ctrlPressed);
}

static bool ShouldUseClickThrough(bool clickThrough,
                                  bool dragging,
                                  bool ctrlPressed) {
    return clickThrough && !dragging && !ctrlPressed;
}

#ifndef PING_WIDGET_CORE_TEST

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#endif
#include <winsock2.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <objbase.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <shellapi.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>
#include <new>
#include <thread>
#include <utility>
#include <vector>

using namespace Gdiplus;

// Declared locally because some Windhawk MinGW packages don't ship
// shobjidl_core.h, while this public COM interface only needs IUnknown.
struct IVirtualDesktopManager : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
        HWND topLevelWindow, BOOL* onCurrentDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
        HWND topLevelWindow, GUID* desktopId) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
        HWND topLevelWindow, REFGUID desktopId) = 0;
};

#pragma message("Compiling Froggo Desktop Ping Panel v0.1.11")

namespace {

constexpr int kMaxTargets = 10;
constexpr UINT kMessageResult = WM_APP + 1;
constexpr UINT kMessageReloadSettings = WM_APP + 2;
constexpr UINT kMessageExit = WM_APP + 3;
constexpr UINT_PTR kEnvironmentTimer = 1;
constexpr UINT_PTR kInteractionTimer = 2;
constexpr UINT kInteractionPollMs = 25;
constexpr wchar_t kWindowClassName[] = L"WindhawkDesktopPingPanelWindow";

constexpr GUID kClsidVirtualDesktopManager = {
    0xAA509086,
    0x5CA9,
    0x4C25,
    {0x8F, 0x95, 0x58, 0x9D, 0x3C, 0x07, 0xB4, 0x8A},
};
constexpr GUID kIidVirtualDesktopManager = {
    0xA5CD92FF,
    0x29BE,
    0x454C,
    {0x8D, 0x04, 0xD8, 0x28, 0x79, 0xFB, 0x3F, 0x1B},
};

enum class PanelMode {
    Desktop,
    Topmost,
};

enum class ProbeMethod {
    None,
    Icmp,
    Tcp,
};

struct StatusStyle {
    int warningThresholdMs = 80;
    int badThresholdMs = 150;
    Argb goodColor{255, 0x67, 0xE4, 0x80};
    Argb warningColor{255, 0xFF, 0xD4, 0x5C};
    Argb badColor{255, 0xFF, 0x64, 0x64};
    std::wstring timeoutText = L"TIMEOUT";
    Argb timeoutColor{255, 0x8C, 0x8C, 0x8C};
};

struct TargetConfig {
    bool enabled = false;
    std::wstring name;
    std::wstring address;
    int intervalMs = 1000;
    int timeoutMs = 3000;
    uint16_t tcpPort = 443;
    StatusStyle status;
};

struct PanelSettings {
    PanelMode mode = PanelMode::Desktop;
    bool lockPosition = false;
    bool ctrlDragOverride = true;
    bool clickThrough = false;
    std::wstring fontName = L"Segoe UI";
    int fontSize = 14;
    int fontWeight = 600;
    Argb textColor{255, 255, 255, 255};
    int textVerticalOffset = 2;
    int dotSize = 8;
    int textDotGap = 7;
    int rowGap = 2;
    int paddingX = 8;
    int paddingY = 4;
    Argb backgroundColor{255, 0, 0, 0};
    int backgroundOpacity = 0;
    int cornerRadius = 8;
    bool borderEnabled = false;
    Argb borderColor{255, 255, 255, 255};
    int borderOpacity = 35;
    int borderThickness = 1;
};

struct ModSettings {
    int globalIntervalMs = 1000;
    int globalTimeoutMs = 3000;
    uint16_t globalTcpPort = 443;
    StatusStyle globalStatus;
    PanelSettings panel;
    std::vector<TargetConfig> targets;
};

struct TargetRuntime {
    TargetConfig config;
    LatencyState state = LatencyState::Pending;
    int latencyMs = 0;
    ProbeMethod method = ProbeMethod::None;
};

struct ProbeResult {
    bool success = false;
    int latencyMs = 0;
    ProbeMethod method = ProbeMethod::None;
};

struct ResultMessage {
    uint64_t generation = 0;
    int index = -1;
    ProbeResult result;
};

struct ResolvedAddress {
    sockaddr_storage address{};
    int length = 0;
};

struct MonitorSearchContext {
    std::wstring deviceName;
    HMONITOR result = nullptr;
};

std::atomic<bool> g_exitRequested{false};
HWND g_window = nullptr;
HWND g_desktopHost = nullptr;
HANDLE g_uiThread = nullptr;
DWORD g_uiThreadId = 0;
HANDLE g_workerStopEvent = nullptr;
std::vector<std::thread> g_workers;
uint64_t g_workerGeneration = 0;
ModSettings g_settings;
std::vector<TargetRuntime> g_targets;
IVirtualDesktopManager* g_virtualDesktopManager = nullptr;
ULONG_PTR g_gdiplusToken = 0;

bool g_trackingMouse = false;
int g_hoveredRow = -1;
bool g_dragging = false;
POINT g_dragOffset{};
int g_panelWidth = 1;
int g_panelHeight = 1;
int g_rowHeight = 20;
int g_rowStride = 22;
float g_textColumnWidth = 40.0f;
bool g_hiddenForFullscreen = false;
bool g_allowWindowShow = false;
bool g_loggedNoTargets = false;
bool g_loggedLayeredWindowFailure = false;

static int ClampInt(int value, int minimum, int maximum) {
    return std::clamp(value, minimum, maximum);
}

static bool IsCtrlPressed() {
    return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
}

static bool ShouldPassClicksThrough() {
    return ShouldUseClickThrough(g_settings.panel.clickThrough, g_dragging,
                                 IsCtrlPressed());
}

static uint8_t ApplyOpacity(uint8_t alpha, int percentage) {
    percentage = ClampInt(percentage, 0, 100);
    return static_cast<uint8_t>((static_cast<int>(alpha) * percentage + 50) / 100);
}

static Color ToGdiColor(Argb color, int opacityPercentage = 100) {
    return Color(ApplyOpacity(color.a, opacityPercentage), color.r, color.g,
                 color.b);
}

static std::wstring GetStringSetting(const wchar_t* format, int index = -1) {
    PCWSTR raw = index >= 0 ? Wh_GetStringSetting(format, index)
                            : Wh_GetStringSetting(format);
    std::wstring value = raw ? raw : L"";
    if (raw) {
        Wh_FreeStringSetting(raw);
    }
    return value;
}

static Argb GetColorSetting(const wchar_t* key, Argb fallback) {
    return ParseHexColor(GetStringSetting(key), fallback);
}

static Argb GetIndexedColorSetting(const wchar_t* key,
                                   int index,
                                   Argb fallback) {
    return ParseHexColor(GetStringSetting(key, index), fallback);
}

static void NormalizeStatusStyle(StatusStyle& style) {
    style.warningThresholdMs = ClampInt(style.warningThresholdMs, 0, 60000);
    style.badThresholdMs = ClampInt(style.badThresholdMs, 1, 60000);
    if (style.badThresholdMs <= style.warningThresholdMs) {
        style.badThresholdMs = std::min(60000, style.warningThresholdMs + 1);
    }
    if (style.timeoutText.empty()) {
        style.timeoutText = L"TIMEOUT";
    }
}

static ModSettings LoadSettings() {
    ModSettings settings;

    settings.globalIntervalMs =
        ClampInt(Wh_GetIntSetting(L"General.intervalMs"), 250, 60000);
    settings.globalTimeoutMs =
        ClampInt(Wh_GetIntSetting(L"General.timeoutMs"), 100, 30000);
    settings.globalTcpPort = static_cast<uint16_t>(
        ClampInt(Wh_GetIntSetting(L"General.tcpPort"), 1, 65535));

    settings.globalStatus.warningThresholdMs =
        Wh_GetIntSetting(L"General.warningThresholdMs");
    settings.globalStatus.badThresholdMs =
        Wh_GetIntSetting(L"General.badThresholdMs");
    settings.globalStatus.goodColor = GetColorSetting(
        L"General.goodColor", settings.globalStatus.goodColor);
    settings.globalStatus.warningColor = GetColorSetting(
        L"General.warningColor", settings.globalStatus.warningColor);
    settings.globalStatus.badColor = GetColorSetting(
        L"General.badColor", settings.globalStatus.badColor);
    settings.globalStatus.timeoutText =
        GetStringSetting(L"General.timeoutText");
    settings.globalStatus.timeoutColor = GetColorSetting(
        L"General.timeoutColor", settings.globalStatus.timeoutColor);
    NormalizeStatusStyle(settings.globalStatus);

    std::wstring mode = GetStringSetting(L"Panel.mode");
    settings.panel.mode =
        _wcsicmp(mode.c_str(), L"topmost") == 0 ? PanelMode::Topmost
                                                : PanelMode::Desktop;
    settings.panel.lockPosition =
        Wh_GetIntSetting(L"Panel.lockPosition") != 0;
    settings.panel.ctrlDragOverride =
        Wh_GetIntSetting(L"Panel.ctrlDragOverride") != 0;
    settings.panel.clickThrough =
        Wh_GetIntSetting(L"Panel.clickThrough") != 0;
    settings.panel.fontName = GetStringSetting(L"Panel.fontName");
    if (settings.panel.fontName.empty()) {
        settings.panel.fontName = L"Segoe UI";
    }
    settings.panel.fontSize =
        ClampInt(Wh_GetIntSetting(L"Panel.fontSize"), 8, 72);
    settings.panel.fontWeight =
        ClampInt(Wh_GetIntSetting(L"Panel.fontWeight"), 100, 900);
    settings.panel.textColor =
        GetColorSetting(L"Panel.textColor", settings.panel.textColor);
    settings.panel.textVerticalOffset =
        ClampInt(Wh_GetIntSetting(L"Panel.textVerticalOffset"), -20, 20);
    settings.panel.dotSize =
        ClampInt(Wh_GetIntSetting(L"Panel.dotSize"), 2, 40);
    settings.panel.textDotGap =
        ClampInt(Wh_GetIntSetting(L"Panel.textDotGap"), 0, 50);
    settings.panel.rowGap =
        ClampInt(Wh_GetIntSetting(L"Panel.rowGap"), -40, 100);
    settings.panel.paddingX =
        ClampInt(Wh_GetIntSetting(L"Panel.paddingX"), 0, 100);
    settings.panel.paddingY =
        ClampInt(Wh_GetIntSetting(L"Panel.paddingY"), 0, 100);
    settings.panel.backgroundColor = GetColorSetting(
        L"Panel.backgroundColor", settings.panel.backgroundColor);
    settings.panel.backgroundOpacity =
        ClampInt(Wh_GetIntSetting(L"Panel.backgroundOpacity"), 0, 100);
    settings.panel.cornerRadius =
        ClampInt(Wh_GetIntSetting(L"Panel.cornerRadius"), 0, 100);
    settings.panel.borderEnabled =
        Wh_GetIntSetting(L"Panel.borderEnabled") != 0;
    settings.panel.borderColor =
        GetColorSetting(L"Panel.borderColor", settings.panel.borderColor);
    settings.panel.borderOpacity =
        ClampInt(Wh_GetIntSetting(L"Panel.borderOpacity"), 0, 100);
    settings.panel.borderThickness =
        ClampInt(Wh_GetIntSetting(L"Panel.borderThickness"), 1, 20);

    settings.targets.reserve(kMaxTargets);
    for (int index = 0; index < kMaxTargets; ++index) {
        std::wstring name =
            GetStringSetting(L"Destinations.targets[%d].name", index);
        std::wstring address =
            GetStringSetting(L"Destinations.targets[%d].address", index);

        if (name.empty() && address.empty()) {
            break;
        }

        TargetConfig target;
        target.enabled =
            Wh_GetIntSetting(L"Destinations.targets[%d].enabled", index) != 0;
        target.name = name.empty() ? address : name;
        target.address = address;
        target.intervalMs = settings.globalIntervalMs;
        target.timeoutMs = settings.globalTimeoutMs;
        target.tcpPort = settings.globalTcpPort;
        target.status = settings.globalStatus;

        if (Wh_GetIntSetting(
                L"Destinations.targets[%d].overrideInterval", index)) {
            target.intervalMs = ClampInt(
                Wh_GetIntSetting(L"Destinations.targets[%d].intervalMs", index),
                250, 60000);
        }

        if (Wh_GetIntSetting(
                L"Destinations.targets[%d].overrideTcpPort", index)) {
            target.tcpPort = static_cast<uint16_t>(ClampInt(
                Wh_GetIntSetting(L"Destinations.targets[%d].tcpPort", index),
                1, 65535));
        }

        if (Wh_GetIntSetting(
                L"Destinations.targets[%d].overrideStatus", index)) {
            target.status.warningThresholdMs = Wh_GetIntSetting(
                L"Destinations.targets[%d].warningThresholdMs", index);
            target.status.badThresholdMs = Wh_GetIntSetting(
                L"Destinations.targets[%d].badThresholdMs", index);
            target.status.goodColor = GetIndexedColorSetting(
                L"Destinations.targets[%d].goodColor", index,
                target.status.goodColor);
            target.status.warningColor = GetIndexedColorSetting(
                L"Destinations.targets[%d].warningColor", index,
                target.status.warningColor);
            target.status.badColor = GetIndexedColorSetting(
                L"Destinations.targets[%d].badColor", index,
                target.status.badColor);
            std::wstring customTimeoutText = GetStringSetting(
                L"Destinations.targets[%d].timeoutText", index);
            if (!customTimeoutText.empty()) {
                target.status.timeoutText = std::move(customTimeoutText);
            }
            target.status.timeoutColor = GetIndexedColorSetting(
                L"Destinations.targets[%d].timeoutColor", index,
                target.status.timeoutColor);
            NormalizeStatusStyle(target.status);
        }

        settings.targets.push_back(std::move(target));
    }

    return settings;
}

static bool ResolveTarget(const std::wstring& address,
                          std::vector<ResolvedAddress>& output) {
    output.clear();

    ADDRINFOW hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    PADDRINFOW result = nullptr;
    int error = GetAddrInfoW(address.c_str(), nullptr, &hints, &result);
    if (error != 0 || !result) {
        Wh_Log(L"GetAddrInfoW failed for %s: %d", address.c_str(), error);
        return false;
    }

    for (PADDRINFOW item = result; item; item = item->ai_next) {
        if (item->ai_family != AF_INET && item->ai_family != AF_INET6) {
            continue;
        }
        if (item->ai_addrlen > sizeof(sockaddr_storage)) {
            continue;
        }

        ResolvedAddress resolved;
        memcpy(&resolved.address, item->ai_addr, item->ai_addrlen);
        resolved.length = static_cast<int>(item->ai_addrlen);
        output.push_back(resolved);
    }

    FreeAddrInfoW(result);

    std::stable_sort(output.begin(), output.end(),
                     [](const ResolvedAddress& left,
                        const ResolvedAddress& right) {
                         return left.address.ss_family == AF_INET &&
                                right.address.ss_family == AF_INET6;
                     });
    return !output.empty();
}

static int RemainingMilliseconds(
    std::chrono::steady_clock::time_point deadline) {
    auto now = std::chrono::steady_clock::now();
    if (now >= deadline) {
        return 0;
    }
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                         deadline - now)
                         .count();
    return static_cast<int>(std::clamp<int64_t>(remaining, 1, 30000));
}

static ProbeResult ProbeIcmpAddress(const ResolvedAddress& resolved,
                                    int timeoutMs) {
    static const char payload[] = "WindhawkDesktopPing";

    if (resolved.address.ss_family == AF_INET) {
        HANDLE handle = IcmpCreateFile();
        if (handle == INVALID_HANDLE_VALUE) {
            return {};
        }

        auto destination =
            reinterpret_cast<const sockaddr_in*>(&resolved.address);
        std::vector<unsigned char> reply(sizeof(ICMP_ECHO_REPLY) +
                                         sizeof(payload) + 32);
        DWORD replies = IcmpSendEcho(
            handle, destination->sin_addr.S_un.S_addr,
            const_cast<char*>(payload), static_cast<WORD>(sizeof(payload)),
            nullptr, reply.data(), static_cast<DWORD>(reply.size()), timeoutMs);

        ProbeResult result;
        if (replies > 0) {
            auto* echo = reinterpret_cast<ICMP_ECHO_REPLY*>(reply.data());
            if (echo->Status == IP_SUCCESS) {
                result.success = true;
                result.latencyMs = static_cast<int>(echo->RoundTripTime);
                result.method = ProbeMethod::Icmp;
            }
        }

        IcmpCloseHandle(handle);
        return result;
    }

    if (resolved.address.ss_family == AF_INET6) {
        HANDLE handle = Icmp6CreateFile();
        if (handle == INVALID_HANDLE_VALUE) {
            return {};
        }

        sockaddr_in6 source{};
        source.sin6_family = AF_INET6;
        sockaddr_in6 destination =
            *reinterpret_cast<const sockaddr_in6*>(&resolved.address);
        std::vector<unsigned char> reply(sizeof(ICMPV6_ECHO_REPLY) +
                                         sizeof(payload) + 32);

        DWORD replies = Icmp6SendEcho2(
            handle, nullptr, nullptr, nullptr, &source, &destination,
            const_cast<char*>(payload), static_cast<WORD>(sizeof(payload)),
            nullptr, reply.data(), static_cast<DWORD>(reply.size()), timeoutMs);

        ProbeResult result;
        if (replies > 0) {
            auto* echo = reinterpret_cast<ICMPV6_ECHO_REPLY*>(reply.data());
            if (echo->Status == IP_SUCCESS) {
                result.success = true;
                result.latencyMs = static_cast<int>(echo->RoundTripTime);
                result.method = ProbeMethod::Icmp;
            }
        }

        IcmpCloseHandle(handle);
        return result;
    }

    return {};
}

static ProbeResult ProbeIcmp(const std::vector<ResolvedAddress>& addresses,
                             int timeoutMs) {
    if (addresses.empty()) {
        return {};
    }

    auto deadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(timeoutMs);

    for (size_t index = 0; index < addresses.size(); ++index) {
        int remaining = RemainingMilliseconds(deadline);
        if (remaining <= 0) {
            break;
        }

        int addressesLeft = static_cast<int>(addresses.size() - index);
        int attemptTimeout = std::max(100, remaining / addressesLeft);
        ProbeResult result =
            ProbeIcmpAddress(addresses[index], attemptTimeout);
        if (result.success) {
            return result;
        }
    }

    return {};
}

static ProbeResult ProbeTcpAddress(const ResolvedAddress& resolved,
                                   uint16_t port,
                                   int timeoutMs) {
    sockaddr_storage target = resolved.address;
    if (target.ss_family == AF_INET) {
        reinterpret_cast<sockaddr_in*>(&target)->sin_port = htons(port);
    } else if (target.ss_family == AF_INET6) {
        reinterpret_cast<sockaddr_in6*>(&target)->sin6_port = htons(port);
    } else {
        return {};
    }

    SOCKET socketHandle =
        socket(target.ss_family, SOCK_STREAM, IPPROTO_TCP);
    if (socketHandle == INVALID_SOCKET) {
        return {};
    }

    u_long nonBlocking = 1;
    if (ioctlsocket(socketHandle, FIONBIO, &nonBlocking) != 0) {
        closesocket(socketHandle);
        return {};
    }

    auto started = std::chrono::steady_clock::now();
    int connectResult = connect(socketHandle,
                                reinterpret_cast<const sockaddr*>(&target),
                                resolved.length);

    bool connected = false;
    if (connectResult == 0) {
        connected = true;
    } else {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS ||
            error == WSAEINVAL) {
            fd_set writeSet;
            fd_set errorSet;
            FD_ZERO(&writeSet);
            FD_ZERO(&errorSet);
            FD_SET(socketHandle, &writeSet);
            FD_SET(socketHandle, &errorSet);

            timeval waitTime{};
            waitTime.tv_sec = timeoutMs / 1000;
            waitTime.tv_usec = (timeoutMs % 1000) * 1000;

            int selected =
                select(0, nullptr, &writeSet, &errorSet, &waitTime);
            if (selected > 0 && FD_ISSET(socketHandle, &writeSet)) {
                int socketError = 0;
                int socketErrorLength = sizeof(socketError);
                if (getsockopt(socketHandle, SOL_SOCKET, SO_ERROR,
                               reinterpret_cast<char*>(&socketError),
                               &socketErrorLength) == 0 &&
                    socketError == 0) {
                    connected = true;
                }
            }
        }
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - started)
                       .count();
    closesocket(socketHandle);

    if (!connected) {
        return {};
    }

    ProbeResult result;
    result.success = true;
    result.latencyMs = static_cast<int>(std::max<int64_t>(0, elapsed));
    result.method = ProbeMethod::Tcp;
    return result;
}

static ProbeResult ProbeTcp(const std::vector<ResolvedAddress>& addresses,
                            uint16_t port,
                            int timeoutMs) {
    if (addresses.empty()) {
        return {};
    }

    auto deadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(timeoutMs);

    for (size_t index = 0; index < addresses.size(); ++index) {
        int remaining = RemainingMilliseconds(deadline);
        if (remaining <= 0) {
            break;
        }

        int addressesLeft = static_cast<int>(addresses.size() - index);
        int attemptTimeout = std::max(100, remaining / addressesLeft);
        ProbeResult result =
            ProbeTcpAddress(addresses[index], port, attemptTimeout);
        if (result.success) {
            return result;
        }
    }

    return {};
}

static ProbeResult ProbeTarget(const TargetConfig& target,
                               std::vector<ResolvedAddress>& cachedAddresses,
                               std::chrono::steady_clock::time_point& resolvedAt) {
    auto now = std::chrono::steady_clock::now();
    bool cacheExpired =
        resolvedAt == std::chrono::steady_clock::time_point::min() ||
        now - resolvedAt >= std::chrono::seconds(60);
    if (cachedAddresses.empty() || cacheExpired) {
        ResolveTarget(target.address, cachedAddresses);
        resolvedAt = now;
    }

    if (cachedAddresses.empty()) {
        return {};
    }

    ProbeResult result = ProbeIcmp(cachedAddresses, target.timeoutMs);
    if (result.success) {
        return result;
    }

    result = ProbeTcp(cachedAddresses, target.tcpPort, target.timeoutMs);
    if (!result.success) {
        cachedAddresses.clear();
    }
    return result;
}

static BOOL CALLBACK FindMonitorByNameCallback(HMONITOR monitor,
                                               HDC,
                                               LPRECT,
                                               LPARAM parameter) {
    auto* context = reinterpret_cast<MonitorSearchContext*>(parameter);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (GetMonitorInfoW(monitor, &info) &&
        _wcsicmp(info.szDevice, context->deviceName.c_str()) == 0) {
        context->result = monitor;
        return FALSE;
    }
    return TRUE;
}

static HMONITOR FindMonitorByName(const std::wstring& deviceName) {
    if (deviceName.empty()) {
        return nullptr;
    }

    MonitorSearchContext context;
    context.deviceName = deviceName;
    EnumDisplayMonitors(nullptr, nullptr, FindMonitorByNameCallback,
                        reinterpret_cast<LPARAM>(&context));
    return context.result;
}

static HMONITOR GetPrimaryMonitor() {
    POINT origin{0, 0};
    return MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
}

static std::wstring GetMonitorDeviceName(HMONITOR monitor) {
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!monitor || !GetMonitorInfoW(monitor, &info)) {
        return L"";
    }
    return info.szDevice;
}

static RECT GetMonitorWorkArea(HMONITOR monitor) {
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!monitor || !GetMonitorInfoW(monitor, &info)) {
        RECT fallback{
            0,
            0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
        };
        return fallback;
    }
    return info.rcWork;
}

static void SavePosition() {
    if (!g_window) {
        return;
    }

    RECT rect{};
    if (!GetWindowRect(g_window, &rect)) {
        return;
    }

    HMONITOR monitor =
        MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);
    std::wstring monitorName = GetMonitorDeviceName(monitor);

    Wh_SetIntValue(L"positionX", rect.left);
    Wh_SetIntValue(L"positionRight", rect.right);
    Wh_SetIntValue(L"positionY", rect.top);
    if (!monitorName.empty()) {
        Wh_SetStringValue(L"positionMonitor", monitorName.c_str());
    }
}

static RECT GetRestoredPositionRect(int width, int height) {
    wchar_t monitorBuffer[128]{};
    Wh_GetStringValue(L"positionMonitor", monitorBuffer,
                      ARRAYSIZE(monitorBuffer));
    HMONITOR monitor = FindMonitorByName(monitorBuffer);
    bool savedMonitorMissing = !monitor;
    if (!monitor) {
        monitor = GetPrimaryMonitor();
    }

    RECT workArea = GetMonitorWorkArea(monitor);
    int savedX = Wh_GetIntValue(L"positionX", INT_MIN);
    int savedRight = Wh_GetIntValue(L"positionRight", INT_MIN);
    int savedY = Wh_GetIntValue(L"positionY", INT_MIN);

    RectI rect{};
    if ((savedX == INT_MIN && savedRight == INT_MIN) ||
        savedY == INT_MIN || savedMonitorMissing) {
        rect.left = workArea.right - width - 30;
        rect.top = workArea.top + 60;
    } else {
        rect.left = savedRight != INT_MIN ? savedRight - width : savedX;
        rect.top = savedY;
    }
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;

    RectI bounds{workArea.left, workArea.top, workArea.right,
                 workArea.bottom};
    rect = ClampRectToBounds(rect, bounds);
    return RECT{rect.left, rect.top, rect.right, rect.bottom};
}

static HWND GetDesktopInsertAfter();
static HWND GetPanelInsertAfter();
static void RenderPanel();

static void ClampWindowToAvailableMonitor(bool preferSavedMonitor) {
    if (!g_window) {
        return;
    }

    RECT windowRect{};
    if (!GetWindowRect(g_window, &windowRect)) {
        return;
    }

    HMONITOR monitor = nullptr;

    if (preferSavedMonitor) {
        wchar_t monitorBuffer[128]{};
        Wh_GetStringValue(
            L"positionMonitor",
            monitorBuffer,
            ARRAYSIZE(monitorBuffer));

        if (monitorBuffer[0]) {
            monitor = FindMonitorByName(monitorBuffer);

            if (!monitor) {
                monitor = GetPrimaryMonitor();
            }
        }
    }

    if (!monitor) {
        monitor = MonitorFromRect(
            &windowRect,
            MONITOR_DEFAULTTONEAREST);
    }

    if (!monitor) {
        monitor = GetPrimaryMonitor();
    }

    RECT workArea = GetMonitorWorkArea(monitor);

    RectI clamped = ClampRectToBounds(
        {
            windowRect.left,
            windowRect.top,
            windowRect.right,
            windowRect.bottom,
        },
        {
            workArea.left,
            workArea.top,
            workArea.right,
            workArea.bottom,
        });

    SetWindowPos(
        g_window,
        GetPanelInsertAfter(),
        clamped.left,
        clamped.top,
        clamped.right - clamped.left,
        clamped.bottom - clamped.top,
        SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

static BOOL CALLBACK FindDesktopHostCallback(HWND topLevelWindow,
                                             LPARAM parameter) {
    HWND shellView = FindWindowExW(topLevelWindow, nullptr,
                                   L"SHELLDLL_DefView", nullptr);
    if (!shellView) {
        return TRUE;
    }

    HWND worker = FindWindowExW(nullptr, topLevelWindow, L"WorkerW", nullptr);
    if (worker) {
        *reinterpret_cast<HWND*>(parameter) = worker;
        return FALSE;
    }

    return TRUE;
}

static HWND FindDesktopHost() {
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (progman) {
        DWORD_PTR ignored = 0;
        SendMessageTimeoutW(progman, 0x052C, 0, 0,
                            SMTO_NORMAL | SMTO_ABORTIFHUNG, 1000, &ignored);
    }

    HWND worker = nullptr;
    EnumWindows(FindDesktopHostCallback, reinterpret_cast<LPARAM>(&worker));
    if (worker) {
        return worker;
    }

    if (progman) {
        HWND shellView =
            FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
        if (shellView) {
            return progman;
        }
    }

    return GetDesktopWindow();
}

static RECT GetVirtualScreenBounds() {
    RECT bounds{
        GetSystemMetrics(SM_XVIRTUALSCREEN),
        GetSystemMetrics(SM_YVIRTUALSCREEN),
        GetSystemMetrics(SM_XVIRTUALSCREEN) +
            GetSystemMetrics(SM_CXVIRTUALSCREEN),
        GetSystemMetrics(SM_YVIRTUALSCREEN) +
            GetSystemMetrics(SM_CYVIRTUALSCREEN),
    };

    if (bounds.right <= bounds.left || bounds.bottom <= bounds.top) {
        HMONITOR primaryMonitor = GetPrimaryMonitor();
        return GetMonitorWorkArea(primaryMonitor);
    }

    return bounds;
}

// Returns a window that is immediately above the desktop host in the Z order.
// SetWindowPos places our panel directly behind this window, leaving the panel
// above the wallpaper host but below ordinary application windows.
static HWND GetDesktopInsertAfter() {
    if (!g_desktopHost || !IsWindow(g_desktopHost) ||
        g_desktopHost == GetDesktopWindow()) {
        return HWND_TOP;
    }

    HWND windowAboveDesktop =
        GetWindow(g_desktopHost, GW_HWNDPREV);

    // When the panel is already immediately above the desktop, don't pass the
    // panel itself as hWndInsertAfter.
    if (windowAboveDesktop == g_window) {
        windowAboveDesktop =
            GetWindow(g_window, GW_HWNDPREV);
    }

    return windowAboveDesktop ? windowAboveDesktop : HWND_TOP;
}

static HWND GetPanelInsertAfter() {
    return g_settings.panel.mode == PanelMode::Topmost
               ? HWND_TOPMOST
               : GetDesktopInsertAfter();
}

static void SetWindowScreenRect(
    const RECT& screenRect,
    HWND insertAfter) {
    SetWindowPos(
        g_window,
        insertAfter,
        screenRect.left,
        screenRect.top,
        screenRect.right - screenRect.left,
        screenRect.bottom - screenRect.top,
        SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

static void ApplyWindowMode() {
    if (!g_window) {
        return;
    }

    RECT screenRect{};

    if (!GetWindowRect(g_window, &screenRect)) {
        Wh_Log(
            L"GetWindowRect failed while changing panel mode: %u",
            GetLastError());
        return;
    }

    // Never turn the per-pixel-alpha layered window into a WorkerW child.
    // Keep it as a top-level popup and change only its Z-order.
    if (GetParent(g_window)) {
        SetParent(g_window, nullptr);
    }

    LONG_PTR style =
        GetWindowLongPtrW(g_window, GWL_STYLE);

    style &= ~WS_CHILD;
    style |= WS_POPUP;

    SetWindowLongPtrW(
        g_window,
        GWL_STYLE,
        style);

    LONG_PTR extendedStyle =
        GetWindowLongPtrW(g_window, GWL_EXSTYLE);

    extendedStyle |=
        WS_EX_LAYERED |
        WS_EX_TOOLWINDOW |
        WS_EX_NOACTIVATE;

    if (g_settings.panel.mode == PanelMode::Desktop) {
        extendedStyle &= ~WS_EX_TOPMOST;
        g_desktopHost = FindDesktopHost();
        g_hiddenForFullscreen = false;
    } else {
        extendedStyle |= WS_EX_TOPMOST;
        g_desktopHost = nullptr;
    }

    if (ShouldPassClicksThrough()) {
        extendedStyle |= WS_EX_TRANSPARENT;
    } else {
        extendedStyle &= ~WS_EX_TRANSPARENT;
    }

    SetWindowLongPtrW(
        g_window,
        GWL_EXSTYLE,
        extendedStyle);

    int width = screenRect.right - screenRect.left;
    int height = screenRect.bottom - screenRect.top;

    if (g_settings.panel.mode == PanelMode::Desktop) {
        // First remove the window from the topmost band.
        if (!SetWindowPos(
                g_window,
                HWND_NOTOPMOST,
                screenRect.left,
                screenRect.top,
                width,
                height,
                SWP_NOACTIVATE |
                    SWP_NOOWNERZORDER |
                    SWP_FRAMECHANGED)) {
            Wh_Log(
                L"Failed to remove panel from topmost band: %u",
                GetLastError());
        }

        // Then place it immediately above the desktop host.
        if (!SetWindowPos(
                g_window,
                GetDesktopInsertAfter(),
                0,
                0,
                0,
                0,
                SWP_NOMOVE |
                    SWP_NOSIZE |
                    SWP_NOACTIVATE |
                    SWP_NOOWNERZORDER)) {
            Wh_Log(
                L"Failed to attach panel Z-order to desktop: %u",
                GetLastError());
        }
    } else {
        if (!SetWindowPos(
                g_window,
                HWND_TOPMOST,
                screenRect.left,
                screenRect.top,
                width,
                height,
                SWP_NOACTIVATE |
                    SWP_NOOWNERZORDER |
                    SWP_FRAMECHANGED)) {
            Wh_Log(
                L"Failed to set panel as topmost: %u",
                GetLastError());
        }
    }

    if (g_allowWindowShow &&
        !(g_settings.panel.mode == PanelMode::Topmost &&
          g_hiddenForFullscreen)) {
        ShowWindow(
            g_window,
            SW_SHOWNOACTIVATE);
    }

    Wh_Log(
        L"Panel mode applied: %s, position=%d,%d, size=%dx%d, desktopHost=0x%p",
        g_settings.panel.mode == PanelMode::Topmost
            ? L"topmost"
            : L"desktop",
        screenRect.left,
        screenRect.top,
        width,
        height,
        g_desktopHost);
}

static void UpdateClickThroughStyle() {
    if (!g_window) {
        return;
    }

    LONG_PTR extendedStyle = GetWindowLongPtrW(g_window, GWL_EXSTYLE);
    bool currentlyClickThrough =
        (extendedStyle & WS_EX_TRANSPARENT) != 0;
    bool shouldClickThrough = ShouldPassClicksThrough();

    if (currentlyClickThrough != shouldClickThrough) {
        if (shouldClickThrough) {
            extendedStyle |= WS_EX_TRANSPARENT;
        } else {
            extendedStyle &= ~WS_EX_TRANSPARENT;
        }

        SetWindowLongPtrW(g_window, GWL_EXSTYLE, extendedStyle);
        SetWindowPos(g_window, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    if (shouldClickThrough && g_hoveredRow != -1) {
        g_hoveredRow = -1;
        RenderPanel();
    }
}

static bool IsShellWindowClass(HWND window) {
    wchar_t className[128]{};
    GetClassNameW(window, className, ARRAYSIZE(className));
    return _wcsicmp(className, L"Progman") == 0 ||
           _wcsicmp(className, L"WorkerW") == 0 ||
           _wcsicmp(className, L"Shell_TrayWnd") == 0;
}

static bool IsForegroundWindowFullscreen() {
    HWND foreground = GetForegroundWindow();
    if (!foreground || foreground == g_window ||
        !IsWindowVisible(foreground) || IsIconic(foreground) ||
        IsShellWindowClass(foreground)) {
        return false;
    }

    RECT windowRect{};
    if (FAILED(DwmGetWindowAttribute(foreground,
                                     DWMWA_EXTENDED_FRAME_BOUNDS,
                                     &windowRect, sizeof(windowRect)))) {
        if (!GetWindowRect(foreground, &windowRect)) {
            return false;
        }
    }

    HMONITOR monitor =
        MonitorFromWindow(foreground, MONITOR_DEFAULTTONULL);
    if (!monitor) {
        return false;
    }

    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return false;
    }

    constexpr int tolerance = 2;
    return windowRect.left <= info.rcMonitor.left + tolerance &&
           windowRect.top <= info.rcMonitor.top + tolerance &&
           windowRect.right >= info.rcMonitor.right - tolerance &&
           windowRect.bottom >= info.rcMonitor.bottom - tolerance;
}

static void MoveToCurrentVirtualDesktop() {
    if (!g_virtualDesktopManager || !g_window ||
        g_settings.panel.mode != PanelMode::Topmost) {
        return;
    }

    BOOL onCurrentDesktop = TRUE;
    if (FAILED(g_virtualDesktopManager->IsWindowOnCurrentVirtualDesktop(
            g_window, &onCurrentDesktop)) ||
        onCurrentDesktop) {
        return;
    }

    HWND referenceWindow = GetForegroundWindow();
    if (!referenceWindow || referenceWindow == g_window ||
        IsShellWindowClass(referenceWindow)) {
        referenceWindow = FindWindowW(L"Shell_TrayWnd", nullptr);
    }
    if (!referenceWindow) {
        return;
    }

    GUID desktopId{};
    if (SUCCEEDED(g_virtualDesktopManager->GetWindowDesktopId(referenceWindow,
                                                               &desktopId))) {
        g_virtualDesktopManager->MoveWindowToDesktop(g_window, desktopId);
    }
}

static void UpdateEnvironmentState() {
    if (!g_window) {
        return;
    }

    if (g_settings.panel.mode == PanelMode::Desktop) {
        if (!g_desktopHost || !IsWindow(g_desktopHost)) {
            ApplyWindowMode();
        } else {
            SetWindowPos(
                g_window,
                GetDesktopInsertAfter(),
                0,
                0,
                0,
                0,
                SWP_NOMOVE |
                    SWP_NOSIZE |
                    SWP_NOACTIVATE |
                    SWP_NOOWNERZORDER);
        }

        if (!IsWindowVisible(g_window)) {
            ShowWindow(
                g_window,
                SW_SHOWNOACTIVATE);
        }

        return;
    }

    MoveToCurrentVirtualDesktop();

    bool shouldHide = IsForegroundWindowFullscreen();
    if (shouldHide != g_hiddenForFullscreen) {
        g_hiddenForFullscreen = shouldHide;
        ShowWindow(g_window,
                   shouldHide ? SW_HIDE : SW_SHOWNOACTIVATE);
    }

    if (!shouldHide) {
        SetWindowPos(g_window, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

static void AddRoundedRectangle(GraphicsPath& path,
                                const RectF& rectangle,
                                float radius) {
    radius = std::max(0.0f, std::min(
                                radius,
                                std::min(rectangle.Width, rectangle.Height) /
                                    2.0f));
    if (radius <= 0.0f) {
        path.AddRectangle(rectangle);
        return;
    }

    float diameter = radius * 2.0f;
    path.AddArc(rectangle.X, rectangle.Y, diameter, diameter, 180.0f, 90.0f);
    path.AddArc(rectangle.GetRight() - diameter, rectangle.Y, diameter,
                diameter, 270.0f, 90.0f);
    path.AddArc(rectangle.GetRight() - diameter,
                rectangle.GetBottom() - diameter, diameter, diameter, 0.0f,
                90.0f);
    path.AddArc(rectangle.X, rectangle.GetBottom() - diameter, diameter,
                diameter, 90.0f, 90.0f);
    path.CloseFigure();
}

static int GetFontStyle() {
    return g_settings.panel.fontWeight >= 600 ? FontStyleBold
                                              : FontStyleRegular;
}

static const Argb& GetDotColor(const TargetRuntime& target) {
    switch (target.state) {
        case LatencyState::Good:
            return target.config.status.goodColor;
        case LatencyState::Warning:
            return target.config.status.warningColor;
        case LatencyState::Bad:
            return target.config.status.badColor;
        case LatencyState::Pending:
        case LatencyState::Timeout:
        default:
            return target.config.status.timeoutColor;
    }
}

static void CalculateLayout(const Font& font, Graphics& graphics) {
    g_rowHeight = std::max(
        g_settings.panel.dotSize,
        static_cast<int>(std::ceil(g_settings.panel.fontSize * 1.45f)));
    g_rowStride = CalculateRowStride(g_rowHeight, g_settings.panel.rowGap);

    StringFormat format;
    format.SetFormatFlags(StringFormatFlagsNoWrap |
                          StringFormatFlagsMeasureTrailingSpaces);

    float maxWidth = 1.0f;
    auto measure = [&](const std::wstring& text) {
        if (text.empty()) {
            return;
        }

        RectF bounds;
        if (graphics.MeasureString(
                text.c_str(), static_cast<INT>(text.size()), &font,
                PointF(0.0f, 0.0f), &format, &bounds) == Ok) {
            maxWidth = std::max(maxWidth, std::ceil(bounds.Width));
        }
    };

    // Keep the compact numeric view stable instead of resizing for every digit.
    measure(L"99999");
    for (size_t index = 0; index < g_targets.size(); ++index) {
        const TargetRuntime& target = g_targets[index];
        bool hovered = static_cast<int>(index) == g_hoveredRow;
        measure(BuildDisplayText(
            hovered, target.config.name, target.state, target.latencyMs,
            target.config.status.timeoutText));
    }

    g_textColumnWidth = maxWidth;
    g_panelWidth = std::max(
        1, g_settings.panel.paddingX * 2 +
               static_cast<int>(std::ceil(g_textColumnWidth)) +
               g_settings.panel.textDotGap + g_settings.panel.dotSize);

    g_panelHeight = CalculatePanelHeight(
        g_settings.panel.paddingY, static_cast<int>(g_targets.size()),
        g_rowHeight, g_settings.panel.rowGap);
}

static void RenderPanel() {
    if (!g_window) {
        return;
    }

    HDC screenDc = GetDC(nullptr);
    if (!screenDc) {
        return;
    }

    HDC measureDc = CreateCompatibleDC(screenDc);
    if (!measureDc) {
        ReleaseDC(nullptr, screenDc);
        return;
    }

    FontFamily configuredFamily(g_settings.panel.fontName.c_str());
    const FontFamily* family =
        configuredFamily.GetLastStatus() == Ok
            ? &configuredFamily
            : FontFamily::GenericSansSerif();
    Font font(family, static_cast<REAL>(g_settings.panel.fontSize),
              GetFontStyle(), UnitPixel);
    Graphics measureGraphics(measureDc);
    CalculateLayout(font, measureGraphics);
    DeleteDC(measureDc);

    if (g_targets.empty()) {
        if (!g_loggedNoTargets) {
            Wh_Log(L"No enabled ping destinations were loaded; hiding panel");
            g_loggedNoTargets = true;
        }
        ShowWindow(g_window, SW_HIDE);
        ReleaseDC(nullptr, screenDc);
        return;
    }
    g_loggedNoTargets = false;

    RECT oldRect{};
    GetWindowRect(g_window, &oldRect);
    int newLeft = oldRect.left;
    if (g_allowWindowShow && oldRect.right > oldRect.left) {
        // Keep the status badge column fixed while labels expand to the left.
        newLeft = oldRect.right - g_panelWidth;
    }
    RECT newRect{newLeft, oldRect.top, newLeft + g_panelWidth,
                 oldRect.top + g_panelHeight};
    SetWindowScreenRect(newRect, GetPanelInsertAfter());

    HDC memoryDc = CreateCompatibleDC(screenDc);
    if (!memoryDc) {
        ReleaseDC(nullptr, screenDc);
        return;
    }

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = g_panelWidth;
    bitmapInfo.bmiHeader.biHeight = -g_panelHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* pixels = nullptr;
    HBITMAP bitmapHandle = CreateDIBSection(memoryDc, &bitmapInfo,
                                            DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!bitmapHandle || !pixels) {
        if (bitmapHandle) {
            DeleteObject(bitmapHandle);
        }
        DeleteDC(memoryDc);
        ReleaseDC(nullptr, screenDc);
        return;
    }

    HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmapHandle);
    Bitmap bitmap(g_panelWidth, g_panelHeight, g_panelWidth * 4,
                  PixelFormat32bppPARGB,
                  reinterpret_cast<BYTE*>(pixels));
    Graphics graphics(&bitmap);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    graphics.SetCompositingMode(CompositingModeSourceCopy);

    // Alpha 1 keeps the fully transparent panel mouse-interactive.
    graphics.Clear(Color(1, 0, 0, 0));
    graphics.SetCompositingMode(CompositingModeSourceOver);

    RectF panelRect(0.0f, 0.0f, static_cast<REAL>(g_panelWidth),
                    static_cast<REAL>(g_panelHeight));
    GraphicsPath panelPath;
    AddRoundedRectangle(panelPath, panelRect,
                        static_cast<REAL>(g_settings.panel.cornerRadius));

    if (g_settings.panel.backgroundOpacity > 0) {
        SolidBrush backgroundBrush(ToGdiColor(
            g_settings.panel.backgroundColor,
            g_settings.panel.backgroundOpacity));
        graphics.FillPath(&backgroundBrush, &panelPath);
    }

    if (g_settings.panel.borderEnabled &&
        g_settings.panel.borderOpacity > 0) {
        REAL thickness =
            static_cast<REAL>(g_settings.panel.borderThickness);
        RectF borderRect(thickness / 2.0f, thickness / 2.0f,
                         std::max<REAL>(0.0f, panelRect.Width - thickness),
                         std::max<REAL>(0.0f, panelRect.Height - thickness));
        GraphicsPath borderPath;
        AddRoundedRectangle(
            borderPath, borderRect,
            std::max<REAL>(0.0f,
                           static_cast<REAL>(g_settings.panel.cornerRadius) -
                               thickness / 2.0f));
        Pen borderPen(
            ToGdiColor(g_settings.panel.borderColor,
                       g_settings.panel.borderOpacity),
            thickness);
        graphics.DrawPath(&borderPen, &borderPath);
    }

    StringFormat textFormat;
    textFormat.SetAlignment(StringAlignmentFar);
    textFormat.SetLineAlignment(StringAlignmentCenter);
    textFormat.SetFormatFlags(StringFormatFlagsNoWrap);
    textFormat.SetTrimming(StringTrimmingEllipsisCharacter);

    REAL textX = static_cast<REAL>(g_settings.panel.paddingX);
    REAL textWidth = g_textColumnWidth;
    REAL dotX = textX + textWidth +
                static_cast<REAL>(g_settings.panel.textDotGap);

    for (size_t index = 0; index < g_targets.size(); ++index) {
        TargetRuntime& target = g_targets[index];
        REAL rowY = static_cast<REAL>(
            g_settings.panel.paddingY +
            static_cast<int>(index) * g_rowStride);

        bool hovered = static_cast<int>(index) == g_hoveredRow;
        std::wstring text = BuildDisplayText(
            hovered, target.config.name, target.state, target.latencyMs,
            target.config.status.timeoutText);

        Argb textColor =
            !hovered && target.state == LatencyState::Timeout
                ? target.config.status.timeoutColor
                : g_settings.panel.textColor;
        SolidBrush textBrush(ToGdiColor(textColor));
        RectF textRect(
            textX,
            CalculateTextTop(rowY, g_settings.panel.textVerticalOffset),
            textWidth, static_cast<REAL>(g_rowHeight));
        graphics.DrawString(text.c_str(), static_cast<INT>(text.size()), &font,
                            textRect, &textFormat, &textBrush);

        const Argb& dotColor = GetDotColor(target);
        SolidBrush dotBrush(ToGdiColor(dotColor));
        REAL dotY = rowY +
                    (static_cast<REAL>(g_rowHeight -
                                       g_settings.panel.dotSize) /
                     2.0f);
        graphics.FillEllipse(
            &dotBrush, dotX, dotY,
            static_cast<REAL>(g_settings.panel.dotSize),
            static_cast<REAL>(g_settings.panel.dotSize));
    }

    POINT sourcePoint{0, 0};
    SIZE size{g_panelWidth, g_panelHeight};
    BLENDFUNCTION blendFunction{
        AC_SRC_OVER,
        0,
        255,
        AC_SRC_ALPHA,
    };
    if (!UpdateLayeredWindow(g_window, screenDc, nullptr, &size, memoryDc,
                             &sourcePoint, 0, &blendFunction, ULW_ALPHA)) {
        if (!g_loggedLayeredWindowFailure) {
            Wh_Log(L"UpdateLayeredWindow failed: %u", GetLastError());
            g_loggedLayeredWindowFailure = true;
        }
    } else {
        g_loggedLayeredWindowFailure = false;
    }

    SelectObject(memoryDc, oldBitmap);
    DeleteObject(bitmapHandle);
    DeleteDC(memoryDc);
    ReleaseDC(nullptr, screenDc);

    if (g_allowWindowShow && !g_hiddenForFullscreen) {
        ShowWindow(g_window, SW_SHOWNOACTIVATE);
    }
}

static void StopWorkers() {
    if (!g_workerStopEvent) {
        return;
    }

    SetEvent(g_workerStopEvent);
    for (std::thread& worker : g_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    g_workers.clear();
    ResetEvent(g_workerStopEvent);
}

static void WorkerMain(int index,
                       uint64_t generation,
                       TargetConfig config) {
    std::vector<ResolvedAddress> cachedAddresses;
    auto resolvedAt = std::chrono::steady_clock::time_point::min();

    while (!g_exitRequested.load(std::memory_order_relaxed)) {
        if (WaitForSingleObject(g_workerStopEvent, 0) == WAIT_OBJECT_0) {
            break;
        }

        ProbeResult probe =
            ProbeTarget(config, cachedAddresses, resolvedAt);

        auto* message = new (std::nothrow) ResultMessage;
        if (message) {
            message->generation = generation;
            message->index = index;
            message->result = probe;
            if (!g_window ||
                !PostMessageW(g_window, kMessageResult, 0,
                              reinterpret_cast<LPARAM>(message))) {
                delete message;
            }
        }

        if (WaitForSingleObject(g_workerStopEvent, config.intervalMs) ==
            WAIT_OBJECT_0) {
            break;
        }
    }
}

static void StartWorkers() {
    if (!g_workerStopEvent || g_targets.empty()) {
        return;
    }

    ResetEvent(g_workerStopEvent);
    ++g_workerGeneration;
    uint64_t generation = g_workerGeneration;
    g_workers.reserve(g_targets.size());

    for (size_t index = 0; index < g_targets.size(); ++index) {
        TargetConfig config = g_targets[index].config;
        g_workers.emplace_back(WorkerMain, static_cast<int>(index), generation,
                               std::move(config));
    }
}

static void RebuildRuntimeTargets() {
    g_targets.clear();
    g_targets.reserve(g_settings.targets.size());
    for (const TargetConfig& config : g_settings.targets) {
        if (!config.enabled || config.address.empty()) {
            continue;
        }
        TargetRuntime runtime;
        runtime.config = config;
        g_targets.push_back(std::move(runtime));
    }
    g_hoveredRow = -1;
    g_loggedNoTargets = false;
}

static void ReloadSettingsOnUiThread() {
    StopWorkers();

    if (g_dragging) {
        g_dragging = false;
        ReleaseCapture();
    }

    g_settings = LoadSettings();
    RebuildRuntimeTargets();
    ApplyWindowMode();
    UpdateClickThroughStyle();
    RenderPanel();
    ClampWindowToAvailableMonitor(false);
    StartWorkers();
    UpdateEnvironmentState();
}

static int HitTestRow(int clientY) {
    int relativeY = clientY - g_settings.panel.paddingY;
    if (relativeY < 0) {
        return -1;
    }

    int stride = g_rowStride;
    if (stride <= 0) {
        return -1;
    }

    int row = relativeY / stride;
    int withinRow = relativeY % stride;
    if (row < 0 || row >= static_cast<int>(g_targets.size()) ||
        (g_settings.panel.rowGap >= 0 && withinRow >= g_rowHeight)) {
        return -1;
    }

    return row;
}

static void MoveWindowDuringDrag(POINT cursor) {
    RECT currentRect{};

    if (!GetWindowRect(g_window, &currentRect)) {
        return;
    }

    int width =
        currentRect.right - currentRect.left;

    int height =
        currentRect.bottom - currentRect.top;

    int requestedX =
        cursor.x - g_dragOffset.x;

    int requestedY =
        cursor.y - g_dragOffset.y;

    RectI candidate{
        requestedX,
        requestedY,
        requestedX + width,
        requestedY + height,
    };

    RectI bounds{};

    if (GetSystemMetrics(SM_CMONITORS) <= 1) {
        // A single-monitor setup must keep the complete panel inside the
        // usable work area, including the taskbar boundary.
        HMONITOR monitor = MonitorFromRect(
            &currentRect,
            MONITOR_DEFAULTTOPRIMARY);

        RECT workArea =
            GetMonitorWorkArea(monitor);

        bounds = {
            workArea.left,
            workArea.top,
            workArea.right,
            workArea.bottom,
        };
    } else {
        // With multiple monitors, allow negative coordinates and movement
        // between displays, but don't allow losing the panel outside the
        // complete virtual desktop.
        RECT virtualBounds =
            GetVirtualScreenBounds();

        bounds = {
            virtualBounds.left,
            virtualBounds.top,
            virtualBounds.right,
            virtualBounds.bottom,
        };
    }

    RectI clamped =
        ClampRectToBounds(candidate, bounds);

    SetWindowPos(
        g_window,
        GetPanelInsertAfter(),
        clamped.left,
        clamped.top,
        0,
        0,
        SWP_NOSIZE |
            SWP_NOACTIVATE |
            SWP_NOOWNERZORDER);
}

static LRESULT CALLBACK WindowProcedure(HWND window,
                                        UINT message,
                                        WPARAM wParam,
                                        LPARAM lParam) {
    switch (message) {
        case WM_MOUSEMOVE: {
            if (g_dragging) {
                POINT cursor{};
                GetCursorPos(&cursor);
                MoveWindowDuringDrag(cursor);
                return 0;
            }

            if (!g_trackingMouse) {
                TRACKMOUSEEVENT tracking{};
                tracking.cbSize = sizeof(tracking);
                tracking.dwFlags = TME_LEAVE;
                tracking.hwndTrack = window;
                if (TrackMouseEvent(&tracking)) {
                    g_trackingMouse = true;
                }
            }

            int row = HitTestRow(GET_Y_LPARAM(lParam));
            if (row != g_hoveredRow) {
                g_hoveredRow = row;
                RenderPanel();
            }
            return 0;
        }

        case WM_MOUSELEAVE:
            g_trackingMouse = false;
            if (g_hoveredRow != -1) {
                g_hoveredRow = -1;
                RenderPanel();
            }
            return 0;

        case WM_LBUTTONDOWN:
            if (ShouldStartDrag(
                    g_settings.panel.lockPosition,
                    g_settings.panel.ctrlDragOverride, IsCtrlPressed())) {
                RECT rect{};
                POINT cursor{};
                GetWindowRect(window, &rect);
                GetCursorPos(&cursor);
                g_dragOffset.x = cursor.x - rect.left;
                g_dragOffset.y = cursor.y - rect.top;
                g_dragging = true;
                SetCapture(window);
            }
            return 0;

        case WM_LBUTTONUP:
            if (g_dragging) {
                g_dragging = false;
                ReleaseCapture();
                ClampWindowToAvailableMonitor(false);
                SavePosition();
                UpdateClickThroughStyle();
            }
            return 0;

        case WM_CAPTURECHANGED:
            g_dragging = false;
            UpdateClickThroughStyle();
            return 0;

        case WM_DISPLAYCHANGE:
            ClampWindowToAvailableMonitor(true);
            SavePosition();
            return 0;

        case WM_TIMER:
            if (wParam == kEnvironmentTimer) {
                UpdateEnvironmentState();
            } else if (wParam == kInteractionTimer) {
                UpdateClickThroughStyle();
            }
            return 0;

        case kMessageResult: {
            std::unique_ptr<ResultMessage> result(
                reinterpret_cast<ResultMessage*>(lParam));
            if (!result || result->generation != g_workerGeneration ||
                result->index < 0 ||
                result->index >= static_cast<int>(g_targets.size())) {
                return 0;
            }

            TargetRuntime& target = g_targets[result->index];
            target.latencyMs = result->result.latencyMs;
            target.method = result->result.method;
            target.state = ClassifyLatency(
                result->result.success, result->result.latencyMs,
                target.config.status.warningThresholdMs,
                target.config.status.badThresholdMs);
            RenderPanel();
            return 0;
        }

        case kMessageReloadSettings:
            ReloadSettingsOnUiThread();
            return 0;

        case kMessageExit:
            DestroyWindow(window);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_CLOSE:
            DestroyWindow(window);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

static DWORD WINAPI UiThreadMain(void*) {
    g_uiThreadId = GetCurrentThreadId();
    Wh_Log(L"UI thread started. PID=%u TID=%u", GetCurrentProcessId(),
           g_uiThreadId);
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool comInitialized = SUCCEEDED(comResult);

    WSADATA winsockData{};
    if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0) {
        Wh_Log(L"WSAStartup failed");
        if (comInitialized) {
            CoUninitialize();
        }
        return 1;
    }

    GdiplusStartupInput gdiplusInput;
    if (GdiplusStartup(&g_gdiplusToken, &gdiplusInput, nullptr) != Ok) {
        Wh_Log(L"GdiplusStartup failed");
        WSACleanup();
        if (comInitialized) {
            CoUninitialize();
        }
        return 1;
    }

    CoCreateInstance(kClsidVirtualDesktopManager, nullptr,
                     CLSCTX_INPROC_SERVER, kIidVirtualDesktopManager,
                     reinterpret_cast<void**>(&g_virtualDesktopManager));

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kWindowClassName;
    if (!RegisterClassExW(&windowClass)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            Wh_Log(L"RegisterClassExW failed: %u", error);
            if (g_virtualDesktopManager) {
                g_virtualDesktopManager->Release();
                g_virtualDesktopManager = nullptr;
            }
            GdiplusShutdown(g_gdiplusToken);
            WSACleanup();
            if (comInitialized) {
                CoUninitialize();
            }
            return 1;
        }
    }

    g_settings = LoadSettings();
    RebuildRuntimeTargets();
    Wh_Log(L"Settings loaded. Configured targets=%llu, enabled targets=%llu, mode=%s, clickThrough=%d",
           static_cast<unsigned long long>(g_settings.targets.size()),
           static_cast<unsigned long long>(g_targets.size()),
           g_settings.panel.mode == PanelMode::Topmost ? L"topmost" : L"desktop",
           g_settings.panel.clickThrough);

    DWORD initialExtendedStyle =
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
    if (ShouldPassClicksThrough()) {
        initialExtendedStyle |= WS_EX_TRANSPARENT;
    }

    g_window = CreateWindowExW(
        initialExtendedStyle,
        kWindowClassName, L"Froggo Desktop Ping Panel", WS_POPUP, 0, 0, 1, 1,
        nullptr, nullptr, windowClass.hInstance, nullptr);
    if (!g_window) {
        Wh_Log(L"CreateWindowExW failed: %u", GetLastError());
        if (g_virtualDesktopManager) {
            g_virtualDesktopManager->Release();
            g_virtualDesktopManager = nullptr;
        }
        GdiplusShutdown(g_gdiplusToken);
        WSACleanup();
        if (comInitialized) {
            CoUninitialize();
        }
        return 1;
    }

    Wh_Log(L"Panel window created: 0x%p", g_window);
    RenderPanel();
    RECT restored = GetRestoredPositionRect(g_panelWidth, g_panelHeight);
    SetWindowPos(g_window, nullptr, restored.left, restored.top,
                 restored.right - restored.left,
                 restored.bottom - restored.top,
                 SWP_NOACTIVATE | SWP_NOZORDER);
    g_allowWindowShow = true;
    ApplyWindowMode();
    RenderPanel();

    SetTimer(g_window, kEnvironmentTimer, 500, nullptr);
    SetTimer(g_window, kInteractionTimer, kInteractionPollMs, nullptr);
    StartWorkers();
    UpdateEnvironmentState();
    Wh_Log(L"Panel initialization complete. Visible=%d, size=%dx%d",
           IsWindowVisible(g_window), g_panelWidth, g_panelHeight);

    MSG message{};
    while (!g_exitRequested.load(std::memory_order_relaxed) &&
           GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    KillTimer(g_window, kEnvironmentTimer);
    KillTimer(g_window, kInteractionTimer);
    StopWorkers();

    if (g_window && IsWindow(g_window)) {
        DestroyWindow(g_window);
    }
    g_window = nullptr;

    if (g_virtualDesktopManager) {
        g_virtualDesktopManager->Release();
        g_virtualDesktopManager = nullptr;
    }

    UnregisterClassW(kWindowClassName, windowClass.hInstance);
    GdiplusShutdown(g_gdiplusToken);
    WSACleanup();
    if (comInitialized) {
        CoUninitialize();
    }
    return 0;
}

static BOOL WhTool_ModInit() {
    Wh_Log(L"Starting Froggo Desktop Ping Panel host");
    g_exitRequested.store(false, std::memory_order_relaxed);
    g_uiThreadId = 0;
    g_workerStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_workerStopEvent) {
        Wh_Log(L"CreateEventW failed: %u", GetLastError());
        return FALSE;
    }

    DWORD createdThreadId = 0;
    g_uiThread = CreateThread(nullptr, 0, UiThreadMain, nullptr, 0,
                              &createdThreadId);
    if (!g_uiThread) {
        Wh_Log(L"CreateThread failed: %u", GetLastError());
        CloseHandle(g_workerStopEvent);
        g_workerStopEvent = nullptr;
        return FALSE;
    }

    Wh_Log(L"UI thread created: %u", createdThreadId);
    return TRUE;
}

static void WhTool_ModSettingsChanged() {
    Wh_Log(L"Settings change received");
    HWND window = g_window;
    if (window) {
        PostMessageW(window, kMessageReloadSettings, 0, 0);
    }
}

static void WhTool_ModUninit() {
    Wh_Log(L"Stopping Froggo Desktop Ping Panel host");
    g_exitRequested.store(true, std::memory_order_relaxed);
    if (g_workerStopEvent) {
        SetEvent(g_workerStopEvent);
    }

    HWND window = g_window;
    if (window) {
        PostMessageW(window, kMessageExit, 0, 0);
    } else if (g_uiThreadId) {
        PostThreadMessageW(g_uiThreadId, WM_QUIT, 0, 0);
    }

    if (g_uiThread) {
        DWORD waitResult = WaitForSingleObject(g_uiThread, 15000);
        if (waitResult == WAIT_TIMEOUT) {
            Wh_Log(L"UI thread did not stop within 15 seconds");
        }
        CloseHandle(g_uiThread);
        g_uiThread = nullptr;
        g_uiThreadId = 0;
    }

    if (g_workerStopEvent) {
        CloseHandle(g_workerStopEvent);
        g_workerStopEvent = nullptr;
    }
}

// Windhawk tool-mod host. The launcher runs in the normal Windhawk process and
// starts a dedicated `windhawk.exe -tool-mod "froggo-desktop-ping-panel"` process.
// The dedicated process keeps its main thread alive with a real message loop.
// This is important: terminating the entry-point thread causes Windhawk to tear
// down the tool process before the widget can remain visible.

bool g_isToolModProcessLauncher = false;
HANDLE g_toolModProcessMutex = nullptr;
DWORD g_toolModMainThreadId = 0;

static void WINAPI ToolModEntryPointHook() {
    g_toolModMainThreadId = GetCurrentThreadId();
    Wh_Log(L"Tool process main loop started. PID=%u, thread=%u",
           GetCurrentProcessId(), g_toolModMainThreadId);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    Wh_Log(L"Tool process main loop stopped");
}

}  // namespace

BOOL Wh_ModInit() {
    bool isServiceProcess = false;
    bool isAnyToolModProcess = false;
    bool isThisToolModProcess = false;

    int argumentCount = 0;
    LPWSTR* arguments =
        CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    if (!arguments) {
        Wh_Log(L"CommandLineToArgvW failed: %u", GetLastError());
        return FALSE;
    }

    for (int index = 1; index < argumentCount; ++index) {
        if (wcscmp(arguments[index], L"-service") == 0 ||
            wcscmp(arguments[index], L"-service-start") == 0 ||
            wcscmp(arguments[index], L"-service-stop") == 0) {
            isServiceProcess = true;
            break;
        }
    }

    for (int index = 1; index < argumentCount - 1; ++index) {
        if (wcscmp(arguments[index], L"-tool-mod") == 0) {
            isAnyToolModProcess = true;
            if (wcscmp(arguments[index + 1], WH_MOD_ID) == 0) {
                isThisToolModProcess = true;
            }
            break;
        }
    }

    LocalFree(arguments);

    if (isServiceProcess) {
        return FALSE;
    }

    if (isThisToolModProcess) {
        g_toolModProcessMutex =
            CreateMutexW(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutexW failed: %u", GetLastError());
            ExitProcess(1);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            Wh_Log(L"Froggo Desktop Ping Panel tool process is already running");
            ExitProcess(1);
        }

        Wh_Log(L"Initializing dedicated Froggo Desktop Ping Panel process. PID=%u",
               GetCurrentProcessId());

        if (!WhTool_ModInit()) {
            Wh_Log(L"WhTool_ModInit failed");
            ExitProcess(1);
        }

        auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(
            GetModuleHandleW(nullptr));
        if (!dosHeader || dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            Wh_Log(L"Invalid DOS header in tool process");
            WhTool_ModUninit();
            ExitProcess(1);
        }

        auto* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(
            reinterpret_cast<BYTE*>(dosHeader) + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            Wh_Log(L"Invalid NT header in tool process");
            WhTool_ModUninit();
            ExitProcess(1);
        }

        DWORD entryPointRva = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        void* entryPoint = reinterpret_cast<BYTE*>(dosHeader) + entryPointRva;
        if (!Wh_SetFunctionHook(entryPoint,
                                reinterpret_cast<void*>(ToolModEntryPointHook),
                                nullptr)) {
            Wh_Log(L"Failed to hook tool-process entry point");
            WhTool_ModUninit();
            ExitProcess(1);
        }

        return TRUE;
    }

    if (isAnyToolModProcess) {
        return FALSE;
    }

    g_isToolModProcessLauncher = true;
    Wh_Log(L"Froggo Desktop Ping Panel launcher initialized. PID=%u",
           GetCurrentProcessId());
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher) {
        return;
    }

    wchar_t currentProcessPath[MAX_PATH]{};
    DWORD pathLength = GetModuleFileNameW(nullptr, currentProcessPath,
                                          ARRAYSIZE(currentProcessPath));
    if (pathLength == 0 || pathLength >= ARRAYSIZE(currentProcessPath)) {
        Wh_Log(L"GetModuleFileNameW failed: %u", GetLastError());
        return;
    }

    wchar_t commandLine[MAX_PATH + 128]{};
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"",
               currentProcessPath, WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandleW(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandleW(L"kernel32.dll");
    }
    if (!kernelModule) {
        Wh_Log(L"kernelbase.dll/kernel32.dll not found");
        return;
    }

    using CreateProcessInternalWFunction = BOOL(WINAPI*)(
        HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR,
        LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE);

    auto createProcessInternal =
        reinterpret_cast<CreateProcessInternalWFunction>(GetProcAddress(
            kernelModule, "CreateProcessInternalW"));
    if (!createProcessInternal) {
        Wh_Log(L"CreateProcessInternalW not found");
        return;
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK;
    PROCESS_INFORMATION processInfo{};

    Wh_Log(L"Launching dedicated tool process: %s", commandLine);
    if (!createProcessInternal(nullptr, currentProcessPath, commandLine,
                               nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                               nullptr, nullptr, &startupInfo, &processInfo,
                               nullptr)) {
        Wh_Log(L"CreateProcessInternalW failed: %u", GetLastError());
        return;
    }

    Wh_Log(L"Dedicated tool process created. PID=%u",
           processInfo.dwProcessId);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}

void Wh_ModSettingsChanged() {
    if (!g_isToolModProcessLauncher) {
        WhTool_ModSettingsChanged();
    }
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModUninit();

    if (g_toolModMainThreadId) {
        PostThreadMessageW(g_toolModMainThreadId, WM_QUIT, 0, 0);
    }

    if (g_toolModProcessMutex) {
        ReleaseMutex(g_toolModProcessMutex);
        CloseHandle(g_toolModProcessMutex);
        g_toolModProcessMutex = nullptr;
    }

    ExitProcess(0);
}

#else  // PING_WIDGET_CORE_TEST

#include <cassert>

int main() {
    Argb parsed = ParseHexColor(L"#12ABEF", {255, 1, 2, 3});
    assert(parsed.a == 255 && parsed.r == 0x12 && parsed.g == 0xAB &&
           parsed.b == 0xEF);

    Argb parsedAlpha = ParseHexColor(L"#8012ABEF", {255, 1, 2, 3});
    assert(parsedAlpha.a == 0x80 && parsedAlpha.r == 0x12 &&
           parsedAlpha.g == 0xAB && parsedAlpha.b == 0xEF);

    Argb fallback = ParseHexColor(L"invalid", {9, 8, 7, 6});
    assert(fallback.a == 9 && fallback.r == 8 && fallback.g == 7 &&
           fallback.b == 6);

    assert(ClassifyLatency(false, 0, 80, 150) == LatencyState::Timeout);
    assert(ClassifyLatency(true, 79, 80, 150) == LatencyState::Good);
    assert(ClassifyLatency(true, 80, 80, 150) == LatencyState::Warning);
    assert(ClassifyLatency(true, 149, 80, 150) == LatencyState::Warning);
    assert(ClassifyLatency(true, 150, 80, 150) == LatencyState::Bad);

    assert(BuildDisplayText(false, L"Google", LatencyState::Good, 27,
                            L"TIMEOUT") == L"27");
    assert(BuildDisplayText(true, L"Google", LatencyState::Good, 27,
                            L"TIMEOUT") == L"Google");
    assert(BuildDisplayText(false, L"Google", LatencyState::Timeout, 0,
                            L"OFFLINE") == L"OFFLINE");

    RectI clamped = ClampRectToBounds(
        {1900, 1060, 2020, 1120}, {0, 0, 1920, 1080});
    assert(clamped.left == 1800 && clamped.top == 1020 &&
           clamped.right == 1920 && clamped.bottom == 1080);

    RectI oversized =
        ClampRectToBounds({50, 50, 2050, 1250}, {0, 0, 1920, 1080});
    assert(oversized.left == 0 && oversized.top == 0);

    assert(CalculateRowStride(20, 5) == 25);
    assert(CalculateRowStride(20, -4) == 16);
    assert(CalculatePanelHeight(4, 3, 20, 5) == 78);
    assert(CalculatePanelHeight(4, 0, 20, 5) == 1);

    // Positive offsets move the unchanged GDI+ text rectangle downward.
    assert(CalculateTextTop(100.0f, 2) == 102.0f);
    assert(CalculateTextTop(100.0f, -3) == 97.0f);

    assert(ShouldStartDrag(false, false, false));
    assert(ShouldStartDrag(true, true, true));
    assert(!ShouldStartDrag(true, true, false));
    assert(!ShouldStartDrag(true, false, true));

    assert(ShouldUseClickThrough(true, false, false));
    assert(!ShouldUseClickThrough(true, false, true));
    assert(!ShouldUseClickThrough(true, true, false));
    assert(!ShouldUseClickThrough(false, false, false));

    return 0;
}

#endif  // PING_WIDGET_CORE_TEST
