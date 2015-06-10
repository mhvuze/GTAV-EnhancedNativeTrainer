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

#include "keyboard.h"

bool get_key_pressed(int nVirtKey);

void get_button_state(bool *a, bool *b, bool *up, bool *down, bool *l, bool *r);

bool trainer_switch_pressed();

void reset_trainer_switch();

bool airbrake_switch_pressed();
