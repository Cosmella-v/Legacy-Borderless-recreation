#include "Scanner.hpp"
#include <Geode/Geode.hpp>
#include <cstring>
#include <utility>

namespace Sig {
Patch::Patch(const char *name, const char *sig, std::vector<Edit> edits)
    : m_name(name), m_sig(sig), m_edits(std::move(edits)) {}

geode::Result<uint8_t *> Patch::find(uint8_t *base) const {
	if (!base)
		return geode::Err("Patch base is null");

	auto dos = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return geode::Err("Invalid header");

	auto nt = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
	if (nt->Signature != IMAGE_NT_SIGNATURE)
		return geode::Err("Invalid PE signature");

	auto sec = IMAGE_FIRST_SECTION(nt);

	for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
		if (std::memcmp(sec->Name, ".text", 5) != 0)
			continue;

		auto begin = base + sec->VirtualAddress;
		size_t size = sec->Misc.VirtualSize;
		std::vector<int> pattern;

		for (size_t i = 0; m_sig[i];) {
			if (m_sig[i] == ' ') {
				++i;
				continue;
			}

			if (m_sig[i] == '?') {
				pattern.push_back(-1);
				++i;
				if (m_sig[i] == '?')
					++i;
				continue;
			}

			if (!m_sig[i + 1])
				return geode::Err("Invalid sig format");

			auto Code = geode::utils::numFromString<int>(std::string(m_sig + i, 2), 16);
			if (Code.isOk()) {
				pattern.push_back(
				    Code.unwrap());
			} else {
				return geode::Err(Code.unwrapErr());
			};
			i += 2;
		}

		if (pattern.empty())
			return geode::Err("Signature is empty");

		if (pattern.size() > size)
			return geode::Err("Signature is larger than the section");

		uint8_t *found = nullptr;
		size_t count = 0;

		for (size_t i = 0; i <= size - pattern.size(); ++i) {
			bool match = true;

			for (size_t j = 0; j < pattern.size(); ++j) {
				if (pattern[j] >= 0 && begin[i + j] != pattern[j]) {
					match = false;
					break;
				}
			}

			if (match) {
				found = begin + i;
				++count;

				if (count > 1)
					return geode::Err("Signature matched with more then one");
			}
		}

		if (count == 0)
			return geode::Err("Signature not found");

		return geode::Ok(found);
	}

	return geode::Err(".text section not found");
}

geode::Result<Patch::PatchResult> Patch::apply(uint8_t* base) const {
    auto result = find(base);
    if (result.isErr())
        return geode::Err(result.unwrapErr());

    auto address = result.unwrap();

    if (m_edits.empty())
        return geode::Err("Patch has no edits");

    size_t start = m_edits.front().offset;
    size_t end = start;

    for (const auto& edit : m_edits) {
        if (edit.offset >= length())
            return geode::Err("Patch edit offset is outside the signature");

        if (address[edit.offset] != edit.expectedValue)
            return geode::Err("Expected byte does not match");

        start = std::min(start, edit.offset);
        end = std::max(end, edit.offset);
    }

    geode::ByteVector bytes(address + start, address + end + 1);

    for (const auto& edit : m_edits)
        bytes[edit.offset - start] = edit.newValue;

    return geode::Ok(PatchResult{address + start, bytes});
}

const char *Patch::name() const {
	return m_name;
}

size_t Patch::length() const {
	size_t length = 0;

	for (size_t i = 0; m_sig[i];) {
		if (m_sig[i] == ' ') {
			++i;
		} else if (m_sig[i] == '?') {
			++length;
			++i;
			if (m_sig[i] == '?')
				++i;
		} else {
			++length;
			i += 2;
		}
	}

	return length;
}
} // namespace Sig
