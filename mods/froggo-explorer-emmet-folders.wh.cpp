// ==WindhawkMod==
// @id              froggo-explorer-emmet-folders
// @name            Froggo Explorer Emmet Folders
// @description     Create folder trees from Emmet-like expressions typed in File Explorer's native rename editor.
// @version         1.0.4
// @author          Sapito Sucio
// @github          https://github.com/SapitoSucio
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -std=c++20 -lole32 -loleaut32 -lshell32 -lshlwapi -luuid -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Froggo Explorer Emmet Folders

Type compact expressions while renaming a folder in File Explorer:

- `Temporada {1..8}`
- `Serie/Temporada {1..3}/Capítulos+Extras+Especiales`
- `Anime {1..2}/Temporada {1..3}+Películas+OVAs`

The original folder becomes the first missing top-level result and keeps all of
its contents. Existing directories are reused without merging or overwriting.

Version 1.0.4 removes per-keystroke text recapture and represents typed `/`
with a Windows-valid visual separator while editing. The parser normalizes it
back to `/`, avoiding rename flicker and Explorer's illegal-character dialog.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- previewThreshold: 10
  $name: Preview threshold
  $description: Show confirmation when more than this many directories would be affected.
- maximumGeneratedDirectories: 500
  $name: Maximum generated directories
  $description: Reject expressions that would generate more directories than this limit.
- maximumDepth: 16
  $name: Maximum nesting depth
  $description: Reject structures deeper than this number of folder levels.
- showSuccessNotification: false
  $name: Show success notification
  $description: Show a compact completion message after successful expansion.
- enableDebugLogging: false
  $name: Enable debug logging
  $description: Log parsing, planning, skipped paths, and operation failures.
*/
// ==/WindhawkModSettings==

#include <algorithm>
#include <cstddef>
#include <cwctype>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace eef {

struct Limits {
    std::size_t maximumGeneratedDirectories = 500;
    std::size_t maximumDepth = 16;
    std::size_t maximumExpressionLength = 1024;
};

struct Range {
    unsigned start = 0;
    unsigned end = 0;
};

struct Branch {
    std::wstring prefix;
    std::optional<Range> range;
    std::wstring suffix;
};

struct Level {
    std::vector<Branch> branches;
};

struct Expression {
    std::vector<Level> levels;
};

enum class ParseDisposition {
    NativeFallback,
    Expand,
    SyntaxError,
};

struct ParseResult {
    ParseDisposition disposition = ParseDisposition::NativeFallback;
    std::optional<Expression> expression;
    std::wstring error;
};

struct NameValidationResult {
    bool ok = false;
    std::wstring normalized;
    std::wstring error;
};

struct ExpandedPath {
    std::vector<std::wstring> components;
};

struct ExpansionResult {
    bool ok = false;
    std::vector<ExpandedPath> paths;
    std::vector<std::wstring> topLevelNames;
    std::wstring error;
};

enum class PathKind {
    Missing,
    Directory,
    Other,
};

class IPathProbe {
public:
    virtual ~IPathProbe() = default;
    virtual PathKind Probe(std::wstring_view absolutePath) const = 0;
};

struct PlannedDirectory {
    std::wstring absolutePath;
    std::size_t depth = 0;
    bool exists = false;
    bool fulfilledBySource = false;
};

struct ExecutionPlan {
    bool ok = false;
    bool sourceStaysInPlace = false;
    std::wstring sourcePath;
    std::wstring sourceTargetPath;
    std::vector<PlannedDirectory> directories;
    std::size_t affectedCount = 0;
    std::size_t skippedCount = 0;
    std::wstring error;
};

std::wstring NormalizeExpressionSeparators(std::wstring_view value) {
    std::wstring normalized(value);
    for (wchar_t& ch : normalized) {
        if (ch == L'\u2215' || ch == L'\u2044' || ch == L'\uFF0F') {
            ch = L'/';
        }
    }
    return normalized;
}

static std::wstring Trim(std::wstring_view value) {
    std::size_t first = 0;
    while (first < value.size() && std::iswspace(value[first])) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::iswspace(value[last - 1])) {
        --last;
    }

    return std::wstring(value.substr(first, last - first));
}

static std::wstring ToUpper(std::wstring_view value) {
    std::wstring out(value);
    std::transform(out.begin(), out.end(), out.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towupper(ch));
    });
    return out;
}

static bool IsReservedDeviceName(std::wstring_view value) {
    std::wstring normalized = ToUpper(value);
    const std::size_t dot = normalized.find(L'.');
    if (dot != std::wstring::npos) {
        normalized.resize(dot);
    }

    if (normalized == L"CON" || normalized == L"PRN" ||
        normalized == L"AUX" || normalized == L"NUL") {
        return true;
    }

    if (normalized.size() == 4 &&
        (normalized.rfind(L"COM", 0) == 0 || normalized.rfind(L"LPT", 0) == 0) &&
        normalized[3] >= L'1' && normalized[3] <= L'9') {
        return true;
    }

    return false;
}

NameValidationResult ValidateFolderName(std::wstring_view value) {
    NameValidationResult result;
    result.normalized = Trim(value);

    if (result.normalized.empty()) {
        result.error = L"Folder names may not be empty.";
        return result;
    }

    if (result.normalized.size() > 255) {
        result.error = L"A generated folder name exceeds Windows' 255-character component limit.";
        return result;
    }

    if (result.normalized.back() == L'.' || result.normalized.back() == L' ') {
        result.error = L"Folder names may not end in a dot or space.";
        return result;
    }

    constexpr std::wstring_view illegal = L"\\:*?\"<>|/";
    for (wchar_t ch : result.normalized) {
        if (ch < 32 || illegal.find(ch) != std::wstring_view::npos) {
            result.error = L"A generated folder name contains an invalid character.";
            return result;
        }
    }

    if (IsReservedDeviceName(result.normalized)) {
        result.error = L"A generated folder name is reserved by Windows.";
        return result;
    }

    result.ok = true;
    return result;
}

static bool ParsePositiveUnsigned(std::wstring_view text, unsigned& value) {
    if (text.empty()) {
        return false;
    }
    if (text.size() > 1 && text.front() == L'0') {
        return false;
    }

    unsigned long long accumulator = 0;
    for (wchar_t ch : text) {
        if (ch < L'0' || ch > L'9') {
            return false;
        }
        accumulator = accumulator * 10 + static_cast<unsigned>(ch - L'0');
        if (accumulator > std::numeric_limits<unsigned>::max()) {
            return false;
        }
    }

    if (accumulator == 0) {
        return false;
    }

    value = static_cast<unsigned>(accumulator);
    return true;
}

static std::vector<std::wstring_view> SplitPreserveEmpty(std::wstring_view text,
                                                         wchar_t delimiter) {
    std::vector<std::wstring_view> pieces;
    std::size_t start = 0;
    while (true) {
        const std::size_t next = text.find(delimiter, start);
        if (next == std::wstring_view::npos) {
            pieces.push_back(text.substr(start));
            break;
        }
        pieces.push_back(text.substr(start, next - start));
        start = next + 1;
    }
    return pieces;
}

static ParseResult InvalidExpression(bool containsSlash, std::wstring message) {
    ParseResult result;
    result.disposition = containsSlash ? ParseDisposition::SyntaxError
                                       : ParseDisposition::NativeFallback;
    if (result.disposition == ParseDisposition::SyntaxError) {
        result.error = std::move(message);
    }
    return result;
}

ParseResult ParseExpression(std::wstring_view text, const Limits& limits) {
    const std::wstring normalizedText = NormalizeExpressionSeparators(text);
    text = normalizedText;

    const bool containsSlash = text.find(L'/') != std::wstring_view::npos;
    const bool containsPlus = text.find(L'+') != std::wstring_view::npos;
    const bool containsBrace = text.find(L'{') != std::wstring_view::npos ||
                               text.find(L'}') != std::wstring_view::npos;

    if (text.size() > limits.maximumExpressionLength) {
        ParseResult result;
        result.disposition = ParseDisposition::SyntaxError;
        result.error = L"The expression exceeds the configured length limit.";
        return result;
    }

    auto levelTexts = SplitPreserveEmpty(text, L'/');
    if (levelTexts.size() > limits.maximumDepth) {
        ParseResult result;
        result.disposition = ParseDisposition::SyntaxError;
        result.error = L"The expression exceeds the configured nesting depth.";
        return result;
    }

    Expression expression;
    bool hasValidRange = false;

    for (std::wstring_view levelText : levelTexts) {
        if (levelText.empty()) {
            return InvalidExpression(containsSlash, L"Folder levels may not be empty.");
        }

        Level level;
        auto branchTexts = SplitPreserveEmpty(levelText, L'+');
        for (std::wstring_view branchText : branchTexts) {
            if (branchText.empty()) {
                return InvalidExpression(containsSlash, L"Sibling branches may not be empty.");
            }

            const std::size_t open = branchText.find(L'{');
            const std::size_t close = branchText.find(L'}');
            Branch branch;

            if (open == std::wstring_view::npos && close == std::wstring_view::npos) {
                auto validation = ValidateFolderName(branchText);
                if (!validation.ok) {
                    return InvalidExpression(containsSlash, validation.error);
                }
                branch.prefix = std::move(validation.normalized);
            } else {
                if (open == std::wstring_view::npos || close == std::wstring_view::npos ||
                    close < open ||
                    branchText.find(L'{', open + 1) != std::wstring_view::npos ||
                    branchText.find(L'}', close + 1) != std::wstring_view::npos) {
                    return InvalidExpression(containsSlash, L"Malformed numeric range.");
                }

                std::wstring_view body = branchText.substr(open + 1, close - open - 1);
                const std::size_t separator = body.find(L"..");
                if (separator == std::wstring_view::npos ||
                    body.find(L"..", separator + 2) != std::wstring_view::npos) {
                    return InvalidExpression(containsSlash, L"Numeric ranges must use {start..end}.");
                }

                unsigned start = 0;
                unsigned end = 0;
                if (!ParsePositiveUnsigned(body.substr(0, separator), start) ||
                    !ParsePositiveUnsigned(body.substr(separator + 2), end) ||
                    end < start) {
                    return InvalidExpression(containsSlash,
                                             L"Version 1 supports ascending positive ranges without padding.");
                }

                branch.prefix = std::wstring(branchText.substr(0, open));
                branch.range = Range{start, end};
                branch.suffix = std::wstring(branchText.substr(close + 1));
                hasValidRange = true;
            }

            level.branches.push_back(std::move(branch));
        }

        expression.levels.push_back(std::move(level));
    }

    if (!containsSlash && !containsPlus && !hasValidRange) {
        return {};
    }

    if (containsBrace && !hasValidRange && !containsSlash && !containsPlus) {
        return {};
    }

    ParseResult result;
    result.disposition = ParseDisposition::Expand;
    result.expression = std::move(expression);
    return result;
}

