/*
Some of this code began its life as a part of GTA V SCRIPT HOOK SDK.
http://dev-c.com
(C) Alexander Blade 2015

It is now part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

/*
	F4					activate
	NUM2/8/4/6			navigate thru the menus and lists (numlock must be on)
	NUM5 				select
	BACKSPACE/F4		back
	NUM9/3 				use vehicle boost when active
	NUM+ 				use vehicle rockets when active
	NUMx				wipe wanted level
	NUM-				fix player
*/

#include "io.h"
#include "menu_functions.h"
#include "skins.h"
#include "script.h"
#include "vehicles.h"
#include "teleportation.h"
#include <string>
#include <sstream> 

#include <ctime>
#include <vector>

#pragma warning(disable : 4244 4305) // double <-> float conversions

// features
bool featurePlayerInvincible			=	false;
bool featurePlayerInvincibleUpdated		=	false;
bool featurePlayerNeverWanted			=	false;
bool featurePlayerIgnored				=	false;
bool featurePlayerIgnoredUpdated		=	false;
bool featurePlayerUnlimitedAbility		=	false;
bool featurePlayerNoNoise				=	false;
bool featurePlayerNoNoiseUpdated		=	false;
bool featurePlayerFastSwim				=	false;
bool featurePlayerFastSwimUpdated		=	false;
bool featurePlayerFastRun				=	false;
bool featurePlayerFastRunUpdated		=	false;
bool featurePlayerSuperJump				=	false;

bool featureWeaponNoReload				=	false;
bool featureWeaponUnlimAmmoA = false;
bool featureWeaponUnlimAmmoB = false;
bool featureWeaponFireAmmo				=	false;
bool featureWeaponExplosiveAmmo			=	false;
bool featureWeaponExplosiveMelee		=	false;
bool featureWeaponVehRockets			=	false;

DWORD featureWeaponVehShootLastTime		=	0;

bool featureWorldMoonGravity			=	false;
bool featureWorldRandomCops				=	true;
bool featureWorldRandomTrains			=	true;
bool featureWorldRandomBoats			=	true;
bool featureWorldGarbageTrucks			=	true;

bool featureTimePaused					=	false;
bool featureTimePausedUpdated			=	false;
bool featureTimeSynced					=	false;

bool featureWeatherWind					=	false;

bool featureMiscLockRadio				=	false;
bool featureMiscHideHud					=	false;

// player model control, switching on normal ped model when needed	

LPCSTR animalModels[] = { "a_c_boar", "a_c_chimp", "a_c_cow", "a_c_coyote", "a_c_deer", "a_c_fish", "a_c_hen", "a_c_cat_01", "a_c_chickenhawk",
	"a_c_cormorant", "a_c_crow", "a_c_dolphin", "a_c_humpback", "a_c_killerwhale", "a_c_pigeon", "a_c_seagull", "a_c_sharkhammer",
	"a_c_pig", "a_c_rat", "a_c_rhesus", "a_c_chop", "a_c_husky", "a_c_mtlion", "a_c_retriever", "a_c_sharktiger", "a_c_shepherd" };

void check_player_model() 
{
	// common variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();	

	if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;

	Hash model = ENTITY::GET_ENTITY_MODEL(playerPed);
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
		for (int i = 0; i < (sizeof(animalModels) / sizeof(animalModels[0])); i++) 
			if (GAMEPLAY::GET_HASH_KEY((char *)animalModels[i]) == model)
			{
				set_status_text("Returning player to human");
				WAIT(1000);
				model = GAMEPLAY::GET_HASH_KEY("player_zero");
				STREAMING::REQUEST_MODEL(model);
				while (!STREAMING::HAS_MODEL_LOADED(model))
					WAIT(0);
				PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
				PED::SET_PED_DEFAULT_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID());
				STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);

			// wait until player is ressurected
			while (ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
				WAIT(0);

				break;
			}
}

void update_vehicle_guns()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();	

	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !featureWeaponVehRockets) return;

	bool bSelect = IsKeyDown(0x6B); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) &&	PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_VEHICLE_ROCKET");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh,  (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh,  v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z, 
													 coords0to.x, coords0to.y, coords0to.z, 
													 250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z, 
													 coords1to.x, coords1to.y, coords1to.z, 
													 250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);

		featureWeaponVehShootLastTime = GetTickCount();
	}
}

