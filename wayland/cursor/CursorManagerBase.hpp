#pragma once

#include "CursorBase.hpp"

#include <memory>

class CursorManagerBase {
public:
    virtual ~CursorManagerBase();

    virtual std::unique_ptr<CursorBase> get_cursor(wl_pointer *pointer) = 0;
};
