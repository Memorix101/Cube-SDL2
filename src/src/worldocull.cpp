// worldocull.cpp: occlusion map and occlusion test

// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"

#define NUMRAYS 256

// Begin Intel Corporation code
#ifdef _WIN32_WCE
GLfixed rdist[NUMRAYS];
GLfixed odist = i2x(256);
const GLfixed NUMRAYS_FX = i2x(NUMRAYS);
const GLfixed TWOPI_DIV_NUMRAYS_FX = DivFX(TWOPI_FX, NUMRAYS_FX);
const GLfixed EIGHT_DIV_NUMRAYS_FX = DivFX(i2x(8), NUMRAYS_FX);
const GLfixed NUMRAYS_DIV_EIGHT_FX = DivFX(NUMRAYS_FX, i2x(8));
const GLfixed THREE_HALVES_FX = 0x18000;
#else // End Intel Corporation code
float rdist[NUMRAYS];
float odist = 256;
#endif /* _WIN32_WCE */

bool ocull = true;

void toggleocull() { ocull = !ocull; };

COMMAND(toggleocull, ARG_NONE);

// constructs occlusion map: cast rays in all directions on the 2d plane and record distance.
// done exactly once per frame.

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// fixed point version
void computeraytable(GLfixed vx, GLfixed vy)
{
    if(!ocull) return;

    odist = MulFX(i2x(getvar("fog")), THREE_HALVES_FX);
	
	float apitch = (float)fabsf(player1->pitch);
    float af = getvar("fov")/2+apitch/1.5f+3;

    GLfixed byaw = f2x((player1->yaw-90+af)/360*PI2);
    GLfixed syaw = f2x((player1->yaw-90-af)/360*PI2);

	float fvx = x2f(vx);
	float fvy = x2f(vy);

	int ivx = x2i(vx);
	int ivy = x2i(vy);

    loopi(NUMRAYS)
    {
        GLfixed angle = MulFX(i2x(i), TWOPI_DIV_NUMRAYS_FX);
        if((apitch>45 // must be bigger if fov>120
        || (angle<byaw && angle>syaw)
        || (angle<byaw-TWOPI_FX && angle>syaw-TWOPI_FX)
        || (angle<byaw+TWOPI_FX && angle>syaw+TWOPI_FX))
        && !OUTBORD(fvx, fvy)
        && !SOLID(S(ivx, ivy)))       // try to avoid tracing ray if outside of frustrum
        {
            GLfixed ray = MulFX(i2x(i), EIGHT_DIV_NUMRAYS_FX);
            GLfixed dx, dy;
            if(ray>ONE_FX && ray<THREE_FX) 
			{ 
				dx = -(ray-TWO_FX); 
				dy = ONE_FX; 
			}
            else if(ray>=THREE_FX && ray<i2x(5)) 
			{ 
				dx = -ONE_FX; 
				dy = -(ray-FOUR_FX); 
			}
            else if(ray>=i2x(5) && ray<i2x(7)) 
			{ 
				dx = ray-i2x(6); 
				dy = -ONE_FX; 
			}
            else 
			{ 
				dx = ONE_FX; 
				dy = ray>FOUR_FX ? ray-i2x(8) : ray; 
			};

            GLfixed sx = vx;
            GLfixed sy = vy;
            for(;;)
            {
                sx += dx;
                sy += dy;
                if(SOLID(S(x2i(sx), x2i(sy))))    // 90% of time spend in this function is on this line
                {
                    rdist[i] = absfx(sx-vx) + absfx(sy-vy);
                    break;
                };
            };
        }
        else
        {
            rdist[i] = TWO_FX;
        };
    };
};
#else // End Intel Corporation code
void computeraytable(float vx, float vy)
{
    if(!ocull) return;

    odist = getvar("fog")*1.5f;

    float apitch = (float)fabs(player1->pitch);
    float af = getvar("fov")/2+apitch/1.5f+3;
    float byaw = (player1->yaw-90+af)/360*PI2;
    float syaw = (player1->yaw-90-af)/360*PI2;

    loopi(NUMRAYS)
    {
        float angle = i*PI2/NUMRAYS;
        if((apitch>45 // must be bigger if fov>120
        || (angle<byaw && angle>syaw)
        || (angle<byaw-PI2 && angle>syaw-PI2)
        || (angle<byaw+PI2 && angle>syaw+PI2))
        && !OUTBORD(vx, vy)
        && !SOLID(S(fast_f2nat(vx), fast_f2nat(vy))))       // try to avoid tracing ray if outside of frustrum
        {
            float ray = i*8/(float)NUMRAYS;
            float dx, dy;
            if(ray>1 && ray<3) { dx = -(ray-2); dy = 1; }
            else if(ray>=3 && ray<5) { dx = -1; dy = -(ray-4); }
            else if(ray>=5 && ray<7) { dx = ray-6; dy = -1; }
            else { dx = 1; dy = ray>4 ? ray-8 : ray; };
            float sx = vx;
            float sy = vy;
            for(;;)
            {
                sx += dx;
                sy += dy;
                if(SOLID(S(fast_f2nat(sx), fast_f2nat(sy))))    // 90% of time spend in this function is on this line
                {
                    rdist[i] = (float)(fabs(sx-vx)+fabs(sy-vy));
                    break;
                };
            };
        }
        else
        {
            rdist[i] = 2;
        };
    };
};
#endif /* _WIN32_WCE */

