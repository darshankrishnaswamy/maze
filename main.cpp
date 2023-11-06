#include <iostream>
#include <vector>
#include <unordered_map>
/*
 * Orderings: [F, R, B, L, U, D]
 * */
// indices in form (face(int), x, y). Orientations relative to starting position
// (doesn't really matter)

// neighbors in form (l, u, r, d)

/*
 * For each node:
 *      for each direction:
 *          if on same face:
 *              easy
 *          else:
 *              use current face and direction to find next face
 *              set neighbor to that face's next node
 */

std::vector<int> *getLeft(int face, int i) {
    int left[6][3]= {
            {3, i, 4},
            {0, i, 4},
            {1, i, 4},
            {2, i, 4},
            {3, 0, i},
            {3, 4, 5-i}};
    auto out = new std::vector<int>(3);
    out->push_back(left[face][0]);
    out->push_back(left[face][1]);
    out->push_back(left[face][2]);
    return out;
}

int main() {
    int maze[6][5][5];
    for(auto & i : maze) {
        for(auto & j : i) {
            for(int & k : j) {
                k = 0; // wall
            }
        }
    }

    auto adjMat = new std::vector<std::vector<int>*>[6][5][5]();
    for(int faceNum = 0; faceNum < 6; faceNum++) {
        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 5; j++) {
                // LEFT, UP, RIGHT, DOWN
                // LEFT
                if(j > 0) {
                    auto *x = new std::vector<int>(3);
                    x->push_back(faceNum);
                    x->push_back(i);
                    x->push_back(j-1);
                    adjMat[faceNum][i][j].push_back(x);
                }
                else {
                    // work
                }

                // UP
                if(i > 0) {
                    auto *x = new std::vector<int>(3);
                    x->push_back(faceNum);
                    x->push_back(i-1);
                    x->push_back(j);
                    adjMat[faceNum][i][j].push_back(x);
                }
                else {
                    adjMat[faceNum][i][j].push_back(getLeft(faceNum, i));
                }

                // RIGHT
                if(j < 4) {
                    auto *x = new std::vector<int>(3);
                    x->push_back(faceNum);
                    x->push_back(i);
                    x->push_back(j+1);
                    adjMat[faceNum][i][j].push_back(x);
                }
                else {
                    // work
                }

                // DOWN
                if(i < 4) {
                    auto *x = new std::vector<int>(3);
                    x->push_back(faceNum);
                    x->push_back(i+1);
                    x->push_back(j);
                    adjMat[faceNum][i][j].push_back(x);
                }
                else {
                    // work
                }



            }
        }
    }
}
