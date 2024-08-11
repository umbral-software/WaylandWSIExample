#include "MappedFd.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static constexpr int BAD_FD = -1;

MappedFd::MappedFd(int fd, size_t size)
{
    _fd = fd;
    _size = size;
    _mapping = mmap(nullptr, _size, PROT_WRITE, MAP_PRIVATE, _fd, 0);
}

MappedFd::MappedFd(MappedFd&& other) noexcept {
    _fd = other._fd;
    _size = other._size;
    _mapping = other._mapping;

    other._fd = BAD_FD;
    other._size = 0;
    other._mapping = nullptr;
}

MappedFd::~MappedFd() {
    if (_mapping) {
        munmap(_mapping, _size);
    }

    if (_fd >= 0) {
        close(_fd);
    }
}

MappedFd& MappedFd::operator=(MappedFd&& other) noexcept {
    if (_mapping) {
        munmap(_mapping, _size);
    }

    if (_fd >= 0) {
        close(_fd);
    }

    _fd = other._fd;
    _size = other._size;
    _mapping = other._mapping;

    other._fd = BAD_FD;
    other._size = 0;
    other._mapping = nullptr;

    return *this;
}

int MappedFd::fd() noexcept { return _fd; }
void *MappedFd::map() noexcept { return _mapping; }
const void *MappedFd::map() const noexcept { return _mapping; }
size_t MappedFd::size() const noexcept { return _size; }