static std::vector<std::wstring> ExpandBranch(const Branch& branch,
                                              std::size_t maximumNames,
                                              std::wstring& error) {
    std::vector<std::wstring> names;

    if (!branch.range) {
        auto validation = ValidateFolderName(branch.prefix);
        if (!validation.ok) {
            error = validation.error;
            return {};
        }
        names.push_back(std::move(validation.normalized));
        return names;
    }

    const unsigned start = branch.range->start;
    const unsigned end = branch.range->end;
    const unsigned long long count =
        static_cast<unsigned long long>(end) - start + 1ULL;
    if (count > maximumNames) {
        error = L"The expression exceeds the configured directory limit.";
        return {};
    }
    names.reserve(static_cast<std::size_t>(count));
    for (unsigned current = start; current <= end; ++current) {
        std::wstring generated = branch.prefix + std::to_wstring(current) + branch.suffix;
        auto validation = ValidateFolderName(generated);
        if (!validation.ok) {
            error = validation.error;
            return {};
        }
        names.push_back(std::move(validation.normalized));
        if (current == std::numeric_limits<unsigned>::max()) {
            break;
        }
    }

    return names;
}

ExpansionResult ExpandExpression(const Expression& expression,
                                 const Limits& limits) {
    ExpansionResult result;
    if (expression.levels.empty()) {
        result.error = L"The expression does not contain any folder levels.";
        return result;
    }
    if (expression.levels.size() > limits.maximumDepth) {
        result.error = L"The expression exceeds the configured nesting depth.";
        return result;
    }

    std::vector<std::vector<std::vector<std::wstring>>> expandedLevels;
    expandedLevels.resize(expression.levels.size());

    for (std::size_t levelIndex = 0; levelIndex < expression.levels.size(); ++levelIndex) {
        for (const auto& branch : expression.levels[levelIndex].branches) {
            std::wstring error;
            auto names = ExpandBranch(branch, limits.maximumGeneratedDirectories, error);
            if (!error.empty()) {
                result.error = std::move(error);
                return result;
            }
            expandedLevels[levelIndex].push_back(std::move(names));
        }
    }

    std::function<bool(std::size_t, std::vector<std::wstring>&)> visit;
    visit = [&](std::size_t levelIndex, std::vector<std::wstring>& components) {
        for (const auto& branchNames : expandedLevels[levelIndex]) {
            for (const auto& name : branchNames) {
                components.push_back(name);
                result.paths.push_back({components});
                if (result.paths.size() > limits.maximumGeneratedDirectories) {
                    result.error = L"The expression exceeds the configured directory limit.";
                    return false;
                }

                if (levelIndex == 0) {
                    result.topLevelNames.push_back(name);
                }

                if (levelIndex + 1 < expandedLevels.size() &&
                    !visit(levelIndex + 1, components)) {
                    return false;
                }

                components.pop_back();
            }
        }
        return true;
    };

    std::vector<std::wstring> components;
    if (!visit(0, components)) {
        result.paths.clear();
        result.topLevelNames.clear();
        return result;
    }

    result.ok = true;
    return result;
}

static std::wstring JoinPath(std::wstring_view left, std::wstring_view right) {
    if (left.empty()) {
        return std::wstring(right);
    }
    if (right.empty()) {
        return std::wstring(left);
    }

    std::wstring result(left);
    if (result.back() != L'\\' && result.back() != L'/') {
        result.push_back(L'\\');
    }
    result.append(right);
    return result;
}

static std::wstring JoinComponents(std::wstring_view parent,
                                   const std::vector<std::wstring>& components) {
    std::wstring path(parent);
    for (const auto& component : components) {
        path = JoinPath(path, component);
    }
    return path;
}

static bool PathEquals(std::wstring_view left, std::wstring_view right) {
    auto trimTrailing = [](std::wstring_view value) {
        while (value.size() > 3 && (value.back() == L'\\' || value.back() == L'/')) {
            value.remove_suffix(1);
        }
        return value;
    };

    left = trimTrailing(left);
    right = trimTrailing(right);
    if (left.size() != right.size()) {
        return false;
    }

    for (std::size_t i = 0; i < left.size(); ++i) {
        wchar_t a = left[i] == L'/' ? L'\\' : left[i];
        wchar_t b = right[i] == L'/' ? L'\\' : right[i];
        if (std::towupper(a) != std::towupper(b)) {
            return false;
        }
    }
    return true;
}

ExecutionPlan BuildExecutionPlan(std::wstring_view parentPath,
                                 std::wstring_view sourcePath,
                                 const ExpansionResult& expansion,
                                 const IPathProbe& probe) {
    ExecutionPlan plan;
    plan.sourcePath = std::wstring(sourcePath);

    if (!expansion.ok || expansion.topLevelNames.empty()) {
        plan.error = expansion.error.empty() ? L"The expansion is empty." : expansion.error;
        return plan;
    }

    for (const auto& topLevelName : expansion.topLevelNames) {
        std::wstring candidate = JoinPath(parentPath, topLevelName);
        if (PathEquals(candidate, sourcePath)) {
            plan.sourceStaysInPlace = true;
            plan.sourceTargetPath = std::move(candidate);
            break;
        }
    }

    if (!plan.sourceStaysInPlace) {
        for (const auto& topLevelName : expansion.topLevelNames) {
            std::wstring candidate = JoinPath(parentPath, topLevelName);
            PathKind kind = probe.Probe(candidate);
            if (kind == PathKind::Other) {
                plan.error = L"A generated top-level path collides with a non-directory item: " + candidate;
                return plan;
            }
            if (kind == PathKind::Missing) {
                plan.sourceTargetPath = std::move(candidate);
                break;
            }
        }
    }

    if (plan.sourceTargetPath.empty()) {
        plan.error = L"Every generated top-level directory already exists.";
        return plan;
    }

    auto relativeSuffix = [](std::wstring_view path,
                             std::wstring_view root) -> std::optional<std::wstring_view> {
        while (root.size() > 3 && (root.back() == L'\\' || root.back() == L'/')) {
            root.remove_suffix(1);
        }
        if (path.size() <= root.size()) {
            return std::nullopt;
        }
        for (std::size_t i = 0; i < root.size(); ++i) {
            wchar_t a = path[i] == L'/' ? L'\\' : path[i];
            wchar_t b = root[i] == L'/' ? L'\\' : root[i];
            if (std::towupper(a) != std::towupper(b)) {
                return std::nullopt;
            }
        }
        if (path[root.size()] != L'\\' && path[root.size()] != L'/') {
            return std::nullopt;
        }
        return path.substr(root.size() + 1);
    };

    std::vector<std::pair<std::wstring, PathKind>> classified;
    classified.reserve(expansion.paths.size());

    for (const auto& relative : expansion.paths) {
        std::wstring absolute = JoinComponents(parentPath, relative.components);
        PathKind kind = PathKind::Missing;

        if (PathEquals(absolute, sourcePath)) {
            kind = PathKind::Directory;
        } else if (!plan.sourceStaysInPlace &&
                   PathEquals(absolute, plan.sourceTargetPath)) {
            kind = PathKind::Missing;
        } else if (!plan.sourceStaysInPlace) {
            auto suffix = relativeSuffix(absolute, plan.sourceTargetPath);
            if (suffix) {
                std::wstring currentSourcePath = JoinPath(sourcePath, *suffix);
                PathKind movingKind = probe.Probe(currentSourcePath);
                kind = movingKind == PathKind::Missing ? probe.Probe(absolute)
                                                       : movingKind;
            } else {
                kind = probe.Probe(absolute);
            }
        } else {
            kind = probe.Probe(absolute);
        }

        if (kind == PathKind::Other) {
            plan.error = L"A generated path collides with a non-directory item: " + absolute;
            return plan;
        }
        classified.emplace_back(std::move(absolute), kind);
    }

    if (!plan.sourceStaysInPlace) {
        ++plan.affectedCount;
    }

    for (std::size_t i = 0; i < expansion.paths.size(); ++i) {
        const auto& relative = expansion.paths[i];
        const auto& [absolute, kind] = classified[i];

        PlannedDirectory directory;
        directory.absolutePath = absolute;
        directory.depth = relative.components.size();
        directory.exists = kind == PathKind::Directory;
        directory.fulfilledBySource = !plan.sourceStaysInPlace &&
                                      PathEquals(absolute, plan.sourceTargetPath);

        if (directory.fulfilledBySource) {
            // The source rename creates this top-level directory.
        } else if (directory.exists) {
            ++plan.skippedCount;
        } else {
            ++plan.affectedCount;
        }

        plan.directories.push_back(std::move(directory));
    }

    plan.ok = true;
    return plan;
}

}  // namespace eef

#ifndef EEF_CORE_ONLY

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER _WIN32_WINNT
#endif

