
#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/Hook.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

inline void toggle(geode::Hook* hook, bool enable) {
    if (!hook) return;

    if (enable) {
        if (auto res = hook->enable(); res.isErr()) {
            geode::log::error("Failed to enable {} hook: {}", hook->getDisplayName(), res.unwrapErr());
        }
    }
    else {
        if (auto res = hook->disable(); res.isErr()) {
            geode::log::error("Failed to disable {} hook: {}", hook->getDisplayName(), res.unwrapErr());
        }
    }
}
inline void toggle(geode::Patch* hook, bool enable) {
    if (!hook) return;

    if (enable) {
        if (auto res = hook->enable(); res.isErr()) {
            geode::log::error("Failed to enable {} hook: {}", hook->getAddress(), res.unwrapErr());
        }
    }
    else {
        if (auto res = hook->disable(); res.isErr()) {
            geode::log::error("Failed to disable {} hook: {}", hook->getAddress(), res.unwrapErr());
        }
    }
}
namespace Cosmella::Patch {
    namespace Setting {
        static inline geode::Hook* Attach(geode::Hook* hook, std::string setting, geode::Mod* mod = geode::Mod::get()) {
            bool enabled = mod->getSettingValue<bool>(setting);
            hook->setAutoEnable(enabled);
            geode::listenForSettingChanges<bool>(setting, [hook](bool value) {
                toggle(hook, value);
            }, mod);
            return hook;
        };
    };
    namespace Patch {
        static inline geode::Patch* Attach(geode::Patch* hook, std::string setting, geode::Mod* mod = geode::Mod::get()) {
            bool enabled = mod->getSettingValue<bool>(setting);
            toggle(hook, enabled);
            geode::listenForSettingChanges<bool>(setting, [hook](bool value) {
                toggle(hook, value);
            }, mod);
            return hook;
        };
    };
}
