// renderextras.cpp: misc gl render code and the HUD

// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"

bool los(float lx, float ly, float lz, float bx, float by, float bz, vec &v, bool player = false);

void line(int x1, int y1, float z1, int x2, int y2, float z2)
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// no immediate mode in OpenGL ES
	// conversion from GL_POLYGON to GL_TRIANGLES
	GLfixed vertexarray[18] = { i2x(x1), f2x(z1), i2x(y1), // 1
							    i2x(x1), f2x(z1), f2x(y1+0.01f), // 2
							    i2x(x2), f2x(z2), f2x(y2+0.01f), // 3
							    i2x(x1), f2x(z1), i2x(y1), // 1
							    i2x(x2), f2x(z2), f2x(y2+0.01f), // 3
							    i2x(x2), f2x(z2), i2x(y2) }; // 4
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, vertexarray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableClientState(GL_VERTEX_ARRAY);	
#else // End Intel Corporation code
    glBegin(GL_POLYGON);
    glVertex3f((float)x1, z1, (float)y1);
    glVertex3f((float)x1, z1, y1+0.01f);
    glVertex3f((float)x2, z2, y2+0.01f);
    glVertex3f((float)x2, z2, (float)y2);
    glEnd();
#endif /* _WIN32_WCE */
    xtraverts += 4;
};

void linestyle(float width, int r, int g, int b)
{

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	glLineWidthx(f2x(width));
	glColor4x(i2x(r/255), i2x(g/255), i2x(b/255), ONE_FX);
#else // End Intel Corporation code
	glLineWidth(width);
    glColor3ub(r,g,b);
#endif /* _WIN32_WCE */
};

void box(block &b, float z1, float z2, float z3, float z4)
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// lines through the middle of edit boxes fix, 2x number of vertices needed
	GLfixed vertexarray[36] = { i2x(b.x), f2x(z1), i2x(b.y), // 1
								i2x((b.x+b.xs)), f2x(z2), i2x(b.y), // 2
								i2x((b.x+b.xs)), f2x(z2), f2x((b.y+0.01f)), // 2 weird
							    i2x((b.x+b.xs)), f2x(z2), i2x(b.y), // 2
								i2x((b.x+b.xs)), f2x(z3), i2x((b.y+b.ys)), // 3 
							    i2x((b.x+b.xs)), f2x(z3), f2x((b.y+b.ys+0.01f)), // 3 weird
								i2x((b.x+b.xs)), f2x(z3), i2x((b.y+b.ys)), // 3
								i2x(b.x), f2x(z4), i2x((b.y+b.ys)), // 4
								i2x(b.x), f2x(z4), f2x((b.y+b.ys+0.01f)), // 4 weird
								i2x(b.x), f2x(z4), i2x((b.y+b.ys)), // 4
								i2x(b.x), f2x(z1), i2x(b.y), // 1
								i2x(b.x), f2x(z1), f2x((b.y+0.01f)) }; // 1 weird
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, vertexarray);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableClientState(GL_VERTEX_ARRAY);

	/* lines through the middle of the boxes in edit mode, but faster than no lines
	GLfixed vertexarray[18] = { GLfixed(b.x), GLfixed(z1), GLfixed(b.y),
								GLfixed(b.x+b.xs), GLfixed(z2), GLfixed(b.y),
							    GLfixed(b.x+b.xs), GLfixed(z3), GLfixed(b.y+b.ys), 
							    GLfixed(b.x), GLfixed(z1), GLfixed(b.y),
							    GLfixed(b.x+b.xs), GLfixed(z3), GLfixed(b.y+b.ys),
							    GLfixed(b.x), GLfixed(z4), GLfixed(b.y+b.ys) }; 
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, vertexarray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableClientState(GL_VERTEX_ARRAY); */
#else // End Intel Corporation code
    glBegin(GL_POLYGON);
    glVertex3f((float)b.x,      z1, (float)b.y);
    glVertex3f((float)b.x+b.xs, z2, (float)b.y);
    glVertex3f((float)b.x+b.xs, z3, (float)b.y+b.ys);
    glVertex3f((float)b.x,      z4, (float)b.y+b.ys);
    glEnd();
