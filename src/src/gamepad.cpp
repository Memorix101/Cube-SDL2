#include "cube.h"

#define max_controllers 1
SDL_GameController *controller[max_controllers];
SDL_Haptic *haptic[max_controllers];

bool unlock_controller = false;

void gametest()
{
	conoutf("test");
}

void add_controller(int id)
{
	SDL_GameController *ctrl = NULL;
	int num = SDL_NumJoysticks();
	const char* name;

	if (!SDL_IsGameController(id)) {
		conoutf("Not a controller - ID: %d", id);
		//return false;
	}

	ctrl = SDL_GameControllerOpen(id);
	controller[id] = ctrl;

	if (ctrl == NULL) {
		conoutf("Open controller failed: %s", SDL_GetError());
		//return false;
	}
	name = SDL_GameControllerName(ctrl);
	conoutf("Controller: ", name, " ID: ", id);

	// open force feedback device
	if (SDL_HapticRumbleSupported(haptic[id])) {
		conoutf("Force Feedback Supported! ID: %d", id);

		haptic[id] = SDL_HapticOpen(id);
		if (haptic[id] == NULL)
			conoutf("Open force feedback failed: %s", SDL_GetError());

		// Initialize simple rumble
		if (SDL_HapticRumbleInit(haptic[id]) != 0)
			conoutf("Init force feedback failed: %s", SDL_GetError());
	}
	else
	{
		conoutf("Force feedback is not supported!: ", name, " ID: ", id);
	}
}

void remove_controller(int id)
{
	conoutf( "Controller has been removed!");
	SDL_GameControllerClose(controller[id]);

	if (haptic[id] != NULL)
		SDL_HapticClose(haptic[id]);
}