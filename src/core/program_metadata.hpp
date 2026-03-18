#pragma once

#include "core/resource_descriptor.hpp"
#include "core/syscall_profile.hpp"
#include "core/types.hpp"

#include <string>

namespace visos {

struct ProgramMetadata {
    std::string program_name;
    ResourceDescriptor required_resources;
    Tick estimated_cpu_time = 0;
    SyscallProfile syscall_profile;
};

} // namespace visos