#endif /* _WIN32_WCE */

    xtraverts += 4;
};

void dot(int x, int y, float z)
{
	const float DOF = 0.1f;

// Begin Intel Corporation code
#ifdef _WIN32_WCE	
	const GLfixed zz = f2x(z);
	const GLfixed xmdof = f2x(x - DOF);
	const GLfixed xpdof = f2x(x + DOF);
	const GLfixed ymdof = f2x(y - DOF);
	const GLfixed ypdof = f2x(y + DOF);

	// gets rid of the annoying middle line in the box
	GLfixed vertexarray[36] = { xmdof, zz, ymdof, // 1
								xpdof, zz, ymdof, // 2
								xpdof, zz, f2x(y-DOF+0.01f), // 2 weird
								xpdof, zz, ymdof, // 2
								xpdof, zz, ypdof, // 3
								xpdof, zz, f2x(y+DOF+0.01f), // 3 weird
								xpdof, zz, ypdof, // 3
								xmdof, zz, ypdof, // 4
								xmdof, zz, f2x(y+DOF+0.01f), // 4 weird
								xmdof, zz, ypdof, // 4
								xmdof, zz, ymdof, // 1
								xmdof, zz, f2x(y-DOF+0.01f) }; // 1 weird

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FIXED, 0, vertexarray);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableClientState(GL_VERTEX_ARRAY);
#else // End Intel Corporation code
    glBegin(GL_POLYGON);
    glVertex3f(x-DOF, (float)z, y-DOF);
    glVertex3f(x+DOF, (float)z, y-DOF);
    glVertex3f(x+DOF, (float)z, y+DOF);
    glVertex3f(x-DOF, (float)z, y+DOF);
    glEnd();
#endif /* _WIN32_WCE */
    xtraverts += 4;
};

void blendbox(int x1, int y1, int x2, int y2, bool border)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

// Begin Intel Corporation code
#ifdef _WIN32_WCE

	GLfixed xi = i2x(x1);
	GLfixed xii = i2x(x2);
	GLfixed yi = i2x(y1);
	GLfixed yii = i2x(y2);

	GLfixed vertexarray1[12] = { xi, yi,
								 xii, yi,
								 xii, yii,
								 xi, yi,
								 xii, yii,
								 xi, yii };

	GLfixed vertexarray2[16] = { xi, yi,
								 xii, yi,
								 xii, yi,
								 xii, yii,
								 xii, yii,
								 xi, yii,
								 xi, yii,
								 xi, yi };
								 	
	if(border) glColor4x(HALF_FX,f2x(0.3f),f2x(0.4f),ONE_FX);
	else glColor4x(ONE_FX,ONE_FX,ONE_FX,ONE_FX);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FIXED, 0, vertexarray1);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glColor4x(f2x(0.2f),f2x(0.7f),f2x(0.4f),ONE_FX);
	glVertexPointer(2, GL_FIXED, 0, vertexarray2);
	glDrawArrays(GL_LINES, 0, 8);

	glDisableClientState(GL_VERTEX_ARRAY);

#else // End Intel Corporation code

    glBegin(GL_QUADS);
    if(border) glColor3d(0.5, 0.3, 0.4); 
    else glColor3d(1.0, 1.0, 1.0);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    glColor3d(0.2, 0.7, 0.4); 
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#endif /* _WIN32_WCE */

    xtraverts += 8;
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
};

const int MAXSPHERES = 50;
struct sphere { vec o; float size, max; int type; sphere *next; };
sphere spheres[MAXSPHERES], *slist = NULL, *sempty = NULL;
bool sinit = false;

void newsphere(vec &o, float max, int type)
{
    if(!sinit)
    {
        loopi(MAXSPHERES)
        {
            spheres[i].next = sempty;
            sempty = &spheres[i];
        };
        sinit = true;
    };
    if(sempty)
    {
        sphere *p = sempty;
        sempty = p->next;
        p->o = o;
        p->max = max;
        p->size = 1;
        p->type = type;
        p->next = slist;
        slist = p;
    };
};

