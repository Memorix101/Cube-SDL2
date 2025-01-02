// rendercubes.cpp: sits in between worldrender.cpp and rendergl.cpp and fills the vertex array for different cube surfaces.

// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"

vertex *verts = NULL;

int curvert;
int curmaxverts = 10000;

void setarraypointers()
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// pointers for fixed arrays
	glVertexPointer(3, GL_FIXED, sizeof(vertex), &verts[0].x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
    glTexCoordPointer(2, GL_FIXED, sizeof(vertex), &verts[0].u);
#else // End Intel Corporation code
    glVertexPointer(3, GL_FLOAT, sizeof(vertex), &verts[0].x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
    glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &verts[0].u);
#endif /* _WIN32_WCE */
};

void reallocv()
{
    verts = (vertex *)realloc(verts, (curmaxverts *= 2)*sizeof(vertex));
    curmaxverts -= 10;
    if(!verts) fatal("no vertex memory!");
    setarraypointers();
};

// generating the actual vertices is done dynamically every frame and sits at the
// leaves of all these functions, and are part of the cpu bottleneck on really slow
// machines, hence the macros.

#define vertcheck() { if(curvert>=curmaxverts) reallocv(); }

#define vertf(v1, v2, v3, ls, t1, t2) { vertex &v = verts[curvert++]; \
    v.u = t1; v.v = t2; \
    v.x = v1; v.y = v2; v.z = v3; \
    v.r = ls->r; v.g = ls->g; v.b = ls->b; v.a = 255; \
};

// Begin Intel Corporation code
#ifdef _WIN32_WCE
#define vert vertf // assume incoming vertices are fixed point since GLfixed is typedef int
#else // End Intel Corporation code
#define vert(v1, v2, v3, ls, t1, t2) { vertf((float)(v1), (float)(v2), (float)(v3), ls, (float)t1, (float)t2); }
#endif /* _WIN32_WCE */

int nquads;

// Begin Intel Corporation code
#ifdef _WIN32_WCE
const GLfixed TEXTURESCALE = i2x(32);
#else // End Intel Corporation code
const float TEXTURESCALE = 32.0f;
#endif /* _WIN32_WCE */

bool floorstrip = false, deltastrip = false;
int oh, oy, ox, ogltex;                         // the o* vars are used by the stripification
int ol3r, ol3g, ol3b, ol4r, ol4g, ol4b;      
int firstindex;
bool showm = false;

void showmip() { showm = !showm; };
void mipstats(int a, int b, int c) { if(showm) conoutf("1x1/2x2/4x4: %d / %d / %d", a, b, c); };

COMMAND(showmip, ARG_NONE);

void stripend() 
{ 
	if(floorstrip || deltastrip) 
	{ 
		addstrip(ogltex, firstindex, curvert-firstindex); 
		floorstrip = deltastrip = false; 
	}; 
};
void finishstrips() { stripend(); };

sqr sbright, sdark;
VAR(lighterror,1,8,100);

