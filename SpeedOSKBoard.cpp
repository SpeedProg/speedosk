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
#include <string>
#include "SpeedOSKBoard.h"
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include "Config.h"

SpeedOSKBoard::SpeedOSKBoard() {
	this->currentlevel = 0;
	this->currentzone = 4;
	this->zonechanged = true;
	this->lastbuttonpressed = 5;
	this->button_is_down = false;
	this->windowVisible = true;
	this->display = NULL;
	this->LoadCharmap();

	int rendererFlags = SDL_RENDERER_ACCELERATED;
	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(0, &mode);
	this->window = SDL_CreateWindow("SpeedProgOSK", mode.w-OSK_WIDTH, mode.h-OSK_HEIGHT, OSK_WIDTH, OSK_HEIGHT, SDL_WINDOW_BORDERLESS);
	if (!this->window)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to open %d x %d window: %s\n", OSK_WIDTH, OSK_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetWindowAlwaysOnTop(this->window, SDL_TRUE);
	if(TTF_Init() < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Could not init ttf: %s\n", SDL_GetError());
		exit(1);
	}

	if(SDL_SetWindowOpacity(this->window, 0.5f) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set opacity on window: %s\n", SDL_GetError());
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	this->renderer = SDL_CreateRenderer(this->window, -1, rendererFlags);
	if (!this->renderer)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	this->font = TTF_OpenFont(INSTALL_PREFIX "/share/speedosk/LiberationSans-Bold.ttf", OSK_HEIGHT/9);
	if(!this->font) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s\n", SDL_GetError());
		exit(1);
	}

	this->display = XOpenDisplay(NULL);
	if(!this->display) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to open Xorg Display for sending keyboard events");
		exit(1);
	}

}

SpeedOSKBoard::~SpeedOSKBoard() {
	if(this->button_is_down){
		this->ButtonChanged(this->lastbuttonpressed, false);
	}
	if (this->renderer)
		SDL_DestroyRenderer(this->renderer);
	if (this->window)
		SDL_DestroyWindow(this->window);
	if (this->font)
		TTF_CloseFont(font);
	TTF_Quit();
	if (this->display)
		XCloseDisplay(this->display);
}