void renderspheres(int time)
{
// no spheres for PocketPC since no GLU
#ifndef _WIN32_WCE
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, 4);  

    for(sphere *p, **pp = &slist; p = *pp;)
    {
        glPushMatrix();
        float size = p->size/p->max;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f-size);
        glTranslatef(p->o.x, p->o.z, p->o.y);
        glRotatef(lastmillis/5.0f, 1, 1, 1);
        glScalef(p->size, p->size, p->size);
        glCallList(1);
        glScalef(0.8f, 0.8f, 0.8f);
        glCallList(1);
        glPopMatrix();
        xtraverts += 12*6*2;

        if(p->size>p->max)
        {
            *pp = p->next;
            p->next = sempty;
            sempty = p;
        }
        else
        {
            p->size += time/100.0f;   
            pp = &p->next;
        };
    };

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
#endif
};

string closeent;
char *entnames[] =
{
    "none?", "light", "playerstart",
    "shells", "bullets", "rockets", "riflerounds",
    "health", "healthboost", "greenarmour", "yellowarmour", "quaddamage", 
    "teleport", "teledest", 
    "mapmodel", "monster", "trigger", "jumppad",
    "?", "?", "?", "?", "?", 
};

void renderents()       // show sparkly thingies for map entities in edit mode
{
    closeent[0] = 0;
    if(!editmode) return;
    loopv(ents)
    {
        entity &e = ents[i];
        if(e.type==NOTUSED) continue;
        vec v = { e.x, e.y, e.z };
        particle_splash(2, 2, 40, v);
    };
    int e = closestent();
    if(e>=0)
    {
        entity &c = ents[e];
        sprintf_s(closeent)("closest entity = %s (%d, %d, %d, %d), selection = (%d, %d)", entnames[c.type], c.attr1, c.attr2, c.attr3, c.attr4, getvar("selxs"), getvar("selys"));
    };
};

void loadsky(char *basename)
{
    static string lastsky = "";

    if(strcmp(lastsky, basename)==0) return;
    char *side[] = { "ft", "bk", "lf", "rt", "dn", "up" };
    int texnum = 14;
    loopi(6)
    {
        sprintf_sd(name)("packages/%s_%s.jpg", basename, side[i]);
        int xs, ys;
        if(!installtex(texnum+i, path(name), xs, ys, true)) conoutf("could not load sky textures");
    };
    strcpy_s(lastsky, basename);
};

COMMAND(loadsky, ARG_1STR);

float cursordepth = 0.9f;

GLint viewport[4];

// Begin Intel Corporation code
#ifdef _WIN32_WCE
GLfixed mm[16], pm[16];
#else // End Intel Corporation code
GLdouble mm[16], pm[16];
#endif /* _WIN32_WCE */

vec worldpos;

void readmatrices()
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// OpenGL ES does not support GL_VIEWPORT, GL_MODELVIEW_MATRIX, 
	// and GL_PROJECTION_MATRIX in glGet*, so emulate it

	// this becomes a bug if viewport isnt entire screen
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = GetSystemMetrics(SM_CXSCREEN);
	viewport[3] = GetSystemMetrics(SM_CYSCREEN);

	for(int i = 0; i < 16; i++)
	{
		mm[i] = modelview[i];
		pm[i] = projection[i];
	}
#else // End Intel Corporation code
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mm);
    glGetDoublev(GL_PROJECTION_MATRIX, pm);
#endif /* _WIN32_WCE */
};

// stupid function to cater for stupid ATI drivers that return incorrect depth values

float depthcorrect(float d)
{
	static int when = 0;
	static float factor = 1;
	//if(when++==5) factor = 0.992205f/d;		// this magic value is tied to the fixed start spawn point in metl3
	return d*factor;
};

// find out the 3d target of the crosshair in the world easily and very acurately.
// sadly many very old cards and drivers appear to fuck up on gluUnProject() and give false
// coordinates, making shooting and such impossible.
// also hits map entities which is unwanted.
// could be replaced by a more acurate version of monster.cpp los() if needed

