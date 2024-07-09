#include "MemFd.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static constexpr int BAD_FD = -1;

MemFd::MemFd() {
    _size = 0;
    _fd = -1;
    _mapping = nullptr;
}

MemFd::MemFd(const char *name, size_t size)
    :MemFd()
{
    _size = size;
    _fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING | MFD_NOEXEC_SEAL);
    ftruncate(_fd, static_cast<off_t>(_size));
    fcntl(_fd, F_ADD_SEALS, F_SEAL_SEAL | F_SEAL_SHRINK );

    _mapping = mmap(nullptr, _size, PROT_WRITE, MAP_SHARED_VALIDATE, _fd, 0);
}

MemFd::MemFd(MemFd&& other) noexcept {
    _size = other._size;
    _fd = other._fd;
    _mapping = other._mapping;

    other._size = 0;
    other._fd = BAD_FD;
    other._mapping = nullptr;
}

MemFd::~MemFd() {
    if (_mapping) {
        munmap(_mapping, _size);
    }

    if (_fd >= 0) {
        close(_fd);
    }
}

MemFd& MemFd::operator=(MemFd&& other) noexcept {
    if (_mapping) {
        munmap(_mapping, _size);
    }

    if (_fd >= 0) {
        close(_fd);
    }

    _size = other._size;
    _fd = other._fd;
    _mapping = other._mapping;

    other._size = 0;
    other._fd = BAD_FD;
    other._mapping = nullptr;

    return *this;
}

int MemFd::fd() noexcept { return _fd; }
void *MemFd::map() noexcept { return _mapping; }
const void *MemFd::map() const noexcept { return _mapping; }
size_t MemFd::size() const noexcept { return _size; }
