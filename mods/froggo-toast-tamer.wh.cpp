// ==WindhawkMod==
// @id              froggo-toast-tamer
// @name            Froggo Toast Tamer
// @description     Let clicks pass through the invisible toast margin and optionally snooze an app after swiping its notification away
// @version         0.2.1
// @author          sapitosucio
// @github          https://github.com/SapitoSucio
// @include         ShellExperienceHost.exe
// @include         ShellHost.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Froggo Toast Tamer

Windows 11 toast windows are larger than the visible notification card. The
transparent part can consequently swallow clicks intended for the application
underneath. Froggo Toast Tamer removes hit-test backgrounds from the empty XAML
containers while leaving the notification itself interactive.

Optionally, swiping a notification horizontally snoozes subsequent banners from
the same sender for a configurable time. Windows exposes the sender identity to
the notification UI (usually the app name/AUMID), which isn't necessarily the
literal executable filename. Snoozing only hides popup banners; notifications
remain available in Notification Center.

The mod supports Windows 11 only.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- clickThroughInvisibleMargin: true
  $name: Click through the invisible notification margin
  $description: Let clicks reach windows behind the transparent area around a toast.
- froggoSwipeSnooze: true
  $name: Froggo swipe snooze
  $description: After horizontally swiping a notification away, hide new popup banners from the same app temporarily.
- froggoSnoozeMinutes: 30
  $name: Froggo snooze duration (minutes)
  $description: How long an app is snoozed after its notification is swiped away.
- swipeThreshold: 80
  $name: Swipe distance (pixels)
  $description: Minimum horizontal pointer travel that counts as a swipe.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/base.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;

struct Settings {
    bool clickThroughInvisibleMargin;
    bool froggoSwipeSnooze;
    int froggoSnoozeMinutes;
    int swipeThreshold;
} g_settings;

struct PointerStart {
    float x;
    float y;
    std::wstring sender;
};

struct ObservedToast {
    winrt::weak_ref<FrameworkElement> element;
    bool suppressed;
    winrt::event_token pointerPressedToken;
    winrt::event_token pointerReleasedToken;
    winrt::event_token pointerCanceledToken;
};

struct ClearedBackground {
    winrt::weak_ref<Panel> panel;
    Media::Brush originalBackground;
};

std::mutex g_stateMutex;
std::unordered_map<uint32_t, PointerStart> g_pointerStarts;
std::unordered_map<std::wstring, std::chrono::steady_clock::time_point>
    g_snoozedSenders;
std::vector<ObservedToast> g_observedToasts;
std::vector<ClearedBackground> g_clearedBackgrounds;

using SetWindowPos_t = decltype(&SetWindowPos);
SetWindowPos_t SetWindowPos_Original;
std::atomic<bool> g_loggedWindowHook;
std::atomic<bool> g_loggedMissingWindow;
std::atomic<bool> g_loggedMissingRoot;
std::atomic<bool> g_loggedMissingToastPage;
std::atomic<bool> g_loggedToastPageFound;
std::atomic<bool> g_loggedWindowRegionApplied;
std::atomic<HWND> g_toastWindow;
DispatcherTimer g_regionTimer{nullptr};
winrt::event_token g_regionTimerToken{};

FrameworkElement FindDescendant(
    FrameworkElement parent,
    const std::function<bool(const FrameworkElement&)>& predicate) {
    const int count = Media::VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = Media::VisualTreeHelper::GetChild(parent, i)
                         .try_as<FrameworkElement>();
        if (!child) {
            continue;
        }

        if (predicate(child)) {
            return child;
        }

        if (auto descendant = FindDescendant(child, predicate)) {
            return descendant;
        }
    }

    return nullptr;
}

void ForEachDescendant(
    FrameworkElement parent,
    const std::function<void(const FrameworkElement&)>& callback) {
    const int count = Media::VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = Media::VisualTreeHelper::GetChild(parent, i)
                         .try_as<FrameworkElement>();
        if (!child) {
            continue;
        }

        callback(child);
        ForEachDescendant(child, callback);
    }
}

bool IsToastView(const FrameworkElement& element) {
    const auto className = winrt::get_class_name(element);
    if (className != L"ActionCenter.FlexibleToastView") {
        return false;
    }

    const auto name = element.Name();
    return name.empty() || name.starts_with(L"FlexibleNormalToastView") ||
           name.starts_with(L"FlexiblePriorityToastView");
}

std::wstring GetToastSender(const FrameworkElement& toast) {
    auto senderElement = FindDescendant(toast, [](const FrameworkElement& child) {
        return child.Name() == L"SenderName";
    });

    if (auto text = senderElement.try_as<TextBlock>()) {
        return text.Text().c_str();
    }

    return {};
}

