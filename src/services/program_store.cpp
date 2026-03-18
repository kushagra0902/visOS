#include "services/program_store.hpp"

namespace visos {

ProgramID ProgramStore::store(ProgramMetadata metadata) {
    ProgramID id = next_id_++;
    programs_.emplace(id, std::move(metadata));
    return id;
}

std::optional<ProgramMetadata> ProgramStore::load(ProgramID id) const {
    auto it = programs_.find(id);
    if (it != programs_.end()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace visos
