// rendergl.cpp: core opengl rendering stuff

// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"

// Begin Intel Corporation code
#ifdef _WIN32_WCE
#include "t2700G.h"
#endif /* _WIN32_WCE */
// End Intel Corporation code

#ifdef DARWIN
#define GL_COMBINE_EXT GL_COMBINE_ARB
#define GL_COMBINE_RGB_EXT GL_COMBINE_RGB_ARB
#define GL_SOURCE0_RBG_EXT GL_SOURCE0_RGB_ARB
#define GL_SOURCE1_RBG_EXT GL_SOURCE1_RGB_ARB
#define GL_RGB_SCALE_EXT GL_RGB_SCALE_ARB
#endif

// Begin Intel Corporation code
#ifdef _WIN32_WCE
GLfixed modelview[16];
GLfixed projection[16];
vector<GLfixed *>modelstack;
vector<GLfixed *>projstack;
string texturenames[1000]; // there has to be a better way to do this
#endif /* _WIN32_WCE */
// End Intel Corporation code

extern int curvert;

bool hasoverbright = false;

#ifndef _WIN32_WCE
// No OpenGL ES port of GLU was available, so no spheres
GLUquadric *qsphere = NULL;
#endif /* _WIN32_WCE */

void purgetextures();

int glmaxtexsize = 256;

void gl_init(int w, int h)
{
    //#define fogvalues 0.5f, 0.6f, 0.7f, 1.0f

    glViewport(0, 0, w, h);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	glClearDepthx(ONE_FX);
#else // End Intel Corporation code
    glClearDepth(1.0);
#endif /* _WIN32_WCE */

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_FOG);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
    glFogx(GL_FOG_MODE, GL_LINEAR);
    glFogx(GL_FOG_DENSITY, f2x(0.25f));
#else // End Intel Corporation code
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_DENSITY, 0.25);
#endif /* _WIN32_WCE */

    glHint(GL_FOG_HINT, GL_NICEST);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// OGL ES 1.0 does not support GL_POLYGON_OFFSET_LINE
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffsetx(-THREE_FX, -THREE_FX);
#else // End Intel Corporation code
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-3.0, -3.0);
#endif /* _WIN32_WCE */

    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);

    char *exts = (char *)glGetString(GL_EXTENSIONS);
    
    if(strstr(exts, "GL_EXT_texture_env_combine")) hasoverbright = true;
    else conoutf("WARNING: cannot use overbright lighting, using old lighting model!");
        
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glmaxtexsize);
        
    purgetextures();

#ifndef _WIN32_WCE
	// No GLU implementation for OGL ES, so no spheres
    if(!(qsphere = gluNewQuadric())) fatal("glu sphere");
    gluQuadricDrawStyle(qsphere, GLU_FILL);
    gluQuadricOrientation(qsphere, GLU_INSIDE);
    gluQuadricTexture(qsphere, GL_TRUE);
    glNewList(1, GL_COMPILE);
    gluSphere(qsphere, 1, 12, 6);
    glEndList();
#endif /* _WIN32_WCE */
};

void cleangl()
{
#ifndef _WIN32_WCE
    if(qsphere) gluDeleteQuadric(qsphere);
#endif /* _WIN32_WCE */
};

// Begin Intel Corporation code
#ifdef _WIN32_WCE

// Special helper class for storing rgb values during mipmapping
class Texel 
{ 
public:
	GLubyte r, g, b;
	
	Texel(GLubyte red, GLubyte green, GLubyte blue) { r = red; g = green; b = blue; }
	Texel() { r = 0; g = 0; b = 0; }
};

// Get an rgb texel from the memory pointer at the offset given, 
// wont work for rgba textures unless modified
__inline Texel GetTexel(const GLubyte *s, int offset) { return Texel(s[offset], s[offset+1], s[offset+2]); };