void render_flat(int wtex, int x, int y, int size, int h, sqr *l1, sqr *l2, sqr *l3, sqr *l4, bool isceil)  // floor/ceil quads
{
    vertcheck();
    if(showm) { l3 = l1 = &sbright; l4 = l2 = &sdark; };

    int sx, sy;
    int gltex = lookuptexture(wtex, sx, sy);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	GLfixed sizefx = i2x(size);
	GLfixed xfx = i2x(x);
	GLfixed yfx = i2x(y);
	GLfixed hfx = i2x(h);

    GLfixed xf = DivFX(TEXTURESCALE, i2x(sx), true);
    GLfixed yf = DivFX(TEXTURESCALE, i2x(sy), true);
    GLfixed xs = MulFX(sizefx,xf);
    GLfixed ys = MulFX(sizefx,yf);
    GLfixed xo = MulFX(xf,xfx);
    GLfixed yo = MulFX(yf,yfx);
	GLfixed xoxs = xo + xs;
	GLfixed yoys = yo + ys;
	GLfixed xsize = xfx + sizefx;
	GLfixed ysize = yfx + sizefx;
#else // End Intel Corporation code
    float xf = TEXTURESCALE/sx;
    float yf = TEXTURESCALE/sy;
    float xs = size*xf;
    float ys = size*yf;
    float xo = xf*x;
    float yo = yf*y;
#endif /* _WIN32_WCE */

    bool first = !floorstrip || y!=oy+size || ogltex!=gltex || h!=oh || x!=ox;

    if(first)       // start strip here
    {
        stripend();
        firstindex = curvert;
        ogltex = gltex;
        oh = h;
        ox = x;
        floorstrip = true;
        if(isceil)
        {
// Begin Intel Corporation code
#ifdef _WIN32_WCE
            vertf(xsize, hfx, yfx, l2, xoxs, yo);
            vertf(xfx,      hfx, yfx, l1, xo, yo);
#else // End Intel Corporation code
            vert(float(x+size), float(h), float(y), l2, xo+xs, yo);
            vert(float(x),      float(h), float(y), l1, xo, yo);
#endif /* _WIN32_WCE */
        }
        else
        {
// Begin Intel Corporation code
#ifdef _WIN32_WCE
            vertf(xfx,      hfx, yfx, l1, xo,    yo);
            vertf(xsize, hfx, yfx, l2, xoxs, yo);
#else // End Intel Corporation code
            vert(float(x),      float(h), float(y), l1, xo,    yo);
            vert(float(x+size), float(h), float(y), l2, xo+xs, yo);
#endif /* _WIN32_WCE */
        };
        ol3r = l1->r;
        ol3g = l1->g;
        ol3b = l1->b;
        ol4r = l2->r;
        ol4g = l2->g;
        ol4b = l2->b;
    }
    else        // continue strip
    {
        int lighterr = lighterror*2;
        if((abs(ol3r-l3->r)<lighterr && abs(ol4r-l4->r)<lighterr        // skip vertices if light values are close enough
        &&  abs(ol3g-l3->g)<lighterr && abs(ol4g-l4->g)<lighterr
        &&  abs(ol3b-l3->b)<lighterr && abs(ol4b-l4->b)<lighterr) || !wtex)   
        {
            curvert -= 2;
            nquads--;
        }
        else
        {
            uchar *p3 = (uchar *)(&verts[curvert-1].r);
            ol3r = p3[0];  
            ol3g = p3[1];  
            ol3b = p3[2];
            uchar *p4 = (uchar *)(&verts[curvert-2].r);  
            ol4r = p4[0];
            ol4g = p4[1];
            ol4b = p4[2];
        };
    };

    if(isceil)
    {
// Begin Intel Corporation code
#ifdef _WIN32_WCE
        vertf(xsize, hfx, ysize, l3, xoxs, yoys);
        vertf(xfx,      hfx, ysize, l4, xo,    yoys); 
#else // End Intel Corporation code
        vert(float(x+size), float(h), float(y+size), l3, xo+xs, yo+ys);
        vert(float(x),      float(h), float(y+size), l4, xo,    yo+ys); 
#endif /* _WIN32_WCE */
    }
    else
    {
// Begin Intel Corporation code
#ifdef _WIN32_WCE
        vertf(xfx,      hfx, ysize, l4, xo,    yoys);
        vertf(xsize, hfx, ysize, l3, xoxs, yoys);
#else // End Intel Corporation code
        vert(float(x),      float(h), float(y+size), l4, xo,    yo+ys);
        vert(float(x+size), float(h), float(y+size), l3, xo+xs, yo+ys);
#endif /* _WIN32_WCE */
    };

    oy = y;
    nquads++;
};

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// fixed version of render_flatdelta, passing less floats in 
// often used functions as parameters is a speed optimization
void render_flatdelta(int wtex, int x, int y, int size, GLfixed h1, GLfixed h2, GLfixed h3, GLfixed h4, sqr *l1, sqr *l2, sqr *l3, sqr *l4, bool isceil)  // floor/ceil quads on a slope
{
    vertcheck();
    if(showm) { l3 = l1 = &sbright; l4 = l2 = &sdark; };

    int sx, sy;
    int gltex = lookuptexture(wtex, sx, sy);

	GLfixed yfx = i2x(y);
	GLfixed xfx = i2x(x);
	GLfixed sizefx = i2x(size);
	GLfixed xsizefx = xfx+sizefx;
	GLfixed ysizefx = yfx+sizefx;
    GLfixed xf = DivFX(TEXTURESCALE,i2x(sx),true);
    GLfixed yf = DivFX(TEXTURESCALE,i2x(sy),true);
    GLfixed xs = MulFX(sizefx,xf);
    GLfixed xo = MulFX(xf,xfx);
	GLfixed xoxs = xo+xs;
	GLfixed ys = MulFX(sizefx,yf);
    GLfixed yo = MulFX(yf,yfx);
	GLfixed yoys = yo+ys;

    bool first = !deltastrip || y!=oy+size || ogltex!=gltex || x!=ox; 

    if(first) 
    {
        stripend();
        firstindex = curvert;
        ogltex = gltex;
        ox = x;
        deltastrip = true;
        if(isceil)
        {
            vertf(xsizefx, h2, yfx,      l2, xoxs, yo);
            vertf(xfx,      h1, yfx,      l1, xo,    yo);
        }
        else
        {
            vertf(xfx,      h1, yfx,      l1, xo,    yo);
            vertf(xsizefx, h2, yfx,      l2, xoxs, yo);
        };
        ol3r = l1->r;
        ol3g = l1->g;
        ol3b = l1->b;
        ol4r = l2->r;
        ol4g = l2->g;
        ol4b = l2->b;
    };

    if(isceil)
    {
		vertf(xsizefx, h3, ysizefx, l3, xoxs, yoys); 
        vertf(xfx,      h4, ysizefx, l4, xo,    yoys);
    }
    else
    {
		vertf(xfx,      h4, ysizefx, l4, xo,    yoys);
        vertf(xsizefx, h3, ysizefx, l3, xoxs, yoys); 
    };

    oy = y;
    nquads++;
};
#else // End Intel Corporation code
void render_flatdelta(int wtex, int x, int y, int size, float h1, float h2, float h3, float h4, sqr *l1, sqr *l2, sqr *l3, sqr *l4, bool isceil)  // floor/ceil quads on a slope
{
    vertcheck();
    if(showm) { l3 = l1 = &sbright; l4 = l2 = &sdark; };

    int sx, sy;
    int gltex = lookuptexture(wtex, sx, sy);

    float xf = TEXTURESCALE/sx;
    float yf = TEXTURESCALE/sy;
    float xs = size*xf;
    float ys = size*yf;
    float xo = xf*x;
    float yo = yf*y;

    bool first = !deltastrip || y!=oy+size || ogltex!=gltex || x!=ox; 

    if(first) 
    {
        stripend();
        firstindex = curvert;
        ogltex = gltex;
        ox = x;
        deltastrip = true;
        if(isceil)
        {
            vertf((float)x+size, h2, (float)y,      l2, xo+xs, yo);
            vertf((float)x,      h1, (float)y,      l1, xo,    yo);
        }
        else
        {
            vertf((float)x,      h1, (float)y,      l1, xo,    yo);
            vertf((float)x+size, h2, (float)y,      l2, xo+xs, yo);
        };
        ol3r = l1->r;
        ol3g = l1->g;
        ol3b = l1->b;
        ol4r = l2->r;
        ol4g = l2->g;
        ol4b = l2->b;
    };

    if(isceil)
    {
        vertf((float)x+size, h3, (float)y+size, l3, xo+xs, yo+ys); 
        vertf((float)x,      h4, (float)y+size, l4, xo,    yo+ys);
    }
    else
    {
        vertf((float)x,      h4, (float)y+size, l4, xo,    yo+ys);
        vertf((float)x+size, h3, (float)y+size, l3, xo+xs, yo+ys); 
    };

    oy = y;
    nquads++;
};
#endif /* _WIN32_WCE */

