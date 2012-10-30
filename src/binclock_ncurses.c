#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
//#include <unistd.h>
//#include <ncurses.h>

void uc2bin(int c, char *s);
void finish(int sig);

int main(void) {

  struct tm *tm;
  time_t t;
  char m[4][7] = { {0} };
  char p[CHAR_BIT + 1];
  char s[512];
  int i, j, k;

  //initscr();
  //curs_set(0);
    signal(SIGINT, finish); /* for ^C */

  m[0][4] = m[0][2] = m[1][0] = m[0][0] = ' ';
  //while(1) {

  time(&t);
  tm = localtime(&t);
  if(tm == NULL) exit(EXIT_FAILURE);
  strftime(s, sizeof s, "%H%M%S", tm);
  for(i = 0; i < 6; s[i++] -= '0');

#define loop(a, b, c) \
    uc2bin(s[b], p); \
do { for(i = a, k = c, j = CHAR_BIT - 1; k--; i--, j--) \
    m[i][b] = p[j]; } while(0)

    loop(3, 1, 4);
    loop(3, 0, 2);
    loop(3, 3, 4);
    loop(3, 2, 3);
    loop(3, 5, 4);
    loop(3, 4, 3);
#undef loop


  //        move(0, 0);
    sprintf(s, "%s\n%s\n%s\n%s", m[0], m[1], m[2], m[3]);
    puts(s);
  //        addstr(s);
  //        refresh();
  //        sleep(1);
  //}

    return 0;
}

void finish(int sig) {
    //endwin();
    exit(0);
}

void uc2bin(int c, char *s) { /* s must point to at least CHAR_BIT+1 chars */

size_t i;
unsigned char uc = c;


for(i = 0; i < CHAR_BIT; i++, uc <<= 1)
    *s++ = !!(uc & (~(UCHAR_MAX >> 1))) + '0';
*s = 0;
}