void readdepth(int w, int h)
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	/*  Generate coordinates of a point on a sphere with player 
	at the center, based on his orientation. Cast a ray along 
	this line (the crosshair), searching bounding areas of 
	world objects for any that it crosses. Find the one that 
	is the closest of those it crosses. */

	vec a;

	float pitch = (90-player1->pitch) * (PI/180);
	float yaw = (player1->yaw-90) * (PI/180);

	a.x = 1000 * (float)sin(pitch) * (float)cos(yaw);
	a.y = 1000 * (float)sin(pitch) * (float)sin(yaw);
	a.z = 1000 * (float)cos(pitch);
	
	los(player1->o.x, player1->o.y, player1->o.z, a.x, a.y, a.z, worldpos, true);

    vec r = { x2f(mm[0]), x2f(mm[4]), x2f(mm[8]) };
    vec u = { x2f(mm[1]), x2f(mm[5]), x2f(mm[9]) };
    setorient(r, u);
#else // End Intel Corporation code
    glReadPixels(w/2, h/2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursordepth);
    double worldx = 0, worldy = 0, worldz = 0;
    
	gluUnProject(w/2, h/2, depthcorrect(cursordepth), mm, pm, viewport, &worldx, &worldz, &worldy);
    worldpos.x = (float)worldx;
    worldpos.y = (float)worldy;
    worldpos.z = (float)worldz;
    vec r = { (float)mm[0], (float)mm[4], (float)mm[8] };
    vec u = { (float)mm[1], (float)mm[5], (float)mm[9] };
    setorient(r, u);
#endif /* _WIN32_WCE */
};

void drawicon(float tx, float ty, int x, int y)
{
    glBindTexture(GL_TEXTURE_2D, 5);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	GLfixed s = i2x(120);
	GLfixed o = f2x(1/3.0f);
	GLfixed ttx = f2x(tx/192);
	GLfixed tty = f2x(ty/192);
	GLfixed xi = i2x(x);
	GLfixed yi = i2x(y);

	GLfixed vertexarray[12] = { xi, yi,
								xi+s, yi,
								xi+s, yi+s,
								xi, yi,
								xi+s, yi+s,
								xi, yi+s };
	
	GLfixed texarray[12] = { ttx, tty,
							 ttx+o, tty,
							 ttx+o, tty+o,
							 ttx, tty,
							 ttx+o, tty+o,
							 ttx, tty+o };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FIXED, 0, vertexarray);
	glTexCoordPointer(2, GL_FIXED, 0, texarray);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

#else // End Intel Corporation code
    glBegin(GL_QUADS);
    tx /= 192;
    ty /= 192;
    float o = 1/3.0f;
    int s = 120;
    glTexCoord2f(tx,   ty);   glVertex2i(x,   y);
    glTexCoord2f(tx+o, ty);   glVertex2i(x+s, y);
    glTexCoord2f(tx+o, ty+o); glVertex2i(x+s, y+s);
    glTexCoord2f(tx,   ty+o); glVertex2i(x,   y+s);
    glEnd();
#endif /* _WIN32_WCE */
    xtraverts += 4;
};

void invertperspective()
{
    // This only generates a valid inverse matrix for matrices generated by gluPerspective()

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	GLfixed inv[16];

	memset(inv, 0, sizeof(inv));

	inv[0*4+0] = DivFX(ONE_FX, pm[0*4+0]);
    inv[1*4+1] = DivFX(ONE_FX, pm[1*4+1]);
    inv[2*4+3] = DivFX(ONE_FX, pm[3*4+2]);
    inv[3*4+2] = -ONE_FX;
    inv[3*4+3] = DivFX(pm[2*4+2], pm[3*4+2]);

	for(int i = 0; i < 16; i++)
		modelview[i] = inv[i];

	glLoadMatrixx(modelview);

#else // End Intel Corporation code
	GLdouble inv[16];
    memset(inv, 0, sizeof(inv));

    inv[0*4+0] = 1.0/pm[0*4+0];
    inv[1*4+1] = 1.0/pm[1*4+1];
    inv[2*4+3] = 1.0/pm[3*4+2];
    inv[3*4+2] = -1.0;
    inv[3*4+3] = pm[2*4+2]/pm[3*4+2];

    glLoadMatrixd(inv);
#endif /* _WIN32_WCE */
};

