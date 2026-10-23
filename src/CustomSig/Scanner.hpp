#pragma once

#include <Geode/Geode.hpp>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Sig {
    struct Edit {
        size_t offset;
        uint8_t expectedValue;
        uint8_t newValue;
    };

    class Patch {
    public:
        Patch(const char* name, const char* sig, std::vector<Edit> edits);
        struct PatchResult {
            uint8_t* address;
            geode::ByteVector bytes;
        };
        geode::Result<PatchResult> apply(uint8_t* base) const;

        const char* name() const;
        size_t length() const;
    private:
        geode::Result<uint8_t*> find(uint8_t* base) const;

        const char* m_name;
        const char* m_sig;
        std::vector<Edit> m_edits;
    };
}
