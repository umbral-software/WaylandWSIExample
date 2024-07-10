#include "MappableFd.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static constexpr int BAD_FD = -1;

MappableFd::MappableFd(int fd, size_t size)
{
    _fd = fd;
    _size = size;
    ftruncate(_fd, static_cast<off_t>(_size));
    fcntl(_fd, F_ADD_SEALS, F_SEAL_SEAL | F_SEAL_SHRINK );

    _mapping = mmap(nullptr, _size, PROT_WRITE, MAP_PRIVATE, _fd, 0);
}

MappableFd::MappableFd(MappableFd&& other) noexcept {
    _fd = other._fd;
    _size = other._size;
    _mapping = other._mapping;

    other._fd = BAD_FD;
    other._size = 0;
    other._mapping = nullptr;
}

MappableFd::~MappableFd() {
    if (_mapping) {
        munmap(_mapping, _size);
    }

    if (_fd >= 0) {
        close(_fd);
    }
}

MappableFd& MappableFd::operator=(MappableFd&& other) noexcept {
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

int MappableFd::fd() noexcept { return _fd; }
void *MappableFd::map() noexcept { return _mapping; }
const void *MappableFd::map() const noexcept { return _mapping; }
size_t MappableFd::size() const noexcept { return _size; }
