#pragma once

#include "core/program_metadata.hpp"
#include "core/types.hpp"

#include <optional>
#include <unordered_map>

namespace visos {

class ProgramStore {
public:
    [[nodiscard]] ProgramID store(ProgramMetadata metadata);
    [[nodiscard]] std::optional<ProgramMetadata> load(ProgramID id) const;

private:
    ProgramID next_id_ = 1;
    std::unordered_map<ProgramID, ProgramMetadata> programs_;
};

} // namespace visos
