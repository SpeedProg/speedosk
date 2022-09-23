/*
 *   SpeedOSK is a vitrual keyboard intended to be used with a controller
 *   Copyright (C) 2022 Constantin Wenger
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "SpeedOSK.h"
#include "SpeedOSKBoard.h"

SpeedOSK::SpeedOSK() {
	this->LoadSettings();
	this->InitSDL();
	this->SetupJoystick();
}

SpeedOSK::~SpeedOSK() {
	SDL_Quit();
}

int SpeedOSK::Run() {
	SpeedOSKBoard osk;
	SDL_bool done = SDL_FALSE;
	SDL_Event event;
	int ZONE_1 = 32767/2;
	int height_zone = 1; // 0 = top, 1 = middle, 2 = bottom
	int width_zone = 1; // 0 = left, 1 = moddle, 2 = right
	int current_board_level = 0;
	bool left_trigger_down = false;
	bool right_trigger_down = false;
	int lastzone = 4;
	int lwv = 1;
	int lhv = 1;
	int button_map[4] = {2, 1, 3, 0};
	Uint8 last_hat_button_down = -1;
	while(!done) {
		while(SDL_WaitEvent(&event)){
			switch(event.type) {
			case SDL_QUIT:
				done = SDL_TRUE;
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				// L1= b4, R1 = b5
				// L4 = b3, L5 = b9, R4 = b1, R5 = b10
				if (event.jbutton.button >= 0 && event.jbutton.button < 4) {
					osk.ButtonChanged(button_map[event.jbutton.button], event.jbutton.state == 1);
					SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "JBUTTON: button: %d which-j: %d button-state: %d\n", event.jbutton.button, event.jbutton.which, event.jbutton.state);
				} else if (event.jbutton.button == 10 && event.jbutton.state == 1) {
					return 0;
				} else if (event.jbutton.button == 9 && event.jbutton.state == 1) {
					osk.ToggleVisibility();
				}
				break;
			case SDL_JOYAXISMOTION:
				// axis 2 left trigger, axis 5 right trigger
				//printf("J %d, A %d, V %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
				if (event.jaxis.axis == 1 || event.jaxis.axis == 4) {
					if (abs(event.jaxis.value) < ZONE_1) { // middle based on height
						height_zone = 1;
					} else if (event.jaxis.value < 0) {
						height_zone = 0;
					} else {
						height_zone = 2;
					}
					lhv = event.jaxis.value;
				} else if(event.jaxis.axis == 0 || event.jaxis.axis == 3) {
					if (abs(event.jaxis.value) < ZONE_1) {
						width_zone = 1;
					} else if (event.jaxis.value < 0) {
						width_zone = 0;
					} else {
						width_zone = 2;
					}
					lwv = event.jaxis.value;
				} else if(event.jaxis.axis == 2) {
					if (event.jaxis.value > 10000 && !left_trigger_down) {
						left_trigger_down = true;
						if(this->level_mode_steps) {
							if(current_board_level <= 1)
								++current_board_level;
							osk.ChangeLevel(current_board_level);
						}else {
							osk.ChangeLevel(1);
						}
					} else if (event.jaxis.value <= 10000 && left_trigger_down){
						left_trigger_down = false;
						if (!this->level_mode_steps)
							osk.ChangeLevel(0);
					}
				} else if(event.jaxis.axis == 5) {
					if (event.jaxis.value > 10000 && !right_trigger_down) {
						right_trigger_down = true;
						if (this->level_mode_steps) {
							if (current_board_level >= 1)
								--current_board_level;
							osk.ChangeLevel(current_board_level);
						} else {
							osk.ChangeLevel(2);
						}
					} else if (event.jaxis.value <= 10000 && right_trigger_down) {
						right_trigger_down = false;
						if(!this->level_mode_steps)
							osk.ChangeLevel(0);
					}
				}
				if (height_zone * 3 + width_zone != lastzone) {
					lastzone = height_zone * 3 + width_zone;
					SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "JAXIS: h-zone: %d w-zone: %d and last-h-zone: %d last-w-zone: %d\n" , height_zone, width_zone, lhv, lwv);
					osk.ChangeZone(lastzone);
				}
				break;
			case SDL_JOYHATMOTION:
				/*
				*a  bit indicates a specific direction being down for the hat
				* there can be multiple down at the same time
				* for our purpose we only accept exactly one
				* 0     0     0     0
				* left  down  right up
				*/
				SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "JHAT: value: %d", event.jhat.value);
				switch(event.jhat.value) {
				case 0:
					if (last_hat_button_down == -1)
						break;
					osk.ButtonChanged((int)last_hat_button_down, false);
					last_hat_button_down = -1;
					break;
				case 1:
					osk.ButtonChanged(0, true);
					last_hat_button_down = 0;
					break;
				case 2:
					osk.ButtonChanged(1, true);
					last_hat_button_down = 1;
					break;
				case 4:
					osk.ButtonChanged(2, true);
					last_hat_button_down = 2;
					break;
				case 8:
					osk.ButtonChanged(3, true);
					last_hat_button_down = 3;
					break;
				default:
					break;
				}
				break;
				case SDL_JOYDEVICEREMOVED:
					if (this->stick && event.jdevice.which == SDL_JoystickInstanceID(this->stick)) {
						SDL_JoystickClose(this->stick);
						this->stick = NULL;
					}
					break;
				case SDL_JOYDEVICEADDED:
					if (!this->stick) {
						this->stick = SDL_JoystickOpen(event.jdevice.which);
					}
					break;
				default:
					break;
			}

			osk.Draw();
		}
	}
	return 0;
}

void SpeedOSK::InitSDL() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
}

void SpeedOSK::SetupJoystick() {
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	if(SDL_NumJoysticks() > 0){
		this->stick = SDL_JoystickOpen(0);
	} else {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "No Joystick found, exiting");
		exit(1); // we can not work without one
	}
}

void SpeedOSK::LoadSettings() {
	this->level_mode_steps = false;
	const char* home = std::getenv("HOME");
	if (home == NULL) {
		return;
	}

	std::string config_file = std::string(home) + "/.config/speedosk/config";
	std::ifstream config(config_file);

	if (config.fail()) {
		std::cerr << "Error: " << strerror(errno);
		return;
	}

	if (config.is_open()) {
		for(std::string name; std::getline(config, name, '='); ) {
			std::string value;
			if (std::getline(config, value)) {
				if (name == "level_mode") {
					if (value == "steps") {
						this->level_mode_steps = true;
					}
				}
			} else {
				SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Config file is faulty");
				break;
			}
		}
	}
}