#include <windows.h>
#include <commctrl.h>
#include <exdisp.h>
#include <knownfolders.h>
#include <oleauto.h>
#include <process.h>
#include <servprov.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <ShlGuid.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#include <atomic>
#include <cstdarg>
#include <cwchar>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "comctl32.lib")

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT reinterpret_cast<HINSTANCE>(&__ImageBase)

#ifndef FOFX_SHOWELEVATIONPROMPT
#define FOFX_SHOWELEVATIONPROMPT 0x00040000
#endif

#ifndef FOFX_NOCOPYHOOKS
#define FOFX_NOCOPYHOOKS 0x00800000
#endif

#ifndef FOFX_EARLYFAILURE
#define FOFX_EARLYFAILURE 0x00100000
#endif

namespace {

constexpr wchar_t kDisplayPathSeparator = L'\u2215';
constexpr UINT kWorkerProcessRequest = WM_APP + 0x451;
constexpr UINT kWorkerShutdown = WM_APP + 0x452;
constexpr int kPreviewCreateButton = 1001;
constexpr std::size_t kPreviewEntryLimit = 100;

struct SettingsSnapshot {
    int previewThreshold = 10;
    std::size_t maximumGeneratedDirectories = 500;
    std::size_t maximumDepth = 16;
    std::size_t maximumExpressionLength = 1024;
    bool showSuccessNotification = false;
    bool enableDebugLogging = false;
};

std::atomic<int> g_previewThreshold{10};
std::atomic<std::size_t> g_maximumGeneratedDirectories{500};
std::atomic<std::size_t> g_maximumDepth{16};
std::atomic<bool> g_showSuccessNotification{false};
std::atomic<bool> g_enableDebugLogging{false};
std::atomic<bool> g_unloading{false};
std::atomic<bool> g_acceptingRequests{false};
std::atomic<bool> g_operationRunning{false};

std::atomic<HWINEVENTHOOK> g_renameEditEventHook{nullptr};
HANDLE g_winEventThread = nullptr;
HANDLE g_winEventReadyEvent = nullptr;
HANDLE g_winEventStopEvent = nullptr;
std::atomic<bool> g_winEventStopping{false};
struct RenameEditSession;
std::mutex g_subclassedEditsMutex;
std::map<HWND, RenameEditSession*> g_editSessions;
HANDLE g_workerThread = nullptr;
std::atomic<DWORD> g_workerThreadId{0};
std::atomic<bool> g_workerComInitialized{false};
HANDLE g_workerReadyEvent = nullptr;


template <typename T>
void SafeRelease(T*& value) {
    if (value) {
        value->Release();
        value = nullptr;
    }
}

void DebugLog(const wchar_t* format, ...) {
    if (!g_enableDebugLogging.load(std::memory_order_relaxed)) {
        return;
    }

    wchar_t buffer[2048] = {};
    va_list args;
    va_start(args, format);
    vswprintf(buffer, std::size(buffer), format, args);
    va_end(args);
    Wh_Log(L"%ls", buffer);
}

SettingsSnapshot GetSettingsSnapshot() {
    SettingsSnapshot settings;
    settings.previewThreshold = g_previewThreshold.load(std::memory_order_relaxed);
    settings.maximumGeneratedDirectories =
        g_maximumGeneratedDirectories.load(std::memory_order_relaxed);
    settings.maximumDepth = g_maximumDepth.load(std::memory_order_relaxed);
    settings.showSuccessNotification =
        g_showSuccessNotification.load(std::memory_order_relaxed);
    settings.enableDebugLogging =
        g_enableDebugLogging.load(std::memory_order_relaxed);
    return settings;
}

eef::Limits ToCoreLimits(const SettingsSnapshot& settings) {
    eef::Limits limits;
    limits.maximumGeneratedDirectories = settings.maximumGeneratedDirectories;
    limits.maximumDepth = settings.maximumDepth;
    limits.maximumExpressionLength = settings.maximumExpressionLength;
    return limits;
}

void LoadSettings() {
    int previewThreshold = Wh_GetIntSetting(L"previewThreshold");
    int maximumDirectories = Wh_GetIntSetting(L"maximumGeneratedDirectories");
    int maximumDepth = Wh_GetIntSetting(L"maximumDepth");

    previewThreshold = std::clamp(previewThreshold, 0, 100000);
    maximumDirectories = std::clamp(maximumDirectories, 1, 100000);
    maximumDepth = std::clamp(maximumDepth, 1, 64);

    g_previewThreshold.store(previewThreshold, std::memory_order_relaxed);
    g_maximumGeneratedDirectories.store(
        static_cast<std::size_t>(maximumDirectories), std::memory_order_relaxed);
    g_maximumDepth.store(static_cast<std::size_t>(maximumDepth),
                         std::memory_order_relaxed);
    g_showSuccessNotification.store(
        Wh_GetIntSetting(L"showSuccessNotification") != 0,
        std::memory_order_relaxed);
    g_enableDebugLogging.store(Wh_GetIntSetting(L"enableDebugLogging") != 0,
                               std::memory_order_relaxed);
}

bool IsClassName(HWND hwnd, const wchar_t* expected) {
    if (!hwnd || !expected) {
        return false;
    }

    wchar_t className[128] = {};
    if (!GetClassNameW(hwnd, className, static_cast<int>(std::size(className)))) {
        return false;
    }
    return _wcsicmp(className, expected) == 0;
}

bool IsExplorerTopLevel(HWND hwnd) {
    return IsClassName(hwnd, L"CabinetWClass") ||
           IsClassName(hwnd, L"ExploreWClass");
}

bool IsDesktopTopLevel(HWND hwnd) {
    return hwnd == GetShellWindow() || IsClassName(hwnd, L"Progman") ||
           IsClassName(hwnd, L"WorkerW");
}

HWND FindAncestorOrSelfByClass(HWND hwnd,
                               const wchar_t* className,
                               HWND stopAt = nullptr) {
    for (HWND current = hwnd; current; current = GetParent(current)) {
        if (IsClassName(current, className)) {
            return current;
        }
        if (current == stopAt) {
            break;
        }
    }
    return nullptr;
}

bool IsSupportedRenameEditClass(HWND hwnd) {
    return IsClassName(hwnd, L"Edit") ||
           IsClassName(hwnd, L"RichEditD2DPT") ||
           IsClassName(hwnd, L"RichEdit50W");
}

HWND FindRenameEditAncestor(HWND hwnd) {
    for (HWND current = hwnd; current; current = GetParent(current)) {
        if (IsSupportedRenameEditClass(current)) {
            HWND defView = FindAncestorOrSelfByClass(current,
                                                     L"SHELLDLL_DefView");
            if (defView) {
                HWND root = GetAncestor(defView, GA_ROOT);
                if (IsExplorerTopLevel(root) || IsDesktopTopLevel(root)) {
                    return current;
                }
            }
        }
    }
    return nullptr;
}

HWND GetThreadFocusWindow(HWND topLevel) {
    if (!topLevel) {
        return nullptr;
    }

    const DWORD threadId = GetWindowThreadProcessId(topLevel, nullptr);
    GUITHREADINFO info = {};
    info.cbSize = sizeof(info);
    if (!GetGUIThreadInfo(threadId, &info)) {
        return nullptr;
    }
    return info.hwndFocus ? info.hwndFocus : info.hwndActive;
}

struct DefViewSearchContext {
    HWND found = nullptr;
};

BOOL CALLBACK FindVisibleDefView(HWND child, LPARAM parameter) {
    auto* context = reinterpret_cast<DefViewSearchContext*>(parameter);
    if (IsWindowVisible(child) && IsClassName(child, L"SHELLDLL_DefView")) {
        context->found = child;
        return FALSE;
    }
    return TRUE;
}

struct ShellTabSearchContext {
    HWND bestTab = nullptr;
    LONG_PTR bestArea = 0;
};

BOOL CALLBACK FindVisibleShellTab(HWND hwnd, LPARAM parameter) {
    auto* context = reinterpret_cast<ShellTabSearchContext*>(parameter);
    if (!IsWindowVisible(hwnd) || !IsClassName(hwnd, L"ShellTabWindowClass")) {
        return TRUE;
    }

    DefViewSearchContext defViewContext;
    EnumChildWindows(hwnd, FindVisibleDefView,
                     reinterpret_cast<LPARAM>(&defViewContext));
    if (!defViewContext.found) {
        return TRUE;
    }

    RECT bounds = {};
    if (!GetWindowRect(defViewContext.found, &bounds)) {
        return TRUE;
    }

    const LONG_PTR area =
        static_cast<LONG_PTR>(bounds.right - bounds.left) *
        static_cast<LONG_PTR>(bounds.bottom - bounds.top);
    if (area > context->bestArea) {
        context->bestArea = area;
        context->bestTab = hwnd;
    }
    return TRUE;
}

HWND GetActiveShellTabHwnd(HWND explorerHwnd) {
    if (!explorerHwnd || !IsExplorerTopLevel(explorerHwnd)) {
        return nullptr;
    }

    HWND focus = GetThreadFocusWindow(explorerHwnd);
    if (focus && (focus == explorerHwnd || IsChild(explorerHwnd, focus))) {
        HWND tab = FindAncestorOrSelfByClass(focus, L"ShellTabWindowClass",
                                             explorerHwnd);
        if (tab) {
            return tab;
        }
    }

    ShellTabSearchContext context;
    EnumChildWindows(explorerHwnd, FindVisibleShellTab,
                     reinterpret_cast<LPARAM>(&context));
    return context.bestTab;
}

bool ViewMatchesTarget(IShellView* view,
                       HWND owner,
                       HWND targetDefView,
                       HWND activeTab) {
    if (!view) {
        return false;
    }

    HWND viewHwnd = nullptr;
    if (FAILED(view->GetWindow(&viewHwnd)) || !viewHwnd) {
        return false;
    }

    if (targetDefView) {
        if (viewHwnd != targetDefView && !IsChild(targetDefView, viewHwnd) &&
            !IsChild(viewHwnd, targetDefView)) {
            return false;
        }
    } else if (GetAncestor(viewHwnd, GA_ROOT) != owner) {
        return false;
    }

    if (activeTab) {
        HWND viewTab = FindAncestorOrSelfByClass(viewHwnd,
                                                 L"ShellTabWindowClass",
                                                 owner);
        if (viewTab != activeTab) {
            return false;
        }
    }

    return true;
}

bool ResolveActiveShellView(HWND owner,
                            HWND targetWindow,
                            IShellView** outputView) {
    if (!outputView) {
        return false;
    }
    *outputView = nullptr;

    HWND targetDefView = FindAncestorOrSelfByClass(targetWindow,
                                                   L"SHELLDLL_DefView");
    // An exact DefView is stronger evidence than the current focus. During a
    // click-away commit, focus may already have moved to another tab.
    HWND activeTab = (!targetDefView && IsExplorerTopLevel(owner))
                         ? GetActiveShellTabHwnd(owner)
                         : nullptr;

    IShellWindows* shellWindows = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                                  IID_PPV_ARGS(&shellWindows));
    if (FAILED(hr) || !shellWindows) {
        DebugLog(L"ResolveActiveShellView: CLSID_ShellWindows failed, hr=0x%08X",
                 hr);
        return false;
    }

