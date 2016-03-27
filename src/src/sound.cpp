// sound.cpp: uses fmod on windows and sdl_mixer on unix (both had problems on the other platform)

#include "cube.h"

VARP(soundvol, 0, 255, 255);
VARP(musicvol, 0, 128, 255);
bool nosound = false;

#define MAXCHAN 32
#define SOUNDFREQ 22050

struct soundloc { vec loc; bool inuse; } soundlocs[MAXCHAN];

    #include "SDL_mixer.h"
    #define MAXVOL MIX_MAX_VOLUME
    Mix_Music *mod = NULL;
    void *stream = NULL;


void stopsound()
{
    if(nosound) return;
    if(mod)
    {
            Mix_HaltMusic();
            Mix_FreeMusic(mod);
        mod = NULL;
    };
    if(stream)
    {
        stream = NULL;
    };
};

VAR(soundbufferlen, 128, 1024, 4096);

void initsound()
{
    memset(soundlocs, 0, sizeof(soundloc)*MAXCHAN);
        if(Mix_OpenAudio(SOUNDFREQ, MIX_DEFAULT_FORMAT, 2, soundbufferlen)<0)
        {
            conoutf("sound init failed (SDL_mixer): %s", (size_t)Mix_GetError());
            nosound = true;
        };
	    Mix_AllocateChannels(MAXCHAN);	
};

void music(char *name)
{
    if(nosound) return;
    stopsound();
    if(soundvol && musicvol)
    {
        string sn;
        strcpy_s(sn, "packages/");
        strcat_s(sn, name);
            if(mod = Mix_LoadMUS(path(sn)))
            {
                Mix_PlayMusic(mod, -1);
                Mix_VolumeMusic((musicvol*MAXVOL)/255);
            };
    };
};

COMMAND(music, ARG_1STR);

vector<Mix_Chunk *> samples;


cvector snames;

int registersound(char *name)
{
    loopv(snames) if(strcmp(snames[i], name)==0) return i;
    snames.add(newstring(name));
    samples.add(NULL);
    return samples.length()-1;
};

COMMAND(registersound, ARG_1EST);

void cleansound()
{
    if(nosound) return;
    stopsound();
        Mix_CloseAudio();
};

VAR(stereo, 0, 1, 1);

void updatechanvol(int chan, vec *loc)
{
    int vol = soundvol, pan = 255/2;
    if(loc)
    {
        vdist(dist, v, *loc, player1->o);
        vol -= (int)(dist*3*soundvol/255); // simple mono distance attenuation
        if(stereo && (v.x != 0 || v.y != 0))
        {
            float yaw = -atan2(v.x, v.y) - player1->yaw*(PI / 180.0f); // relative angle of sound along X-Y axis
            pan = int(255.9f*(0.5*sin(yaw)+0.5f)); // range is from 0 (left) to 255 (right)
        };
    };
    vol = (vol*MAXVOL)/255;
        Mix_Volume(chan, vol);
        Mix_SetPanning(chan, 255-pan, pan);
};  

void newsoundloc(int chan, vec *loc)
{
    assert(chan>=0 && chan<MAXCHAN);
    soundlocs[chan].loc = *loc;
    soundlocs[chan].inuse = true;
};

void updatevol()
{
    if(nosound) return;
    loopi(MAXCHAN) if(soundlocs[i].inuse)
    {
            if(Mix_Playing(i))
                updatechanvol(i, &soundlocs[i].loc);
            else soundlocs[i].inuse = false;
    };
};

void playsoundc(int n) { addmsg(0, 2, SV_SOUND, n); playsound(n); };

int soundsatonce = 0, lastsoundmillis = 0;

void playsound(int n, vec *loc)
{
    if(nosound) return;
    if(!soundvol) return;
    if(lastmillis==lastsoundmillis) soundsatonce++; else soundsatonce = 1;
    lastsoundmillis = lastmillis;
    if(soundsatonce>5) return;  // avoid bursts of sounds with heavy packetloss and in sp
    if(n<0 || n>=samples.length()) { conoutf("unregistered sound: %d", n); return; };

    if(!samples[n])
    {
        sprintf_sd(buf)("packages/sounds/%s.wav", snames[n]);

            samples[n] = Mix_LoadWAV(path(buf));

        if(!samples[n]) { conoutf("failed to load sample: %s", buf); return; };
    };

        int chan = Mix_PlayChannel(-1, samples[n], 0);
    if(chan<0) return;
    if(loc) newsoundloc(chan, loc);
    updatechanvol(chan, loc);
};

void sound(int n) { playsound(n, NULL); };
COMMAND(sound, ARG_1INT);
