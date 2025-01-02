// main.cpp: initialisation & main loop

#include "cube.h"

#define ENET_IMPLEMENTATION
#include <enet/enet.h>

SDL_GameController* controller = nullptr;

void cleanup(char* msg)         // single program exit point;
{
	stop();
	disconnect(true);
	writecfg();
	cleangl();
	cleansound();
	cleanupserver();
	SDL_ShowCursor(1);
	if (msg)
	{
#ifdef WIN32
		MessageBox(NULL, msg, "cube fatal error", MB_OK | MB_SYSTEMMODAL);
#else
		printf(msg);
#endif
	};
	SDL_Quit();
	exit(1);
};

void quit()                     // normal exit
{
	writeservercfg();
	cleanup(NULL);
};

void fatal(char* s, char* o)    // failure exit
{
	sprintf_sd(msg)("%s%s (%s)\n", s, o, SDL_GetError());
	cleanup(msg);
};

void* alloc(int s)              // for some big chunks... most other allocs use the memory pool
{
	void* b = calloc(1, s);
	if (!b) fatal("out of memory!");
	return b;
};

int scr_w = 640;
int scr_h = 480;

void screenshot()
{
	SDL_Surface* image;
	SDL_Surface* temp;
	int idx;
	if (image = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
	{
		if (temp = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
		{
			glReadPixels(0, 0, scr_w, scr_h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
			for (idx = 0; idx < scr_h; idx++)
			{
				char* dest = (char*)temp->pixels + 3 * scr_w * idx;
				memcpy(dest, (char*)image->pixels + 3 * scr_w * (scr_h - 1 - idx), 3 * scr_w);
				endianswap(dest, 3, scr_w);
			};
			sprintf_sd(buf)("screenshots/screenshot_%d.bmp", lastmillis);
			SDL_SaveBMP(temp, path(buf));
			SDL_FreeSurface(temp);
		};
		SDL_FreeSurface(image);
	};
};

COMMAND(screenshot, ARG_NONE);
COMMAND(quit, ARG_NONE);

void keyrepeat(bool on)
{
	/*
	SDL_EnableKeyRepeat(on ? SDL_DEFAULT_REPEAT_DELAY : 0,
							 SDL_DEFAULT_REPEAT_INTERVAL);
		*/
};

VARF(gamespeed, 10, 100, 1000, if (multiplayer()) gamespeed = 100);
VARP(minmillis, 0, 5, 1000);

int islittleendian = 1;
int framesinmap = 0;

SDL_Event event;
int lasttypemouse = SDL_RELEASED, lastbutmouse = -1;
int lasttypectrl = SDL_RELEASED, lastbutctrl = -1;

SDL_GameController* findController()
{
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			return SDL_GameControllerOpen(i);
		}
	}

	return nullptr;
}

void controller_loop() {

	if (!controller)
		return;

	SDL_GameControllerUpdate();

	float axes[SDL_CONTROLLER_AXIS_MAX];
	for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i) {
		axes[i] = SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)i) / 32768.0f;
	}

	unsigned char buttons[SDL_CONTROLLER_BUTTON_MAX];
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
		buttons[i] = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)i);
	}

	int sensitivity = 100; // 25
	if (axes[SDL_CONTROLLER_AXIS_RIGHTX] >= 0.5f || axes[SDL_CONTROLLER_AXIS_RIGHTX] <= -0.5f ||
		axes[SDL_CONTROLLER_AXIS_RIGHTY] >= 0.5f || axes[SDL_CONTROLLER_AXIS_RIGHTY] <= -0.5f) {
		mousemove(axes[SDL_CONTROLLER_AXIS_RIGHTX] * sensitivity, axes[SDL_CONTROLLER_AXIS_RIGHTY] * sensitivity);
	}

	// Print buttons
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
		if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)i)) {
			printf("Button %d is pressed\n", i);
		}
	}

	/*
	keypress(87, axes[SDL_CONTROLLER_AXIS_LEFTY] <= -0.5f, 0); // W
	keypress(83, axes[SDL_CONTROLLER_AXIS_LEFTY] >= 0.5f, 0); // S
	keypress(65, axes[SDL_CONTROLLER_AXIS_LEFTX] <= -0.5f, 0); // A
	keypress(68, axes[SDL_CONTROLLER_AXIS_LEFTX] >= 0.5f, 0); // D

	keypress(-0, axes[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] >= 0.5f, 0);
	keypress(-1, axes[SDL_CONTROLLER_AXIS_TRIGGERLEFT] >= 0.5f, 0);
	*/

	// For thumbsticks, the state is a value ranging from -32768 (up/left) to 32767 (down/right).
	//printf("%f\n", SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f);
	keypress(115, (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f) >= 0.5f, 0);
	keypress(119, (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f) <= -0.5f, 0);
	keypress(276, (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f) <= -0.5f, 0);
	keypress(275, (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f) >= 0.5f, 0);

	// Right Shoulder Button
	keypress(-0, buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER], 0);

	// A Button
	if (buttons[SDL_CONTROLLER_BUTTON_A]) {
		if (lasttypectrl == 0 && lastbutctrl != SDL_CONTROLLER_BUTTON_A) {
			keypress(-3, true, 0);
			menukey(13, false);
		}
		lasttypectrl = 1;
		lastbutctrl = SDL_CONTROLLER_BUTTON_A;
	}
	else if (lastbutctrl == SDL_CONTROLLER_BUTTON_A) {
		keypress(-3, false, 0);
		lasttypectrl = 0;
		lastbutctrl = -1;
	}

	// Right Shoulder Button
	//keypress(-1, buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER], 0); // shoot
	keypress(-1, (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f) >= 0.5f, 0); // shoot

	//if((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f) >= 0.5f)
		//SDL_GameControllerRumbleTriggers(controller, 0, 0xFFFF, 100); // Xbox One / Series  only

	// Back Button
	keypress(9, buttons[SDL_CONTROLLER_BUTTON_BACK], 0); // show scoreboard

	if (buttons[SDL_CONTROLLER_BUTTON_START]) { // show menu
		if (lasttypectrl == 0 && lastbutctrl != SDL_CONTROLLER_BUTTON_START) {
			keypress(27, true, 0);
		}
		lasttypectrl = 1;
		lastbutctrl = SDL_CONTROLLER_BUTTON_START;
	}
	else if (lastbutctrl == SDL_CONTROLLER_BUTTON_START) {
		keypress(27, false, 0);
		lasttypectrl = 0;
		lastbutctrl = -1;
	}

	// X Button
	if (buttons[SDL_CONTROLLER_BUTTON_X]) {
		if (lasttypectrl == 0 && lastbutctrl != SDL_CONTROLLER_BUTTON_X) {
			keypress(-2, true, 0);
		}
		lasttypectrl = 1;
		lastbutctrl = SDL_CONTROLLER_BUTTON_X;
	}
	else if (lastbutctrl == SDL_CONTROLLER_BUTTON_X) {
		keypress(-2, false, 0);
		lasttypectrl = 0;
		lastbutctrl = -1;
	}

	if (buttons[SDL_CONTROLLER_BUTTON_DPAD_UP]) {
		if (lasttypectrl == 0 && lastbutctrl != SDL_CONTROLLER_BUTTON_DPAD_UP) {
			keypress(1073741906, true, 1);
		}
		lasttypectrl = 1;
		lastbutctrl = SDL_CONTROLLER_BUTTON_DPAD_UP;
	}
	else if (lastbutctrl == SDL_CONTROLLER_BUTTON_DPAD_UP) {
		keypress(1073741906, false, 0);
		lasttypectrl = 0;
		lastbutctrl = -1;
	}


	if (buttons[SDL_CONTROLLER_BUTTON_DPAD_DOWN]) {
		if (lasttypectrl == 0 && lastbutctrl != SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
			keypress(1073741905, true, 1);
		}
		lasttypectrl = 1;
		lastbutctrl = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
	}
	else if (lastbutctrl == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
		keypress(1073741905, false, 0);
		lasttypectrl = 0;
		lastbutctrl = -1;
	}

	// DPad Buttons
	//keypress(273, buttons[SDL_CONTROLLER_BUTTON_DPAD_UP], 0);
	//keypress(274, buttons[SDL_CONTROLLER_BUTTON_DPAD_DOWN], 0);
	//keypress(276, buttons[SDL_CONTROLLER_BUTTON_DPAD_LEFT], 0);
	//keypress(275, buttons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT], 0);

	// Start Button
	//keypress(-2, buttons[SDL_CONTROLLER_BUTTON_START], 0);

	SDL_Delay(16); // Limit to ~60fps	
}