    long count = 0;
    shellWindows->get_Count(&count);
    for (long index = count - 1; index >= 0; --index) {
        VARIANT itemIndex;
        VariantInit(&itemIndex);
        itemIndex.vt = VT_I4;
        itemIndex.lVal = index;

        IDispatch* dispatch = nullptr;
        if (FAILED(shellWindows->Item(itemIndex, &dispatch)) || !dispatch) {
            VariantClear(&itemIndex);
            continue;
        }

        IServiceProvider* serviceProvider = nullptr;
        hr = dispatch->QueryInterface(IID_PPV_ARGS(&serviceProvider));
        SafeRelease(dispatch);
        if (FAILED(hr) || !serviceProvider) {
            VariantClear(&itemIndex);
            continue;
        }

        IShellBrowser* browser = nullptr;
        hr = serviceProvider->QueryService(SID_STopLevelBrowser,
                                           IID_PPV_ARGS(&browser));
        SafeRelease(serviceProvider);
        if (FAILED(hr) || !browser) {
            VariantClear(&itemIndex);
            continue;
        }

        IShellView* view = nullptr;
        hr = browser->QueryActiveShellView(&view);
        SafeRelease(browser);
        if (SUCCEEDED(hr) && view &&
            ViewMatchesTarget(view, owner, targetDefView, activeTab)) {
            *outputView = view;
            shellWindows->Release();
            VariantClear(&itemIndex);
            return true;
        }

        SafeRelease(view);
        VariantClear(&itemIndex);
    }

    // The desktop Shell view isn't guaranteed to be returned by Item(), so use
    // the documented desktop lookup as an explicit fallback.
    if (IsDesktopTopLevel(owner)) {
        VARIANT emptyLocation;
        VARIANT emptyRoot;
        VariantInit(&emptyLocation);
        VariantInit(&emptyRoot);
        long desktopHwnd = 0;
        IDispatch* desktopDispatch = nullptr;
        hr = shellWindows->FindWindowSW(&emptyLocation, &emptyRoot, SWC_DESKTOP,
                                        &desktopHwnd, SWFO_NEEDDISPATCH,
                                        &desktopDispatch);
        if (SUCCEEDED(hr) && desktopDispatch) {
            IServiceProvider* serviceProvider = nullptr;
            hr = desktopDispatch->QueryInterface(IID_PPV_ARGS(&serviceProvider));
            SafeRelease(desktopDispatch);
            if (SUCCEEDED(hr) && serviceProvider) {
                IShellBrowser* browser = nullptr;
                hr = serviceProvider->QueryService(SID_STopLevelBrowser,
                                                   IID_PPV_ARGS(&browser));
                SafeRelease(serviceProvider);
                if (SUCCEEDED(hr) && browser) {
                    IShellView* view = nullptr;
                    hr = browser->QueryActiveShellView(&view);
                    SafeRelease(browser);
                    if (SUCCEEDED(hr) && view &&
                        ViewMatchesTarget(view, owner, targetDefView, nullptr)) {
                        *outputView = view;
                        shellWindows->Release();
                        return true;
                    }
                    SafeRelease(view);
                }
            }
        } else {
            SafeRelease(desktopDispatch);
        }
    }

    shellWindows->Release();
    return false;
}