void render_2tris(sqr *h, sqr *s, int x1, int y1, int x2, int y2, int x3, int y3, sqr *l1, sqr *l2, sqr *l3)   // floor/ceil tris on a corner cube
{
    stripend();
    vertcheck();

    int sx, sy;
    int gltex = lookuptexture(h->ftex, sx, sy);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
    GLfixed xf = DivFX(TEXTURESCALE, i2x(sx), true);
    GLfixed yf = DivFX(TEXTURESCALE, i2x(sy), true);
	GLfixed x1fx = i2x(x1);
	GLfixed x2fx = i2x(x2);
	GLfixed x3fx = i2x(x3);
	GLfixed y1fx = i2x(y1);
	GLfixed y2fx = i2x(y2);
	GLfixed y3fx = i2x(y3);

	vertf(x1fx, i2x((int)h->floor), y1fx, l1, MulFX(xf,x1fx), MulFX(yf,y1fx));
    vertf(x2fx, i2x((int)h->floor), y2fx, l2, MulFX(xf,x2fx), MulFX(yf,y2fx));
    vertf(x3fx, i2x((int)h->floor), y3fx, l3, MulFX(xf,x3fx), MulFX(yf,y3fx));
#else // End Intel Corporation code
    float xf = TEXTURESCALE/sx;
    float yf = TEXTURESCALE/sy;
	
	vert((float)x1, h->floor, (float)y1, l1, xf*x1, yf*y1);
    vert((float)x2, h->floor, (float)y2, l2, xf*x2, yf*y2);
    vert((float)x3, h->floor, (float)y3, l3, xf*x3, yf*y3);
#endif /* _WIN32_WCE */

    addstrip(gltex, curvert-3, 3);

    gltex = lookuptexture(h->ctex, sx, sy);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	xf = DivFX(TEXTURESCALE, i2x(sx), true);
    yf = DivFX(TEXTURESCALE, i2x(sy), true);

    vertf(x3fx, i2x((int)h->ceil), y3fx, l3, MulFX(xf,x3fx), MulFX(yf,y3fx));
    vertf(x2fx, i2x((int)h->ceil), y2fx, l2, MulFX(xf,x2fx), MulFX(yf,y2fx));
    vertf(x1fx, i2x((int)h->ceil), y1fx, l1, MulFX(xf,x1fx), MulFX(yf,y1fx));
#else // End Intel Corporation code
	xf = TEXTURESCALE/sx;
    yf = TEXTURESCALE/sy;

	vert((float)x3, h->ceil, (float)y3, l3, xf*x3, yf*y3);
    vert((float)x2, h->ceil, (float)y2, l2, xf*x2, yf*y2);
    vert((float)x1, h->ceil, (float)y1, l1, xf*x1, yf*y1);
#endif /* _WIN32_WCE */

    addstrip(gltex, curvert-3, 3);
    nquads++;
};

