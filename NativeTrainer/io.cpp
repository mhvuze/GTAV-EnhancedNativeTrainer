/*
Some of this code began its life as a part of GTA V SCRIPT HOOK SDK.
http://dev-c.com
(C) Alexander Blade 2015

It is now part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#include "io.h"

DWORD trainerResetTime = 0;

bool get_key_pressed(int nVirtKey)
{
	//return (GetKeyState(nVirtKey) & 0x8000) != 0;
	return (GetAsyncKeyState(nVirtKey) & 0x8000) != 0;
}

void get_button_state(bool *a, bool *b, bool *up, bool *down, bool *l, bool *r)
{
	if (a) *a = get_key_pressed(VK_NUMPAD5);
	if (b) *b = trainer_switch_pressed() || get_key_pressed(VK_BACK);
	if (up) *up = get_key_pressed(VK_NUMPAD8);
	if (down) *down = get_key_pressed(VK_NUMPAD2);
	if (r) *r = get_key_pressed(VK_NUMPAD6);
	if (l) *l = get_key_pressed(VK_NUMPAD4);
}

bool trainer_switch_pressed()
{
	return (GetTickCount() > trainerResetTime + 1000) && get_key_pressed(VK_F4);
}

void reset_trainer_switch()
{
	trainerResetTime = GetTickCount();
}