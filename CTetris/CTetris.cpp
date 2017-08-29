#include "CTetris.h"
#include "colors.h"

CTetris::CTetris(int cy, int cx) : Tetris(cy, cx) {
  cout << "CTetris() called" << endl;
};

CTetris::~CTetris() { // ~Tetris() is automatically called!
  //  if (iScreen != NULL) { delete iScreen; iScreen = NULL; }
  //  if (oScreen != NULL) { delete oScreen; oScreen = NULL; }
  //  if (currBlk != NULL) { delete currBlk; currBlk = NULL; }  
  cout << "~CTetris() called" << endl;
}

bool CTetris::anyConflict(bool updateNeeded) {
  bool anyConflict;
  Matrix *tCBlk1, *tCBlk2; // color blocks
  Matrix *tBBlk1, *tBBlk2, *tBBlk3; // binary blocks

  tCBlk1 = iScreen->clip(top, left, top + currBlk->get_dy(),
			left + currBlk->get_dx());
  tCBlk2 = tCBlk1->add(currBlk);
  tBBlk1 = tCBlk1->int2bool();
  tBBlk2 = currBlk->int2bool();
  tBBlk3 = tBBlk1->add(tBBlk2);
  
  anyConflict = tBBlk3->anyGreaterThan(1);
  delete tBBlk3;
  delete tBBlk2;
  delete tBBlk1;
  
  if (updateNeeded == true) {
    oScreen->paste(iScreen, 0, 0);
    oScreen->paste(tCBlk2, top, left);
  }
  delete tCBlk2;
  delete tCBlk1;
  
  return anyConflict;
}
