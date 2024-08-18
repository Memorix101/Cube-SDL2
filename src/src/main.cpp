// main.cpp: initialisation & main loop

#include "cube.h"

void cleanup(char *msg)         // single program exit point;
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

void fatal(char *s, char *o)    // failure exit
{
	sprintf_sd(msg)("%s%s (%s)\n", s, o, SDL_GetError());
	cleanup(msg);
};

void *alloc(int s)              // for some big chunks... most other allocs use the memory pool
{
	void *b = calloc(1, s);
	if (!b) fatal("out of memory!");
	return b;
};

int scr_w = 640;
int scr_h = 480;

void screenshot()
{
	SDL_Surface *image;
	SDL_Surface *temp;
	int idx;
	if (image = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
	{
		if (temp = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
		{
			glReadPixels(0, 0, scr_w, scr_h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
			for (idx = 0; idx < scr_h; idx++)
			{
				char *dest = (char *)temp->pixels + 3 * scr_w*idx;
				memcpy(dest, (char *)image->pixels + 3 * scr_w*(scr_h - 1 - idx), 3 * scr_w);
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

int main(int argc, char **argv)
{
	bool dedicated = false;
	int fs = SDL_WINDOW_FULLSCREEN, par = 0, uprate = 0, maxcl = 4;

	char *sdesc = "", *ip = "", *master = NULL, *passwd = "";
	islittleendian = *((char *)&islittleendian);

#define log(s) conoutf("init: %s", s)
	log("sdl");

	for (int i = 1; i < argc; i++)
	{
		char *a = &argv[i][2];
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
	SDL_Window *mainWindow = SDL_CreateWindow("Cube SDL2 ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scr_w, scr_h, fs | SDL_WINDOW_OPENGL);

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

	SDL_Event event;

	//setup controller
	SDL_Joystick *joy;
	if (SDL_Init(SDL_INIT_JOYSTICK) == -1)
		conoutf("Joystick not initialized");

	// Check for joystick
	if (SDL_NumJoysticks() > 0) {
		// Open joystick
		joy = SDL_JoystickOpen(0);

		if (joy)
		{
			conoutf("Opened Joystick");
			conoutf("Name: %s", SDL_JoystickNameForIndex(0));
			conoutf("Number of Axes: %d", SDL_JoystickNumAxes(joy));
			conoutf("Number of Buttons: %d", SDL_JoystickNumButtons(joy));
			conoutf("Number of Balls: %d", SDL_JoystickNumBalls(joy));
		}
		else
			conoutf("Couldn't open Joystick");

		// Close if opened
		/*if (SDL_JoystickOpened(0))
		SDL_JoystickClose(joy);*/

		SDL_JoystickEventState(SDL_ENABLE);
	}

	for (;;)
	{
		int millis = SDL_GetTicks()*gamespeed / 100;
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

			case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */
				joypress(event.jbutton.button, event.jbutton.state == SDL_PRESSED, event.jbutton.button);
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				keypress(event.key.keysym.sym, event.key.state == SDL_PRESSED, event.key.keysym.sym);
				break;

			case SDL_MOUSEMOTION:
				if (ignore) { ignore--; break; };
				mousemove(event.motion.xrel, event.motion.yrel);
				conoutf("event.motion.xrel: %d", event.motion.xrel);
				conoutf("event.motion.yrel: %d", event.motion.yrel);
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
	};
	quit();
	return 1;
};