// Updates all features that can be turned off by the game, being called each game frame
void update_features()
{
	update_status_text();

	update_vehicle_guns();

	// changing player model if died/arrested while being in another skin, since it can cause inf loading loop
	/*if (skinchanger_used)
		check_player_model();*/

	// wait until player is ready, basicly to prevent using the trainer while player is dead or arrested
	while (ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()) || PLAYER::IS_PLAYER_BEING_ARRESTED(PLAYER::PLAYER_ID(), TRUE))
		WAIT(0);

	// read default feature values from the game
	featureWorldRandomCops = PED::CAN_CREATE_RANDOM_COPS() == TRUE;

	// common variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);

	// player invincible
	if (featurePlayerInvincibleUpdated)
	{
		if (bPlayerExists && !featurePlayerInvincible)
			PLAYER::SET_PLAYER_INVINCIBLE(player, FALSE);
		featurePlayerInvincibleUpdated = false;
	}
	if (featurePlayerInvincible)
	{
		if (bPlayerExists)
			PLAYER::SET_PLAYER_INVINCIBLE(player, TRUE);
	}

	// player never wanted
	if (featurePlayerNeverWanted)
	{
		if (bPlayerExists)
			PLAYER::CLEAR_PLAYER_WANTED_LEVEL(player);
	}

	// police ignore player
	if (featurePlayerIgnoredUpdated)
	{
		if (bPlayerExists)
			PLAYER::SET_POLICE_IGNORE_PLAYER(player, featurePlayerIgnored);
		featurePlayerIgnoredUpdated = false;
	}

	// player special ability
	if (featurePlayerUnlimitedAbility)
	{
		if (bPlayerExists)
			PLAYER::_RECHARGE_SPECIAL_ABILITY(player, 1);
	}

	// player no noise
	if (featurePlayerNoNoiseUpdated)
	{
		if (bPlayerExists && !featurePlayerNoNoise)
			PLAYER::SET_PLAYER_NOISE_MULTIPLIER(player, 1.0);
		featurePlayerNoNoiseUpdated = false;
	}
	if (featurePlayerNoNoise)
		PLAYER::SET_PLAYER_NOISE_MULTIPLIER(player, 0.0);

	// player fast swim
	if (featurePlayerFastSwimUpdated)
	{
		if (bPlayerExists && !featurePlayerFastSwim)
			PLAYER::_SET_SWIM_SPEED_MULTIPLIER(player, 1.0);
		featurePlayerFastSwimUpdated = false;
	}
	if (featurePlayerFastSwim)
		PLAYER::_SET_SWIM_SPEED_MULTIPLIER(player, 1.49);

	// player fast run
	if (featurePlayerFastRunUpdated)
	{
		if (bPlayerExists && !featurePlayerFastRun)
			PLAYER::_SET_MOVE_SPEED_MULTIPLIER(player, 1.0);
		featurePlayerFastRunUpdated = false;
	}
	if (featurePlayerFastRun)
		PLAYER::_SET_MOVE_SPEED_MULTIPLIER(player, 1.49);

	// player super jump
	if (featurePlayerSuperJump)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(player);
	}

	// weapon
	if (featureWeaponFireAmmo)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_FIRE_AMMO_THIS_FRAME(player);
	}
	if (featureWeaponExplosiveAmmo)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);
	}
	if (featureWeaponExplosiveMelee)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_EXPLOSIVE_MELEE_THIS_FRAME(player);
	}

	// weapon no reload
	if (bPlayerExists && featureWeaponNoReload)
	{
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(cur))
			{
				int maxAmmo;
				if (WEAPON::GET_MAX_AMMO(playerPed, cur, &maxAmmo))
				{
					WEAPON::SET_PED_AMMO(playerPed, cur, maxAmmo);

					maxAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, cur, 1);
					if (maxAmmo > 0)
						WEAPON::SET_AMMO_IN_CLIP(playerPed, cur, maxAmmo);
				}
			}
		}
	}

	// unlimited ammo no ammo count
	if (bPlayerExists && featureWeaponUnlimAmmoA)
	{
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(cur))
			{
				WEAPON::SET_PED_INFINITE_AMMO(playerPed, 1, cur);
			}
		}
	}

	// unlimited ammo with ammo count
	if (bPlayerExists && featureWeaponUnlimAmmoB)
	{
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(cur))
			{
				int maxAmmo;
				if (WEAPON::GET_MAX_AMMO(playerPed, cur, &maxAmmo))
				{
					WEAPON::SET_PED_AMMO(playerPed, cur, maxAmmo);
				}
			}
		}
	}

	update_vehicle_features(bPlayerExists, playerPed);

	// time pause
	if (featureTimePausedUpdated)
	{
		TIME::PAUSE_CLOCK(featureTimePaused);
		featureTimePausedUpdated = false;
	}

	// time sync
	if (featureTimeSynced)
	{
		time_t now = time(0);
		tm t;
		localtime_s(&t, &now);
		TIME::SET_CLOCK_TIME(t.tm_hour, t.tm_min, t.tm_sec);
	}

	// hide hud
	if (featureMiscHideHud)
	{
		UI::HIDE_HUD_AND_RADAR_THIS_FRAME();
	}

	// more hotkeys
	// wipe wanted level
	if (bPlayerExists && PLAYER::GET_PLAYER_WANTED_LEVEL(player) > 0)
	{
		bool wipeWanted = get_key_pressed(VK_MULTIPLY);

		if (wipeWanted)
		{
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, 0);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, 0);
			set_status_text("wanted level wiped");
		}
	}

	// fix player
	if (bPlayerExists)
	{
		bool fixPlayer = get_key_pressed(VK_SUBTRACT);

		if (fixPlayer)
		{
			ENTITY::SET_ENTITY_HEALTH(playerPed, ENTITY::GET_ENTITY_MAX_HEALTH(playerPed));
			PED::ADD_ARMOUR_TO_PED(playerPed, PLAYER::GET_PLAYER_MAX_ARMOUR(player) - PED::GET_PED_ARMOUR(playerPed));
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
				{
					Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
					if (ENTITY::DOES_ENTITY_EXIST(playerVeh) && !ENTITY::IS_ENTITY_DEAD(playerVeh))
					VEHICLE::SET_VEHICLE_FIXED(playerVeh);
				}
			set_status_text("player fixed");
		}
	}
}