// Average of four mipmap algorithm for square textures
void makeMipMaps(SDL_Surface *parentTexture)
{
	// drop out if not a square texture
	if(parentTexture->w != parentTexture->h)
	{
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //NEAREST);
		glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, parentTexture->w, parentTexture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, parentTexture->pixels);
		return;
	}
	else
	{
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // mips on
		glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	}

	// algorithm adapted from Vincent 3D Rendering Library
	// original code by Hans-Martin Will
	if (parentTexture->w != 1 && parentTexture->h != 1) 
	{
		unsigned int width = parentTexture->w;
		unsigned int height= parentTexture->h;
		int level = 0;

		// destroy original to save memory
		GLubyte *texture = (GLubyte *)parentTexture->pixels;
		//GLubyte *texture = new GLubyte[width * height * 3];

		/* necessary if not destroying original
		// make a normal texture if memory is not sufficient for mipmapping
		if(!texture)
		{
			glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //NEAREST);
			glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, parentTexture->w, parentTexture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, parentTexture->pixels);
			return;
		}*/

		// had some issues with swapping, first reduction as base texture
		//memcpy(texture, pixels, width * height * 3);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

		while (width > 1 || height > 1) 
		{
			width = width >> 1;
			height = height >> 1;

			// if no issues with swapping
			// ++level;

			for(unsigned int row = 0; row < height; ++row)
			{
				for(unsigned int col = 0; col < width; ++col)
				{
					Texel p00 = GetTexel(texture, (2 * row * (width * 2) + 2 * col) * 3);
					Texel p01 = GetTexel(texture, (2 * row * (width * 2) + 2 * col + 1) * 3);
					Texel p10 = GetTexel(texture, ((2 * row + 1) * (width * 2) + 2 * col) * 3);
					Texel p11 = GetTexel(texture, ((2 * row + 1) * (width * 2) + 2 * col + 1) * 3);

					Texel result =
						Texel((p00.r + p01.r + p10.r + p11.r) / 4,
							  (p00.g + p01.g + p10.g + p11.g) / 4,
							  (p00.b + p01.b + p10.b + p11.b) / 4);

					texture[(row * width + col)*3] = result.r;
					texture[(row * width + col)*3+1] = result.g;
					texture[(row * width + col)*3+2] = result.b;								   
				}
			}
			glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
			
			// issues with swapping
			++level;
		}
		//delete [] texture;
	}
};
#endif /* _WIN32_WCE */
// End Intel Corporation code

bool installtex(int tnum, char *texname, int &xs, int &ys, bool clamp)
{
    SDL_Surface *s = IMG_Load(texname);
    if(!s) { conoutf("couldn't load texture %s", (int)texname); return false; };
    if(s->format->BitsPerPixel!=24) { conoutf("texture must be 24bpp: %s", (int)texname); return false; };
    // loopi(s->w*s->h*3) { uchar *p = (uchar *)s->pixels+i; *p = 255-*p; };
    glBindTexture(GL_TEXTURE_2D, tnum);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifndef _WIN32_WCE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
#endif /* _WIN32_WCE */

    xs = s->w;
    ys = s->h;
    while(xs>glmaxtexsize || ys>glmaxtexsize) { xs /= 2; ys /= 2; };
    
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	makeMipMaps(s);

	strcpy(texturenames[tnum], texname);
#else // End Intel Corporation code
	void *scaledimg = s->pixels;
    if(xs!=s->w)
    {
        conoutf("warning: quality loss: scaling %s", (int)texname);     // for voodoo cards under linux
        scaledimg = alloc(xs*ys*3);
        gluScaleImage(GL_RGB, s->w, s->h, GL_UNSIGNED_BYTE, s->pixels, xs, ys, GL_UNSIGNED_BYTE, scaledimg);
    };

	if(gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, xs, ys, GL_RGB, GL_UNSIGNED_BYTE, s->pixels)) fatal("could not build mipmaps");
    if(xs!=s->w) free(scaledimg);
#endif /* _WIN32_WCE */

    SDL_FreeSurface(s);
    return true;
};

// management of texture slots
// each texture slot can have multiple texture frames, of which currently only the first is used
// additional frames can be used for various shaders

const int MAXTEX = 1000;
int texx[MAXTEX];                           // ( loaded texture ) -> ( name, size )
int texy[MAXTEX];                           
string texname[MAXTEX];
int curtex = 0;
const int FIRSTTEX = 100;                   // opengl id = loaded id + FIRSTTEX
// std 1+, sky 14+, mdls 20+

const int MAXFRAMES = 2;                    // increase to allow more complex shader defs
int mapping[256][MAXFRAMES];                // ( cube texture, frame ) -> ( opengl id, name )
string mapname[256][MAXFRAMES];

void purgetextures()
{
    loopi(256) loop(j,MAXFRAMES) mapping[i][j] = 0;
};