void render_tris(int x, int y, int size, bool topleft,
                 sqr *h1, sqr *h2, sqr *s, sqr *t, sqr *u, sqr *v)
{
    if(topleft)
    {
        if(h1) render_2tris(h1, s, x+size, y+size, x, y+size, x, y, u, v, s);
        if(h2) render_2tris(h2, s, x, y, x+size, y, x+size, y+size, s, t, v);
    }
    else
    {
        if(h1) render_2tris(h1, s, x, y, x+size, y, x, y+size, s, t, u);
        if(h2) render_2tris(h2, s, x+size, y, x+size, y+size, x, y+size, t, u, v);
    };
};

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// fixed point version of render_square
void render_square(int wtex, GLfixed floor1, GLfixed floor2, GLfixed ceil1, GLfixed ceil2, int x1, int y1, int x2, int y2, int size, sqr *l1, sqr *l2, bool flip)   // wall quads
{
    stripend();
    vertcheck();
    if(showm) { l1 = &sbright; l2 = &sdark; };

    int sx, sy;
    int gltex = lookuptexture(wtex, sx, sy);

    GLfixed xf = DivFX(TEXTURESCALE, i2x(sx), true);
    GLfixed yf = DivFX(TEXTURESCALE, i2x(sy), true);
    GLfixed xs = MulFX(i2x(size),xf);
    GLfixed xo = MulFX(xf,(x1==x2 ? i2x(min(y1,y2)) : i2x(min(x1,x2))));
	GLfixed xoxs = xo+xs;
	GLfixed x1fx = i2x(x1);
	GLfixed x2fx = i2x(x2);
	GLfixed y1fx = i2x(y1);
	GLfixed y2fx = i2x(y2);

    if(!flip)
    {
        vertf(x2fx, ceil2,  y2fx, l2, xoxs, MulFX(-yf,ceil2));
        vertf(x1fx, ceil1,  y1fx, l1, xo,    MulFX(-yf,ceil1)); 
        vertf(x2fx, floor2, y2fx, l2, xoxs, MulFX(-floor2,yf)); 
        vertf(x1fx, floor1, y1fx, l1, xo,    MulFX(-floor1,yf)); 
    }
    else
    {
        vertf(x1fx, ceil1,  y1fx, l1, xo,    MulFX(-yf,ceil1));
        vertf(x2fx, ceil2,  y2fx, l2, xoxs, MulFX(-yf,ceil2)); 
        vertf(x1fx, floor1, y1fx, l1, xo,    MulFX(-floor1,yf)); 
        vertf(x2fx, floor2, y2fx, l2, xoxs, MulFX(-floor2,yf)); 
    };

    nquads++;
    addstrip(gltex, curvert-4, 4);
};
#else // End Intel Corporation code