int activeLineIndexPlayer = 0;

void process_player_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 16;
	
	std::string caption = "PLAYER  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"PLAYER SKIN", NULL, NULL},
		{"TELEPORT", NULL, NULL},
		{"FIX PLAYER", NULL, NULL},
		{"RESET SKIN", NULL, NULL},
		{"ADD CASH", NULL, NULL},
		{"WANTED UP", NULL, NULL},
		{"WANTED DOWN", NULL, NULL},
		{"CLEAR WANTED", NULL, NULL},
		{"NEVER WANTED", &featurePlayerNeverWanted, NULL},
		{"INVINCIBLE", &featurePlayerInvincible, &featurePlayerInvincibleUpdated},
		{"POLICE IGNORED", &featurePlayerIgnored, &featurePlayerIgnoredUpdated},
		{"UNLIM ABILITY", &featurePlayerUnlimitedAbility, NULL},
		{"NOISELESS", &featurePlayerNoNoise, &featurePlayerNoNoiseUpdated},
		{"FAST SWIM", &featurePlayerFastSwim, &featurePlayerFastSwimUpdated},
		{"FAST RUN", &featurePlayerFastRun, &featurePlayerFastRunUpdated},
		{"SUPER JUMP", &featurePlayerSuperJump, NULL}
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexPlayer)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
								   lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexPlayer].text, lines[activeLineIndexPlayer].pState), 
						   lineWidth + 1.0, 11.0, 56.0 + activeLineIndexPlayer * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();

			// common variables
			BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
			Player player = PLAYER::PLAYER_ID();
			Ped playerPed = PLAYER::PLAYER_PED_ID();

			switch (activeLineIndexPlayer)
			{
				// skin changer
				case 0:
					if (process_skinchanger_menu())	return;
					break;
				// teleport
				case 1:
					if (process_teleport_menu()) return;
					break;
				// fix player
				case 2:
					{
						ENTITY::SET_ENTITY_HEALTH(playerPed, ENTITY::GET_ENTITY_MAX_HEALTH(playerPed));
						PED::ADD_ARMOUR_TO_PED(playerPed, PLAYER::GET_PLAYER_MAX_ARMOUR(player) - PED::GET_PED_ARMOUR(playerPed));
						if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
						{
							Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
							if (ENTITY::DOES_ENTITY_EXIST(playerVeh) && !ENTITY::IS_ENTITY_DEAD(playerVeh))
								VEHICLE::SET_VEHICLE_FIXED(playerVeh);
						}
						set_status_text("Fixed Player");
					}
					break;
				// reset model skin
				case 3:
					{
						PED::SET_PED_DEFAULT_COMPONENT_VARIATION(playerPed);
						set_status_text("using default model skin");
					}
					break;
				// add cash
				case 4: 
					for (int i = 0; i < 3; i++)
					{
						char statNameFull[32];
						sprintf_s(statNameFull, "SP%d_TOTAL_CASH", i);
						Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
						int val;
						STATS::STAT_GET_INT(hash, &val, -1);
						val += 1000000;
						STATS::STAT_SET_INT(hash, val, 1);
					}
					set_status_text("Added Cash");
					break;
				// wanted up
				case 5:	
					if (bPlayerExists && PLAYER::GET_PLAYER_WANTED_LEVEL(player) < 5)
					{
						PLAYER::SET_PLAYER_WANTED_LEVEL(player, PLAYER::GET_PLAYER_WANTED_LEVEL(player) + 1, 0);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, 0);
						set_status_text("Increased Wanted Level");
					}
					break;
				// wanted down
				case 6:	
					if (bPlayerExists && PLAYER::GET_PLAYER_WANTED_LEVEL(player) > 0)
					{
						PLAYER::SET_PLAYER_WANTED_LEVEL(player, PLAYER::GET_PLAYER_WANTED_LEVEL(player) - 1, 0);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, 0);
						set_status_text("Decreased Wanted Level");
					}
					break;
				case 7:
					if (bPlayerExists && PLAYER::GET_PLAYER_WANTED_LEVEL(player) > 0)
					{
						PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, 0);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, 0);
						set_status_text("Cleared Wanted Level");
					}
				// switchable features
				default:
					if (lines[activeLineIndexPlayer].pState)
						*lines[activeLineIndexPlayer].pState = !(*lines[activeLineIndexPlayer].pState);
					if (lines[activeLineIndexPlayer].pUpdated)
						*lines[activeLineIndexPlayer].pUpdated = true;					
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexPlayer == 0) 
				activeLineIndexPlayer = lineCount;
			activeLineIndexPlayer--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexPlayer++;
			if (activeLineIndexPlayer == lineCount) 
				activeLineIndexPlayer = 0;			
			waitTime = 150;
		}
	}
}