int curtexnum = 0;

void texturereset() { curtexnum = 0; };

// Begin Intel Corporation code
#ifdef _WIN32_WCE
void reloadtextures()
{
	// reload default GL states
	glClearDepthx(ONE_FX);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
      
    glEnable(GL_FOG);
    glFogx(GL_FOG_MODE, GL_LINEAR);
    glFogx(GL_FOG_DENSITY, f2x(0.25f));

    glHint(GL_FOG_HINT, GL_NICEST);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffsetx(-THREE_FX, -THREE_FX);

	glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	int xs, ys;

	// delete references to previous textures
	for(unsigned int j = 0; j < MAXTEX; j++)
		glDeleteTextures(1, &j);

	// reload textures
	for(int i = 0; i < MAXTEX; i++)
	{
		if(strcmp(texturenames[i], "") != 0)
			installtex(i, texturenames[i], xs, ys);
	}
}
#endif /* _WIN32_WCE */
// End Intel Corporation code

void texture(char *aframe, char *name)
{
    int num = curtexnum++, frame = atoi(aframe);
    if(num<0 || num>=256 || frame<0 || frame>=MAXFRAMES) return;
    mapping[num][frame] = 1;
    char *n = mapname[num][frame];
    strcpy_s(n, name);
    path(n);
};

COMMAND(texturereset, ARG_NONE);
COMMAND(texture, ARG_2STR);

int lookuptexture(int tex, int &xs, int &ys)
{
    int frame = 0; // other frames?
    int tid = mapping[tex][frame];

    if(tid>=FIRSTTEX)
    {
        xs = texx[tid-FIRSTTEX];
        ys = texy[tid-FIRSTTEX];
        return tid;
    };

    xs = ys = 16;
    if(!tid) return 1;                  // crosshair :)

    loopi(curtex)       // lazily happens once per "texture" command, basically
    {
        if(strcmp(mapname[tex][frame], texname[i])==0)
        {
            mapping[tex][frame] = tid = i+FIRSTTEX;
            xs = texx[i];
            ys = texy[i];
            return tid;
        };
    };

    if(curtex==MAXTEX) fatal("loaded too many textures");

    int tnum = curtex+FIRSTTEX;
    strcpy_s(texname[curtex], mapname[tex][frame]);

    sprintf_sd(name)("packages%c%s", PATHDIV, texname[curtex]);

    if(installtex(tnum, name, xs, ys))
    {
        mapping[tex][frame] = tnum;
        texx[curtex] = xs;
        texy[curtex] = ys;
        curtex++;
        return tnum;
    }
    else
    {
        return mapping[tex][frame] = FIRSTTEX;  // temp fix
    };
};

void setupworld()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    setarraypointers();

    if(hasoverbright)
    {
		// not supported in OGL ES 1.0
#ifndef _WIN32_WCE
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); 
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
#endif /* _WIN32_WCE */
    };
};

int skyoglid;

struct strip { int tex, start, num; };
vector<strip> strips;
vector<vertex> vertlist;

__inline int striptexsort(const void *arg1, const void *arg2)
{
	strip *s = (strip*)arg1;
	strip *r = (strip*)arg2;
	if(s->tex == r->tex)
		return 0;
	else if(s->tex > r->tex)
		return 1;
	else
		return -1;
}

void renderstripssky()
{
    glBindTexture(GL_TEXTURE_2D, skyoglid);
    //loopv(strips) 

	for(int i = strips.length()-1; i>=0; i--)
	{
		if(strips[i].tex==skyoglid) 
			glDrawArrays(GL_TRIANGLE_STRIP, strips[i].start, strips[i].num);
	}
};

void renderstrips()
{
	setupworld();
    int lasttex = -1;

    loopv(strips) if(strips[i].tex!=skyoglid)
    {
        if(strips[i].tex!=lasttex)
        {
            glBindTexture(GL_TEXTURE_2D, strips[i].tex); 
            lasttex = strips[i].tex;
        };
        glDrawArrays(GL_TRIANGLE_STRIP, strips[i].start, strips[i].num);  
    }; 
};

void overbright(float amount) 
{
#ifndef _WIN32_WCE
	// not supported in OGL ES 1.0
	if(hasoverbright) 
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, amount );
#endif /* _WIN32_WCE */
};

void addstrip(int tex, int start, int n)
{
    strip &s = strips.add();
    s.tex = tex;
    s.start = start;
    s.num = n;
};

