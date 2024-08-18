#include "gamepad.h"
#include "cube.h"

#define max_controllers 4
SDL_GameController *controller[max_controllers];
SDL_Haptic *haptic[max_controllers];

bool unlock_controller = false;

void init_state(const Uint8* ks)
{
	currentKeyState = ks;
}

void add_controller(int id)
{
	SDL_GameController *ctrl = NULL;
	int num = SDL_NumJoysticks();
	const char* name;

	if (!SDL_IsGameController(id)) {
		printf("Not a controller - ID: %d\n", id);
		//return false;
	}

	ctrl = SDL_GameControllerOpen(id);
	controller[id] = ctrl;

	if (ctrl == NULL) {
		printf("Open controller failed: %s\n", SDL_GetError());
		//return false;
	}
	name = SDL_GameControllerName(ctrl);
	std::cout << "Controller: " << name << " ID: " << id << "\n";

	// open force feedback device
	if (SDL_HapticRumbleSupported(haptic[id])) {
		std::cout << "Force Feedback Supported!" << " ID: " << id << "\n";

		haptic[id] = SDL_HapticOpen(id);
		if (haptic[id] == NULL)
			printf("Open force feedback failed: %s\n", SDL_GetError());

		// Initialize simple rumble
		if (SDL_HapticRumbleInit(haptic[id]) != 0)
			printf("Init force feedback failed: %s\n", SDL_GetError());
	}
	else
	{
		std::cout << "Force Feedback Not Supported!: " << name << " ID: " << id << "\n";
	}
}

void remove_controller(int id)
{
	std::cout << "Controller has been removed! " << "\n";
	SDL_GameControllerClose(controller[id]);

	if (haptic[id] != NULL)
		SDL_HapticClose(haptic[id]);
}