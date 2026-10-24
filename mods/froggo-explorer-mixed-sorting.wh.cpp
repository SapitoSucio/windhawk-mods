// ==WindhawkMod==
// @id              froggo-explorer-mixed-sorting
// @name            Froggo Explorer Mixed Sorting
// @description     Mix folders and files naturally by Date modified/created, including when Group by is None.
// @version         3.1.1
// @author          Sapito Sucio
// @github          https://github.com/SapitoSucio
// @license         MIT
// @include         %SystemRoot%\explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Froggo Explorer Mixed Sorting

## Demonstration / Demostración

The same dummy folder, sorted by Date modified (descending), with Group by: None.
La misma carpeta de prueba, ordenada por Fecha de modificación (descendente),
con Agrupar por: Ninguno.

**Off / Desactivado**

![Files and folders remain separate](https://raw.githubusercontent.com/SapitoSucio/windhawk-mods/88faf004862364e9d831ae9bdb3864d76efc23bb/media/froggo-explorer-mixed-sorting/before.png)

**On / Activado**

![Files and folders interleave by date](https://raw.githubusercontent.com/SapitoSucio/windhawk-mods/88faf004862364e9d831ae9bdb3864d76efc23bb/media/froggo-explorer-mixed-sorting/after.png)

## English

Windows Explorer normally gives folders special placement while sorting.
Froggo Explorer Mixed Sorting makes folders and files participate in the same
chronological order when sorting by **Date modified** or **Date created**.

Features:

- Works with **Group by: None** and explicit grouping.
- Optional mixed sorting by Date modified and Date created.
- Per-view exclusions for Details, List, icon/thumbnail, Tiles and Content.
- Explorer-only injection: `%SystemRoot%\explorer.exe`.
- No file/folder metadata is modified.

### Important

This mod relies on internal Windows Shell symbols. A major Windows update can
change those symbols and require a mod update.

---

## Español

El Explorador de Windows normalmente da un tratamiento especial a las carpetas
al ordenar. Froggo Explorer Mixed Sorting hace que carpetas y archivos
participen en el mismo orden cronológico al ordenar por **Fecha de modificación**
o **Fecha de creación**.

Características:

- Funciona con **Agrupar por: Ninguno** y con agrupaciones explícitas.
- Orden mixto opcional por Fecha de modificación y Fecha de creación.
- Exclusiones por vista: Detalles, Lista, iconos/miniaturas, Mosaicos y Contenido.
- Inyección únicamente en Explorer: `%SystemRoot%\explorer.exe`.
- No modifica archivos, carpetas, fechas ni metadatos.

### Importante

El mod depende de símbolos internos del Shell de Windows. Una actualización
grande de Windows puede cambiar esos símbolos y requerir una actualización del
mod.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- mixDateModified: true
  $name: Mix by Date modified
  $description: Interleave folders and files using their real Date modified value.

- mixDateCreated: false
  $name: Mix by Date created
  $description: Interleave folders and files using their real Date created value.

- ignoreDetailsView: false
  $name: Ignore Details view
  $description: Keep Explorer's original folder separation while the current view is Details.

- ignoreListView: false
  $name: Ignore List view
  $description: Keep Explorer's original folder separation while the current view is List.

- ignoreIconViews: false
  $name: Ignore icon views
  $description: Keep Explorer's original folder separation in icon/thumbnail views.

- ignoreTilesView: false
  $name: Ignore Tiles view
  $description: Keep Explorer's original folder separation while the current view is Tiles.

- ignoreContentView: false
  $name: Ignore Content view
  $description: Keep Explorer's original folder separation while the current view is Content.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <oleauto.h>
#include <exdisp.h>
#include <shldisp.h>
#include <propkey.h>
#include <propsys.h>
#include <servprov.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <shtypes.h>
#include <windhawk_utils.h>

#include <cwchar>

// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

struct Settings {
    bool mixDateModified;
    bool mixDateCreated;
    bool ignoreDetailsView;
    bool ignoreListView;
    bool ignoreIconViews;
    bool ignoreTilesView;
    bool ignoreContentView;
};

static Settings g_settings{};

static void LoadSettings() {
    g_settings.mixDateModified =
        Wh_GetIntSetting(L"mixDateModified") != 0;

    g_settings.mixDateCreated =
        Wh_GetIntSetting(L"mixDateCreated") != 0;

    g_settings.ignoreDetailsView =
        Wh_GetIntSetting(L"ignoreDetailsView") != 0;

    g_settings.ignoreListView =
        Wh_GetIntSetting(L"ignoreListView") != 0;

    g_settings.ignoreIconViews =
        Wh_GetIntSetting(L"ignoreIconViews") != 0;

    g_settings.ignoreTilesView =
        Wh_GetIntSetting(L"ignoreTilesView") != 0;

    g_settings.ignoreContentView =
        Wh_GetIntSetting(L"ignoreContentView") != 0;
}

static bool HasViewExclusions() {
    return g_settings.ignoreDetailsView ||
           g_settings.ignoreListView ||
           g_settings.ignoreIconViews ||
           g_settings.ignoreTilesView ||
           g_settings.ignoreContentView;
}

// -----------------------------------------------------------------------------
// Window / active-tab helpers
// -----------------------------------------------------------------------------

static bool GetWindowClass(HWND hwnd, WCHAR (&className)[128]) {
    className[0] = L'\0';

    return hwnd &&
           GetClassNameW(hwnd, className, ARRAYSIZE(className)) > 0;
}

static bool WindowBelongsToCurrentProcess(HWND hwnd) {
    if (!hwnd) {
        return false;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    return processId == GetCurrentProcessId();
}

static bool IsExplorerFrame(HWND hwnd) {
    WCHAR className[128];

    if (!GetWindowClass(hwnd, className)) {
        return false;
    }

    return wcscmp(className, L"CabinetWClass") == 0 ||
           wcscmp(className, L"ExploreWClass") == 0;
}


struct EnumExplorerFramesContext {
    HWND onlyFrame = nullptr;
    unsigned count = 0;
};

static BOOL CALLBACK EnumExplorerFramesProc(HWND hwnd, LPARAM lParam) {
    auto* context =
        reinterpret_cast<EnumExplorerFramesContext*>(lParam);

    if (!IsWindowVisible(hwnd) ||
        !WindowBelongsToCurrentProcess(hwnd) ||
        !IsExplorerFrame(hwnd)) {
        return TRUE;
    }

    context->count++;

    if (context->count == 1) {
        context->onlyFrame = hwnd;
    } else {
        context->onlyFrame = nullptr;
    }

    return TRUE;
}

static HWND GetRelevantRootWindow() {
    HWND foreground = GetForegroundWindow();

    if (foreground) {
        HWND root = GetAncestor(foreground, GA_ROOT);
        root = root ? root : foreground;

        if (WindowBelongsToCurrentProcess(root)) {
            return root;
        }
    }

    // A sort can continue for a short moment after focus moved to another
    // process. If Explorer has exactly one visible frame, it is unambiguous.
    EnumExplorerFramesContext context{};
    EnumWindows(
        EnumExplorerFramesProc,
        reinterpret_cast<LPARAM>(&context));

    if (context.count == 1) {
        return context.onlyFrame;
    }

    // Last fallback: use the GUI state of the current thread if it happens to
    // be the view's UI thread.
    GUITHREADINFO guiInfo{};
    guiInfo.cbSize = sizeof(guiInfo);

    if (GetGUIThreadInfo(GetCurrentThreadId(), &guiInfo)) {
        HWND candidate =
            guiInfo.hwndFocus ? guiInfo.hwndFocus : guiInfo.hwndActive;

        if (candidate) {
            HWND root = GetAncestor(candidate, GA_ROOT);
            root = root ? root : candidate;

            if (WindowBelongsToCurrentProcess(root)) {
                return root;
            }
        }
    }

    return nullptr;
}

struct FindTabWindowContext {
    HWND first = nullptr;
    HWND visible = nullptr;
};

static BOOL CALLBACK FindTabWindowProc(HWND hwnd, LPARAM lParam) {
    auto* context =
        reinterpret_cast<FindTabWindowContext*>(lParam);

    WCHAR className[128];

    if (!GetWindowClass(hwnd, className) ||
        wcscmp(className, L"ShellTabWindowClass") != 0) {
        return TRUE;
    }

    if (!context->first) {
        context->first = hwnd;
    }

    if (IsWindowVisible(hwnd)) {
        context->visible = hwnd;
        return FALSE;
    }

    return TRUE;
}

static HWND FindActiveExplorerTabWindow(HWND explorerFrame) {
    if (!explorerFrame) {
        return nullptr;
    }

    FindTabWindowContext context{};

    EnumChildWindows(
        explorerFrame,
        FindTabWindowProc,
        reinterpret_cast<LPARAM>(&context));

    // On Windows 11 only the active ShellTabWindowClass is normally visible.
    // If visibility is temporarily changing during navigation, the first tab
    // is still useful as a fallback.
    return context.visible ? context.visible : context.first;
}

// -----------------------------------------------------------------------------
// Current FOLDERVIEWMODE resolution
// -----------------------------------------------------------------------------

struct ViewModeInfo {
    bool known = false;
    FOLDERVIEWMODE mode = FVM_AUTO;
    HWND rootWindow = nullptr;
    HWND tabWindow = nullptr;

    // Active grouping/sort state is also queried while resolving the view.
    bool groupByKnown = false;
    PROPERTYKEY groupByKey{};
    BOOL groupAscending = TRUE;

    bool sortColumnsKnown = false;
    int sortColumnCount = 0;
    SORTCOLUMN sortColumns[8]{};
};

static bool IsIgnoredView(FOLDERVIEWMODE mode) {
    switch (mode) {
        case FVM_DETAILS:
            return g_settings.ignoreDetailsView;

        case FVM_LIST:
            return g_settings.ignoreListView;

        case FVM_ICON:
        case FVM_SMALLICON:
        case FVM_THUMBNAIL:
        case FVM_THUMBSTRIP:
            return g_settings.ignoreIconViews;

        case FVM_TILE:
            return g_settings.ignoreTilesView;

        case FVM_CONTENT:
            return g_settings.ignoreContentView;

        default:
            return false;
    }
}



static bool GetViewModeFromShellBrowser(
    IShellBrowser* browser,
    FOLDERVIEWMODE* viewMode,
    PROPERTYKEY* groupByKey = nullptr,
    BOOL* groupAscending = nullptr,
    bool* groupByKnown = nullptr,
    SORTCOLUMN* sortColumns = nullptr,
    int sortColumnsCapacity = 0,
    int* sortColumnCount = nullptr,
    bool* sortColumnsKnown = nullptr) {
    if (!browser || !viewMode) {
        return false;
    }

    IShellView* shellView = nullptr;
    HRESULT hr = browser->QueryActiveShellView(&shellView);

    if (FAILED(hr) || !shellView) {
        return false;
    }

    bool success = false;

    IFolderView2* folderView2 = nullptr;
    hr = shellView->QueryInterface(IID_PPV_ARGS(&folderView2));

    if (SUCCEEDED(hr) && folderView2) {
        FOLDERVIEWMODE mode = FVM_AUTO;
        int iconSize = 0;

        if (SUCCEEDED(
                folderView2->GetViewModeAndIconSize(
                    &mode,
                    &iconSize))) {
            *viewMode = mode;
            success = true;
        }

        if (groupByKnown) {
            PROPERTYKEY key{};
            BOOL ascending = TRUE;
            HRESULT groupHr = folderView2->GetGroupBy(&key, &ascending);

            *groupByKnown = SUCCEEDED(groupHr);

            if (SUCCEEDED(groupHr)) {
                if (groupByKey) {
                    *groupByKey = key;
                }

                if (groupAscending) {
                    *groupAscending = ascending;
                }
            }
        }

        if (sortColumnsKnown) {
            int count = 0;
            HRESULT countHr = folderView2->GetSortColumnCount(&count);

            *sortColumnsKnown = SUCCEEDED(countHr);

            if (SUCCEEDED(countHr)) {
                if (sortColumnCount) {
                    *sortColumnCount = count;
                }

                if (sortColumns && sortColumnsCapacity > 0 && count > 0) {
                    int toRead = count;
                    if (toRead > sortColumnsCapacity) {
                        toRead = sortColumnsCapacity;
                    }

                    HRESULT sortHr =
                        folderView2->GetSortColumns(sortColumns, toRead);

                    if (FAILED(sortHr)) {
                        *sortColumnsKnown = false;
                    }
                }
            }
        }

        folderView2->Release();
    }

    if (!success) {
        FOLDERSETTINGS folderSettings{};

        if (SUCCEEDED(
                shellView->GetCurrentInfo(&folderSettings))) {
            *viewMode = static_cast<FOLDERVIEWMODE>(
                folderSettings.ViewMode);
            success = true;
        }
    }

    shellView->Release();
    return success;
}

static HRESULT GetShellBrowserFromWebBrowser(
    IWebBrowser2* webBrowser,
    IShellBrowser** shellBrowser) {
    if (!webBrowser || !shellBrowser) {
        return E_INVALIDARG;
    }

    *shellBrowser = nullptr;

    // Explorer's IWebBrowser2 automation object often exposes IShellBrowser
    // directly. Prefer that because it identifies the individual tab.
    HRESULT hr =
        webBrowser->QueryInterface(IID_PPV_ARGS(shellBrowser));

    if (SUCCEEDED(hr) && *shellBrowser) {
        return hr;
    }

    IServiceProvider* serviceProvider = nullptr;
    hr = webBrowser->QueryInterface(
        IID_PPV_ARGS(&serviceProvider));

    if (FAILED(hr) || !serviceProvider) {
        return hr;
    }

    hr = serviceProvider->QueryService(
        SID_STopLevelBrowser,
        IID_PPV_ARGS(shellBrowser));

    serviceProvider->Release();
    return hr;
}

static bool GetViewModeFromAutomationDocument(
    IWebBrowser2* webBrowser,
    FOLDERVIEWMODE* viewMode) {
    if (!webBrowser || !viewMode) {
        return false;
    }

    IDispatch* document = nullptr;
    HRESULT hr = webBrowser->get_Document(&document);

    if (FAILED(hr) || !document) {
        return false;
    }

    IShellFolderViewDual2* folderViewDual = nullptr;
    hr = document->QueryInterface(
        IID_PPV_ARGS(&folderViewDual));

    document->Release();

    if (FAILED(hr) || !folderViewDual) {
        return false;
    }

    UINT rawMode = 0;
    hr = folderViewDual->get_CurrentViewMode(&rawMode);
    folderViewDual->Release();

    if (FAILED(hr)) {
        return false;
    }

    *viewMode = static_cast<FOLDERVIEWMODE>(rawMode);
    return true;
}

static bool GetExplorerViewMode(
    HWND explorerFrame,
    HWND activeTabWindow,
    FOLDERVIEWMODE* viewMode,
    PROPERTYKEY* groupByKey = nullptr,
    BOOL* groupAscending = nullptr,
    bool* groupByKnown = nullptr,
    SORTCOLUMN* sortColumns = nullptr,
    int sortColumnsCapacity = 0,
    int* sortColumnCount = nullptr,
    bool* sortColumnsKnown = nullptr) {
    if (!explorerFrame || !viewMode) {
        return false;
    }

    // CompareIDs can be entered from a thread on which nobody initialized COM.
    // v2.0.x assumed COM was already initialized, which isn't safe.
    HRESULT coHr = CoInitializeEx(
        nullptr,
        COINIT_APARTMENTTHREADED);

    bool shouldCoUninitialize =
        coHr == S_OK || coHr == S_FALSE;

    // RPC_E_CHANGED_MODE means COM is already initialized on this thread using
    // another apartment model. COM calls are still available, so continue.
    if (FAILED(coHr) && coHr != RPC_E_CHANGED_MODE) {
        Wh_Log(
            L"View mode: CoInitializeEx failed: 0x%08X",
            static_cast<unsigned>(coHr));
        return false;
    }

    IShellWindows* shellWindows = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellWindows,
        nullptr,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_PPV_ARGS(&shellWindows));

    if (FAILED(hr) || !shellWindows) {
        if (shouldCoUninitialize) {
            CoUninitialize();
        }

        Wh_Log(
            L"View mode: CLSID_ShellWindows failed: 0x%08X",
            static_cast<unsigned>(hr));
        return false;
    }

    long count = 0;
    hr = shellWindows->get_Count(&count);

    bool success = false;

    if (SUCCEEDED(hr)) {
        for (long i = 0; i < count && !success; i++) {
            VARIANT index;
            VariantInit(&index);
            index.vt = VT_I4;
            index.lVal = i;

            IDispatch* dispatch = nullptr;
            hr = shellWindows->Item(index, &dispatch);
            VariantClear(&index);

            if (FAILED(hr) || !dispatch) {
                continue;
            }

            IWebBrowser2* webBrowser = nullptr;
            hr = dispatch->QueryInterface(
                IID_PPV_ARGS(&webBrowser));
            dispatch->Release();

            if (FAILED(hr) || !webBrowser) {
                continue;
            }

            SHANDLE_PTR frameHwndValue = 0;
            hr = webBrowser->get_HWND(&frameHwndValue);

            if (FAILED(hr) ||
                reinterpret_cast<HWND>(frameHwndValue) != explorerFrame) {
                webBrowser->Release();
                continue;
            }

            IShellBrowser* shellBrowser = nullptr;
            hr = GetShellBrowserFromWebBrowser(
                webBrowser,
                &shellBrowser);

            if (SUCCEEDED(hr) && shellBrowser) {
                HWND thisTabWindow = nullptr;
                HRESULT windowHr =
                    shellBrowser->GetWindow(&thisTabWindow);

                // On Windows 11 several tab automation objects can share the
                // same top-level IWebBrowser2::HWND. Match the tab-specific
                // ShellTabWindowClass before asking for its view mode.
                bool isCorrectTab =
                    !activeTabWindow ||
                    (SUCCEEDED(windowHr) &&
                     thisTabWindow == activeTabWindow);

                if (isCorrectTab) {
                    success = GetViewModeFromShellBrowser(
                        shellBrowser,
                        viewMode,
                        groupByKey,
                        groupAscending,
                        groupByKnown,
                        sortColumns,
                        sortColumnsCapacity,
                        sortColumnCount,
                        sortColumnsKnown);

                    // Fallback through the automation Document interface. It
                    // exposes IShellFolderViewDual2::CurrentViewMode.
                    if (!success) {
                        success =
                            GetViewModeFromAutomationDocument(
                                webBrowser,
                                viewMode);
                    }
                }

                shellBrowser->Release();
            } else if (!activeTabWindow) {
                // Older Explorer versions without tabs can still expose the
                // folder view through the automation Document.
                success = GetViewModeFromAutomationDocument(
                    webBrowser,
                    viewMode);
            }

            webBrowser->Release();
        }
    }

    shellWindows->Release();

    if (shouldCoUninitialize) {
        CoUninitialize();
    }

    return success;
}

struct ViewModeCache {
    HWND rootWindow = nullptr;
    HWND tabWindow = nullptr;
    ULONGLONG queriedAt = 0;
    ViewModeInfo info{};
};

static thread_local ViewModeCache g_viewModeCache{};

static void InvalidateViewModeCache() {
    g_viewModeCache = {};
}

static ViewModeInfo ResolveCurrentViewMode() {
    ViewModeInfo result{};

    HWND rootWindow = GetRelevantRootWindow();

    if (!rootWindow) {
        return result;
    }

    result.rootWindow = rootWindow;

    if (!IsExplorerFrame(rootWindow)) {
        return result;
    }

    result.tabWindow =
        FindActiveExplorerTabWindow(rootWindow);

    ULONGLONG now = GetTickCount64();

    // Sorting can call CompareIDs thousands of times. Resolve the Shell view
    // only once per short comparison burst.
    constexpr ULONGLONG kCacheLifetimeMs = 100;

    if (g_viewModeCache.rootWindow == result.rootWindow &&
        g_viewModeCache.tabWindow == result.tabWindow &&
        now - g_viewModeCache.queriedAt <=
            kCacheLifetimeMs) {
        return g_viewModeCache.info;
    }

    result.known = GetExplorerViewMode(
        result.rootWindow,
        result.tabWindow,
        &result.mode,
        &result.groupByKey,
        &result.groupAscending,
        &result.groupByKnown,
        result.sortColumns,
        ARRAYSIZE(result.sortColumns),
        &result.sortColumnCount,
        &result.sortColumnsKnown);

    g_viewModeCache.rootWindow = result.rootWindow;
    g_viewModeCache.tabWindow = result.tabWindow;
    g_viewModeCache.queriedAt = now;
    g_viewModeCache.info = result;

    return result;
}

// -----------------------------------------------------------------------------
// CFSFolder sorting hook
// -----------------------------------------------------------------------------

using CFSFolder_MapColumnToSCID_t =
    HRESULT(WINAPI*)(
        void* self,
        UINT column,
        PROPERTYKEY* key);

using CFSFolder_GetDetailsEx_t =
    HRESULT(WINAPI*)(
        void* self,
        const ITEMID_CHILD* item,
        const PROPERTYKEY* key,
        VARIANT* value);

using CFSFolder_CompareIDs_t =
    HRESULT(WINAPI*)(
        void* self,
        LPARAM sortParam,
        const ITEMIDLIST_RELATIVE* item1,
        const ITEMIDLIST_RELATIVE* item2);

static CFSFolder_MapColumnToSCID_t g_mapColumnToScid;
static CFSFolder_GetDetailsEx_t g_getDetailsEx;
static CFSFolder_CompareIDs_t g_compareIds;


// -----------------------------------------------------------------------------
// Windows 11 sort-pipeline priming.
//
// In the ungrouped Windows 11 path, Explorer can keep an implicit folder-first
// arrangement even while the UI says it is sorted by a date property.
//
// The working v2.6 trace showed that resolving the active IFolderView2 state at
// the internal sort boundary makes Explorer expose the real DateModified/
// DateCreated sort column and route the ungrouped sort through DVCompareColumns.
//
// Keep this intentionally small: no stack walking, symbol enumeration, counters
// or logging. ResolveCurrentViewMode() is cached for short comparison bursts.
using DVCompareColumns_t =
    int(__cdecl*)(
        const void* itemKey1,
        const void* itemKey2,
        __int64 context);

using CSortTask_InternalResumeRT_t =
    HRESULT(WINAPI*)(void* self);

static DVCompareColumns_t g_dvCompareColumns;
static CSortTask_InternalResumeRT_t g_cSortTaskInternalResumeRT;

// A freshly opened folder is initially ordered by CEnumTask::_SortItems.
// During that path, CFSFolder::CompareIDs can receive a Shell sort parameter
// which MapColumnToSCID can't translate, even though the active Explorer view
// is clearly sorted by Date modified/created.
//
// Keep the active date property only for the duration of that synchronous
// CEnumTask sort. CFSFolder::CompareIDs uses it strictly as a fallback when
// normal column mapping fails.
struct InitialEnumSortContext {
    bool active = false;
    PROPERTYKEY key{};
};

static thread_local InitialEnumSortContext g_initialEnumSortContext{};

using CEnumTask_SortItems_t =
    void(WINAPI*)(
        void* self,
        void* itemArray,
        IShellFolder2* shellFolder,
        void* itemStore);

static CEnumTask_SortItems_t g_cEnumTaskSortItems;

static thread_local bool g_inSortPipelinePrime = false;

static void PrimeActiveSortContext() {
    if (g_inSortPipelinePrime) {
        return;
    }

    if (!g_settings.mixDateModified &&
        !g_settings.mixDateCreated &&
        !HasViewExclusions()) {
        return;
    }

    g_inSortPipelinePrime = true;

    // The getters used by ResolveCurrentViewMode are deliberately called here:
    // GetViewModeAndIconSize, GetGroupBy and GetSortColumns.
    //
    // Apart from supplying the per-view exclusion state, this synchronizes the
    // active sort description before Explorer's asynchronous sort task runs.
    (void)ResolveCurrentViewMode();

    g_inSortPipelinePrime = false;
}

static void WINAPI CEnumTask_SortItems_Hook(
    void* self,
    void* itemArray,
    IShellFolder2* shellFolder,
    void* itemStore) {
    InitialEnumSortContext previousContext =
        g_initialEnumSortContext;

    g_initialEnumSortContext = {};

    // Navigation keeps the same Explorer frame/tab HWND, so a short-lived
    // view-state cache from the previous folder can otherwise survive into
    // the first sort of the newly opened folder.
    InvalidateViewModeCache();

    ViewModeInfo view = ResolveCurrentViewMode();

    bool ignored =
        view.known &&
        IsIgnoredView(view.mode);

    if (!ignored &&
        view.sortColumnsKnown &&
        view.sortColumnCount > 0) {
        const PROPERTYKEY& activeKey =
            view.sortColumns[0].propkey;

        if ((g_settings.mixDateModified &&
             IsEqualPropertyKey(
                 activeKey,
                 PKEY_DateModified)) ||
            (g_settings.mixDateCreated &&
             IsEqualPropertyKey(
                 activeKey,
                 PKEY_DateCreated))) {
            g_initialEnumSortContext.active = true;
            g_initialEnumSortContext.key = activeKey;
        }
    }

    // Keep the same priming used by the later CSortTask path, but do it before
    // the initial enumeration actually starts comparing items.
    PrimeActiveSortContext();

    g_cEnumTaskSortItems(
        self,
        itemArray,
        shellFolder,
        itemStore);

    g_initialEnumSortContext =
        previousContext;
}

static int __cdecl DVCompareColumns_Hook(
    const void* itemKey1,
    const void* itemKey2,
    __int64 context) {
    PrimeActiveSortContext();

    return g_dvCompareColumns(
        itemKey1,
        itemKey2,
        context);
}

static HRESULT WINAPI CSortTask_InternalResumeRT_Hook(
    void* self) {
    PrimeActiveSortContext();
    return g_cSortTaskInternalResumeRT(self);
}

enum class MixedSortProperty {
    None,
    DateModified,
    DateCreated,
};

static MixedSortProperty GetMixedSortProperty(
    const PROPERTYKEY& key) {
    if (g_settings.mixDateModified &&
        IsEqualPropertyKey(key, PKEY_DateModified)) {
        return MixedSortProperty::DateModified;
    }

    if (g_settings.mixDateCreated &&
        IsEqualPropertyKey(key, PKEY_DateCreated)) {
        return MixedSortProperty::DateCreated;
    }

    return MixedSortProperty::None;
}

static bool GetPropertyVariant(
    void* folder,
    const ITEMIDLIST_RELATIVE* item,
    const PROPERTYKEY& key,
    VARIANT* value) {
    if (!folder || !item || !value) {
        return false;
    }

    VariantInit(value);

    HRESULT hr = g_getDetailsEx(
        folder,
        reinterpret_cast<const ITEMID_CHILD*>(item),
        &key,
        value);

    if (FAILED(hr) ||
        value->vt == VT_EMPTY ||
        value->vt == VT_NULL) {
        VariantClear(value);
        return false;
    }

    return true;
}

static bool VariantToDateValue(
    const VARIANT& source,
    DATE* result) {
    if (!result) {
        return false;
    }

    if (source.vt == VT_DATE) {
        *result = source.date;
        return true;
    }

    VARIANT converted;
    VariantInit(&converted);

    HRESULT hr = VariantChangeType(
        &converted,
        const_cast<VARIANT*>(&source),
        0,
        VT_DATE);

    if (FAILED(hr) || converted.vt != VT_DATE) {
        VariantClear(&converted);
        return false;
    }

    *result = converted.date;
    VariantClear(&converted);
    return true;
}

static HRESULT CompareResult(int result) {
    // IShellFolder::CompareIDs encodes the signed result in the low WORD.
    if (result < 0) {
        return static_cast<HRESULT>(0x0000FFFF);
    }

    if (result > 0) {
        return static_cast<HRESULT>(0x00000001);
    }

    return S_OK;
}

static HRESULT CompareDateProperty(
    void* folder,
    const ITEMIDLIST_RELATIVE* item1,
    const ITEMIDLIST_RELATIVE* item2,
    const PROPERTYKEY& key,
    bool* handled) {
    *handled = false;

    VARIANT value1;
    VARIANT value2;

    if (!GetPropertyVariant(
            folder,
            item1,
            key,
            &value1)) {
        return S_OK;
    }

    if (!GetPropertyVariant(
            folder,
            item2,
            key,
            &value2)) {
        VariantClear(&value1);
        return S_OK;
    }

    DATE date1 = 0;
    DATE date2 = 0;

    bool success =
        VariantToDateValue(value1, &date1) &&
        VariantToDateValue(value2, &date2);

    VariantClear(&value1);
    VariantClear(&value2);

    if (!success) {
        return S_OK;
    }

    *handled = true;

    if (date1 < date2) {
        return CompareResult(-1);
    }

    if (date1 > date2) {
        return CompareResult(1);
    }

    return S_OK;
}

static HRESULT WINAPI CFSFolder_CompareIDs_Hook(
    void* self,
    LPARAM sortParam,
    const ITEMIDLIST_RELATIVE* item1,
    const ITEMIDLIST_RELATIVE* item2) {
    auto callOriginal = [&]() {
        return g_compareIds(
            self,
            sortParam,
            item1,
            item2);
    };

    if (!self || !item1 || !item2) {
        return callOriginal();
    }

    // IShellFolder::CompareIDs receives an LPARAM, not a bare column index.
    // The low WORD identifies the sort column while upper bits can contain
    // SHCIDS_* flags. Passing the complete value to MapColumnToSCID is wrong.
    const UINT column = static_cast<UINT>(
        static_cast<ULONG_PTR>(sortParam) &
        0x0000FFFFu);

    PROPERTYKEY key{};

    HRESULT mapHr =
        g_mapColumnToScid(
            self,
            column,
            &key);

    if (FAILED(mapHr)) {
        // Fresh-folder enumeration can use an internal/sentinel sort parameter
        // which isn't mappable even after extracting LOWORD. We only override
        // that case while synchronously inside CEnumTask::_SortItems, using the
        // exact active Date modified/created key captured from IFolderView2.
        if (!g_initialEnumSortContext.active) {
            return callOriginal();
        }

        key = g_initialEnumSortContext.key;
    }

    MixedSortProperty property =
        GetMixedSortProperty(key);

    if (property == MixedSortProperty::None) {
        return callOriginal();
    }

    if (HasViewExclusions()) {
        ViewModeInfo view = ResolveCurrentViewMode();

        if (view.known && IsIgnoredView(view.mode)) {
            return callOriginal();
        }
    }

    bool handled = false;
    HRESULT result = S_OK;

    switch (property) {
        case MixedSortProperty::DateModified:
        case MixedSortProperty::DateCreated:
            result = CompareDateProperty(
                self,
                item1,
                item2,
                key,
                &handled);
            break;

        default:
            break;
    }

    if (!handled || result == S_OK) {
        // Preserve Explorer's own tie-breaking behavior.
        return callOriginal();
    }

    return result;
}

static bool HookWindowsStorage() {
    HMODULE windowsStorage = LoadLibraryExW(
        L"windows.storage.dll",
        nullptr,
        LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (!windowsStorage) {
        Wh_Log(L"Failed to load windows.storage.dll");
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK commonHooks[] = {
        {
            {
#ifdef _WIN64
                LR"(public: virtual long __cdecl CFSFolder::MapColumnToSCID(unsigned int,struct _tagpropertykey *))",
#else
                LR"(public: virtual long __stdcall CFSFolder::MapColumnToSCID(unsigned int,struct _tagpropertykey *))",
#endif
            },
            &g_mapColumnToScid,
        },
        {
            {
#ifdef _WIN64
                LR"(public: virtual long __cdecl CFSFolder::GetDetailsEx(struct _ITEMID_CHILD const __unaligned *,struct _tagpropertykey const *,struct tagVARIANT *))",
#else
                LR"(public: virtual long __stdcall CFSFolder::GetDetailsEx(struct _ITEMID_CHILD const *,struct _tagpropertykey const *,struct tagVARIANT *))",
#endif
            },
            &g_getDetailsEx,
        },
        {
            {
#ifdef _WIN64
                LR"(public: virtual long __cdecl CFSFolder::CompareIDs(__int64,struct _ITEMIDLIST_RELATIVE const __unaligned *,struct _ITEMIDLIST_RELATIVE const __unaligned *))",
#else
                LR"(public: virtual long __stdcall CFSFolder::CompareIDs(long,struct _ITEMIDLIST_RELATIVE const *,struct _ITEMIDLIST_RELATIVE const *))",
#endif
            },
            &g_compareIds,
            CFSFolder_CompareIDs_Hook,
        },
    };

    if (!WindhawkUtils::HookSymbols(
            windowsStorage,
            commonHooks,
            ARRAYSIZE(commonHooks))) {
        Wh_Log(L"Failed to resolve/hook CFSFolder symbols");
        return false;
    }

#ifdef _WIN64
    WindhawkUtils::SYMBOL_HOOK explorerPipelineHooks[] = {
        {
            {
                LR"(private: void __cdecl CEnumTask::_SortItems(class CDSA<struct tagITEMKEY> *,struct IShellFolder2 *,struct IItemStore *))",
            },
            &g_cEnumTaskSortItems,
            CEnumTask_SortItems_Hook,
        },
        {
            {
                LR"(int __cdecl DVCompareColumns(struct tagITEMKEY const *,struct tagITEMKEY const *,__int64))",
            },
            &g_dvCompareColumns,
            DVCompareColumns_Hook,
        },
        {
            {
                LR"(public: virtual long __cdecl CSortTask::InternalResumeRT(void))",
            },
            &g_cSortTaskInternalResumeRT,
            CSortTask_InternalResumeRT_Hook,
        },
    };

    if (!WindhawkUtils::HookSymbols(
            windowsStorage,
            explorerPipelineHooks,
            ARRAYSIZE(explorerPipelineHooks))) {
        Wh_Log(L"Failed to resolve/hook Explorer sort-pipeline symbols");
        return false;
    }
#endif

    return true;
}

// -----------------------------------------------------------------------------
// Windhawk lifecycle
// -----------------------------------------------------------------------------

BOOL Wh_ModInit() {
    LoadSettings();

    if (!HookWindowsStorage()) {
        return FALSE;
    }

    Wh_Log(L"Froggo Explorer Mixed Sorting 3.1.1 ready");
    return TRUE;
}

BOOL Wh_ModSettingsChanged(BOOL* bReload) {
    (void)bReload;

    LoadSettings();
    InvalidateViewModeCache();

    Wh_Log(L"Settings reloaded");
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninitializing");
}