int SpeedOSKBoard::Draw() {
	if (!this->zonechanged)
		return 0;
	int ret;
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(this->renderer);
	SDL_SetRenderDrawColor(this->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	ret = SDL_RenderDrawLine(this->renderer, 0, 0, OSK_WIDTH, 0);
	ret = SDL_RenderDrawLine(this->renderer, 0, 0, 0, OSK_HEIGHT-1);
	ret = SDL_RenderDrawLine(this->renderer, 0, OSK_HEIGHT-1, OSK_WIDTH-1, OSK_HEIGHT-1);
	ret = SDL_RenderDrawLine(this->renderer, OSK_WIDTH-1, OSK_HEIGHT-1, OSK_WIDTH-1, 0);

	ret = SDL_RenderDrawLine(this->renderer, OSK_WIDTH/3, OSK_HEIGHT, OSK_WIDTH/3, 0);
	ret = SDL_RenderDrawLine(this->renderer, OSK_WIDTH*2/3, OSK_HEIGHT, OSK_WIDTH*2/3, 0);
	ret = SDL_RenderDrawLine(this->renderer, 0, OSK_HEIGHT/3, OSK_WIDTH-1, OSK_HEIGHT/3);
	ret = SDL_RenderDrawLine(this->renderer, 0, OSK_HEIGHT*2/3, OSK_WIDTH-1, OSK_HEIGHT*2/3);
	for(int q=0; q<9; q++) {
		for(int i=0; i<4; i++) {
			this->DrawChar(this->key_table_exp[this->currentlevel][q][i].label.c_str(), q, i);
		}
	}
	SDL_RenderPresent(this->renderer);
	return ret;
}

void SpeedOSKBoard::DrawChar(const char *c, short quadrant, short inner) {
	short qy =  quadrant/3;
	short qx = quadrant%3;
	short ix = 0, iy = 0;
	switch(inner) {
	case 0:
		iy=4;
		ix=OSK_WIDTH/6;
		break;
	case 1:
		iy=OSK_HEIGHT/6;
		ix=OSK_WIDTH/3;
		break;
	case 2:
		iy=OSK_HEIGHT/3;
		ix=OSK_WIDTH/6;
		break;
	case 3:
		iy=OSK_HEIGHT/6;
		ix=4;
		break;
	default:
		iy=ix=0;
		break;
	}

	SDL_Color font_color_use;
	if (quadrant == this->currentzone)
		if (inner == lastbuttonpressed)
			font_color_use = font_color_pressed;
		else
			font_color_use = font_color_quadrant;
	else
		font_color_use = font_color_default;
	SDL_Surface *s = TTF_RenderText_Solid(this->font, c, font_color_use);
	if(!s) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to create surface %d %d for %s: %s\n", quadrant, inner, c, SDL_GetError());
		exit(1);
	}
	SDL_Texture *t = SDL_CreateTextureFromSurface(this->renderer, s);
	if(!t) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_Rect pos;
	int subx = 0, suby = 0;
	switch(inner) {
	case 0:
		subx = s->w/2;
		break;
	case 1:
		subx =  s->w + 4;
		suby = s->h/2;
		break;
	case 2:
		subx = s->w/2;
		suby = s->h + 4;
		break;
	case 3:
		subx = 0;
		suby = s->h/2;
		break;
	}
	pos.y = (OSK_HEIGHT/3) * qy + iy - suby;
	pos.x = (OSK_WIDTH/3) * qx + ix - subx;
	pos.w = s->w;
	pos.h = s->h;
	if(SDL_RenderCopy(this->renderer, t, NULL, &pos) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to create render copy: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_FreeSurface(s);
	SDL_DestroyTexture(t);
}

void SpeedOSKBoard::LoadCharmap() {
	/*
	 * Load config file
	 * (it is 36 times alternating string and keycode) x3
	 * keycodes > 1000 < 2000 => shift
	 * keycodes > 3000 < 4000 => alt
	 * keycodes > 5000 < 6000 => ctrl
	 * keycodes > 4000 < 5000 => alt+shift
	 * keycodes > 6000 < 7000 => ctrl+shift
	 * keycodes > 8000 < 9000 => ctrl+alt
	 * keycodes > 9000 < 10000 => ctrl+alt+shift
	 */
	int level = 0;
	int quadrant = 0;
	int inner = 0;
	bool first = true;
	std::string line;
	std::ifstream config("~/.config/speedosk/keymap.conf");
	if (config.fail()) {
		config  = std::ifstream(INSTALL_PREFIX "/share/speedosk/keymap_us.conf");
	}
	if (config.is_open()) {
		if(std::getline(config, line)) {
			this->keyCodeShift = std::stoi(line);
		} else {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Keymap not valid, missing shift code");
		}
		if(std::getline(config, line)) {
			this->keyCodeAlt = std::stoi(line);
		} else {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Keymap not valid, missing alt code");
		}
		if(std::getline(config, line)) {
			this->keyCodeCtrl = std::stoi(line);
		} else {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Keymap not valid, missing ctrl code");
		}
		while(std::getline(config, line)) {
			if(first) {
				this->key_table_exp[level][quadrant][inner].label = line;
			} else {
				this->key_table_exp[level][quadrant][inner].keyCode = std::stoi(line);
			}
			first = !first;
			if (first) { // count up after each pair
				inner++;
				if (inner > 3) {
					quadrant++;
					inner = 0;
				}
				if (quadrant > 8) { // there is max 9 quadrants 0..8
					level++;
					quadrant = 0;
					if (level > 2) // there is max 3 levels supported 0..2
						break;
				}
			}

		}
		if (level != 3 || quadrant != 0 || inner != 0) {
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "The keymap is not complete, filling with space key\n");
			for(int i=inner; i < 4; i++) {
				this->key_table_exp[level][quadrant][i].label = "sp";
				this->key_table_exp[level][quadrant][i].keyCode = 65;
			}
			for(int q=quadrant+1; q < 9; q++) {
				for(int i=0; i < 4; i++) {
					this->key_table_exp[level][q][i].label = "sp";
					this->key_table_exp[level][q][i].keyCode = 65;
				}
			}
			for(int l=level+1; l < 3; l++) {
				for(int q=0; q < 9; q++) {
					for(int i=0; i < 4; i++) {
						this->key_table_exp[l][q][i].label = "sp";
						this->key_table_exp[l][q][i].keyCode = 65;
					}
				}
			}
		}
	}

}