// test occlusion for a cube... one of the most computationally expensive functions in the engine
// as its done for every cube and entity, but its effect is more than worth it!

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// most divides happen here
__forceinline GLfixed ca(GLfixed x, GLfixed y) {	return x > y ? DivFX(y,x,true) : TWO_FX - DivFX(x, y,true); };
__forceinline GLfixed ma(GLfixed x, GLfixed y) { return x==0 ? (y>0 ? TWO_FX : -TWO_FX) : DivFX(y,x,true); };
#else // End Intel Corporation code
inline float ca(float x, float y) { return x>y ? y/x : 2-x/y; }; 
inline float ma(float x, float y) { return x==0 ? (y>0 ? 2 : -2) : y/x; };
#endif /* _WIN32_WCE */

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// fixed point version
int isoccluded(GLfixed vx, GLfixed vy, GLfixed cx, GLfixed cy, GLfixed csize)     // v = viewer, c = cube to test 
{
    if(!ocull) return 0;

    GLfixed nx = vx, ny = vy;     // n = point on the border of the cube that is closest to v
    if(nx<cx) nx = cx;
    else if(nx>cx+csize) nx = cx+csize;
    if(ny<cy) ny = cy;
    else if(ny>cy+csize) ny = cy+csize;
	GLfixed xdist = absfx(nx-vx);
	GLfixed ydist = absfx(ny-vy);
    if(xdist>odist || ydist>odist) return 2;
    
	GLfixed dist = xdist+ydist-ONE_FX; // 1 needed?

    // ABC
    // D E
    // FGH

    // - check middle cube? BG

    // find highest and lowest angle in the occlusion map that this cube spans, based on its most left and right
    // points on the border from the viewer pov... I see no easier way to do this than this silly code below

    GLfixed h, l;
    if(cx<=vx)              // ABDFG
    {
        if(cx+csize<vx)     // ADF
        {
            if(cy<=vy)      // AD
            {
                if(cy+csize<vy) 
				{ 
					h = ca(-(cx-vx), -(cy+csize-vy))+FOUR_FX; 
					l = ca(-(cx+csize-vx), -(cy-vy))+FOUR_FX; 
				}         // A
                else            
				{ 
					h = ma(-(cx+csize-vx), -(cy+csize-vy))+FOUR_FX; 
					l =  ma(-(cx+csize-vx), -(cy-vy))+FOUR_FX; 
				}  // D
            }
            else                
			{ 
				h = ca(cy+csize-vy, -(cx+csize-vx))+TWO_FX; 
				l = ca(cy-vy, -(cx-vx))+TWO_FX; 
			};              // F
        }
        else                // BG
        {
            if(cy<=vy)
            {
                if(cy+csize<vy) 
				{ 
					h = ma(-(cy+csize-vy), cx-vx)+i2x(6); 
					l = ma(-(cy+csize-vy), cx+csize-vx)+i2x(6); 
				}         // B
                else return 0;
            }
            else     
			{ 
				h = ma(cy-vy, -(cx+csize-vx))+TWO_FX; 
				l = ma(cy-vy, -(cx-vx))+TWO_FX; 
			};                               // G
        };
    }
    else                    // CEH
    {
        if(cy<=vy)          // CE
        {
            if(cy+csize<vy) 
			{ 
				h = ca(-(cy-vy), cx-vx)+i2x(6); 
				l = ca(-(cy+csize-vy), cx+csize-vx)+i2x(6); 
			}                   // C
            else            
			{ 
				h = ma(cx-vx, cy-vy); 
				l = ma(cx-vx, cy+csize-vy); 
			};                                  // E
        }
        else                
		{ 
			h = ca(cx+csize-vx, cy-vy); 
			l = ca(cx-vx, cy+csize-vy); 
		};                            // H
    };

	int si = x2i(MulFX(h, NUMRAYS_DIV_EIGHT_FX) + NUMRAYS_FX);
	int ei = x2i(MulFX(l, NUMRAYS_DIV_EIGHT_FX) + NUMRAYS_FX + ONE_FX);
    if(ei<=si) ei += NUMRAYS;

    for(int i = si; i<=ei; i++)
    {
        if(dist<rdist[i&(NUMRAYS-1)]) return 0;     // if any value in this segment of the occlusion map is further away then cube is not occluded
    };

    return 1;                                       // cube is entirely occluded
};
#else // End Intel Corporation code
int isoccluded(float vx, float vy, float cx, float cy, float csize)     // v = viewer, c = cube to test 
{
    if(!ocull) return 0;

    float nx = vx, ny = vy;     // n = point on the border of the cube that is closest to v
    if(nx<cx) nx = cx;
    else if(nx>cx+csize) nx = cx+csize;
    if(ny<cy) ny = cy;
    else if(ny>cy+csize) ny = cy+csize;
    float xdist = (float)fabs(nx-vx);
    float ydist = (float)fabs(ny-vy);
    if(xdist>odist || ydist>odist) return 2;
    float dist = xdist+ydist-1; // 1 needed?

    // ABC
    // D E
    // FGH

    // - check middle cube? BG

    // find highest and lowest angle in the occlusion map that this cube spans, based on its most left and right
    // points on the border from the viewer pov... I see no easier way to do this than this silly code below

    float h, l;
    if(cx<=vx)              // ABDFG
    {
        if(cx+csize<vx)     // ADF
        {
            if(cy<=vy)      // AD
            {
                if(cy+csize<vy) 
				{ 
					h = ca(-(cx-vx), -(cy+csize-vy))+4; 
					l = ca(-(cx+csize-vx), -(cy-vy))+4; 
				}         // A
                else            
				{ 
					h = ma(-(cx+csize-vx), -(cy+csize-vy))+4; 
					l =  ma(-(cx+csize-vx), -(cy-vy))+4; 
				}  // D
            }
            else                
			{ 
				h = ca(cy+csize-vy, -(cx+csize-vx))+2; 
				l = ca(cy-vy, -(cx-vx))+2; 
			};              // F
        }
        else                // BG
        {
            if(cy<=vy)
            {
                if(cy+csize<vy) 
				{ 
					h = ma(-(cy+csize-vy), cx-vx)+6; 
					l = ma(-(cy+csize-vy), cx+csize-vx)+6; 
				}         // B
                else return 0;
            }
            else     
			{ 
				h = ma(cy-vy, -(cx+csize-vx))+2; 
				l = ma(cy-vy, -(cx-vx))+2; 
			};                               // G
        };
    }
    else                    // CEH
    {
        if(cy<=vy)          // CE
        {
            if(cy+csize<vy) 
			{ 
				h = ca(-(cy-vy), cx-vx)+6; 
				l = ca(-(cy+csize-vy), cx+csize-vx)+6; 
			}                   // C
            else            
			{ 
				h = ma(cx-vx, cy-vy); 
				l = ma(cx-vx, cy+csize-vy); 
			};                                  // E
        }
        else                
		{ 
			h = ca(cx+csize-vx, cy-vy); 
			l = ca(cx-vx, cy+csize-vy); 
		};                            // H
    };
    int si = fast_f2nat(h*(NUMRAYS/8))+NUMRAYS;     // get indexes into occlusion map from angles
    int ei = fast_f2nat(l*(NUMRAYS/8))+NUMRAYS+1; 
    if(ei<=si) ei += NUMRAYS;

    for(int i = si; i<=ei; i++)
    {
        if(dist<rdist[i&(NUMRAYS-1)]) return 0;     // if any value in this segment of the occlusion map is further away then cube is not occluded
    };

    return 1;                                       // cube is entirely occluded
};
#endif /* _WIN32_WCE */