/*VARF(gamma, 30, 100, 300,
{
    float f = gamma/100.0f;
    if(SDL_SetGamma(f,f,f)==-1)
    {
        conoutf("Could not set gamma (card/driver doesn't support it?)");
        conoutf("sdl: %s", (int)SDL_GetError());
    };
});*/

void transplayer()
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	identity(modelview);
	rotate(modelview,f2x(player1->roll),0,0,ONE_FX);
	rotate(modelview,f2x(player1->pitch),i2x(-1),0,0);
	rotate(modelview,f2x(player1->yaw),0,ONE_FX,0);

	if(player1->state == CS_DEAD)
		translate(modelview, f2x(-player1->o.x), f2x(player1->eyeheight-0.2f-player1->o.z), f2x(-player1->o.y));
	else
		translate(modelview, f2x(-player1->o.x), f2x(-player1->o.z), f2x(-player1->o.y));
#else // End Intel Corporation code
	glLoadIdentity();
    glRotated(player1->roll,0.0,0.0,1.0);
    glRotated(player1->pitch,-1.0,0.0,0.0);
    glRotated(player1->yaw,0.0,1.0,0.0);
    glTranslated(-player1->o.x, (player1->state==CS_DEAD ? player1->eyeheight-0.2f : 0)-player1->o.z, -player1->o.y);
#endif /* _WIN32_WCE */
};

VAR(fov, 10, 105, 120);

int xtraverts;

VAR(fog, 64, 180, 1024);
VAR(fogcolour, 0, 0x8099B3, 0xFFFFFF);

VAR(hudgun,0,1,1);

char *hudgunnames[] = { "hudguns/fist", "hudguns/shotg", "hudguns/chaing", "hudguns/rocket", "hudguns/rifle" };

void drawhudmodel(int start, int end, float speed, int base)
{
    rendermodel(hudgunnames[player1->gunselect], start, end, 0, 1.0f, player1->o.x, player1->o.z, player1->o.y, player1->yaw+90, player1->pitch, false, 1.0f, speed, 0, base);
};

void drawhudgun(float fovy, float aspect, int farplane)
{
    if(!hudgun /*|| !player1->gunselect*/) return;
    
    glEnable(GL_CULL_FACE);
    
    glMatrixMode(GL_PROJECTION);
    
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	identity(projection);
	GLfloat range = 0.3f*(float)tan((PI*fovy)/360.0f);
	frust(projection, f2x(-range*aspect),f2x(range*aspect),f2x(-range),f2x(range),f2x(0.3f),i2x(farplane));
#else // End Intel Corporation code
	glLoadIdentity();
	GLfloat range = 0.3f*(float)tan((PI*fovy)/360.0f);
	glFrustum(-range*aspect,range*aspect,-range,range,0.3f,farplane);
#endif /* _WIN32_WCE */
    glMatrixMode(GL_MODELVIEW);
    
    //glClear(GL_DEPTH_BUFFER_BIT);
    int rtime = reloadtime(player1->gunselect);
    if(player1->lastattackgun==player1->gunselect && lastmillis-player1->lastaction<rtime)
    {
        drawhudmodel(7, 18, rtime/18.0f, player1->lastaction);
    }
    else
    {
        drawhudmodel(6, 1, 100, 0);
    };

    glMatrixMode(GL_PROJECTION);
    
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	identity(projection);
	range = 0.15f*(float)tan((PI*fovy)/360.0f);
	frust(projection, f2x(-range*aspect),f2x(range*aspect),f2x(-range),f2x(range),f2x(0.15f),i2x(farplane));
#else // End Intel Corporation code
	glLoadIdentity();
	range = 0.15f*(float)tan((PI*fovy)/360.0f);
	glFrustum(-range*aspect,range*aspect,-range,range,0.15f,farplane);
#endif /* _WIN32_WCE */

    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_CULL_FACE);
};

