// console.cpp: the console buffer, its display, and command line control

// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"
#include <ctype.h>

struct cline { char *cref; int outtime; };
vector<cline> conlines;

const int ndraw = 5;
const int WORDWRAP = 80;
int conskip = 0;

bool saycommandon = false;
string commandbuf;

void setconskip(int n)
{
    conskip += n;
    if(conskip<0) conskip = 0;
};

COMMANDN(conskip, setconskip, ARG_1INT);

void conline(const char *sf, bool highlight)        // add a line to the console buffer
{
    cline cl;
    cl.cref = conlines.length()>100 ? conlines.pop().cref : newstringbuf("");   // constrain the buffer size
    cl.outtime = lastmillis;                        // for how long to keep line on screen
    conlines.insert(0,cl);
    if(highlight)                                   // show line in a different colour, for chat etc.
    {
        cl.cref[0] = '\f';
        cl.cref[1] = 0;
        strcat_s(cl.cref, sf);
    }
    else
    {
        strcpy_s(cl.cref, sf);
    };
    puts(cl.cref);
};

void conoutf(const char *s, int a, int b, int c)
{
    sprintf_sd(sf)(s, a, b, c);
    s = sf;
    int n = 0;
    while(strlen(s)>WORDWRAP)                       // cut strings to fit on screen
    {
        string t;
        strn0cpy(t, s, WORDWRAP+1);
        conline(t, n++!=0);
        s += WORDWRAP;
    };
    conline(s, n!=0);
};

void renderconsole()                                // render buffer taking into account time & scrolling
{
    int nd = 0;
    char *refs[ndraw];
    loopv(conlines) if(conskip ? i>=conskip-1 || i>=conlines.length()-ndraw : lastmillis-conlines[i].outtime<20000)
    {
        refs[nd++] = conlines[i].cref;
        if(nd==ndraw) break;
    };
    loopj(nd)
    {
        draw_text(refs[j], FONTH/3, (FONTH/4*5)*(nd-j-1)+FONTH/3, 2);
    };
};

// keymap is defined externally in keymap.cfg

struct keym { int code; char *name; char *action; } keyms[256];
int numkm = 0;                                     

void keymap(char *code, char *key, char *action)
{
    keyms[numkm].code = atoi(code);
    keyms[numkm].name = newstring(key);
    keyms[numkm++].action = newstringbuf(action);
};

COMMAND(keymap, ARG_3STR);

void bindkey(char *key, char *action)
{
    for(char *x = key; *x; x++) *x = toupper(*x);
    loopi(numkm) if(strcmp(keyms[i].name, key)==0)
    {
        strcpy_s(keyms[i].action, action);
        return;
    };
    conoutf("unknown key \"%s\"", (int)key);   
};

COMMANDN(bind, bindkey, ARG_2STR);

void saycommand(char *init)                         // turns input to the command line on or off
{
    //SDL_EnableUNICODE(saycommandon = (init!=NULL));
    if(!editmode) keyrepeat(saycommandon);
    if(!init) init = "";
    strcpy_s(commandbuf, init);
};

void mapmsg(char *s) { strn0cpy(hdr.maptitle, s, 128); };

COMMAND(saycommand, ARG_VARI);
COMMAND(mapmsg, ARG_1STR);

#if !defined(_WIN32_WCE) && !defined(WIN32)
#include <X11/Xlib.h>
#include <SDL_syswm.h>
#endif /* _WIN32_WCE && WIN32 */

void pasteconsole()
{
    #if defined(WIN32) || defined(_WIN32_WCE)
    if(!IsClipboardFormatAvailable(CF_TEXT)) return; 
    if(!OpenClipboard(NULL)) return;
    char *cb = (char *)GlobalLock(GetClipboardData(CF_TEXT));
    strcat_s(commandbuf, cb);
    GlobalUnlock(cb);
    CloseClipboard();
    #else
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version); 
    wminfo.subsystem = SDL_SYSWM_X11;
    if(!SDL_GetWMInfo(&wminfo)) return;
    int cbsize;
    char *cb = XFetchBytes(wminfo.info.x11.display, &cbsize);
    if(!cb || !cbsize) return;
    int commandlen = strlen(commandbuf);
    for(char *cbline = cb, *cbend; commandlen + 1 < _MAXDEFSTR && cbline < &cb[cbsize]; cbline = cbend + 1)
    {
        cbend = (char *)memchr(cbline, '\0', &cb[cbsize] - cbline);
        if(!cbend) cbend = &cb[cbsize];
        if(commandlen + cbend - cbline + 1 > _MAXDEFSTR) cbend = cbline + _MAXDEFSTR - commandlen - 1;
        memcpy(&commandbuf[commandlen], cbline, cbend - cbline);
        commandlen += cbend - cbline;
        commandbuf[commandlen] = '\n';
        if(commandlen + 1 < _MAXDEFSTR && cbend < &cb[cbsize]) ++commandlen;
        commandbuf[commandlen] = '\0';
    };
    XFree(cb);
    #endif /* WIN32 || _WIN32_WCE */
};

