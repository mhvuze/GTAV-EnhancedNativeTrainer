/*
Some of this code began its life as a part of GTA V SCRIPT HOOK SDK.
http://dev-c.com
(C) Alexander Blade 2015

It is now part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#pragma once

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"

#include "inc\main.h"

#include "menu_functions.h"
#include "database.h"

#include <string>

/***
* METHODS
*/

void process_veh_menu();

bool process_savedveh_menu();

bool process_savedveh_slot_menu(int slot);

bool process_vehmod_menu();

bool process_carspawn_menu();

void update_vehicle_features(BOOL playerExists, Ped playerPed);

void reset_vehicle_globals();

bool process_spawn_menu_cars();

bool process_spawn_menu_indus();

bool process_spawn_menu_generic(int topMenuSelection);

bool spawn_saved_car(int slot, std::string caption);

void save_current_vehicle(int slot);

bool onconfirm_spawn_menu_vehicle_selection(MenuItem<std::string> choice);

bool do_spawn_vehicle(std::string modelName, std::string modelTitle);

Vehicle do_spawn_vehicle(DWORD modelHash, std::string modelTitle, bool cleanup);

//Paint menus

bool process_paint_menu();

bool process_paint_menu_type();

bool process_paint_menu_liveries();

bool process_paint_menu_special(int category);

bool onconfirm_color_menu_selection(MenuItem<std::string> choice);

void onhighlight_color_menu_selection(MenuItem<std::string> choice);

//Vehicle mod getters and setters

bool is_custom_tyres(std::vector<int> extras);

void set_custom_tyres(bool applied, std::vector<int> extras);

bool is_turbocharged(std::vector<int> extras);

void set_turbocharged(bool applied, std::vector<int> extras);

bool is_bulletproof_tyres(std::vector<int> extras);

void set_bulletproof_tyres(bool applied, std::vector<int> extras);

bool is_xenon_headlights(std::vector<int> extras);

void set_xenon_headlights(bool applied, std::vector<int> extras);

bool is_extra_enabled(std::vector<int> extras);

void set_extra_enabled(bool applied, std::vector<int> extras);

void set_plate_text(MenuItem<int> choice);

int find_menu_index_to_restore(int category, int actualCategory, Vehicle veh);

bool vehicle_menu_interrupt();

bool vehicle_save_menu_interrupt();

bool vehicle_save_slot_menu_interrupt();

std::vector<FeatureEnabledLocalDefinition> get_feature_enablements_vehicles();