void gl_drawframe(int w, int h, float changelod, float curfps)
{
    float hf = hdr.waterlevel-0.3f;
    float fovy = (float)fov*h/w;
    float aspect = w/(float)h;
    bool underwater = player1->o.z<hf;

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// fog changed from game default to make far plane look better on PDA screen
	glFogx(GL_FOG_START, i2x((fog+64)/8));
	glFogx(GL_FOG_END, i2x(fog/3));
	GLfixed fogc[4] = { 0, 0, 0, ONE_FX };
	//GLfixed fogc[4] = { f2x((fogcolour>>16)/256.0f), f2x(((fogcolour>>8)&255)/256.0f), f2x((fogcolour&255)/256.0f), ONE_FX};
	glFogxv(GL_FOG_COLOR, fogc);
	glClearColorx(fogc[0], fogc[1], fogc[2], ONE_FX);
#else // End Intel Corporation code
    glFogi(GL_FOG_START, (fog+64)/8);
    glFogi(GL_FOG_END, fog);
    float fogc[4] = { (fogcolour>>16)/256.0f, ((fogcolour>>8)&255)/256.0f, (fogcolour&255)/256.0f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogc);
    glClearColor(fogc[0], fogc[1], fogc[2], 1.0f);
#endif /* _WIN32_WCE */

    if(underwater)
    {
        fovy += (float)sin(lastmillis/1000.0)*2.0f;
        aspect += (float)sin(lastmillis/1000.0+PI)*0.1f;

// Begin Intel Corporation code
#ifdef _WIN32_WCE
        glFogx(GL_FOG_START, 0);
        glFogx(GL_FOG_END, i2x((fog+96)/8));
#else // End Intel Corporation code
        glFogi(GL_FOG_START, 0);
        glFogi(GL_FOG_END, (fog+96)/8);
#endif /* _WIN32_WCE */
    };
    
    glClear((player1->outsidemap ? GL_COLOR_BUFFER_BIT : 0) | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    
    int farplane = fog*5/2;

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	identity(projection);
	GLfloat range = 0.15f*(float)tan((PI*fovy)/360.0f);

	// frustum can replace gluPerspective
	frust(projection, f2x(-range*aspect),f2x(range*aspect),f2x(-range),f2x(range),f2x(0.15f),i2x(farplane));
#else // End Intel Corporation code
	glLoadIdentity();
	GLfloat range = 0.15f*(float)tan((PI*fovy)/360.0f);
	glFrustum(-range*aspect,range*aspect,-range,range,0.15f,farplane);

    //gluPerspective(fovy, aspect, 0.15f, (float)farplane);
#endif /* _WIN32_WCE */
    
    glMatrixMode(GL_MODELVIEW);

    transplayer();

    glEnable(GL_TEXTURE_2D);
    
    int xs, ys;
    skyoglid = lookuptexture(DEFAULT_SKY, xs, ys);
   
    resetcubes();
            
    curvert = 0;
    strips.setsize(0);
 
	render_world(player1->o.x, player1->o.y, player1->o.z, changelod,
		(int)player1->yaw, (int)player1->pitch, (float)fov, w, h);

	finishstrips();

	//strips.sort(striptexsort);

    setupworld();

    renderstripssky();

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	identity(modelview);
	rotate(modelview, f2x(player1->pitch), i2x(-1), 0, 0);
	rotate(modelview, f2x(player1->yaw),   0, ONE_FX, 0);
	rotate(modelview, i2x(90), ONE_FX, 0, 0);
    glColor4x(ONE_FX, ONE_FX, ONE_FX, ONE_FX);
#else // End Intel Corporation code
	glLoadIdentity();
    glRotated(player1->pitch, -1.0, 0.0, 0.0);
    glRotated(player1->yaw,   0.0, 1.0, 0.0);
    glRotated(90.0, 1.0, 0.0, 0.0);
    glColor3f(1.0f, 1.0f, 1.0f);
#endif /* _WIN32_WCE */

    glDisable(GL_FOG);
    glDepthFunc(GL_GREATER);
    draw_envbox(14, fog*4/3);
    glDepthFunc(GL_LESS);
    glEnable(GL_FOG);

    transplayer();
        
    overbright(2);
    
    renderstrips();

    xtraverts = 0;

    renderclients();
    monsterrender();

    renderentities();

    renderspheres(curtime);
    renderents();

    glDisable(GL_CULL_FACE);

    drawhudgun(fovy, aspect, farplane);

    overbright(1);
    int nquads = renderwater(hf);
    
    overbright(2);
    render_particles(curtime);
    overbright(1);

    glDisable(GL_FOG);

    glDisable(GL_TEXTURE_2D);

    gl_drawhud(w, h, (int)curfps, nquads, curvert, underwater);

    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
};

