#include "Tetris.h"

///////////////////////////////////////////////////////////////////
///////////////////////// Member variables ////////////////////////
///////////////////////////////////////////////////////////////////
int Tetris::nobj = 0;
int Tetris::nops = 0;
Tetris::TetrisOperation *Tetris::operations[MAX_TET_OPS];
int Tetris::iScreenDw = 0;
int Tetris::nBlockDegrees = 0;
int Tetris::nBlockTypes = 0;
Matrix ***Tetris::setOfBlockObjects = NULL;

///////////////////////////////////////////////////////////////////
///////////////////// Non-member functions ////////////////////////
///////////////////////////////////////////////////////////////////
int max(int a, int b) { return (a > b ? a : b); }

int findBlockSize(int array[]) {
  int len, size;
  for (len = 0; array[len] != -1; len++);
  for (size = 0; size*size < len; size++);
  return size;
}

///////////////////////////////////////////////////////////////////
///////////////////////// Member functions ////////////////////////
///////////////////////////////////////////////////////////////////
int *Tetris::createArrayScreen(int dy, int dx, int dw) {
  int y, x;
  int Dy = dy + dw;
  int Dx = dx + 2 * dw;
  int *array = new int[Dy * Dx];
  for (y = 0; y < dy + dw; y++)
    for (x = 0; x < dx + 2 * dw; x++)
      array[y * Dx + x] = 0;
  for (y = 0; y < dy; y++) {
    for (x = 0; x < dw; x++)
      array[y * Dx + x] = 1;
    for (x = dw + dx; x < Dx; x++)
      array[y * Dx + x] = 1;
  }
  for (y = dy; y < Dy; y++)
    for (x = 0; x < Dx; x++)
      array[y * Dx + x] = 1;
  return array;
}

int Tetris::findLargestBlockSize(int *setOfArrays[]) {
  int size, max_size = 0;
  for (int t = 0; t < nBlockTypes; t++) {
    for (int d = 0; d < nBlockDegrees; d++) {
      size = findBlockSize(setOfArrays[t * nBlockDegrees + d]);
      max_size = max(max_size, size);
    }
  }
  return max_size;
}

Matrix ***Tetris::createSetOfBlocks(int *setOfArrays[]) {
  int t, d, size;
  Matrix ***setOfObjects = new Matrix**[nBlockTypes];
  for (t = 0; t < nBlockTypes; t++)
    setOfObjects[t] = new Matrix*[nBlockDegrees];
  for (t = 0; t < nBlockTypes; t++) {
    for (d = 0; d < nBlockDegrees; d++) {
      size = findBlockSize(setOfArrays[t * nBlockDegrees + d]);
      setOfObjects[t][d] = new Matrix(setOfArrays[t * nBlockDegrees + d],
				      size, size);
    }
  }
  return setOfObjects;
}

void Tetris::init(int *setOfBlockArrays[], int types, int degrees) {
  nBlockTypes = types; //nBlockTypes = setOfBlockArrays.length;
  nBlockDegrees = degrees; //nBlockDegrees = setOfBlockArrays[0].length;
  setOfBlockObjects = createSetOfBlocks(setOfBlockArrays);
  iScreenDw = findLargestBlockSize(setOfBlockArrays);
}

Tetris::TetrisOperation *Tetris::findOperationByKey(char key) {
  TetrisOperation *hop = NULL;
  for (int id = 0; operations[id] != NULL; id++) {
    if (operations[id]->key == key) {
      hop = operations[id];
      break;
    }
  }
  return hop;
}

void Tetris::setOperation(char key,
			  TetrisState preState, ActionHandler *hDo,
			  TetrisState postStateDo, ActionHandler *hUndo,
			  TetrisState postStateUndo) {
  if (nops == MAX_TET_OPS) {
    cerr << "Tetris::operations[] is full." << endl;
    return;
  }

  operations[nops] = new TetrisOperation();
  operations[nops]->key = key;
  operations[nops]->hDo = hDo;
  operations[nops]->hUndo = hUndo;
  operations[nops]->preState = preState;
  operations[nops]->postStateDo = postStateDo;
  operations[nops]->postStateUndo = postStateUndo;
  nops++;
}

Tetris::~Tetris() {
  cout << "~Tetris() called: nobj = " << nobj << endl;
  nobj--;
  if (iScreen != NULL) { delete iScreen; iScreen = NULL; }
  if (oScreen != NULL) { delete oScreen; oScreen = NULL; }
  //if (currBlk != NULL) { delete currBlk; currBlk = NULL; }
  // currBlk should not be freed because it points to a Matrix object
  // created by a static function, i.e., createSetOfBlocks.
  // it should be freed in the below...
  if (nobj == 0) { // free allocations for static members
    for (int n = 0; n < nops; n++)
      delete operations[n];
    for (int t = 0; t < nBlockTypes; t++)
      for (int d = 0; d < nBlockDegrees; d++)
	delete setOfBlockObjects[t][d];
    for (int t = 0; t < nBlockTypes; t++)
      delete setOfBlockObjects[t];
    delete setOfBlockObjects;
    cout << "Tetris' static members are freed." << endl;
  }
}

Tetris::Tetris(int cy, int cx) {
  if (cy < iScreenDw || cx < iScreenDw) {
    cout << "Tetris() called: too small screen!" << endl;
    return;
  }
  nobj++;
  iScreenDy = cy;
  iScreenDx = cx;
  iScreen = new Matrix(createArrayScreen(iScreenDy, iScreenDx, iScreenDw),
		       iScreenDy + iScreenDw, iScreenDx + 2 * iScreenDw);
  oScreen = new Matrix(iScreen);
  state = NewBlock;
  cout << "Tetris() called: nobj = " << nobj << endl;
}

bool Tetris::anyConflict(bool updateNeeded) {
  bool anyConflict;
  Matrix *tBlk1, *tBlk2;
  tBlk1 = iScreen->clip(top, left, top + currBlk->get_dy(),
			  left + currBlk->get_dx());
  tBlk2 = tBlk1->add(currBlk);
  if (updateNeeded == true) {
    oScreen->paste(iScreen, 0, 0);
    oScreen->paste(tBlk2, top, left);
  }
  anyConflict = tBlk2->anyGreaterThan(1);
  delete tBlk2;
  delete tBlk1;  
  
  return anyConflict;
}

TetrisState Tetris::accept(char key) {
  TetrisOperation *hop = findOperationByKey(key);
  if (hop == NULL) {
    cout << "unknown key!" << endl;
    return state;
  }
  if (state != hop->preState) {
    cout << "state mismatch for the current key!" << endl;
    return state;
  }
  if ((hop->hDo)->run(this, key) == false) // no conflicts!
    return state = hop->postStateDo;
  else { // a conflict occurs!
    hop->hUndo->run(this, key); 
    return state = hop->postStateUndo;
  }
}

void Tetris::drawScreen() {
  Matrix *screen = oScreen;
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = iScreenDw;
  int **array = screen->get_array();
  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	cout << "□ ";
      else if (array[y][x] == 1)
	cout << "■ ";
      else
	cout << "X ";
    }
    cout << endl;
  }
}
