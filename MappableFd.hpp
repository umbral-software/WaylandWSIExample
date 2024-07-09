#pragma once

#include <cstddef>

#include <sys/mman.h>

class MappableFd {
public:
    MappableFd();
    MappableFd(int fd, size_t size);
    MappableFd(const MappableFd&) = delete;
    MappableFd(MappableFd&& other) noexcept;
    ~MappableFd();

    MappableFd& operator=(const MappableFd&) = delete;
    MappableFd& operator=(MappableFd&& other) noexcept;

    int fd() noexcept;
    void *map() noexcept;
    const void *map() const noexcept;
    size_t size() const noexcept;

private:
    int _fd;
    size_t _size;
    void *_mapping;
};
