#pragma once

#include <cstdint>
#include <array>

namespace visos {

struct RegisterSet {
    std::int64_t program_counter = 0;
    std::int64_t stack_pointer = 0;
    std::array<std::int64_t, 8> general_purpose{};

    bool operator==(const RegisterSet&) const = default;
};

} // namespace visos