int activeLineIndexWeapon = 0;

void process_weapon_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 8;

	std::string caption = "WEAPON  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"GET ALL WEAPON",	NULL,								NULL},
		{"NO RELOAD",		&featureWeaponNoReload,				NULL},
		{"UNLIMITED AMMO", &featureWeaponUnlimAmmoA,			NULL },
		{"UNLIMITED AMMO w/COUNT", &featureWeaponUnlimAmmoB,	NULL },
		{"FIRE AMMO",		&featureWeaponFireAmmo,				NULL},
		{"EXPLOSIVE AMMO",  &featureWeaponExplosiveAmmo,		NULL},
		{"EXPLOSIVE MELEE", &featureWeaponExplosiveMelee,		NULL},
		{"VEHICLE ROCKETS", &featureWeaponVehRockets,			NULL}
	};

	static LPCSTR weaponNames[] = {
		"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR", 
		"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG", 
		"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
		"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
		"WEAPON_STUNGUN", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
		"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
		"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN",
		"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
		"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
		"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_HATCHET", "WEAPON_RAILGUN"
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexWeapon)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
								   lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexWeapon].text, lines[activeLineIndexWeapon].pState), 
						   lineWidth + 1.0, 11.0, 56.0 + activeLineIndexWeapon * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();

			// common variables
			BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
			Player player = PLAYER::PLAYER_ID();
			Ped playerPed = PLAYER::PLAYER_PED_ID();

			switch (activeLineIndexWeapon)
			{
				case 0:
					for (int i = 0; i < sizeof(weaponNames) / sizeof(weaponNames[0]); i++)
						WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, GAMEPLAY::GET_HASH_KEY((char *)weaponNames[i]), 1000, 0);
					set_status_text("Added all weapons");
					break;
				// switchable features
				default:
					if (lines[activeLineIndexWeapon].pState)
						*lines[activeLineIndexWeapon].pState = !(*lines[activeLineIndexWeapon].pState);
					if (lines[activeLineIndexWeapon].pUpdated)
						*lines[activeLineIndexWeapon].pUpdated = true;					
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexWeapon == 0) 
				activeLineIndexWeapon = lineCount;
			activeLineIndexWeapon--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexWeapon++;
			if (activeLineIndexWeapon == lineCount) 
				activeLineIndexWeapon = 0;			
			waitTime = 150;
		}
	}
}