void SpeedOSKBoard::ChangeZone(int zone) {
	if (this->currentzone != zone) {
		this->ButtonChanged(this->lastbuttonpressed, false);
	}
	this->currentzone = zone;
	this->lastbuttonpressed = 5;
	SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,"Changed Zone to %d\n", this->currentzone);
}

void SpeedOSKBoard::ButtonChanged(int button, bool down) {
	if ((!this->windowVisible && down) || button < 0 || button > 3) return;
	this->lastbuttonpressed = button;
	this->button_is_down = down;
	int baseCode = this->key_table_exp[this->currentlevel][this->currentzone][button].keyCode;
	KeyCode code = baseCode;
	bool modifier_shift = false;
	bool modifier_alt = false;
	bool modifier_ctrl = false;
	/*
	 * keycodes > 1000 < 2000 => shift
	 * keycodes > 3000 < 4000 => alt
	 * keycodes > 5000 < 6000 => ctrl
	 * keycodes > 4000 < 5000 => alt+shift
	 * keycodes > 6000 < 7000 => ctrl+shift
	 * keycodes > 8000 < 9000 => ctrl+alt
	 * keycodes > 9000 < 10000 => ctrl+alt+shift
	 */
	if (baseCode > 1000 && baseCode < 2000) {
		code = baseCode - 1000;
		modifier_shift = true;
	} else if (baseCode > 3000 && baseCode < 4000) {
		code = baseCode - 3000;
		modifier_alt = true;
	} else if (baseCode > 5000 && baseCode < 6000) {
		code = baseCode - 5000;
		modifier_ctrl = true;
	} else if (baseCode > 4000 && baseCode < 5000) {
		code = baseCode - 4000;
		modifier_alt = true;
		modifier_shift = true;
	} else if (baseCode > 6000 && baseCode < 7000) {
		code = baseCode - 6000;
		modifier_ctrl = true;
		modifier_shift = true;
	} else if (baseCode > 8000 && baseCode < 9000) {
		code = baseCode - 8000;
		modifier_ctrl = true;
		modifier_alt = true;
	} else if (baseCode > 9000 && baseCode < 10000) {
		code = baseCode - 9000;
		modifier_ctrl = true;
		modifier_shift = true;
		modifier_alt = true;
	}
	if (down) {
		if (modifier_ctrl) {
			XTestFakeKeyEvent(this->display, this->keyCodeCtrl, down, 0);
		}
		if (modifier_alt) {
			XTestFakeKeyEvent(this->display, this->keyCodeAlt, down, 0);
		}
		if (modifier_shift) {
			XTestFakeKeyEvent(this->display, this->keyCodeShift, down, 0);
		}
	} else {
		if (modifier_shift) {
			XTestFakeKeyEvent(this->display, this->keyCodeShift, down, 0);
		}
		if (modifier_alt) {
			XTestFakeKeyEvent(this->display, this->keyCodeAlt, down, 0);
		}
		if (modifier_ctrl) {
			XTestFakeKeyEvent(this->display, this->keyCodeCtrl, down, 0);
		}
	}
	XTestFakeKeyEvent(this->display, code, down, 0);
	XFlush(this->display);
	std::cout << "Send " << this->key_table_exp[this->currentlevel][this->currentzone][button].label << " as " << code << " state " << (down ? "down" : "up");
}

void SpeedOSKBoard::ChangeLevel(int level) {
	if(level < 0 || level > 2 || this->currentlevel == level) return;
	this->currentlevel = level;
	this->zonechanged = true;
}

void SpeedOSKBoard::ToggleVisibility() {
	if (this->windowVisible) {
		SDL_SetWindowAlwaysOnTop(this->window, SDL_FALSE);
		SDL_HideWindow(this->window);
	} else {
		SDL_ShowWindow(this->window);
		SDL_SetWindowAlwaysOnTop(this->window, SDL_TRUE);
	}
	this->windowVisible = !this->windowVisible;
}

