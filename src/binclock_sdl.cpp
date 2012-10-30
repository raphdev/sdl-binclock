#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "SDL/SDL.h"

const char* BOX_VALUES[2] = { "img/box_0.bmp", "img/box_1.bmp" };

// display format
const int SCREEN_WIDTH = 1050;
const int SCREEN_HEIGHT = 660;
const int BITS_PER_PIXEL = 32;

// global
SDL_Surface *background;

enum bitbox_value { FALSE, TRUE, UNSET };

class bitbox_t {
  static SDL_Surface *box[2];
  SDL_Surface *sval, *bg;
  bitbox_value val;
  SDL_Rect coords;

  public:
  bitbox_t() {
    val = UNSET;
    sval = NULL;
    bg = NULL;
    coords.x = coords.y = 0;

    
    for(int i = 0; i < 2; i++)
    {
      if(box[i] == NULL)
      {
        SDL_Surface *orig_img = NULL; // original image
        
        orig_img = SDL_LoadBMP(BOX_VALUES[i]);
        
        
        if(orig_img != NULL)
        {
          box[i] = orig_img;
        }
      }
    }
    
  }
  
  ~bitbox_t()
  {
    for(int i = 0; i < 2; i++)
      SDL_FreeSurface(box[i]);
  }
  
  // Call this function only after SDL has been initialized -- enables acceleration and sets color keys for faster blitting
  bool optimize()
  {
    for(int i = 0; i < 2; i++)
    {
      SDL_Surface *oldsurface = box[i];
      SDL_Surface *formatted = NULL;
      
      if(SDL_SetColorKey(oldsurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(oldsurface->format, 0, 0, 0)))
        return false;
      
      // convert to native display format to avoid on-the-fly conversion and enable RLE-encoding optimization
      formatted = SDL_DisplayFormat(oldsurface);
      
      if(!formatted)
        return false;
        
      SDL_FreeSurface(oldsurface);
      box[i] = formatted;
    }
    
    return true;
  }
  
  void setpos(Sint16 x, Sint16 y)
  {
    coords.x = x;
    coords.y = y;
  }
  
  inline void draw(SDL_Surface* dest)
  {
    bg = dest;
    if(sval) {
      SDL_BlitSurface(sval, NULL, dest, &coords);
    }
  }
  
  void setval(bitbox_value value)
  {
    if(value == val)
      return;
    
    switch(value)
    {
    case UNSET:
      {
        SDL_Rect erase = { coords.x, coords.y, (Uint16)box[0]->w, (Uint16)box[0]->h };
        SDL_FillRect(bg, &erase, SDL_MapRGB(box[0]->format, 0, 0, 0));
        sval = NULL;
        val = value;
        break;
      }
    case TRUE:
    case FALSE:
      {
        sval = box[value];
        val = value;
        draw(bg);
        break;
      }
    }
  }
  
  bitbox_value getval() const
  { 
    return val; 
  }
};

void siginthandler(int signum)
{
  SDL_FreeSurface(background);
  exit(EXIT_SUCCESS);
}

// initialize static members
SDL_Surface* bitbox_t::box[2] = { NULL, NULL };

int main(int argc, char** argv)
{
  SDL_Event e;
  bitbox_t valv[4][6];
  int initpx, initpy = 10, pipefds[2] = { 0, 0 };
  char *prog_name = NULL, buf[8] =  { 0 }; 

  prog_name = (argc < 2) ? (char*)"./binclock_ncurses" : argv[1];

  signal(SIGINT, siginthandler);

  if(SDL_Init(SDL_INIT_VIDEO) == -1)
  {
    fprintf(stderr, "Failed to initialize SDL.");
    exit(EXIT_FAILURE);
  }
  atexit(SDL_Quit);
  
  background = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL, SDL_SWSURFACE);
  
  if(!background)
  {
    fprintf(stderr, "Failed to set video mode.");
    exit(EXIT_FAILURE);
  }

  SDL_WM_SetCaption("Binary Clock", NULL);
  
  for(int i = 0; i < 4; i++, initpy += 165)
  {
    initpx = 10;
    for(int o = 0; o < 6; o++, initpx += 175)
    {
      valv[i][o].optimize();
      valv[i][o].setpos(initpx, initpy);
      valv[i][o].draw(background);
      valv[i][o].setval(TRUE);
    }
  }
  
  while(1)
  {
    while(SDL_PollEvent(&e))
    {
      if(e.type == SDL_QUIT)
        raise(SIGINT);
    }
    
    pipe(pipefds);

    if(!fork())
    {
      close(STDOUT_FILENO);
      dup2(pipefds[1], STDOUT_FILENO); // redirect the write-end of the pipe to stdout
      close(pipefds[0]);
      execlp(prog_name, prog_name, NULL);
    }
    else {
      close(STDIN_FILENO);
      dup2(pipefds[0], STDIN_FILENO); // redirect the read-end of the pipe to stdin
      close(pipefds[1]);
    }
    
    for(int i = 0; i < 4; i++)
    {
      fgets(buf, 8, stdin); 
      if(buf == NULL)
        break;

      for(int o = 0; o < 6; o++)
      {
        bitbox_value nval;

        switch(buf[o])
        {
        case '1':
          nval = TRUE;
          break;
        case '0':
          nval = FALSE;
          break;
        case ' ':
          nval = UNSET;
          break;
        }

        valv[i][o].setval(nval);
      }
    }

    
    if(SDL_Flip(background) == -1) {
      SDL_FreeSurface(background);
      exit(EXIT_FAILURE);
    }
    SDL_Delay(1000);
  }

  return 0;
}