bool IsSenderSnoozed(const std::wstring& sender) {
    if (sender.empty()) {
        return false;
    }

    std::scoped_lock lock(g_stateMutex);
    auto found = g_snoozedSenders.find(sender);
    if (found == g_snoozedSenders.end()) {
        return false;
    }

    if (found->second <= std::chrono::steady_clock::now()) {
        g_snoozedSenders.erase(found);
        return false;
    }

    return true;
}

void SnoozeSender(const std::wstring& sender) {
    if (sender.empty()) {
        Wh_Log(L"Swipe detected, but the notification sender wasn't found");
        return;
    }

    const auto duration = std::chrono::minutes(
        std::max(1, g_settings.froggoSnoozeMinutes));
    {
        std::scoped_lock lock(g_stateMutex);
        g_snoozedSenders[sender] = std::chrono::steady_clock::now() + duration;
    }

    Wh_Log(L"Froggo snoozed '%s' for %d minute(s)", sender.c_str(),
           std::max(1, g_settings.froggoSnoozeMinutes));
}

void ObserveToast(const FrameworkElement& toast) {
    for (auto it = g_observedToasts.begin(); it != g_observedToasts.end();) {
        auto existing = it->element.get();
        if (!existing) {
            it = g_observedToasts.erase(it);
            continue;
        }
        if (existing == toast) {
            return;
        }
        ++it;
    }

    const std::wstring sender = GetToastSender(toast);
    if (g_settings.froggoSwipeSnooze && IsSenderSnoozed(sender)) {
        toast.Visibility(Visibility::Collapsed);
        g_observedToasts.push_back({winrt::make_weak(toast), true, {}, {}, {}});
        Wh_Log(L"Suppressed a snoozed banner from '%s'", sender.c_str());
        return;
    }

    auto weakToast = winrt::make_weak(toast);
    auto pointerPressedToken = toast.PointerPressed(
        [weakToast](const winrt::Windows::Foundation::IInspectable&,
                    const Input::PointerRoutedEventArgs& args) {
        if (!g_settings.froggoSwipeSnooze) {
            return;
        }

        auto element = weakToast.get();
        if (!element) {
            return;
        }

        const auto point = args.GetCurrentPoint(element);
        std::scoped_lock lock(g_stateMutex);
        g_pointerStarts[point.PointerId()] = {
            point.Position().X, point.Position().Y, GetToastSender(element)};
        });

    auto pointerReleasedToken = toast.PointerReleased(
        [weakToast](const winrt::Windows::Foundation::IInspectable&,
                    const Input::PointerRoutedEventArgs& args) {
        if (!g_settings.froggoSwipeSnooze) {
            return;
        }

        auto element = weakToast.get();
        if (!element) {
            return;
        }

        const auto point = args.GetCurrentPoint(element);
        PointerStart start{};
        {
            std::scoped_lock lock(g_stateMutex);
            auto found = g_pointerStarts.find(point.PointerId());
            if (found == g_pointerStarts.end()) {
                return;
            }
            start = std::move(found->second);
            g_pointerStarts.erase(found);
        }

        const float dx = point.Position().X - start.x;
        const float dy = point.Position().Y - start.y;
        if (std::abs(dx) >= std::max(1, g_settings.swipeThreshold) &&
            std::abs(dx) > std::abs(dy)) {
            SnoozeSender(start.sender);
        }
        });

    auto pointerCanceledToken = toast.PointerCanceled(
        [](const winrt::Windows::Foundation::IInspectable&,
           const Input::PointerRoutedEventArgs& args) {
            std::scoped_lock lock(g_stateMutex);
            g_pointerStarts.erase(args.Pointer().PointerId());
        });

    g_observedToasts.push_back({std::move(weakToast), false,
                                pointerPressedToken, pointerReleasedToken,
                                pointerCanceledToken});
}

void RestoreUiChanges() {
    for (auto& observed : g_observedToasts) {
        auto toast = observed.element.get();
        if (!toast) {
            continue;
        }

        if (observed.pointerPressedToken.value) {
            toast.PointerPressed(observed.pointerPressedToken);
            toast.PointerReleased(observed.pointerReleasedToken);
            toast.PointerCanceled(observed.pointerCanceledToken);
        }
        if (observed.suppressed) {
            toast.Visibility(Visibility::Visible);
        }
    }
    g_observedToasts.clear();

    for (auto& cleared : g_clearedBackgrounds) {
        if (auto panel = cleared.panel.get()) {
            panel.Background(cleared.originalBackground);
        }
    }
    g_clearedBackgrounds.clear();
}