void render_square(int wtex, float floor1, float floor2, float ceil1, float ceil2, int x1, int y1, int x2, int y2, int size, sqr *l1, sqr *l2, bool flip)   // wall quads
{
    stripend();
    vertcheck();
    if(showm) { l1 = &sbright; l2 = &sdark; };

    int sx, sy;
    int gltex = lookuptexture(wtex, sx, sy);

    float xf = TEXTURESCALE/sx;
    float yf = TEXTURESCALE/sy;
    float xs = size*xf;
    float xo = xf*(x1==x2 ? min(y1,y2) : min(x1,x2));

    if(!flip)
    {
        vertf((float)x2, ceil2,  (float)y2, l2, xo+xs, -yf*ceil2);
        vertf((float)x1, ceil1,  (float)y1, l1, xo,    -yf*ceil1); 
        vertf((float)x2, floor2, (float)y2, l2, xo+xs, -floor2*yf); 
        vertf((float)x1, floor1, (float)y1, l1, xo,    -floor1*yf); 
    }
    else
    {
        vertf((float)x1, ceil1,  (float)y1, l1, xo,    -yf*ceil1);
        vertf((float)x2, ceil2,  (float)y2, l2, xo+xs, -yf*ceil2); 
        vertf((float)x1, floor1, (float)y1, l1, xo,    -floor1*yf); 
        vertf((float)x2, floor2, (float)y2, l2, xo+xs, -floor2*yf); 
    };

    nquads++;
    addstrip(gltex, curvert-4, 4);
};
#endif /* _WIN32_WCE */

int wx1, wy1, wx2, wy2;

