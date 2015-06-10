/*
Some of this code began its life as a part of GTA V SCRIPT HOOK SDK.
http://dev-c.com
(C) Alexander Blade 2015

It is now part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Sondai SMith and fellow contributors 2015
*/

#pragma once

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"

#include "inc\main.h"
#include "menu_functions.h"

#include "database.h"

void process_weapon_menu();

bool process_give_melee();
bool do_give_weapon(std::string modelName);
bool process_weaponlist_menu();

void reset_weapon_globals();

void update_weapon_features(BOOL bPlayerExists, Player player);

void update_vehicle_guns();

void save_player_weapons();

void restore_player_weapons();

bool is_weapon_equipped(std::vector<int> extras);

void set_weapon_equipped(bool equipped, std::vector<int> extras);

bool is_weaponmod_equipped(std::vector<int> extras);

void set_weaponmod_equipped(bool equipped, std::vector<int> extras);

void give_weapon_clip(MenuItem<int> choice);

void fill_weapon_ammo(MenuItem<int> choice);

void onconfirm_open_tint_menu(MenuItem<int> choice);

bool process_weapon_mod_menu_tint();

//Weapon mod menu

bool process_weapon_mod_menu_tint();

bool onconfirm_weapon_mod_menu_tint(MenuItem<int> choice);

std::vector<FeatureEnabledLocalDefinition> get_feature_enablements_weapons();

bool weapon_reequip_interrupt();