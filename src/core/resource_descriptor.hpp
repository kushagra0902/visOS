#pragma once

#include <cstdint>

namespace visos {

struct ResourceDescriptor {
    std::int32_t memory_required = 0;
    std::int32_t io_devices_required = 0;

    bool operator==(const ResourceDescriptor&) const = default;
};

} // namespace visos