void RemoveDeadHitTestAreas(const FrameworkElement& root) {
    if (!g_settings.clickThroughInvisibleMargin) {
        return;
    }

    auto clearBackground = [](const FrameworkElement& element) {
        const auto name = element.Name();
        if (name != L"ToastCenterMainGrid" && name != L"ToastCenterGrid") {
            return;
        }

        if (auto panel = element.try_as<Panel>()) {
            for (const auto& cleared : g_clearedBackgrounds) {
                if (cleared.panel.get() == panel) {
                    return;
                }
            }
            g_clearedBackgrounds.push_back(
                {winrt::make_weak(panel), panel.Background()});
            panel.Background(nullptr);
        }
    };

    clearBackground(root);
    ForEachDescendant(root, clearBackground);
}

FrameworkElement GetVisibleToastBorder(const FrameworkElement& toast) {
    auto border = FindDescendant(toast, [](const FrameworkElement& child) {
        const auto name = child.Name();
        return name == L"ToastBackgroundBorder2" ||
               name == L"ToastBackgroundBorder" ||
               name == L"ItemOpaquePlating";
    });
    return border ? border : toast;
}

void UpdateToastWindowRegion(HWND hWnd, const FrameworkElement& root) {
    if (!hWnd || !g_settings.clickThroughInvisibleMargin) {
        if (hWnd) {
            SetWindowRgn(hWnd, nullptr, TRUE);
        }
        return;
    }

    RECT clientRect{};
    if (!GetClientRect(hWnd, &clientRect) || root.ActualWidth() <= 0 ||
        root.ActualHeight() <= 0) {
        return;
    }

    const double scaleX =
        static_cast<double>(clientRect.right - clientRect.left) /
        root.ActualWidth();
    const double scaleY =
        static_cast<double>(clientRect.bottom - clientRect.top) /
        root.ActualHeight();

    Wh_Log(L"[froggo] Region layout: client=%dx%d root=%.1fx%.1f "
           L"scale=%.3fx%.3f",
           clientRect.right - clientRect.left,
           clientRect.bottom - clientRect.top, root.ActualWidth(),
           root.ActualHeight(), scaleX, scaleY);

    HRGN combinedRegion = CreateRectRgn(0, 0, 0, 0);
    if (!combinedRegion) {
        return;
    }

    int toastCount = 0;
    ForEachDescendant(root, [&](const FrameworkElement& child) {
        if (!IsToastView(child) || child.Visibility() != Visibility::Visible ||
            child.ActualWidth() <= 0 || child.ActualHeight() <= 0) {
            return;
        }

        auto visibleBorder = GetVisibleToastBorder(child);
        auto transform = visibleBorder.TransformToVisual(root);
        auto bounds = transform.TransformBounds(
            {0, 0, static_cast<float>(visibleBorder.ActualWidth()),
             static_cast<float>(visibleBorder.ActualHeight())});

        const int left = static_cast<int>(std::floor(bounds.X * scaleX));
        const int top = static_cast<int>(std::floor(bounds.Y * scaleY));
        const int right =
            static_cast<int>(std::ceil((bounds.X + bounds.Width) * scaleX));
        const int bottom =
            static_cast<int>(std::ceil((bounds.Y + bounds.Height) * scaleY));
        if (right <= left || bottom <= top) {
            return;
        }

        Wh_Log(L"[froggo] Visible toast region: %d,%d-%d,%d", left, top,
               right, bottom);

        HRGN toastRegion = CreateRectRgn(left, top, right, bottom);
        if (toastRegion) {
            CombineRgn(combinedRegion, combinedRegion, toastRegion, RGN_OR);
            DeleteObject(toastRegion);
            toastCount++;
        }
    });

    if (!toastCount) {
        DeleteObject(combinedRegion);
        return;
    }

    if (!SetWindowRgn(hWnd, combinedRegion, TRUE)) {
        DeleteObject(combinedRegion);
        Wh_Log(L"[froggo] SetWindowRgn failed, error %lu", GetLastError());
        return;
    }

    // SetWindowRgn owns combinedRegion after a successful call.
    g_toastWindow = hWnd;
    if (!g_loggedWindowRegionApplied.exchange(true)) {
        Wh_Log(L"[froggo] Limited the toast HWND input region to %d visible "
               L"notification(s)",
               toastCount);
    }
}

void ScheduleToastWindowRegionUpdate(HWND hWnd,
                                     const FrameworkElement& root) {
    // The toast enters with a render transform. A region calculated during that
    // animation clips the card once it reaches its final position, so keep the
    // host unrestricted until layout and animation have settled.
    SetWindowRgn(hWnd, nullptr, TRUE);

    if (g_regionTimer) {
        g_regionTimer.Stop();
        if (g_regionTimerToken.value) {
            g_regionTimer.Tick(g_regionTimerToken);
        }
    }

    auto weakRoot = winrt::make_weak(root);
    g_regionTimer = DispatcherTimer();
    g_regionTimer.Interval(std::chrono::milliseconds(450));
    g_regionTimerToken = g_regionTimer.Tick(
        [hWnd, weakRoot](
            const winrt::Windows::Foundation::IInspectable&,
            const winrt::Windows::Foundation::IInspectable&) {
            g_regionTimer.Stop();
            if (auto currentRoot = weakRoot.get()) {
                UpdateToastWindowRegion(hWnd, currentRoot);
            }
        });
    g_regionTimer.Start();
}

