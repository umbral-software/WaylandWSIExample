#pragma once

#include <cstddef>

class MemFd {
public:
    MemFd();
    MemFd(const char *name, size_t size);
    MemFd(const MemFd&) = delete;
    MemFd(MemFd&& other) noexcept;
    ~MemFd();

    MemFd& operator=(const MemFd&) = delete;
    MemFd& operator=(MemFd&& other) noexcept;

    int fd() noexcept;
    void *map() noexcept;
    const void *map() const noexcept;
    size_t size() const noexcept;

private:
    size_t _size;
    int _fd;
    void *_mapping;
};