LPCSTR vehicleModels[35][10] = {
	{"NINEF", "NINEF2", "BLISTA", "ASEA", "ASEA2", "BOATTRAILER", "BUS", "ARMYTANKER", "ARMYTRAILER", "ARMYTRAILER2"},
	{"SUNTRAP", "COACH", "AIRBUS", "ASTEROPE", "AIRTUG", "AMBULANCE", "BARRACKS", "BARRACKS2", "BALLER", "BALLER2"},
	{"BJXL", "BANSHEE", "BENSON", "BFINJECTION", "BIFF", "BLAZER", "BLAZER2", "BLAZER3", "BISON", "BISON2"},
	{"BISON3", "BOXVILLE", "BOXVILLE2", "BOXVILLE3", "BOBCATXL", "BODHI2", "BUCCANEER", "BUFFALO", "BUFFALO2", "BULLDOZER"},
	{"BULLET", "BLIMP", "BURRITO", "BURRITO2", "BURRITO3", "BURRITO4", "BURRITO5", "CAVALCADE", "CAVALCADE2", "POLICET"},
	{"GBURRITO", "CABLECAR", "CADDY", "CADDY2", "CAMPER", "CARBONIZZARE", "CHEETAH", "COMET2", "COGCABRIO", "COQUETTE"},
	{"CUTTER", "GRESLEY", "DILETTANTE", "DILETTANTE2", "DUNE", "DUNE2", "HOTKNIFE", "DLOADER", "DUBSTA", "DUBSTA2"},
	{"DUMP", "RUBBLE", "DOCKTUG", "DOMINATOR", "EMPEROR", "EMPEROR2", "EMPEROR3", "ENTITYXF", "EXEMPLAR", "ELEGY2"},
	{"F620", "FBI", "FBI2", "FELON", "FELON2", "FELTZER2", "FIRETRUK", "FLATBED", "FORKLIFT", "FQ2"},
	{"FUSILADE", "FUGITIVE", "FUTO", "GRANGER", "GAUNTLET", "HABANERO", "HAULER", "HANDLER", "INFERNUS", "INGOT"},
	{"INTRUDER", "ISSI2", "JACKAL", "JOURNEY", "JB700", "KHAMELION", "LANDSTALKER", "LGUARD", "MANANA", "MESA"},
	{"MESA2", "MESA3", "CRUSADER", "MINIVAN", "MIXER", "MIXER2", "MONROE", "MOWER", "MULE", "MULE2"},
	{"ORACLE", "ORACLE2", "PACKER", "PATRIOT", "PBUS", "PENUMBRA", "PEYOTE", "PHANTOM", "PHOENIX", "PICADOR"},
	{"POUNDER", "POLICE", "POLICE4", "POLICE2", "POLICE3", "POLICEOLD1", "POLICEOLD2", "PONY", "PONY2", "PRAIRIE"},
	{"PRANGER", "PREMIER", "PRIMO", "PROPTRAILER", "RANCHERXL", "RANCHERXL2", "RAPIDGT", "RAPIDGT2", "RADI", "RATLOADER"},
	{"REBEL", "REGINA", "REBEL2", "RENTALBUS", "RUINER", "RUMPO", "RUMPO2", "RHINO", "RIOT", "RIPLEY"},
	{"ROCOTO", "ROMERO", "SABREGT", "SADLER", "SADLER2", "SANDKING", "SANDKING2", "SCHAFTER2", "SCHWARZER", "SCRAP"},
	{"SEMINOLE", "SENTINEL", "SENTINEL2", "ZION", "ZION2", "SERRANO", "SHERIFF", "SHERIFF2", "SPEEDO", "SPEEDO2"},
	{"STANIER", "STINGER", "STINGERGT", "STOCKADE", "STOCKADE3", "STRATUM", "SULTAN", "SUPERD", "SURANO", "SURFER"},
	{"SURFER2", "SURGE", "TACO", "TAILGATER", "TAXI", "TRASH", "TRACTOR", "TRACTOR2", "TRACTOR3", "GRAINTRAILER"},
	{"BALETRAILER", "TIPTRUCK", "TIPTRUCK2", "TORNADO", "TORNADO2", "TORNADO3", "TORNADO4", "TOURBUS", "TOWTRUCK", "TOWTRUCK2"},
	{"UTILLITRUCK", "UTILLITRUCK2", "UTILLITRUCK3", "VOODOO2", "WASHINGTON", "STRETCH", "YOUGA", "ZTYPE", "SANCHEZ", "SANCHEZ2"},
	{"SCORCHER", "TRIBIKE", "TRIBIKE2", "TRIBIKE3", "FIXTER", "CRUISER", "BMX", "POLICEB", "AKUMA", "CARBONRS"},
	{"BAGGER", "BATI", "BATI2", "RUFFIAN", "DAEMON", "DOUBLE", "PCJ", "VADER", "VIGERO", "FAGGIO2"},
	{"HEXER", "ANNIHILATOR", "BUZZARD", "BUZZARD2", "CARGOBOB", "CARGOBOB2", "CARGOBOB3", "SKYLIFT", "POLMAV", "MAVERICK"},
	{"NEMESIS", "FROGGER", "FROGGER2", "CUBAN800", "DUSTER", "STUNT", "MAMMATUS", "JET", "SHAMAL", "LUXOR"},
	{"TITAN", "LAZER", "CARGOPLANE", "SQUALO", "MARQUIS", "DINGHY", "DINGHY2", "JETMAX", "PREDATOR", "TROPIC"},
	{"SEASHARK", "SEASHARK2", "SUBMERSIBLE", "TRAILERS", "TRAILERS2", "TRAILERS3", "TVTRAILER", "RAKETRAILER", "TANKER", "TRAILERLOGS"},
	{"TR2", "TR3", "TR4", "TRFLAT", "TRAILERSMALL", "VELUM", "ADDER", "VOLTIC", "VACCA", "BIFTA"},
	{ "SPEEDER", "PARADISE", "KALAHARI", "JESTER", "TURISMOR", "VESTRA", "ALPHA", "HUNTLEY", "THRUST", "MASSACRO" },
	{ "MASSACRO2", "ZENTORNO", "BLADE", "GLENDALE", "PANTO", "PIGALLE", "WARRENER", "RHAPSODY", "DUBSTA3", "MONSTER" },
	{ "SOVEREIGN", "INNOVATION", "HAKUCHOU", "FUROREGT", "MILJET", "COQUETTE2", "BTYPE", "BUFFALO3", "DOMINATOR2", "GAUNTLET2" },
	{ "MARSHALL", "DUKES", "DUKES2", "STALION", "STALION2", "BLISTA2", "BLISTA3", "DODO", "SUBMERSIBLE2", "HYDRA" },
	{ "INSURGENT", "INSURGENT2", "TECHNICAL", "SAVAGE", "VALKYRIE", "KURUMA", "KURUMA2", "JESTER2", "CASCO", "VELUM2" },
	{ "GUARDIAN", "ENDURO", "LECTRO", "SLAMVAN", "SLAMVAN2", "RATLOADER2", "", "", "", "" }
};


