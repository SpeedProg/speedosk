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

#ifndef SPEEDOSKBOARD_H_
#define SPEEDOSKBOARD_H_
#include <string>
#include <X11/Xlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#define OSK_WIDTH 300
#define OSK_HEIGHT 300

struct keyInfo {
	int keyCode;
	std::string label;
};
class SpeedOSKBoard {
public:
	SpeedOSKBoard();
	virtual ~SpeedOSKBoard();
	int Draw();
	void ChangeZone(int zone);
	void ButtonChanged(int button, bool down);
	void ChangeLevel(int level);
	void ToggleVisibility();
private:
	static const constexpr SDL_Color font_color_default = {255, 0, 255};
	static const constexpr SDL_Color font_color_quadrant = {0, 255, 0};
	static const constexpr SDL_Color font_color_pressed = {0, 0, 255};
	SDL_Renderer *renderer;
	SDL_Window *window;
	TTF_Font *font;
	char key_table[1][37] = {
					"abcdefghijklmnopqrstuvwxyz@#$%&*(){}" //36
	};
	keyInfo key_table_exp[3][9][4];
	int currentlevel;
	int currentzone;
	bool zonechanged;
	int lastbuttonpressed;
	bool button_is_down;
	Display *display;
	int keyCodeShift;
	int keyCodeAlt;
	int keyCodeCtrl;
	bool windowVisible;
	void DrawChar(const char *c, short quadrant, short inner);
	void LoadCharmap();
};

#endif /* SPEEDOSKBOARD_H_ */