VAR(crosshairsize, 0, 15, 50);

int dblend = 0;
void damageblend(int n) { dblend += n; };

VAR(hidestats, 0, 0, 1);
VAR(crosshairfx, 0, 1, 1);

void gl_drawhud(int w, int h, int curfps, int nquads, int curvert, bool underwater)
{
    readmatrices();

    if(editmode)
    {
        if(cursordepth==1.0f) worldpos = player1->o;

// Begin Intel Corporation code
#ifdef _WIN32_WCE
		// OpenGL ES 1.0 does not support glPolygonMode
		cursorupdate();
#else // End Intel Corporation code
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        cursorupdate();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif /* _WIN32_WCE */

    };

    glDisable(GL_DEPTH_TEST);
    invertperspective();

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	pushModelMatrix();
	ortho(modelview, 0, i2x(VIRTW), i2x(VIRTH), 0, i2x(-1), ONE_FX);
#else // End Intel Corporation code
	glPushMatrix();
    glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
#endif /* _WIN32_WCE */
    glEnable(GL_BLEND);

    if(dblend || underwater)
    {
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
		GLfixed vertexarray[12] = { 0, 0,
								  i2x(VIRTW), 0,
								  i2x(VIRTW), i2x(VIRTH),
								  0, 0,
								  i2x(VIRTW), i2x(VIRTH),
								  0, i2x(VIRTH) };

		if(dblend) 
			glColor4x(0, f2x(0.9f), f2x(0.9f), ONE_FX);
        else 
			glColor4x(f2x(0.9f), HALF_FX, 0, ONE_FX);

		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FIXED, 0, vertexarray);

		glDrawArrays(GL_TRIANGLES, 0, 6);

#else // End Intel Corporation code
		glBegin(GL_QUADS);
        if(dblend) 
			glColor3d(0.0f, 0.9f, 0.9f);
        else 
			glColor3d(0.9f, 0.5f, 0.0f);
        glVertex2i(0, 0);
        glVertex2i(VIRTW, 0);
        glVertex2i(VIRTW, VIRTH);
        glVertex2i(0, VIRTH);
        glEnd();
#endif /* _WIN32_WCE */

        glDepthMask(GL_TRUE);
        dblend -= curtime/3;
        if(dblend<0) dblend = 0;
    };

    glEnable(GL_TEXTURE_2D);

    char *command = getcurcommand();
    char *player = playerincrosshair();
    if(command) draw_textf("> %s_", 20, 1570, 2, (int)command);
    else if(closeent[0]) draw_text(closeent, 20, 1570, 2);
    else if(player) draw_text(player, 20, 1570, 2);

    renderscores();
    if(!rendermenu())
    {
        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, 1);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
		glColor4x(ONE_FX, ONE_FX, ONE_FX, ONE_FX);

		if(crosshairfx)
		{
			if(player1->gunwait)
				glColor4x(HALF_FX, HALF_FX, HALF_FX, ONE_FX);
			else if(player1->health<=25)
				glColor4x(ONE_FX, 0, 0, ONE_FX);
			else if(player1->health<=50)
				glColor4x(ONE_FX, HALF_FX, 0, ONE_FX);
		};
		float chsize = (float)crosshairsize;

		GLfixed texarray[12] = { 0, 0,
								 ONE_FX, 0,
								 ONE_FX, ONE_FX,
								 0, 0,
								 ONE_FX, ONE_FX,
								 0, ONE_FX };

		GLfixed vertexarray[12] = { f2x(float(VIRTW/2) - chsize), f2x(float(VIRTH/2) - chsize),
									f2x(float(VIRTW/2) + chsize), f2x(float(VIRTH/2) - chsize),
									f2x(float(VIRTW/2) + chsize), f2x(float(VIRTH/2) + chsize),
									f2x(float(VIRTW/2) - chsize), f2x(float(VIRTH/2) - chsize),
									f2x(float(VIRTW/2) + chsize), f2x(float(VIRTH/2) + chsize),
									f2x(float(VIRTW/2) - chsize), f2x(float(VIRTH/2) + chsize) };
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glTexCoordPointer(2, GL_FIXED, 0, texarray);
		glVertexPointer(2, GL_FIXED, 0, vertexarray);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