int activeLineIndexWorld = 0;

void process_world_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 5;

	std::string caption = "WORLD  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"MOON GRAVITY",	&featureWorldMoonGravity,	NULL},
		{"RANDOM COPS",		&featureWorldRandomCops,	NULL},
		{"RANDOM TRAINS",	&featureWorldRandomTrains,	NULL},
		{"RANDOM BOATS",	&featureWorldRandomBoats,	NULL},
		{"GARBAGE TRUCKS",	&featureWorldGarbageTrucks,	NULL}
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexWorld)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
					lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexWorld].text, lines[activeLineIndexWorld].pState), 
				lineWidth + 1.0, 11.0, 56.0 + activeLineIndexWorld * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();
			switch (activeLineIndexWorld)
			{
				case 0: 
					featureWorldMoonGravity = !featureWorldMoonGravity;
					GAMEPLAY::SET_GRAVITY_LEVEL(featureWorldMoonGravity);
					break;
				case 1: 
					// featureWorldRandomCops being set in update_features
					PED::SET_CREATE_RANDOM_COPS(!featureWorldRandomCops);
					break;
				case 2:
					featureWorldRandomTrains = !featureWorldRandomTrains;
					VEHICLE::SET_RANDOM_TRAINS(featureWorldRandomTrains);
					break;
				case 3:
					featureWorldRandomBoats = !featureWorldRandomBoats;
					VEHICLE::SET_RANDOM_BOATS(featureWorldRandomBoats);
					break;
				case 4:
					featureWorldGarbageTrucks = !featureWorldGarbageTrucks;
					VEHICLE::SET_GARBAGE_TRUCKS(featureWorldGarbageTrucks);
					break;
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexWorld == 0) 
				activeLineIndexWorld = lineCount;
			activeLineIndexWorld--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexWorld++;
			if (activeLineIndexWorld == lineCount) 
				activeLineIndexWorld = 0;			
			waitTime = 150;
		}
	}
}

int activeLineIndexTime = 0;