void RefreshToastUi(HWND hWnd = nullptr) try {
    auto window = Window::Current();
    if (!window) {
        if (!g_loggedMissingWindow.exchange(true)) {
            Wh_Log(L"[froggo] Window::Current is unavailable on this thread");
        }
        return;
    }

    auto root = window.Content().try_as<FrameworkElement>();
    if (!root) {
        if (!g_loggedMissingRoot.exchange(true)) {
            Wh_Log(L"[froggo] The current XAML window has no FrameworkElement root");
        }
        return;
    }

    auto toastPage = FindDescendant(root, [](const FrameworkElement& child) {
        return winrt::get_class_name(child) ==
               L"ActionCenter.ToastCenterPage";
    });
    if (!toastPage) {
        if (!g_loggedMissingToastPage.exchange(true)) {
            Wh_Log(L"[froggo] ActionCenter.ToastCenterPage wasn't found");
        }
        return;
    }

    if (!g_loggedToastPageFound.exchange(true)) {
        Wh_Log(L"[froggo] ToastCenterPage found; applying Froggo behavior");
    }

    RemoveDeadHitTestAreas(toastPage);
    ForEachDescendant(toastPage, [](const FrameworkElement& child) {
        if (IsToastView(child)) {
            ObserveToast(child);
        }
    });
    if (hWnd) {
        ScheduleToastWindowRegionUpdate(hWnd, toastPage);
    }
} catch (const winrt::hresult_error& error) {
    Wh_Log(L"RefreshToastUi failed: %s (0x%08X)", error.message().c_str(),
           error.code().value);
}

BOOL WINAPI SetWindowPos_Hook(HWND hWnd,
                              HWND hWndInsertAfter,
                              int X,
                              int Y,
                              int cx,
                              int cy,
                              UINT flags) {
    BOOL result = SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy,
                                       flags);

    WCHAR className[64]{};
    if (result && GetWindowThreadProcessId(hWnd, nullptr) ==
                      GetCurrentThreadId() &&
        GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
        _wcsicmp(className, L"Windows.UI.Core.CoreWindow") == 0) {
        if (!g_loggedWindowHook.exchange(true)) {
            Wh_Log(L"[froggo] Observed toast CoreWindow SetWindowPos");
        }
        RefreshToastUi(hWnd);
    }

    return result;
}

void LoadSettings() {
    g_settings.clickThroughInvisibleMargin =
        Wh_GetIntSetting(L"clickThroughInvisibleMargin") != 0;
    g_settings.froggoSwipeSnooze =
        Wh_GetIntSetting(L"froggoSwipeSnooze") != 0;
    g_settings.froggoSnoozeMinutes =
        std::max(1, Wh_GetIntSetting(L"froggoSnoozeMinutes"));
    g_settings.swipeThreshold =
        std::max(1, Wh_GetIntSetting(L"swipeThreshold"));
}

BOOL Wh_ModInit() {
    LoadSettings();
    WCHAR processPath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, processPath, ARRAYSIZE(processPath));
    PCWSTR processName = wcsrchr(processPath, L'\\');
    processName = processName ? processName + 1 : processPath;
    Wh_Log(L"[froggo] Froggo Toast Tamer 0.2.1 loaded in %s (PID %lu)",
           processName, GetCurrentProcessId());
    WindhawkUtils::SetFunctionHook(SetWindowPos, SetWindowPos_Hook,
                                   &SetWindowPos_Original);
    return TRUE;
}

void Wh_ModAfterInit() {
    RefreshToastUi();
}

void Wh_ModSettingsChanged() {
    RestoreUiChanges();
    LoadSettings();
    RefreshToastUi();
}

void Wh_ModUninit() {
    if (g_regionTimer) {
        g_regionTimer.Stop();
        if (g_regionTimerToken.value) {
            g_regionTimer.Tick(g_regionTimerToken);
        }
        g_regionTimer = nullptr;
        g_regionTimerToken = {};
    }
    if (HWND hWnd = g_toastWindow.exchange(nullptr)) {
        SetWindowRgn(hWnd, nullptr, TRUE);
    }
    RestoreUiChanges();
    std::scoped_lock lock(g_stateMutex);
    g_pointerStarts.clear();
    g_snoozedSenders.clear();
}