#else // End Intel Corporation code
        glBegin(GL_QUADS);
        glColor3ub(255,255,255);
        if(crosshairfx)
        {
            if(player1->gunwait) glColor3ub(128,128,128);
            else if(player1->health<=25) glColor3ub(255,0,0);
            else if(player1->health<=50) glColor3ub(255,128,0);
        };
        float chsize = (float)crosshairsize;
        glTexCoord2d(0.0, 0.0); glVertex2f(VIRTW/2 - chsize, VIRTH/2 - chsize);
        glTexCoord2d(1.0, 0.0); glVertex2f(VIRTW/2 + chsize, VIRTH/2 - chsize);
        glTexCoord2d(1.0, 1.0); glVertex2f(VIRTW/2 + chsize, VIRTH/2 + chsize);
        glTexCoord2d(0.0, 1.0); glVertex2f(VIRTW/2 - chsize, VIRTH/2 + chsize);
        glEnd();
#endif /* _WIN32_WCE */
    };

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	popModelMatrix();
	pushModelMatrix();
	ortho(modelview, 0, i2x(VIRTW*4/3), i2x(VIRTH*4/3), 0, -ONE_FX, ONE_FX);
#else // End Intel Corporation code
	glPopMatrix();

    glPushMatrix();
    glOrtho(0, VIRTW*4/3, VIRTH*4/3, 0, -1, 1);
#endif /* _WIN32_WCE */
    
	renderconsole();

    if(!hidestats)
    {
// Begin Intel Corporation code
#ifdef _WIN32_WCE
		popModelMatrix();
		pushModelMatrix();
		ortho(modelview, 0, i2x(VIRTW*3/2), i2x(VIRTH*3/2), 0, i2x(-1), ONE_FX);
#else // End Intel Corporation code
		glPopMatrix();
        glPushMatrix();
		glOrtho(0, VIRTW*3/2, VIRTH*3/2, 0, -1, 1);
#endif /* _WIN32_WCE */

        draw_textf("fps %d", 3200, 2320, 2, curfps);
        draw_textf("lod %d", 3200, 2390, 2, lod_factor());
        draw_textf("wqd %d", 3200, 2460, 2, nquads); 
        draw_textf("wvt %d", 3200, 2530, 2, curvert);
        draw_textf("evt %d", 3200, 2600, 2, xtraverts);
    };
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	popModelMatrix();
#else // End Intel Corporation code
    glPopMatrix();
#endif /* _WIN32_WCE */

    if(player1->state==CS_ALIVE)
    {

// Begin Intel Corporation code
#ifdef _WIN32_WCE
		pushModelMatrix();
		ortho(modelview, 0, i2x(VIRTW/2), i2x(VIRTH/2), 0, i2x(-1), ONE_FX);
#else // End Intel Corporation code
		glPushMatrix();
		glOrtho(0, VIRTW/2, VIRTH/2, 0, -1, 1);
#endif /* _WIN32_WCE */

        draw_textf("%d",  90, 827, 2, player1->health);
        if(player1->armour) draw_textf("%d", 390, 827, 2, player1->armour);
        draw_textf("%d", 690, 827, 2, player1->ammo[player1->gunselect]);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
		popModelMatrix();
		pushModelMatrix();
		ortho(modelview, 0, i2x(VIRTW), i2x(VIRTH), 0, i2x(-1), ONE_FX);
#else // End Intel Corporation code
		glPopMatrix();
        glPushMatrix();
		glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
#endif /* _WIN32_WCE */

        glDisable(GL_BLEND);
        drawicon(128, 128, 20, 1650);
        if(player1->armour) drawicon((float)(player1->armourtype*64), 0, 620, 1650); 
        int g = player1->gunselect;
        int r = 64;
        if(g>2) { g -= 3; r = 128; };
        drawicon((float)(g*64), (float)r, 1220, 1650);  
	
// Begin Intel Corporation code		
#ifdef _WIN32_WCE
		popModelMatrix();
#else // End Intel Corporation code
        glPopMatrix();
#endif /* _WIN32_WCE */
    };

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
};