void process_time_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 4;

	std::string caption = "TIME  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"HOUR FORWARD",	 NULL,				 NULL},
		{"HOUR BACKWARD",	 NULL,				 NULL},
		{"CLOCK PAUSED",	 &featureTimePaused, &featureTimePausedUpdated},
		{"SYNC WITH SYSTEM", &featureTimeSynced, NULL}
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexTime)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
					lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexTime].text, lines[activeLineIndexTime].pState), 
				lineWidth + 1.0, 11.0, 56.0 + activeLineIndexTime * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();
			switch (activeLineIndexTime)
			{
			// hour forward/backward
			case 0: 
			case 1:
				{
					int h = TIME::GET_CLOCK_HOURS();
					if (activeLineIndexTime == 0) h = (h == 23) ? 0 : h + 1; else h = (h == 0) ? 23 : h - 1;
					int m = TIME::GET_CLOCK_MINUTES();
					TIME::SET_CLOCK_TIME(h, m, 0);
					char text[32];
					sprintf_s(text, "time %d:%d", h, m);
					set_status_text(text);
				}
				break;
			// switchable features
			default:
				if (lines[activeLineIndexTime].pState)
					*lines[activeLineIndexTime].pState = !(*lines[activeLineIndexTime].pState);
				if (lines[activeLineIndexTime].pUpdated)
					*lines[activeLineIndexTime].pUpdated = true;	
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexTime == 0) 
				activeLineIndexTime = lineCount;
			activeLineIndexTime--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexTime++;
			if (activeLineIndexTime == lineCount) 
				activeLineIndexTime = 0;			
			waitTime = 150;
		}
	}
}

int activeLineIndexWeather = 0;

void process_weather_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 15;

	std::string caption = "WEATHER  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"WIND",		&featureWeatherWind,	NULL},
		{"EXTRASUNNY",	NULL,					NULL},
		{"CLEAR",		NULL,					NULL},
		{"CLOUDS",		NULL,					NULL},
		{"SMOG",		NULL,					NULL},
		{"FOGGY",		NULL,					NULL},
		{"OVERCAST",	NULL,					NULL},
		{"RAIN",		NULL,					NULL},
		{"THUNDER",		NULL,					NULL},
		{"CLEARING",	NULL,					NULL},
		{"NEUTRAL",		NULL,					NULL},
		{"SNOW",		NULL,					NULL},
		{"BLIZZARD",	NULL,					NULL},
		{"SNOWLIGHT",	NULL,					NULL},
		{"XMAS",		NULL,					NULL}
	};


	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexWeather)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
					lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexWeather].text, lines[activeLineIndexWeather].pState), 
				lineWidth + 1.0, 11.0, 56.0 + activeLineIndexWeather * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();
			switch (activeLineIndexWeather)
			{
			// wind
			case 0: 
				featureWeatherWind = !featureWeatherWind;
				if (featureWeatherWind)
				{
					GAMEPLAY::SET_WIND(1.0);
					GAMEPLAY::SET_WIND_SPEED(11.99);
					GAMEPLAY::SET_WIND_DIRECTION(ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
				} else
				{
					GAMEPLAY::SET_WIND(0.0);
					GAMEPLAY::SET_WIND_SPEED(0.0);
				}
				break;
			// set weather
			default:
				GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST((char *)lines[activeLineIndexWeather].text);
				GAMEPLAY::CLEAR_WEATHER_TYPE_PERSIST();	
				set_status_text(lines[activeLineIndexWeather].text);
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexWeather == 0) 
				activeLineIndexWeather = lineCount;
			activeLineIndexWeather--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexWeather++;
			if (activeLineIndexWeather == lineCount) 
				activeLineIndexWeather = 0;			
			waitTime = 150;
		}
	}
}

int activeLineIndexMisc = 0;

void process_misc_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 2;

	std::string caption = "MISC  OPTIONS";

	static struct {
		LPCSTR		text;
		bool		*pState;
		bool		*pUpdated;
	} lines[lineCount] = {
		{"NEXT RADIO TRACK",	NULL,					NULL},
		{"HIDE HUD",			&featureMiscHideHud,	NULL}		
	};


	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexMisc)
					draw_menu_line(line_as_str(lines[i].text, lines[i].pState), 
					lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(line_as_str(lines[activeLineIndexMisc].text, lines[activeLineIndexMisc].pState), 
				lineWidth + 1.0, 11.0, 56.0 + activeLineIndexMisc * 36.0, 0.0, 7.0, true, false);

			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();
			switch (activeLineIndexMisc)
			{
			// next radio track
			case 0: 
				if (ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID()) && 
					PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0))
						AUDIO::SKIP_RADIO_FORWARD();
				break;
			// switchable features
			default:
				if (lines[activeLineIndexMisc].pState)
					*lines[activeLineIndexMisc].pState = !(*lines[activeLineIndexMisc].pState);
				if (lines[activeLineIndexMisc].pUpdated)
					*lines[activeLineIndexMisc].pUpdated = true;
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexMisc == 0) 
				activeLineIndexMisc = lineCount;
			activeLineIndexMisc--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexMisc++;
			if (activeLineIndexMisc == lineCount) 
				activeLineIndexMisc = 0;			
			waitTime = 150;
		}
	}
}

