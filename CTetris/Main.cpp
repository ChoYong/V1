#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "colors.h"
#include "CTetris.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {
  char ch;
  int n;
  tty_raw(0);
  while (1) {
    if ((n = read(0, &ch, 1)) > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
	if (saved_key != 0) {
	  ch = saved_key;
	  saved_key = 0;
	  break;
	}
      }
    }
  }
  tty_reset(0);
  return ch;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "signal error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

int T0D0[] = { 0, 10, 0, 10, 10, 10, 0,  0, 0, -1 };
int T0D1[] = { 0, 10, 0,  0, 10, 10, 0, 10, 0, -1 };
int T0D2[] = { 0,  0, 0, 10, 10, 10, 0, 10, 0, -1 };
int T0D3[] = { 0, 10, 0, 10, 10,  0, 0, 10, 0, -1 };

int T1D0[] = { 20,  0,  0, 20, 20, 20,  0,  0,  0, -1 };
int T1D1[] = {  0, 20, 20,  0, 20,  0,  0, 20,  0, -1 };
int T1D2[] = {  0,  0,  0, 20, 20, 20,  0,  0, 20, -1 };
int T1D3[] = {  0, 20,  0,  0, 20,  0, 20, 20,  0, -1 };

int T2D0[] = {  0,  0, 30, 30, 30, 30,  0,  0,  0, -1 };
int T2D1[] = {  0, 30,  0,  0, 30,  0,  0, 30, 30, -1 };
int T2D2[] = {  0,  0,  0, 30, 30, 30, 30,  0,  0, -1 };
int T2D3[] = { 30, 30,  0,  0, 30,  0,  0, 30,  0, -1 };

int T3D0[] = {  0, 40, 0, 40, 40,  0, 40, 0, 0, -1 };
int T3D1[] = { 40, 40, 0,  0, 40, 40,  0, 0, 0, -1 };
int T3D2[] = {  0, 40, 0, 40, 40,  0, 40, 0, 0, -1 };
int T3D3[] = { 40, 40, 0,  0, 40, 40,  0, 0, 0, -1 };

int T4D0[] = { 0, 50, 0, 0, 50, 50,  0,  0, 50, -1 };
int T4D1[] = { 0,  0, 0, 0, 50, 50, 50, 50,  0, -1 };
int T4D2[] = { 0, 50, 0, 0, 50, 50,  0,  0, 50, -1 };
int T4D3[] = { 0,  0, 0, 0, 50, 50, 50, 50,  0, -1 };

int T5D0[] = { 0, 0, 0, 0, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T5D1[] = { 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, -1 };
int T5D2[] = { 0, 0, 0, 0, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T5D3[] = { 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, -1 };

int T6D0[] = { 70, 70, 70, 70, -1 };
int T6D1[] = { 70, 70, 70, 70, -1 };
int T6D2[] = { 70, 70, 70, 70, -1 };
int T6D3[] = { 70, 70, 70, 70, -1 };
  
int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen_1(CTetris *board)
{
  int dy = board->oScreen->get_dy();
  int dx = board->oScreen->get_dx();
  int dw = board->iScreenDw;
  int **array = board->oScreen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	cout << "□ ";
      else if (array[y][x] == 10)
	cout << "◈ ";
      else if (array[y][x] == 20)
	cout << "★ ";
      else if (array[y][x] == 30)
	cout << "● ";
      else if (array[y][x] == 40)
	cout << "◆ ";
      else if (array[y][x] == 50)
	cout << "▲ ";
      else if (array[y][x] == 60)
	cout << "♣ ";
      else if (array[y][x] == 70)
	cout << "♥ ";
      else
	cout << "■ ";
    }
    cout << endl;
  }
}

void drawScreen_2(CTetris *board)
{
  int dy = board->oScreen->get_dy();
  int dx = board->oScreen->get_dx();
  int dw = board->iScreenDw;
  int **array = board->oScreen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	cout << color_black << "□ " << color_normal;
      else if (array[y][x] == 10)
	cout << color_black << "■ " << color_normal;
      else if (array[y][x] == 20)
	cout << color_red << "■ " << color_normal;
      else if (array[y][x] == 30)
	cout << color_green << "■ " << color_normal;
      else if (array[y][x] == 40)
	cout << color_yellow << "■ " << color_normal;
      else if (array[y][x] == 50)
	cout << color_blue << "■ " << color_normal;
      else if (array[y][x] == 60)
	cout << color_magenta << "■ " << color_normal;
      else if (array[y][x] == 70)
	cout << color_cyan << "■ " << color_normal;
      else // array[y][x] == 1 // wall
	cout << b_color_black << "■ " << color_normal;
    }
    cout << endl;
  }
}

void drawScreen(CTetris *board) { drawScreen_2(board); }
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

void doRun(CTetris *board)
{
  char key;
  TetrisState state;

  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);

  while (key != 'q') {
    state = board->accept(key);
    if (state == NewBlock) {
      key = (char)('0' + rand() % MAX_BLK_TYPES);
      state = board->accept(key);
      if (state == Finished) {
	drawScreen(board);
	cout << endl;
	break;
      }
    }
    drawScreen(board);
    cout << endl;
    key = getch();
    cout << key << endl;
  }
}

int main(int argc, char *argv[]) {
  char line[128];
  int dy, dx;

  if (argc != 3) {
    cout << "usage: " << argv[0] << " dy dx" << endl;
    exit(1);
  }
  if ((dy = atoi(argv[1])) <= 0 || (dx = atoi(argv[2])) <= 0) {
    cout << "dy and dx should be greater than 0" << endl;
    exit(1);
  }

  CTetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  OnCLeft  *myOnLeft  = new OnCLeft();
  OnCRight *myOnRight = new OnCRight();
  OnCDown  *myOnDown  = new OnCDown();
  OnCUp    *myOnUp    = new OnCUp();
  OnCDrop  *myOnDrop  = new OnCDrop();
  OnCCw    *myOnCw    = new OnCCw();
  OnCCcw   *myOnCcw   = new OnCCcw();
  OnCNewBlock *myOnNewBlock = new OnCNewBlock();
  OnCFinished *myOnFinished = new OnCFinished();
  CTetris::setOperation('a', Running, myOnLeft, Running, myOnRight, Running);
  CTetris::setOperation('d', Running, myOnRight, Running, myOnLeft, Running);
  CTetris::setOperation('s', Running, myOnDown, Running, myOnUp, NewBlock);
  CTetris::setOperation('w', Running, myOnCw, Running, myOnCcw, Running);
  CTetris::setOperation(' ', Running, myOnDrop, Running, myOnUp, NewBlock);
  CTetris::setOperation('0', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('1', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('2', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('3', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('4', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('5', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);
  CTetris::setOperation('6', NewBlock, myOnNewBlock, Running, myOnFinished, Finished);

  CTetris *board = new CTetris(dy, dx);

  registerAlarm();
  doRun(board);
  
  delete board;
  delete myOnLeft;
  delete myOnRight;
  delete myOnDown;
  delete myOnUp;
  delete myOnCw;
  delete myOnCcw;
  delete myOnNewBlock;
  delete myOnFinished;

  cout << "Program terminated!" << endl;

  return 0;
}