VAR(watersubdiv, 1, 4, 64);
VARF(waterlevel, -128, -128, 127, if(!noteditmode()) hdr.waterlevel = waterlevel);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// Water has precision issues that are difficult to convert to fixed point
__inline void vertw(int v1, float v2, int v3, sqr *c, float t1, float t2, float t)
{
    vertcheck();

    vertf(i2x(v1), f2x(v2-(float)sin(v1*v3*0.1+t)*0.2f), i2x(v3), c, f2x(t1), f2x(t2));
};
#else // End Intel Corporation code
__inline void vertw(int v1, float v2, int v3, sqr *c, float t1, float t2, float t)
{
    vertcheck();

    vertf((float)v1, v2-(float)sin(v1*v3*0.1+t)*0.2f, (float)v3, c, t1, t2);
};
#endif /* _WIN32_WCE */

__inline float dx(float x) { return x+(float)sin(x*2+lastmillis/1000.0f)*0.04f; };
__inline float dy(float x) { return x+(float)sin(x*2+lastmillis/900.0f+PI/5)*0.05f; };

// renders water for bounding rect area that contains water... simple but very inefficient

int renderwater(float hf)
{
    if(wx1<0) return nquads;

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_COLOR);
    int sx, sy;
    glBindTexture(GL_TEXTURE_2D, lookuptexture(DEFAULT_LIQUID, sx, sy));  

    wx1 &= ~(watersubdiv-1);
    wy1 &= ~(watersubdiv-1);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	// other functions make use of the pointers
	setarraypointers(); 
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

    float xf = x2f(TEXTURESCALE)/sx;
    float yf = x2f(TEXTURESCALE)/sy;
#else // End Intel Corporation code
    float xf = TEXTURESCALE/sx;
    float yf = TEXTURESCALE/sy;
#endif /* _WIN32_WCE */
    float xs = watersubdiv*xf;
    float ys = watersubdiv*yf;
    float t1 = lastmillis/300.0f;
    float t2 = lastmillis/4000.0f;
    
    sqr dl;
    dl.r = dl.g = dl.b = 255;
    
    for(int xx = wx1; xx<wx2; xx += watersubdiv)
    {
        for(int yy = wy1; yy<wy2; yy += watersubdiv)
        {
            float xo = xf*(xx+t2);
            float yo = yf*(yy+t2);
            if(yy==wy1)
            {
                vertw(xx,             hf, yy,             &dl, dx(xo),    dy(yo), t1);
                vertw(xx+watersubdiv, hf, yy,             &dl, dx(xo+xs), dy(yo), t1);
            };
            vertw(xx,             hf, yy+watersubdiv, &dl, dx(xo),    dy(yo+ys), t1);
            vertw(xx+watersubdiv, hf, yy+watersubdiv, &dl, dx(xo+xs), dy(yo+ys), t1); 
        };   
        int n = (wy2-wy1-1)/watersubdiv;
        nquads += n;
        n = (n+2)*2;
        glDrawArrays(GL_TRIANGLE_STRIP, curvert -= n, n);
    };

// Begin Intel Corporation code
#ifdef _WIN32_WCE
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#endif /* _WIN32_WCE */
// End Intel Corporation code
    
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    
    return nquads;
};

void addwaterquad(int x, int y, int size)       // update bounding rect that contains water
{
    int x2 = x+size;
    int y2 = y+size;
    if(wx1<0)
    {
        wx1 = x;
        wy1 = y;
        wx2 = x2;
        wy2 = y2;
    }
    else
    {
        if(x<wx1) wx1 = x;
        if(y<wy1) wy1 = y;
        if(x2>wx2) wx2 = x2;
        if(y2>wy2) wy2 = y2;
    };
};

void resetcubes()
{
    if(!verts) reallocv();
    floorstrip = deltastrip = false;
    wx1 = -1;
    nquads = 0;
    sbright.r = sbright.g = sbright.b = 255;
    sdark.r = sdark.g = sdark.b = 0;
};


