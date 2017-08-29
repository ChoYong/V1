#pragma once
#include "Matrix.h"

#define MAX_TET_OPS 100

enum TetrisState { Running, NewBlock, Finished };

class Tetris;

class ActionHandler {
public:
  virtual bool run(Tetris *t, char key) = 0;
};

class Tetris {
  class TetrisOperation { 
  public:
    char key;
    ActionHandler *hDo, *hUndo;
    TetrisState preState, postStateDo, postStateUndo;
  };
  ///////////////////////////////////////////////////////////////
  /////////////// static variables and functions ////////////////
  ///////////////////////////////////////////////////////////////  
 protected:
  static int nobj;
  static int nops;
  static TetrisOperation *operations[MAX_TET_OPS];
  static TetrisOperation *findOperationByKey(char key);
  static int nBlockTypes; // number of block types
  static int findLargestBlockSize(int *setOfArrays[]);
  static Matrix ***createSetOfBlocks(int *setOfArrays[]);
 public:
  static int nBlockDegrees; // number of block degrees
  static Matrix ***setOfBlockObjects; // Matrix object array of all blocks
  static int iScreenDw; // size of the largest block
  static void init(int *setOfBlockArrays[], int types, int degrees);
  static void setOperation(char key, TetrisState preState, ActionHandler *hDo,
			   TetrisState postStateDo, ActionHandler *hUndo,
			   TetrisState postStateUndo);
  ///////////////////////////////////////////////////////////////
  /////////////// dynamic variables and functions ///////////////
  ///////////////////////////////////////////////////////////////  
 protected:
  TetrisState state;
  int *createArrayScreen(int dy, int dx, int dw);
 public:
  int iScreenDy; // height of the background screen
  int iScreenDx; // width of the background screen
  int top; // y of the top left corner of the current block
  int left; // x of the top left corner of the current block
  int idxBlockType; // index for the current block type
  int idxBlockDegree; // index for the current block degree
  Matrix *iScreen = NULL; // input screen (as background)
  Matrix *oScreen = NULL; // output screen
  Matrix *currBlk = NULL; // current block
  Tetris(int cy, int cx);
  ~Tetris();
  void drawScreen();
  bool anyConflict(bool updateNeeded);
  TetrisState accept(char key);
};

class OnLeft : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->left = t->left - 1;
    return t->anyConflict(true);
  }
};

class OnRight : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->left = t->left + 1;
    return t->anyConflict(true);
  }
};

class OnDown : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->top = t->top + 1;
    return t->anyConflict(true);
  }
};

class OnUp : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->top = t->top - 1;
    return t->anyConflict(true);
  }
};

class OnDrop : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    do { t->top = t->top + 1; }
    while (t->anyConflict(false) == false);
    return t->anyConflict(true);
  }
};

class OnCw : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->idxBlockDegree = (t->idxBlockDegree + 1) % t->nBlockDegrees;
    t->currBlk = t->setOfBlockObjects[t->idxBlockType][t->idxBlockDegree];
    return t->anyConflict(true);
  }
};

class OnCcw : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->idxBlockDegree = (t->idxBlockDegree + t->nBlockDegrees - 1) % t->nBlockDegrees;
    t->currBlk = t->setOfBlockObjects[t->idxBlockType][t->idxBlockDegree];
    return t->anyConflict(true);
  }
};

class OnNewBlock : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    t->oScreen = deleteFullLines(t->oScreen, t->currBlk, t->top,
				 t->iScreenDy, t->iScreenDx, t->iScreenDw);
    t->iScreen = new Matrix(t->oScreen);
    t->idxBlockType = key - '0';
    t->idxBlockDegree = 0;
    t->currBlk = t->setOfBlockObjects[t->idxBlockType][t->idxBlockDegree];
    t->top = 0;
    t->left = t->iScreenDw + t->iScreenDx / 2 - (t->currBlk->get_dx()+1) / 2;
    return t->anyConflict(true);
  }
 protected:
  Matrix *deleteFullLines(Matrix *screen, Matrix *blk, int top,
			  int dy, int dx, int dw) {
    Matrix *line, *zero, *temp;
    if (blk == NULL) // called right after the game starts. 
      return screen; // no lines to be deleted
    int cy, y, nDeleted = 0, nScanned = blk->get_dy();
    if (top + blk->get_dy() - 1 >= dy)
      nScanned -= (top + blk->get_dy() - dy);
    zero = new Matrix(1, dx - 2*dw);
    for (y = nScanned - 1; y >= 0; y--) {
      cy = top + y + nDeleted;
      line = screen->clip(cy, 0, cy + 1, screen->get_dx());
      if (line->sum() == screen->get_dx()) {
	temp = screen->clip(0, 0, cy, screen->get_dx());
	screen->paste(temp, 1, 0);
	screen->paste(zero, 0, dw);
	nDeleted++;
	delete temp;
      }
      delete line;
    }
    delete zero;
    return screen;
  }
};

class OnFinished : public ActionHandler {
 public:
  bool run(Tetris *t, char key) {
    cout << "OnFinished.run() called" << endl;
    return false;
  }
};