cvector vhistory;
int histpos = 0;

void history(int n)
{
    static bool rec = false;
    if(!rec && n>=0 && n<vhistory.length())
    {
        rec = true;
        execute(vhistory[vhistory.length()-n-1]);
        rec = false;
    };
};

COMMAND(history, ARG_1INT);

// Begin Intel Corporation code
#ifdef _WIN32_WCE
// special exit combo needed for terminating an "unexitable" game
vector<int> exitcombo;
#endif /* _WIN32_WCE */
// End Intel Corporation code

void keypress(int code, bool isdown, int cooked)
{
// Begin Intel Corporation code
#ifdef _WIN32_WCE 
	// special exit code combination for shows, 31415 (ie PI in F buttons)
	if(isdown)
	{
		if(code == SDLK_F4 && exitcombo.length() == 2)
			exitcombo.add(code);
		else if(code == SDLK_F1 && (exitcombo.length() == 1 || exitcombo.length() == 3))
			exitcombo.add(code);
		else if(code == SDLK_F3 && exitcombo.length() == 0)
			exitcombo.add(code);
		else if(code == SDLK_F5 && exitcombo.length() == 4)
			quit();
		else
			exitcombo.setsize(0);
	}
#endif /* _WIN32_WCE */
// End Intel Corporation code
	
    if(saycommandon)                                // keystrokes go to commandline
    {
        if(isdown)
        {
            switch(code)
            {
                case SDLK_RETURN:
                    break;

                case SDLK_BACKSPACE:
                case SDLK_LEFT:
                {
                    for(int i = 0; commandbuf[i]; i++) if(!commandbuf[i+1]) commandbuf[i] = 0;
                    resetcomplete();
                    break;
                };
                    
                case SDLK_UP:
                    if(histpos) strcpy_s(commandbuf, vhistory[--histpos]);
                    break;
                
                case SDLK_DOWN:
                    if(histpos<vhistory.length()) strcpy_s(commandbuf, vhistory[histpos++]);
                    break;
                    
                case SDLK_TAB:
                    complete(commandbuf);
                    break;

                case SDLK_v:
                    if(SDL_GetModState()&(KMOD_LCTRL|KMOD_RCTRL)) { pasteconsole(); return; };

                default:
                    resetcomplete();
                    if(cooked) { char add[] = { cooked, 0 }; strcat_s(commandbuf, add); };
            };
        }
        else
        {
            if(code==SDLK_RETURN)
            {
                if(commandbuf[0])
                {
                    if(vhistory.empty() || strcmp(vhistory.last(), commandbuf))
                    {
                        vhistory.add(newstring(commandbuf));  // cap this?
                    };
                    histpos = vhistory.length();
                    if(commandbuf[0]=='/') execute(commandbuf, true);
                    else toserver(commandbuf);
                };
                saycommand(NULL);
            }
#ifdef _WIN32_WCE // Begin Intel Corporation code
			// 3rd button is menu key
			else if(code==SDLK_F3)
#else // End Intel Corporation code
            else if(code==SDLK_ESCAPE)
#endif /* _WIN32_WCE */
            {
                saycommand(NULL);
            };
        };
    }
    else if(!menukey(code, isdown))                 // keystrokes go to menu
    {
        loopi(numkm) if(keyms[i].code==code)        // keystrokes go to game, lookup in keymap and execute
        {
            string temp;
            strcpy_s(temp, keyms[i].action);
            execute(temp, isdown); 
            return;
        };
    };
};

char *getcurcommand()
{
    return saycommandon ? commandbuf : NULL;
};

