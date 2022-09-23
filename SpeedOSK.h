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

#ifndef SPEEDOSK_H_
#define SPEEDOSK_H_
#include <SDL2/SDL.h>

class SpeedOSK {
public:
	SpeedOSK();
	virtual ~SpeedOSK();
	int Run();
private:
	SDL_Joystick *stick;
	bool level_mode_steps;
	void InitSDL();
	void SetupJoystick();
	void HandleEvents();
	void LoadSettings();
};

#endif /* SPEEDOSK_H_ */
