#ifdef GEODE_IS_WINDOWS
#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include "borderlessClassic.hpp"
#include "../PatchPlus/Macros.hpp"
#include "../CustomSig/Scanner.hpp"
#include <optional>
#include <string_view>
#include <vector>
inline void loadFullscreen(){
 if (auto CCEGL = CCEGLView::get()){
        if (CCEGL->getIsFullscreen()) return;
    }
    bool FFS = Mod::get()->getSettingValue<bool>("full-screen-Toggle");
    if (FFS != Cosmella::FakeFullScreen::isActive()) {
        Cosmella::FakeFullScreen::setFakeFullscreen(FFS);
    };
}
$on_game(Loaded) {
        auto l = Loader::get();
        l->queueInMainThread([]{
        KeybindSettingPressedEventV3(Mod::get(), "full-screen").listen([](Keybind const &keybind, bool down, bool repeat, double timestamp) {
                if (down) {
                    if (auto CCEGL = CCEGLView::get()){ // block fullscreen because it broken
                            if (CCEGL->getIsFullscreen()) {
                                return ListenerResult::Propagate;
                            }
                        }
                        bool FFS = !Cosmella::FakeFullScreen::isActive();
                        Mod::get()->setSettingValue<bool>("full-screen-Toggle", FFS);
                        return ListenerResult::Stop;
                    }
                return ListenerResult::Propagate;
            })
            .leak();
    });
    geode::listenForSettingChanges<bool>("full-screen-Toggle", [l](bool value) {
            l->queueInMainThread([value]{
                Cosmella::FakeFullScreen::setFakeFullscreen(value);
            });
        });
}
// these patches are free resizing but for fixing the edges of fullscreen
// well it also allows free resize but you can't really do anything with that
static const std::vector<Sig::Patch> cAspectPatch = {
    // WM_SIZING drag enforcement (applyAspectRatio) -> ret
    {
        "applyAspect",
        "40 53 57 41 56 41 57 8B B9 C8 02 00",
        {{0, 0x40, 0xC3}}},
    // WM_SIZE handler: skip ratio clamp (je -> jmp)
    {
        "wmSizeClamp",
        "8B 8E C8 02 00 00 0F B7 FB 48 C1 EB 10 0F B7 DB 85 C9 74",
        {{18, 0x74, 0xEB}}},
    // EXITSIZEMOVE: skip ratio clamp (je -> jmp)
    {
        "exitSizeMoveClamp",
        "8B 8E C8 02 00 00 66 0F 6E C0 8B 45 A3 2B 45 9B "
        "0F 5B C0 F3 0F 2C D0 66 0F 6E C0 0F 5B C0 "
        "0F 5A C8 0F 54 0D ?? ?? ?? ?? F2 44 0F 2C C1 85 C9 74",
        {{47, 0x74, 0xEB}}},
    // fit-to-monitor fixes it being capped
   {
        "fitToMonitor",
        "0F 2F E5 72 0A F3 0F 2C DD F3 0F 2C",
        {{3, 0x72, 0x90}, {4, 0x0A, 0x90}, {8, 0xDD, 0xDC}}
    }
    
};
$on_game(TexturesUnloaded) {
    Cosmella::FakeFullScreen::setFakeFullscreen(false);
};
$on_game(TexturesLoaded) {
    loadFullscreen();
};

$on_mod(Loaded) {
    auto l = Loader::get();
    l->queueInMainThread([]{
        loadFullscreen(); // first boot
    });
    static uint8_t* base = (uint8_t*)geode::base::getCocos();

    for (const auto& p : cAspectPatch) {
        auto result = p.apply(base);

        if (result.isErr()) {
            log::error("{}: {}", p.name(), result.unwrapErr());
            continue;
        }

        auto [addr, bytes] = result.unwrap();

        auto r = Mod::get()->patch(addr, bytes);

        if (r.isErr()) {
            log::error("{}: patch failed: {}", p.name(), r.unwrapErr());
            continue;
        }

        Cosmella::Patch::Patch::Attach(
            r.unwrapOrDefault(),
            "full-screen-Toggle"
        );
        log::debug("Patched base 0x{:X} with offset 0x{:X}: {}", (uintptr_t)base, addr - base, bytes);
    }
}
#endif
