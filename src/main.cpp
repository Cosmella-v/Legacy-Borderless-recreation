#include <Geode/Geode.hpp>
#include "PatchPlus/Macros.hpp"
using namespace geode::prelude;
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>

class $modify(GameStatsManager) {
	static void onModify(auto& self){
		Cosmella::Patch::Setting::Attach(
			self.getHook("GameStatsManager::isItemUnlocked").unwrapOrDefault(),
			"free-unlock"
		);
	}
	bool isItemUnlocked(UnlockType type, int key) {
		return GameStatsManager::isItemUnlocked(type, key) || (type == UnlockType::GJItem && (key >= 16 && key <= 20));
	}
};

class $modify(GameManager) {
	static void onModify(auto& self){
		Cosmella::Patch::Setting::Attach(
			self.getHook("GameManager::isColorUnlocked").unwrapOrDefault(),
			"free-unlock"
		);
		Cosmella::Patch::Setting::Attach(
			self.getHook("GameManager::isIconUnlocked").unwrapOrDefault(),
			"free-unlock"
		);
	}
	bool isColorUnlocked(int key, UnlockType type) {
		return true;
	};

	bool isIconUnlocked(int key, IconType type) {
		return true;
	};
};