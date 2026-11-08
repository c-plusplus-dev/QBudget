#ifndef QBUDGET_PLATFORM_H
#define QBUDGET_PLATFORM_H

#include <cstdint>
#include <filesystem>
#include <cstddef>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

inline std::filesystem::path getExecutableDirectory(
    const int argc,
    char* const argv[]
) {
#ifdef _WIN32
    std::vector<wchar_t> buffer(32768);
    const DWORD length = ::GetModuleFileNameW(
        nullptr,
        buffer.data(),
        static_cast<DWORD>(buffer.size())
    );

    if (length > 0 && static_cast<std::size_t>(length) < buffer.size()) {
        return std::filesystem::path(
            std::wstring(buffer.data(), length)
        ).parent_path();
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    if (size > 0) {
        std::vector<char> buffer(size);
        if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
            std::error_code error;
            const auto resolved = std::filesystem::weakly_canonical(
                std::filesystem::path(buffer.data()),
                error
            );

            return (error
                ? std::filesystem::path(buffer.data())
                : resolved).parent_path();
        }
    }
#elif defined(__linux__)
    std::vector<char> buffer(1024);

    while (buffer.size() <= 1024 * 1024) {
        const auto length = ::readlink(
            "/proc/self/exe",
            buffer.data(),
            buffer.size()
        );

        if (length < 0) {
            break;
        }

        if (static_cast<std::size_t>(length) < buffer.size()) {
            return std::filesystem::path(
                std::string(buffer.data(), static_cast<std::size_t>(length))
            ).parent_path();
        }

        buffer.resize(buffer.size() * 2);
    }
#endif

    if (argc > 0 && argv && argv[0]) {
        std::error_code error;
        const auto executable = std::filesystem::absolute(argv[0], error);
        if (!error) {
            return executable.parent_path();
        }
    }

    std::error_code error;
    const auto current = std::filesystem::current_path(error);
    return error ? std::filesystem::path{} : current;
}

#endif // QBUDGET_PLATFORM_H