int main(int argc, char** argv)
{
	bool dedicated = false;
	int fs = SDL_WINDOW_FULLSCREEN, par = 0, uprate = 0, maxcl = 4;

	char* sdesc = "", * ip = "", * master = NULL, * passwd = "";
	islittleendian = *((char*)&islittleendian);

#define log(s) conoutf("init: %s", s)
	log("sdl");

	for (int i = 1; i < argc; i++)
	{
		char* a = &argv[i][2];
		if (argv[i][0] == '-') switch (argv[i][1])
		{
		case 'd': dedicated = true; break;
		case 't': fs = 0; break;
		case 'w': scr_w = atoi(a); break;
		case 'h': scr_h = atoi(a); break;
		case 'u': uprate = atoi(a); break;
		case 'n': sdesc = a; break;
		case 'i': ip = a; break;
		case 'm': master = a; break;
		case 'p': passwd = a; break;
		case 'c': maxcl = atoi(a); break;
		default:  conoutf("unknown commandline option");
		}
		else conoutf("unknown commandline argument");
	};

#ifdef _DEBUG
	par = SDL_INIT_NOPARACHUTE;
	fs = 0;
#endif

	//Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | par) < 0) fatal("Unable to initialize SDL");
	SDL_Window* mainWindow = SDL_CreateWindow("Cube SDL2 ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scr_w, scr_h, fs | SDL_WINDOW_OPENGL);

	log("net");
	if (enet_initialize() < 0) fatal("Unable to initialise network module");

	initclient();
	initserver(dedicated, uprate, sdesc, ip, master, passwd, maxcl);  // never returns if dedicated

	log("world");
	empty_world(7, true);

	log("video: sdl2");
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) fatal("Unable to initialize SDL Video");

	log("video: mode");
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (mainWindow == NULL) fatal("Unable to create OpenGL screen");

	log("video: misc");
	//SDL_WM_SetCaption("cube engine", NULL);
	SDL_SetWindowTitle(mainWindow, "Cube SDL2");
	// SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	keyrepeat(false);
	SDL_ShowCursor(0);

	log("gl");
	SDL_GL_CreateContext(mainWindow); // init opengl in sdl2
	gl_init(scr_w, scr_h);

	log("basetex");
	int xs, ys;
	if (!installtex(2, path(newstring("data/newchars.png")), xs, ys) ||
		!installtex(3, path(newstring("data/martin/base.png")), xs, ys) ||
		!installtex(6, path(newstring("data/martin/ball1.png")), xs, ys) ||
		!installtex(7, path(newstring("data/martin/smoke.png")), xs, ys) ||
		!installtex(8, path(newstring("data/martin/ball2.png")), xs, ys) ||
		!installtex(9, path(newstring("data/martin/ball3.png")), xs, ys) ||
		!installtex(4, path(newstring("data/explosion.jpg")), xs, ys) ||
		!installtex(5, path(newstring("data/items.png")), xs, ys) ||
		!installtex(1, path(newstring("data/crosshair.png")), xs, ys)) fatal("could not find core textures (hint: run cube from the parent of the bin directory)");

	log("sound");
	initsound();

	log("cfg");
	newmenu("frags\tpj\tping\tteam\tname");
	newmenu("ping\tplr\tserver");
	exec("data/keymap.cfg");
	exec("data/menus.cfg");
	exec("data/prefabs.cfg");
	exec("data/sounds.cfg");
	exec("servers.cfg");
	if (!execfile("config.cfg")) execfile("data/defaults.cfg");
	exec("autoexec.cfg");

	log("localconnect");
	localconnect();
	changemap("metl3");		// if this map is changed, also change depthcorrect()

	log("mainloop");
	int ignore = 5;

	// Initialize SDL game controller system
	log("controller");
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
	{
		printf("Failed to initialize GameController: %s\n", SDL_GetError());
		// return;
	}

	// Open the first available controller
	for (int i = 0; i < SDL_NumJoysticks(); ++i)
	{
		if (SDL_IsGameController(i))
		{
			controller = SDL_GameControllerOpen(i);
			if (controller)
			{
				printf("GameController opened: %s\n", SDL_GameControllerName(controller));
				break;
			}
			else
			{
				printf("Could not open GameController: %s\n", SDL_GetError());
			}
		}
	}

	unsigned char buttons[SDL_CONTROLLER_BUTTON_MAX];
	if (!controller)
	{
		printf("No game controller found.\n");
		// return;
	}
	else
	{
		for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
		{
			buttons[i] = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)i);
		}
	}

	SDL_GameControllerEventState(SDL_ENABLE);

	for (;;)
	{
		int millis = SDL_GetTicks() * gamespeed / 100;
		if (millis - lastmillis > 200) lastmillis = millis - 200;
		else if (millis - lastmillis < 1) lastmillis = millis - 1;
		if (millis - lastmillis < minmillis) SDL_Delay(minmillis - (millis - lastmillis));
		cleardlights();
		updateworld(millis);
		if (!demoplayback) serverslice((int)time(NULL), 0);
		static float fps = 30.0f;
		fps = (1000.0f / curtime + fps * 50) / 51;
		computeraytable(player1->o.x, player1->o.y);
		readdepth(scr_w, scr_h);
		// SDL_GL_SwapBuffers();
		SDL_GL_SwapWindow(mainWindow);
		extern void updatevol(); updatevol();
		if (framesinmap++ < 5)	// cheap hack to get rid of initial sparklies, even when triple buffering etc.
		{
			player1->yaw += 5;
			gl_drawframe(scr_w, scr_h, fps);
			player1->yaw -= 5;
		};
		gl_drawframe(scr_w, scr_h, fps);
		int lasttype = 0, lastbut = 0;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit();
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				keypress(event.key.keysym.sym, event.key.state == SDL_PRESSED, event.key.keysym.sym);
				break;

			case SDL_CONTROLLERDEVICEADDED:
				if (!controller)
				{
					controller = SDL_GameControllerOpen(event.cdevice.which);
				}
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				if (controller && event.cdevice.which == SDL_JoystickInstanceID(
					SDL_GameControllerGetJoystick(controller)))
				{
					SDL_GameControllerClose(controller);
					controller = findController();
				}
				break;

			case SDL_MOUSEMOTION:
				if (ignore) { ignore--; break; };
				mousemove(event.motion.xrel, event.motion.yrel);
				//conoutf("event.motion.xrel: %d", event.motion.xrel);
				//conoutf("event.motion.yrel: %d", event.motion.yrel);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (lasttype == event.type && lastbut == event.button.button) break; // why?? get event twice without it
				keypress(-event.button.button, event.button.state != 0, 0);
				lasttype = event.type;
				lastbut = event.button.button;
				break;
			};
		};
		controller_loop();
	};
	quit();
	return 1;
};