int activeLineIndexMain = 0;

void process_main_menu()
{
	const float lineWidth = 300.0;
	const int lineCount = 7;	

	std::string caption = "NATIVE TRAINER (AB) ENHANCED";

	static LPCSTR lineCaption[lineCount] = {
		"Player",
		"Weapons",
		"Vehicles",
		"World",
		"Time",
		"Weather",
		"Misc"
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do 
		{
			// draw menu
			draw_menu_line(caption, lineWidth, 15.0, 18.0, 0.0, 5.0, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexMain)
					draw_menu_line(lineCaption[i], lineWidth, 9.0, 60.0 + i * 36.0, 0.0, 9.0, false, false);
			draw_menu_line(lineCaption[activeLineIndexMain], lineWidth + 1.0, 11.0, 56.0 + activeLineIndexMain * 36.0, 0.0, 7.0, true, false);
			
			update_features();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		// process buttons
		bool bSelect, bBack, bUp, bDown;
		get_button_state(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
		if (bSelect)
		{
			menu_beep();
			switch (activeLineIndexMain)
			{
				case 0:
					process_player_menu();					
					break;
				case 1:
					process_weapon_menu();
					break;
				case 2:
					process_veh_menu();
					break;
				case 3:
					process_world_menu();
					break;
				case 4:
					process_time_menu();
					break;
				case 5:
					process_weather_menu();
					break;
				case 6:
					process_misc_menu();
					break;
			}
			waitTime = 200;
		} else
		if (bBack || trainer_switch_pressed())
		{
			menu_beep();
			break;
		} else
		if (bUp)
		{
			menu_beep();
			if (activeLineIndexMain == 0) 
				activeLineIndexMain = lineCount;
			activeLineIndexMain--;
			waitTime = 150;
		} else
		if (bDown)
		{
			menu_beep();
			activeLineIndexMain++;
			if (activeLineIndexMain == lineCount) 
				activeLineIndexMain = 0;			
			waitTime = 150;
		}
	}
}

void reset_globals()
{
	reset_skin_globals();

	reset_vehicle_globals();

	reset_teleporter_globals();

	activeLineIndexMain			=
	activeLineIndexPlayer		=
	activeLineIndexWeapon		=
	activeLineIndexWorld		=
	activeLineIndexWeather		=	0;

	featurePlayerInvincible			=
	featurePlayerInvincibleUpdated	=
	featurePlayerNeverWanted		=
	featurePlayerIgnored			=
	featurePlayerIgnoredUpdated		=
	featurePlayerUnlimitedAbility	=
	featurePlayerNoNoise			=
	featurePlayerNoNoiseUpdated		=
	featurePlayerFastSwim			=
	featurePlayerFastSwimUpdated	=
	featurePlayerFastRun			=
	featurePlayerFastRunUpdated		=
	featurePlayerSuperJump			=
	featureWeaponNoReload			=
	featureWeaponUnlimAmmoA			=
	featureWeaponUnlimAmmoB			=
	featureWeaponFireAmmo			=
	featureWeaponExplosiveAmmo		=
	featureWeaponExplosiveMelee		=
	featureWeaponVehRockets			=
	featureWorldMoonGravity			=
	featureTimePaused				=
	featureTimePausedUpdated		=
	featureTimeSynced				=
	featureWeatherWind				=
	featureMiscLockRadio			=
	featureMiscHideHud				=	false;

	featureWorldRandomCops		=
	featureWorldRandomTrains	=
	featureWorldRandomBoats		=
	featureWorldGarbageTrucks	=	true;

	set_custom_skin_applied(false);
}

void main()
{	
	reset_globals();

	set_periodic_feature_call(update_features);

	while (true)
	{
		if (trainer_switch_pressed())
		{
			reset_trainer_switch();
			process_main_menu();
			DWORD time = GetTickCount() + 1000;
			while (GetTickCount() < time)
			{
				update_features();
				WAIT(0);
			}
			reset_trainer_switch();
		}
		update_features();

		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}