bool GetFolderPathFromViewItem(HWND owner,
                               HWND targetWindow,
                               int itemIndex,
                               std::wstring& outputPath) {
    outputPath.clear();

    IShellView* shellView = nullptr;
    if (!ResolveActiveShellView(owner, targetWindow, &shellView)) {
        return false;
    }

    IFolderView2* folderView = nullptr;
    HRESULT hr = shellView->QueryInterface(IID_PPV_ARGS(&folderView));
    SafeRelease(shellView);
    if (FAILED(hr) || !folderView) {
        return false;
    }

    if (itemIndex < 0) {
        hr = folderView->GetFocusedItem(&itemIndex);
        if (FAILED(hr) || itemIndex < 0) {
            SafeRelease(folderView);
            return false;
        }
    }

    PITEMID_CHILD childPidl = nullptr;
    hr = folderView->Item(itemIndex, &childPidl);
    if (FAILED(hr) || !childPidl) {
        SafeRelease(folderView);
        return false;
    }

    IShellFolder* parentFolder = nullptr;
    hr = folderView->GetFolder(IID_PPV_ARGS(&parentFolder));
    SafeRelease(folderView);
    if (FAILED(hr) || !parentFolder) {
        CoTaskMemFree(childPidl);
        return false;
    }

    IPersistFolder2* persistFolder = nullptr;
    hr = parentFolder->QueryInterface(IID_PPV_ARGS(&persistFolder));
    if (FAILED(hr) || !persistFolder) {
        SafeRelease(parentFolder);
        CoTaskMemFree(childPidl);
        return false;
    }

    PIDLIST_ABSOLUTE parentPidl = nullptr;
    hr = persistFolder->GetCurFolder(&parentPidl);
    SafeRelease(persistFolder);
    if (FAILED(hr) || !parentPidl) {
        SafeRelease(parentFolder);
        CoTaskMemFree(childPidl);
        return false;
    }

    IShellItem* shellItem = nullptr;
    hr = SHCreateItemWithParent(parentPidl, parentFolder, childPidl,
                                IID_PPV_ARGS(&shellItem));
    CoTaskMemFree(parentPidl);
    SafeRelease(parentFolder);
    CoTaskMemFree(childPidl);
    if (FAILED(hr) || !shellItem) {
        return false;
    }

    SFGAOF attributes = 0;
    hr = shellItem->GetAttributes(SFGAO_FOLDER | SFGAO_FILESYSTEM, &attributes);
    if (FAILED(hr) || (attributes & SFGAO_FOLDER) == 0 ||
        (attributes & SFGAO_FILESYSTEM) == 0) {
        SafeRelease(shellItem);
        return false;
    }

    PWSTR fileSystemPath = nullptr;
    hr = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &fileSystemPath);
    SafeRelease(shellItem);
    if (FAILED(hr) || !fileSystemPath || !*fileSystemPath) {
        CoTaskMemFree(fileSystemPath);
        return false;
    }

    outputPath = fileSystemPath;
    CoTaskMemFree(fileSystemPath);

    DWORD attributesOnDisk = GetFileAttributesW(outputPath.c_str());
    return attributesOnDisk != INVALID_FILE_ATTRIBUTES &&
           (attributesOnDisk & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


void ShowError(HWND owner,
               const wchar_t* title,
               const std::wstring& message) {
    MessageBoxW(owner, message.c_str(), title,
                MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

bool PasteClipboardTextContainingSlash(HWND edit) {
    if (!OpenClipboard(edit)) {
        return false;
    }

    bool handled = false;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data) {
        const wchar_t* text = static_cast<const wchar_t*>(GlobalLock(data));
        if (text) {
            std::wstring clipboardText(text);
            if (clipboardText.find(L'/') != std::wstring::npos) {
                std::replace(clipboardText.begin(), clipboardText.end(), L'/',
                             kDisplayPathSeparator);
                SendMessageW(edit, EM_REPLACESEL, TRUE,
                             reinterpret_cast<LPARAM>(clipboardText.c_str()));
                handled = true;
            }
            GlobalUnlock(data);
        }
    }

    CloseClipboard();
    return handled;
}

struct DirectoryIdentity {
    bool valid = false;
    DWORD volumeSerialNumber = 0;
    DWORD fileIndexHigh = 0;
    DWORD fileIndexLow = 0;
};

DirectoryIdentity CaptureDirectoryIdentity(const std::wstring& path) {
    DirectoryIdentity identity;
    HANDLE directory = CreateFileW(
        path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (directory == INVALID_HANDLE_VALUE) {
        return identity;
    }

    BY_HANDLE_FILE_INFORMATION information = {};
    if (GetFileInformationByHandle(directory, &information)) {
        identity.valid = true;
        identity.volumeSerialNumber = information.dwVolumeSerialNumber;
        identity.fileIndexHigh = information.nFileIndexHigh;
        identity.fileIndexLow = information.nFileIndexLow;
    }
    CloseHandle(directory);
    return identity;
}

bool DirectoryMatchesIdentity(const std::wstring& path,
                              const DirectoryIdentity& expected) {
    DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return false;
    }
    if (!expected.valid) {
        return true;
    }

    const DirectoryIdentity actual = CaptureDirectoryIdentity(path);
    return actual.valid &&
           actual.volumeSerialNumber == expected.volumeSerialNumber &&
           actual.fileIndexHigh == expected.fileIndexHigh &&
           actual.fileIndexLow == expected.fileIndexLow;
}

std::wstring GetParentPath(std::wstring_view fullPath);
std::wstring GetLeafName(std::wstring_view fullPath);

struct RenameRequest {
    HWND owner = nullptr;
    std::wstring sourcePath;
    std::wstring literalSourceCandidate;
    DirectoryIdentity sourceIdentity;
    std::wstring expression;
};

bool SubmitRenameRequest(std::unique_ptr<RenameRequest> request) {
    const DWORD workerThreadId =
        g_workerThreadId.load(std::memory_order_acquire);
    if (!request || !g_acceptingRequests.load(std::memory_order_acquire) ||
        workerThreadId == 0) {
        return false;
    }

    RenameRequest* raw = request.release();
    if (!PostThreadMessageW(workerThreadId, kWorkerProcessRequest,
                            reinterpret_cast<WPARAM>(raw), 0)) {
        delete raw;
        return false;
    }
    return true;
}

std::wstring ResolveRequestSourcePath(const RenameRequest& request) {
    // A valid simple component may be committed by Explorer before our worker
    // runs. Give that native rename a short window to settle, then follow the
    // directory by its stable filesystem identity. Expressions containing '/'
    // cannot be committed literally, so the original path remains authoritative.
    if (!request.literalSourceCandidate.empty() &&
        request.sourceIdentity.valid) {
        for (int attempt = 0; attempt < 12; ++attempt) {
            if (DirectoryMatchesIdentity(request.literalSourceCandidate,
                                         request.sourceIdentity)) {
                DebugLog(L"Recovered source after Explorer literal rename: '%ls'",
                         request.literalSourceCandidate.c_str());
                return request.literalSourceCandidate;
            }
            Sleep(25);
        }
    }

    if (DirectoryMatchesIdentity(request.sourcePath, request.sourceIdentity)) {
        return request.sourcePath;
    }

    if (!request.literalSourceCandidate.empty() &&
        DirectoryMatchesIdentity(request.literalSourceCandidate,
                                 request.sourceIdentity)) {
        DebugLog(L"Recovered source after Explorer literal rename: '%ls'",
                 request.literalSourceCandidate.c_str());
        return request.literalSourceCandidate;
    }

    const std::wstring parent = GetParentPath(request.sourcePath);
    if (parent.empty() || !request.sourceIdentity.valid) {
        return {};
    }

    std::wstring pattern = eef::JoinPath(parent, L"*");
    WIN32_FIND_DATAW findData = {};
    HANDLE find = FindFirstFileW(pattern.c_str(), &findData);
    if (find == INVALID_HANDLE_VALUE) {
        return {};
    }

    std::wstring recovered;
    do {
        if (wcscmp(findData.cFileName, L".") == 0 ||
            wcscmp(findData.cFileName, L"..") == 0 ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            continue;
        }
        std::wstring candidate = eef::JoinPath(parent, findData.cFileName);
        if (DirectoryMatchesIdentity(candidate, request.sourceIdentity)) {
            recovered = std::move(candidate);
            break;
        }
    } while (FindNextFileW(find, &findData));
    FindClose(find);

    if (!recovered.empty()) {
        DebugLog(L"Recovered renamed source by filesystem identity: '%ls'",
                 recovered.c_str());
    }
    return recovered;
}

struct RenameEditSession {
    HWND edit = nullptr;
    HWND shellView = nullptr;
    HWND owner = nullptr;
    DWORD uiThreadId = 0;
    HHOOK messageHook = nullptr;
    std::wstring sourcePath;
    std::wstring originalName;
    std::wstring lastText;
    DirectoryIdentity sourceIdentity;
    bool commitRequested = false;
    bool cancelled = false;
    std::atomic<bool> externalTeardown{false};
};

void CaptureEditText(RenameEditSession* session) {
    if (!session || !session->edit || !IsWindow(session->edit)) {
        return;
    }
    const int length = GetWindowTextLengthW(session->edit);
    if (length < 0) {
        return;
    }
    std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
    const int copied = GetWindowTextW(session->edit, text.data(), length + 1);
    if (copied >= 0) {
        text.resize(static_cast<std::size_t>(copied));
        session->lastText = std::move(text);
    }
}

RenameEditSession* FindSessionForCurrentThread() {
    const DWORD threadId = GetCurrentThreadId();
    std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
    for (const auto& [window, session] : g_editSessions) {
        (void)window;
        if (session && session->uiThreadId == threadId) {
            return session;
        }
    }
    return nullptr;
}

LRESULT CALLBACK RenameMessageHookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code >= 0 && wParam == PM_REMOVE && lParam &&
        !g_unloading.load(std::memory_order_relaxed)) {
        const MSG* message = reinterpret_cast<const MSG*>(lParam);
        if (message->message == WM_KEYDOWN ||
            message->message == WM_SYSKEYDOWN) {
            RenameEditSession* session = FindSessionForCurrentThread();
            if (session) {
                if (message->wParam == VK_ESCAPE) {
                    session->cancelled = true;
                    DebugLog(L"Rename editor received Escape, hwnd=%p",
                             session->edit);
                } else if (message->wParam == VK_RETURN) {
                    session->commitRequested = true;
                    CaptureEditText(session);
                    DebugLog(L"Rename editor commit observed in UI queue, hwnd=%p text='%ls'",
                             session->edit, session->lastText.c_str());
                }
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

void DetachRenameEditSession(HWND hwnd, RenameEditSession* session) {
    {
        std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
        auto found = g_editSessions.find(hwnd);
        if (found != g_editSessions.end() && found->second == session) {
            g_editSessions.erase(found);
        }
    }
    if (session && session->messageHook) {
        UnhookWindowsHookEx(session->messageHook);
        session->messageHook = nullptr;
    }
}

void SubmitCapturedRename(RenameEditSession& session,
                          std::wstring expression) {
    const std::wstring nativeExpression = expression;
    expression = eef::NormalizeExpressionSeparators(expression);

    const SettingsSnapshot settings = GetSettingsSnapshot();
    DebugLog(L"Rename editor finalized, commit=%d cancel=%d text='%ls' source='%ls'",
             session.commitRequested ? 1 : 0,
             session.cancelled ? 1 : 0,
             expression.c_str(), session.sourcePath.c_str());

    if (!session.commitRequested || session.cancelled || expression.empty() ||
        expression == session.originalName) {
        return;
    }

    if (expression.size() > settings.maximumExpressionLength) {
        ShowError(session.owner, L"Froggo Explorer Emmet Folders",
                  L"The expression exceeds the configured length limit.");
        return;
    }

    const eef::Limits limits = ToCoreLimits(settings);
    eef::ParseResult parsed = eef::ParseExpression(expression, limits);
    if (parsed.disposition == eef::ParseDisposition::NativeFallback) {
        DebugLog(L"Finalized text is a native folder name; no expansion needed.");
        return;
    }
    if (parsed.disposition == eef::ParseDisposition::SyntaxError) {
        DebugLog(L"Finalized expression rejected: %ls", parsed.error.c_str());
        ShowError(session.owner, L"Froggo Explorer Emmet Folders", parsed.error);
        return;
    }

    eef::ExpansionResult expanded =
        eef::ExpandExpression(*parsed.expression, limits);
    if (!expanded.ok) {
        DebugLog(L"Finalized expansion rejected: %ls", expanded.error.c_str());
        ShowError(session.owner, L"Froggo Explorer Emmet Folders", expanded.error);
        return;
    }

    auto request = std::make_unique<RenameRequest>();
    request->owner = session.owner;
    request->sourcePath = session.sourcePath;
    request->sourceIdentity = session.sourceIdentity;
    request->expression = std::move(expression);

    const eef::NameValidationResult literalName =
        eef::ValidateFolderName(nativeExpression);
    const std::wstring parent = GetParentPath(request->sourcePath);
    if (literalName.ok && !parent.empty()) {
        request->literalSourceCandidate =
            eef::JoinPath(parent, literalName.normalized);
    }

    DebugLog(L"Captured expression '%ls' for original source '%ls'",
             request->expression.c_str(), request->sourcePath.c_str());
    if (!SubmitRenameRequest(std::move(request))) {
        ShowError(session.owner, L"Froggo Explorer Emmet Folders",
                  L"The background worker is unavailable. The folder expansion was not started.");
    }
}

LRESULT CALLBACK RenameEditSubclassProc(HWND hwnd,
                                        UINT message,
                                        WPARAM wParam,
                                        LPARAM lParam,
                                        DWORD_PTR referenceData) {
    auto* session = reinterpret_cast<RenameEditSession*>(referenceData);

    if (message == WM_NCDESTROY) {
        const bool externallyOwned =
            session && session->externalTeardown.load(std::memory_order_acquire);
        CaptureEditText(session);
        std::wstring finalText = session ? session->lastText : std::wstring();
        DetachRenameEditSession(hwnd, session);
        const LRESULT result = DefSubclassProc(hwnd, message, wParam, lParam);
        if (session && !externallyOwned &&
            !g_unloading.load(std::memory_order_relaxed)) {
            SubmitCapturedRename(*session, std::move(finalText));
        }
        if (!externallyOwned) {
            delete session;
        }
        return result;
    }

    if (g_unloading.load(std::memory_order_relaxed) || !session) {
        return DefSubclassProc(hwnd, message, wParam, lParam);
    }

    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
        if (wParam == VK_ESCAPE) {
            session->cancelled = true;
        } else if (wParam == VK_RETURN) {
            session->commitRequested = true;
        }
    } else if (message == WM_KILLFOCUS) {
        CaptureEditText(session);
        session->commitRequested = true;
        DebugLog(L"Rename editor lost focus, hwnd=%p text='%ls'",
                 hwnd, session->lastText.c_str());
    }

    // A literal slash is illegal in a Windows file name. Let the native Edit
    // control insert a visually equivalent Unicode separator instead of
    // re-entering it with EM_REPLACESEL. The parser canonicalizes it back to
    // '/' when the rename session finishes.
    if (message == WM_CHAR && wParam == L'/') {
        wParam = kDisplayPathSeparator;
    }

    if (message == WM_PASTE && PasteClipboardTextContainingSlash(hwnd)) {
        return 0;
    }

    return DefSubclassProc(hwnd, message, wParam, lParam);
}

bool InstallRenameEditSubclass(HWND edit,
                               HWND shellView,
                               HWND owner,
                               std::wstring sourcePath) {
    {
        std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
        if (g_editSessions.contains(edit)) {
            return true;
        }
    }

    auto session = std::make_unique<RenameEditSession>();
    session->edit = edit;
    session->shellView = shellView;
    session->owner = owner;
    session->uiThreadId = GetWindowThreadProcessId(edit, nullptr);
    session->sourcePath = std::move(sourcePath);
    session->originalName = GetLeafName(session->sourcePath);
    session->lastText = session->originalName;
    session->sourceIdentity = CaptureDirectoryIdentity(session->sourcePath);

    session->messageHook = SetWindowsHookExW(
        WH_GETMESSAGE, RenameMessageHookProc, HINST_THISCOMPONENT,
        session->uiThreadId);
    if (!session->messageHook) {
        DebugLog(L"Failed to install rename UI message hook, thread=%u error=%u",
                 session->uiThreadId, GetLastError());
    }

    RenameEditSession* raw = session.release();
    {
        std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
        if (!g_editSessions.emplace(edit, raw).second) {
            if (raw->messageHook) {
                UnhookWindowsHookEx(raw->messageHook);
            }
            delete raw;
            return true;
        }
    }

    const BOOL installed = WindhawkUtils::SetWindowSubclassFromAnyThread(
        edit, RenameEditSubclassProc,
        reinterpret_cast<DWORD_PTR>(raw));

    std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
    auto found = g_editSessions.find(edit);
    if (found == g_editSessions.end() || found->second != raw) {
        // The editor was destroyed while the cross-thread install was in
        // flight. WM_NCDESTROY already detached and released the session.
        return installed != FALSE;
    }

    if (!installed) {
        g_editSessions.erase(found);
        if (raw->messageHook) {
            UnhookWindowsHookEx(raw->messageHook);
            raw->messageHook = nullptr;
        }
        DebugLog(L"Failed to subclass Explorer rename edit, hwnd=%p error=%u",
                 edit, GetLastError());
        delete raw;
        return false;
    }

    DebugLog(L"Subclassed Explorer rename edit, hwnd=%p source='%ls' identity=%d",
             edit, raw->sourcePath.c_str(), raw->sourceIdentity.valid ? 1 : 0);
    return true;
}

void TrySubclassRenameEdit(HWND candidate) {
    HWND edit = FindRenameEditAncestor(candidate);
    if (!edit) {
        return;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(edit, &processId);
    if (processId != GetCurrentProcessId()) {
        return;
    }

    // WinEvent focus notifications can fire repeatedly while the user types.
    // Bail before any Shell/COM work once this editor already owns a session.
    {
        std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
        if (g_editSessions.contains(edit)) {
            return;
        }
    }

    HWND shellView = FindAncestorOrSelfByClass(edit, L"SHELLDLL_DefView");
    if (!shellView) {
        return;
    }
    HWND owner = GetAncestor(shellView, GA_ROOT);
    if (!owner) {
        return;
    }

    HWND focused = GetThreadFocusWindow(owner);
    if (focused != edit && (!focused || !IsChild(edit, focused))) {
        // CREATE/SHOW can arrive before Explorer transfers focus and before the
        // focused Shell item is updated. EVENT_OBJECT_FOCUS will retry later.
        return;
    }

    std::wstring sourcePath;
    if (!GetFolderPathFromViewItem(owner, shellView, -1, sourcePath)) {
        DebugLog(L"Rename edit focused but source item could not be resolved, edit=%p shellView=%p",
                 edit, shellView);
        return;
    }

    const int editLength = GetWindowTextLengthW(edit);
    std::wstring editText(static_cast<std::size_t>((std::max)(editLength, 0)) + 1,
                          L'\0');
    const int copied = GetWindowTextW(edit, editText.data(), editLength + 1);
    editText.resize(static_cast<std::size_t>((std::max)(copied, 0)));
    const std::wstring sourceName = GetLeafName(sourcePath);
    if (!editText.empty() && _wcsicmp(editText.c_str(), sourceName.c_str()) != 0) {
        // A delayed out-of-context callback can arrive after the first
        // character was typed. The focused Shell item is still authoritative;
        // don't discard the session merely because the edit text has changed.
        DebugLog(L"Rename edit text already changed during discovery, editText='%ls' source='%ls'",
                 editText.c_str(), sourcePath.c_str());
    }

    InstallRenameEditSubclass(edit, shellView, owner, std::move(sourcePath));
}

void CALLBACK RenameEditWinEventProc(HWINEVENTHOOK,
                                     DWORD event,
                                     HWND hwnd,
                                     LONG idObject,
                                     LONG,
                                     DWORD,
                                     DWORD) {
    if (g_unloading.load(std::memory_order_acquire) ||
        g_winEventStopping.load(std::memory_order_acquire) || !hwnd) {
        return;
    }
    if (event != EVENT_OBJECT_CREATE && event != EVENT_OBJECT_SHOW &&
        event != EVENT_OBJECT_FOCUS) {
        return;
    }
    if (idObject != OBJID_WINDOW && idObject != OBJID_CLIENT &&
        idObject != OBJID_CARET) {
        return;
    }
    TrySubclassRenameEdit(hwnd);
}

DWORD WINAPI WinEventHookThread(void*) {
    const HRESULT comResult =
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    MSG message = {};
    PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    HWINEVENTHOOK eventHook = SetWinEventHook(
        EVENT_OBJECT_CREATE, EVENT_OBJECT_FOCUS, nullptr,
        RenameEditWinEventProc, GetCurrentProcessId(), 0,
        WINEVENT_OUTOFCONTEXT);
    g_renameEditEventHook.store(eventHook, std::memory_order_release);

    if (g_winEventReadyEvent) {
        SetEvent(g_winEventReadyEvent);
    }

    if (!eventHook) {
        Wh_Log(L"SetWinEventHook failed, error=%u", GetLastError());
        if (SUCCEEDED(comResult)) {
            CoUninitialize();
        }
        return 1;
    }

    bool quit = false;
    while (!quit) {
        const DWORD waitResult = MsgWaitForMultipleObjectsEx(
            1, &g_winEventStopEvent, INFINITE, QS_ALLINPUT,
            MWMO_INPUTAVAILABLE);
        if (waitResult == WAIT_OBJECT_0) {
            break;
        }
        if (waitResult != WAIT_OBJECT_0 + 1) {
            Wh_Log(L"WinEvent helper wait failed, result=%u error=%u",
                   waitResult, GetLastError());
            break;
        }

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                quit = true;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    UnhookWinEvent(eventHook);
    g_renameEditEventHook.store(nullptr, std::memory_order_release);
    if (SUCCEEDED(comResult)) {
        CoUninitialize();
    }
    return 0;
}

bool StartWinEventMonitor() {
    g_winEventStopping.store(false, std::memory_order_release);
    g_winEventReadyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_winEventStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_winEventReadyEvent || !g_winEventStopEvent) {
        if (g_winEventReadyEvent) {
            CloseHandle(g_winEventReadyEvent);
            g_winEventReadyEvent = nullptr;
        }
        if (g_winEventStopEvent) {
            CloseHandle(g_winEventStopEvent);
            g_winEventStopEvent = nullptr;
        }
        return false;
    }

    g_winEventThread = CreateThread(nullptr, 0, WinEventHookThread, nullptr,
                                    0, nullptr);
    if (!g_winEventThread) {
        CloseHandle(g_winEventReadyEvent);
        CloseHandle(g_winEventStopEvent);
        g_winEventReadyEvent = nullptr;
        g_winEventStopEvent = nullptr;
        return false;
    }

    HANDLE handles[] = {g_winEventReadyEvent, g_winEventThread};
    const DWORD waitResult = WaitForMultipleObjects(
        static_cast<DWORD>(std::size(handles)), handles, FALSE, 5000);
    if (waitResult != WAIT_OBJECT_0 ||
        !g_renameEditEventHook.load(std::memory_order_acquire)) {
        g_winEventStopping.store(true, std::memory_order_release);
        SetEvent(g_winEventStopEvent);
        WaitForSingleObject(g_winEventThread, 5000);
        CloseHandle(g_winEventThread);
        g_winEventThread = nullptr;
        CloseHandle(g_winEventReadyEvent);
        CloseHandle(g_winEventStopEvent);
        g_winEventReadyEvent = nullptr;
        g_winEventStopEvent = nullptr;
        return false;
    }

    CloseHandle(g_winEventReadyEvent);
    g_winEventReadyEvent = nullptr;
    DebugLog(L"WinEvent helper ready, thread=%u",
             GetThreadId(g_winEventThread));
    return true;
}

void StopWinEventMonitor() {
    g_winEventStopping.store(true, std::memory_order_release);
    if (g_winEventStopEvent) {
        SetEvent(g_winEventStopEvent);
    }

    bool helperExited = true;
    if (g_winEventThread) {
        const DWORD waitResult = WaitForSingleObject(g_winEventThread, 10000);
        if (waitResult == WAIT_OBJECT_0) {
            CloseHandle(g_winEventThread);
        } else {
            helperExited = false;
            Wh_Log(L"Warning: WinEvent helper did not exit within 10 seconds; preserving its wait handles.");
        }
        g_winEventThread = nullptr;
    }
    if (g_winEventReadyEvent) {
        CloseHandle(g_winEventReadyEvent);
        g_winEventReadyEvent = nullptr;
    }
    if (g_winEventStopEvent && helperExited) {
        CloseHandle(g_winEventStopEvent);
        g_winEventStopEvent = nullptr;
    }
}

class WindowsPathProbe final : public eef::IPathProbe {
public:
    eef::PathKind Probe(std::wstring_view path) const override {
        std::wstring owned(path);
        DWORD attributes = GetFileAttributesW(owned.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ||
                error == ERROR_INVALID_NAME) {
                return eef::PathKind::Missing;
            }
            return eef::PathKind::Other;
        }
        return (attributes & FILE_ATTRIBUTE_DIRECTORY)
                   ? eef::PathKind::Directory
                   : eef::PathKind::Other;
    }
};

std::wstring GetParentPath(std::wstring_view fullPath) {
    std::wstring result(fullPath);
    while (result.size() > 3 &&
           (result.back() == L'\\' || result.back() == L'/')) {
        result.pop_back();
    }
    const std::size_t separator = result.find_last_of(L"\\/");
    if (separator == std::wstring::npos) {
        return {};
    }
    if (separator == 2 && result.size() >= 3 && result[1] == L':') {
        return result.substr(0, 3);
    }
    return result.substr(0, separator);
}

std::wstring GetLeafName(std::wstring_view fullPath) {
    while (fullPath.size() > 1 &&
           (fullPath.back() == L'\\' || fullPath.back() == L'/')) {
        fullPath.remove_suffix(1);
    }
    const std::size_t separator = fullPath.find_last_of(L"\\/");
    return std::wstring(separator == std::wstring_view::npos
                            ? fullPath
                            : fullPath.substr(separator + 1));
}

bool DirectoryContainsData(const std::wstring& directory) {
    std::wstring pattern = eef::JoinPath(directory, L"*");
    WIN32_FIND_DATAW findData = {};
    HANDLE find = FindFirstFileW(pattern.c_str(), &findData);
    if (find == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool hasData = false;
    do {
        if (wcscmp(findData.cFileName, L".") != 0 &&
            wcscmp(findData.cFileName, L"..") != 0) {
            hasData = true;
            break;
        }
    } while (FindNextFileW(find, &findData));

    FindClose(find);
    return hasData;
}

std::wstring FormatPreview(const eef::ExecutionPlan& plan,
                           bool sourceContainsData) {
    std::wostringstream stream;
    stream << L"Directories to create or rename: " << plan.affectedCount
           << L"\nExisting directories to reuse: " << plan.skippedCount
           << L"\n\n";

    if (!plan.sourceStaysInPlace) {
        stream << L"Current folder:\n  " << plan.sourcePath << L"\nBecomes:\n  "
               << plan.sourceTargetPath << L"\n";
        if (sourceContainsData) {
            stream << L"Its existing contents will move with it unchanged.\n";
        }
        stream << L"\n";
    }

    stream << L"Preview:\n";
    const std::size_t shown =
        std::min(plan.directories.size(), kPreviewEntryLimit);
    for (std::size_t index = 0; index < shown; ++index) {
        const auto& directory = plan.directories[index];
        const std::size_t indentation = directory.depth > 0
                                            ? directory.depth - 1
                                            : 0;
        stream << std::wstring(indentation * 2, L' ') << L"• "
               << GetLeafName(directory.absolutePath);
        if (directory.fulfilledBySource) {
            stream << L"  [current folder]";
        } else if (directory.exists) {
            stream << L"  [existing]";
        }
        stream << L"\n";
    }

    if (plan.directories.size() > shown) {
        stream << L"… " << (plan.directories.size() - shown)
               << L" additional entries omitted\n";
    }

    return stream.str();
}

bool ConfirmExecution(HWND owner,
                      const eef::ExecutionPlan& plan,
                      bool sourceContainsData) {
    std::wstring content = FormatPreview(plan, sourceContainsData);
    TASKDIALOG_BUTTON buttons[] = {
        {kPreviewCreateButton, L"Create"},
        {IDCANCEL, L"Cancel"},
    };

    TASKDIALOGCONFIG config = {};
    config.cbSize = sizeof(config);
    config.hwndParent = owner;
    config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT;
    config.pszWindowTitle = L"Froggo Explorer Emmet Folders";
    config.pszMainIcon = TD_INFORMATION_ICON;
    config.pszMainInstruction = L"Create this folder structure?";
    config.pszContent = content.c_str();
    config.cButtons = static_cast<UINT>(std::size(buttons));
    config.pButtons = buttons;
    config.nDefaultButton = kPreviewCreateButton;

    int pressedButton = IDCANCEL;
    HRESULT hr = TaskDialogIndirect(&config, &pressedButton, nullptr, nullptr);
    return SUCCEEDED(hr) && pressedButton == kPreviewCreateButton;
}

struct OperationResult {
    bool ok = false;
    HRESULT hresult = E_FAIL;
    DWORD win32Error = ERROR_SUCCESS;
    bool aborted = false;
};

DWORD GetFileOperationFlags() {
    return FOF_NOCONFIRMMKDIR | FOFX_SHOWELEVATIONPROMPT |
           FOFX_NOCOPYHOOKS | FOFX_EARLYFAILURE;
}

OperationResult RenameDirectoryWithShell(const std::wstring& sourcePath,
                                         const std::wstring& targetPath,
                                         HWND owner) {
    OperationResult result;

    if (GetFileAttributesW(targetPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        result.hresult = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        result.win32Error = ERROR_ALREADY_EXISTS;
        return result;
    }

    IShellItem* sourceItem = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(sourcePath.c_str(), nullptr,
                                             IID_PPV_ARGS(&sourceItem));
    if (FAILED(hr) || !sourceItem) {
        result.hresult = hr;
        return result;
    }

    IFileOperation* fileOperation = nullptr;
    hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL,
                          IID_PPV_ARGS(&fileOperation));
    if (SUCCEEDED(hr) && fileOperation) {
        if (owner && IsWindow(owner)) {
            fileOperation->SetOwnerWindow(owner);
        }
        fileOperation->SetOperationFlags(GetFileOperationFlags());
        std::wstring targetName = GetLeafName(targetPath);
        hr = fileOperation->RenameItem(sourceItem, targetName.c_str(), nullptr);
        if (SUCCEEDED(hr)) {
            hr = fileOperation->PerformOperations();
        }
        BOOL aborted = FALSE;
        fileOperation->GetAnyOperationsAborted(&aborted);
        result.aborted = aborted != FALSE;
        SafeRelease(fileOperation);
    }
    SafeRelease(sourceItem);

    if (SUCCEEDED(hr) && !result.aborted) {
        DWORD attributes = GetFileAttributesW(targetPath.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            result.ok = true;
            result.hresult = S_OK;
            return result;
        }
    }

    if (!result.aborted &&
        MoveFileExW(sourcePath.c_str(), targetPath.c_str(),
                    MOVEFILE_WRITE_THROUGH)) {
        SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH,
                       sourcePath.c_str(), targetPath.c_str());
        result.ok = true;
        result.hresult = S_OK;
        return result;
    }

    if (result.aborted) {
        result.win32Error = ERROR_CANCELLED;
        result.hresult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    } else {
        result.win32Error = GetLastError();
        result.hresult = FAILED(hr) ? hr : HRESULT_FROM_WIN32(result.win32Error);
    }
    return result;
}

OperationResult CreateDirectoryWithShell(const std::wstring& fullPath,
                                         HWND owner) {
    OperationResult result;
    DWORD existingAttributes = GetFileAttributesW(fullPath.c_str());
    if (existingAttributes != INVALID_FILE_ATTRIBUTES) {
        if (existingAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            result.ok = true;
            result.hresult = S_OK;
        } else {
            result.hresult = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
            result.win32Error = ERROR_ALREADY_EXISTS;
        }
        return result;
    }

    std::wstring parentPath = GetParentPath(fullPath);
    std::wstring leafName = GetLeafName(fullPath);
    if (parentPath.empty() || leafName.empty()) {
        result.hresult = E_INVALIDARG;
        return result;
    }

    IShellItem* parentItem = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(parentPath.c_str(), nullptr,
                                             IID_PPV_ARGS(&parentItem));
    IFileOperation* fileOperation = nullptr;
    if (SUCCEEDED(hr) && parentItem) {
        hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL,
                              IID_PPV_ARGS(&fileOperation));
    }

    if (SUCCEEDED(hr) && fileOperation) {
        if (owner && IsWindow(owner)) {
            fileOperation->SetOwnerWindow(owner);
        }
        fileOperation->SetOperationFlags(GetFileOperationFlags());
        hr = fileOperation->NewItem(parentItem, FILE_ATTRIBUTE_DIRECTORY,
                                    leafName.c_str(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
            hr = fileOperation->PerformOperations();
        }
        BOOL aborted = FALSE;
        fileOperation->GetAnyOperationsAborted(&aborted);
        result.aborted = aborted != FALSE;
    }

    SafeRelease(fileOperation);
    SafeRelease(parentItem);

    if (SUCCEEDED(hr) && !result.aborted) {
        DWORD attributes = GetFileAttributesW(fullPath.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            result.ok = true;
            result.hresult = S_OK;
            return result;
        }
    }

    if (!result.aborted && CreateDirectoryW(fullPath.c_str(), nullptr)) {
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, fullPath.c_str(), nullptr);
        result.ok = true;
        result.hresult = S_OK;
        return result;
    }

    result.win32Error = GetLastError();
    if (result.win32Error == ERROR_ALREADY_EXISTS) {
        DWORD attributes = GetFileAttributesW(fullPath.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            result.ok = true;
            result.hresult = S_OK;
            return result;
        }
    }

    if (result.aborted) {
        result.win32Error = ERROR_CANCELLED;
        result.hresult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    } else {
        result.hresult = FAILED(hr) ? hr : HRESULT_FROM_WIN32(result.win32Error);
    }
    return result;
}

std::wstring FormatOperationFailure(const std::wstring& path,
                                    const OperationResult& result,
                                    std::size_t completedOperations) {
    wchar_t code[64] = {};
    swprintf_s(code, std::size(code), L"0x%08X",
               static_cast<unsigned>(result.hresult));

    std::wostringstream stream;
    stream << L"The operation stopped before all folders were created.\n\n"
           << L"Failed path:\n" << path << L"\n\n"
           << L"Error: " << code;
    if (result.win32Error != ERROR_SUCCESS) {
        stream << L" (Win32 " << result.win32Error << L")";
    }
    if (result.aborted) {
        stream << L"\nThe Shell operation was cancelled.";
    }
    stream << L"\nCompleted operations: " << completedOperations
           << L"\n\nAlready completed folders were left intact; nothing was deleted.";
    return stream.str();
}

void ProcessRenameRequest(const RenameRequest& request) {
    bool expected = false;
    if (!g_operationRunning.compare_exchange_strong(expected, true)) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                  L"Another folder expansion is already running. The original folder was not changed.");
        return;
    }

    struct RunningReset {
        ~RunningReset() {
            g_operationRunning.store(false, std::memory_order_release);
        }
    } runningReset;

    Sleep(30);

    SettingsSnapshot settings = GetSettingsSnapshot();
    eef::Limits limits = ToCoreLimits(settings);
    eef::ParseResult parsed = eef::ParseExpression(request.expression, limits);
    if (parsed.disposition != eef::ParseDisposition::Expand ||
        !parsed.expression) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                  parsed.error.empty() ? L"The expression is no longer valid."
                                       : parsed.error);
        return;
    }

    eef::ExpansionResult expansion =
        eef::ExpandExpression(*parsed.expression, limits);
    if (!expansion.ok) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders", expansion.error);
        return;
    }

    std::wstring sourcePath = ResolveRequestSourcePath(request);
    if (sourcePath.empty()) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                  L"The folder being renamed could not be found after Explorer closed the rename editor.");
        return;
    }

    std::wstring parentPath = GetParentPath(sourcePath);
    if (parentPath.empty()) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                  L"The parent directory could not be resolved.");
        return;
    }

    WindowsPathProbe probe;
    eef::ExecutionPlan plan = eef::BuildExecutionPlan(
        parentPath, sourcePath, expansion, probe);
    if (!plan.ok) {
        ShowError(request.owner, L"Froggo Explorer Emmet Folders", plan.error);
        return;
    }

    DebugLog(L"Plan source='%ls' target='%ls' affected=%zu skipped=%zu",
             plan.sourcePath.c_str(), plan.sourceTargetPath.c_str(),
             plan.affectedCount, plan.skippedCount);

    const bool sourceContainsData = DirectoryContainsData(sourcePath);
    if (plan.affectedCount >
            static_cast<std::size_t>(settings.previewThreshold) &&
        !ConfirmExecution(request.owner, plan, sourceContainsData)) {
        DebugLog(L"User cancelled preview for '%ls'", request.expression.c_str());
        return;
    }

    std::size_t completedOperations = 0;
    if (!plan.sourceStaysInPlace) {
        OperationResult renameResult = RenameDirectoryWithShell(
            plan.sourcePath, plan.sourceTargetPath, request.owner);
        if (!renameResult.ok) {
            ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                      FormatOperationFailure(plan.sourceTargetPath,
                                             renameResult,
                                             completedOperations));
            return;
        }
        ++completedOperations;
    }

    for (const auto& directory : plan.directories) {
        if (directory.exists || directory.fulfilledBySource) {
            continue;
        }

        OperationResult createResult =
            CreateDirectoryWithShell(directory.absolutePath, request.owner);
        if (!createResult.ok) {
            SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH,
                           parentPath.c_str(), nullptr);
            ShowError(request.owner, L"Froggo Explorer Emmet Folders",
                      FormatOperationFailure(directory.absolutePath,
                                             createResult,
                                             completedOperations));
            return;
        }
        ++completedOperations;
    }

    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, parentPath.c_str(), nullptr);

    if (settings.showSuccessNotification) {
        std::wostringstream message;
        message << L"Folder structure created successfully.\n\n"
                << L"Completed operations: " << completedOperations
                << L"\nExisting directories reused: " << plan.skippedCount;
        MessageBoxW(request.owner, message.str().c_str(),
                    L"Froggo Explorer Emmet Folders",
                    MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
    }
}

