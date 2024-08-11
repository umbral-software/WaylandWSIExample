#pragma once

#include <cstddef>

#include <sys/mman.h>

class MappedFd {
public:
    MappedFd(int fd, size_t size);
    MappedFd(const MappedFd&) = delete;
    MappedFd(MappedFd&& other) noexcept;
    ~MappedFd();

    MappedFd& operator=(const MappedFd&) = delete;
    MappedFd& operator=(MappedFd&& other) noexcept;

    int fd() noexcept;
    void *map() noexcept;
    const void *map() const noexcept;
    size_t size() const noexcept;

private:
    int _fd;
    size_t _size;
    void *_mapping;
};