DWORD WINAPI WorkerThreadProc(void*) {
    const HRESULT comResult =
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    MSG message = {};
    PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    g_workerThreadId.store(GetCurrentThreadId(), std::memory_order_release);
    const bool comInitialized = SUCCEEDED(comResult);
    g_workerComInitialized.store(comInitialized, std::memory_order_release);
    g_acceptingRequests.store(comInitialized, std::memory_order_release);
    if (g_workerReadyEvent) {
        SetEvent(g_workerReadyEvent);
    }

    if (!comInitialized) {
        g_workerThreadId.store(0, std::memory_order_release);
        return 0;
    }

    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (message.hwnd == nullptr && message.message == kWorkerShutdown) {
            break;
        }
        if (message.hwnd == nullptr && message.message == kWorkerProcessRequest) {
            std::unique_ptr<RenameRequest> request(
                reinterpret_cast<RenameRequest*>(message.wParam));
            if (!g_unloading.load(std::memory_order_relaxed) && request) {
                ProcessRenameRequest(*request);
            }
            continue;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    g_acceptingRequests.store(false, std::memory_order_release);

    while (PeekMessageW(&message, nullptr, kWorkerProcessRequest,
                        kWorkerProcessRequest, PM_REMOVE)) {
        delete reinterpret_cast<RenameRequest*>(message.wParam);
    }

    g_workerThreadId.store(0, std::memory_order_release);
    g_workerComInitialized.store(false, std::memory_order_release);
    CoUninitialize();
    return 0;
}

bool StartWorker() {
    if (g_workerThread) {
        return true;
    }

    g_workerReadyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_workerReadyEvent) {
        return false;
    }

    g_workerThread = CreateThread(nullptr, 0, WorkerThreadProc, nullptr, 0,
                                  nullptr);
    if (!g_workerThread) {
        CloseHandle(g_workerReadyEvent);
        g_workerReadyEvent = nullptr;
        return false;
    }

    HANDLE waitHandles[] = {g_workerReadyEvent, g_workerThread};
    const DWORD waitResult =
        WaitForMultipleObjects(static_cast<DWORD>(std::size(waitHandles)),
                               waitHandles, FALSE, INFINITE);
    CloseHandle(g_workerReadyEvent);
    g_workerReadyEvent = nullptr;

    const DWORD workerThreadId =
        g_workerThreadId.load(std::memory_order_acquire);
    if (waitResult != WAIT_OBJECT_0 || workerThreadId == 0 ||
        !g_workerComInitialized.load(std::memory_order_acquire)) {
        if (workerThreadId != 0) {
            PostThreadMessageW(workerThreadId, kWorkerShutdown, 0, 0);
        }
        WaitForSingleObject(g_workerThread, INFINITE);
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
        g_workerThreadId.store(0, std::memory_order_release);
        g_workerComInitialized.store(false, std::memory_order_release);
        return false;
    }
    return true;
}

void StopWorker() {
    g_acceptingRequests.store(false, std::memory_order_release);
    HANDLE thread = g_workerThread;
    if (!thread) {
        return;
    }

    const DWORD workerThreadId =
        g_workerThreadId.load(std::memory_order_acquire);
    if (workerThreadId != 0) {
        PostThreadMessageW(workerThreadId, kWorkerShutdown, 0, 0);
    }
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    g_workerThread = nullptr;
    g_workerThreadId.store(0, std::memory_order_release);
    g_workerComInitialized.store(false, std::memory_order_release);
}

void RemoveAllRenameSubclasses() {
    std::vector<std::pair<HWND, RenameEditSession*>> sessions;
    {
        std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
        sessions.reserve(g_editSessions.size());
        for (const auto& [edit, session] : g_editSessions) {
            if (session) {
                session->externalTeardown.store(true,
                                                std::memory_order_release);
            }
            sessions.emplace_back(edit, session);
        }
    }

    for (const auto& [edit, session] : sessions) {
        if (session && session->messageHook) {
            UnhookWindowsHookEx(session->messageHook);
            session->messageHook = nullptr;
        }
        if (IsWindow(edit)) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(
                edit, RenameEditSubclassProc);
        }

        {
            std::lock_guard<std::mutex> lock(g_subclassedEditsMutex);
            auto found = g_editSessions.find(edit);
            if (found != g_editSessions.end() && found->second == session) {
                g_editSessions.erase(found);
            }
        }
        delete session;
    }
}

}  // namespace

BOOL Wh_ModInit() {
    LoadSettings();
    g_unloading.store(false, std::memory_order_relaxed);

    INITCOMMONCONTROLSEX commonControls = {};
    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_STANDARD_CLASSES;
    if (!InitCommonControlsEx(&commonControls)) {
        Wh_Log(L"InitCommonControlsEx failed, error=%u", GetLastError());
        return FALSE;
    }

    if (!StartWorker()) {
        Wh_Log(L"Failed to start the STA worker thread.");
        return FALSE;
    }

    if (!StartWinEventMonitor()) {
        Wh_Log(L"Failed to start the out-of-context WinEvent helper.");
        StopWorker();
        return FALSE;
    }

    Wh_Log(L"Froggo Explorer Emmet Folders 1.0.4 initialized.");
    return TRUE;
}

void Wh_ModUninit() {
    g_unloading.store(true, std::memory_order_release);
    StopWinEventMonitor();
    RemoveAllRenameSubclasses();
    StopWorker();
    Wh_Log(L"Froggo Explorer Emmet Folders unloaded.");
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}

#endif  // EEF_CORE_ONLY
