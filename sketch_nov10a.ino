
/*
 * Orderings: [F, R, B, L, U, D]
 * (front, right, back, left, up, down)
 * The orientations of F, R, B, L are based on horizontal rotations from the starting configuration
 * The orientations of U and D are based on a single up/down rotation from the starting configuration
 * Indices in form (face number, i, j).
 * Neighbors in order (l, u, r, d)
 * Maze states:
 *      0 -> wall
 *      1 -> valid user space
 *      2 -> start space (can also represent user position as they move)
 *      3 -> goal state
 * */



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Here");
  // main();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("H");
}

byte * getLeft(byte face, byte i, byte j) {
    if(j > 0) {
        byte *out = new byte[3] {face, i, j+1};
        return out;
    }
    auto left = new byte[6][3] {
      {3, i, 4},
      {0, i, 4},
      {1, i, 4},
      {2, i, 4},
      {3, 0, i},
      {3, 4, 4-i}
      };
    return left[face];
}

byte * getRight(byte face, byte i, byte j) {
    if(j < 4) {
      byte *out = new byte[3] {face, i, j+1};
      return out;
    }
    auto right = new byte[6][3] {
              {1, i, 0},
              {2, i, 0},
              {3, i, 0},
              {0, i, 0},
              {1, 0, 4-i},
              {1, 4, i}
      };
    return right[face];
}

byte * getUp(byte face, byte i, byte j) {
    if(i > 0) {
      byte *out = new byte[3] {face, i-1, j};
      return out;
    }
    auto up = new byte[6][3] {
            {4, 4, j},
            {4, 4-j, 4},
            {4, 0, 4-j},
            {4, j, 0},
            {2, 0, 4-j},
            {0, 4, j}
    };
    return up[face];

}

byte * getDown(byte face, byte i, byte j) {
    if(i < 4) {
      byte *out = new byte[3] {face, i+1, j};
      return out; 
    }
    auto down = new byte[6][3] {
            {5, 0, j},
            {5, j, 4},
            {5, 4, 4-j},
            {5, 4-j, 0},
            {0, 0, j},
            {2, 4, 4-j}
    };
    return down[face];
  
}

// void prims(byte (&maze)[6][5][5],
//            byte (&adjMat)[6][5][5][4][2][3]) {
//     auto frontier = new byte[100][2][3](); 
//     byte index = 0;
//     // start pobyte will always be [0, 2, 2] -> center of front face
//     maze[0][2][2] = 1;
//     byte arr1[2][3] {{0, 2, 1}, {0, 2, 0}};
//     frontier->push_back(arr1);
//     frontier->push_back({{{0, 2, 3}, {0, 2, 4}}});
//     frontier->push_back({{{0, 1, 2}, {0, 0, 2}}});
//     frontier->push_back({{{0, 3, 2}, {0, 4, 2}}});

//     bool stopCondition = false; // need to figure this out
//     while (!stopCondition) {
//         byte index = rand() % frontier->size(); // better rng might be needed
//         Array<Array<byte, 3>, 2> removed = (*frontier)[index];
//         frontier->erase(frontier->begin() + index);
//         Array<byte, 3> first = removed[0], second = removed[1];
//         maze[first[0]][first[1]][first[2]] = 1;
//         maze[second[0]][second[1]][second[2]] = 1;

//         auto neighbors = adjMat[second[0]][second[1]][second[2]];
//         for(auto neighbor : neighbors) {
//             auto neighbor2 = neighbor[1];
//             if (!maze[neighbor2[0]][neighbor2[1]][neighbor2[2]] && find(frontier->begin(), frontier->end(), neighbor) == frontier->end()) {
//                 frontier->push_back(neighbor);
//             }
//         }
//         stopCondition = frontier->empty();
//     }
//     setGoal(maze);
// }

void setGoal(byte (&maze)[6][5][5]) {
    /**
     * Try to make the goal state the middle position on the back face
     * If not possible, just make it some arbitrary location on the back face
     * If still not possible, just find something not on the front face
     *  (guaranteed to exist)
     */
    auto& backFace = maze[2];
    bool found = false;
    if(backFace[2][2]) {
        backFace[2][2] = 3;
        found = true;
    }
    if(!found) {
        for (auto& row : backFace) {
            if(found)
                break;
            for(auto& item : row) {
                if(found)
                    break;
                else if (item) {
                    item = 3;
                    found = true;
                }
            }
        }
    }
    if(!found) {
        byte remainingFaces[] = {1, 3, 4, 5};
        for(byte i = 0; i < 4; i++) {
            byte faceNum = remainingFaces[i];
            auto face = maze[faceNum];
            for (byte j = 0; j < 5; j++) {
              auto row = face[j];
                if(found)
                    break;
                for(byte k = 0; k < 5; k++) {
                  auto item = row[k];
                    if(found)
                        break;
                    else if (item) {
                        item = 3;
                        found = true;
                    }
                }
            }
        }
    }
}

void dfs(byte (&maze)[6][5][5],
            byte (&adjMat)[6][5][5][4][2][3]) {
    byte stack[100][2][3];
    byte sp = 0;
    for(byte i = 0; i < 100; i++) {
      for(byte j = 0; j < 2; j++) {
        stack[i][j][0] = 0;
        stack[i][j][1] = 0;
        stack[i][j][2] = 0;
      }
    }
    // Array<Array<Array<byte, 3>, 2>> stack;
    bool visited[6][5][5];
    for(byte i = 0; i < 6; i++) {
      for(byte j = 0; j < 5; j++) {
        for(byte k = 0; k < 5; k++) {
          visited[i][j][k] = false;
        }
      }
    }
    maze[0][2][2] = 2;
    visited[0][2][2] = true;
    byte dir1 = (rand() % 2) * 2 - 1;
    byte dir2 = (rand() % 2) * 2 - 1;
    byte initial[2][3] = {{0, 2, 2}, {0, 2, 2}};
    initial[0][dir1+1] += dir2;
    initial[1][dir1+1] += dir2 * 2;
    for(byte i = 0; i < 2; i++) {
      for(byte j = 0; j < 3; j++) {
        stack[sp][i][j] = initial[i][j];
      }
    }
    sp += 1;
    visited[initial[0][0]][initial[0][1]][initial[0][2]] = true;
    visited[initial[1][0]][initial[1][1]][initial[1][2]] = true;
    byte stopCondition = false;
    while(!stopCondition) {
        byte removed[2][3];
        for(byte i = 0; i < 2; i++) {
          for(byte j = 0; j < 3; j++) {
            removed[i][j] = stack[sp-1][i][j];
          }
        }
        // stack.pop_back();
        sp -= 1;
        maze[removed[0][0]][removed[0][1]][removed[0][2]] = 1;
        maze[removed[1][0]][removed[1][1]][removed[1][2]] = 1;
        auto neighbors = adjMat[removed[1][0]][removed[1][1]][removed[1][2]];
        bool validInds[4];
        for(byte i = 0; i < 4; i++) {
          validInds[i] = false;
        }
        byte numValid = 0;
        for(byte i = 0; i < 4; i++) {
            auto neighbor2 = neighbors[i][1];
            if(!visited[neighbor2[0]][neighbor2[1]][neighbor2[2]]) {
                validInds[numValid] = i;
                numValid++;
            }
        }
        if(numValid) {
            byte ind = validInds[rand() % numValid];
            for(byte i = 0; i < 2; i++) {
              for(byte j = 0; j < 3; j++) {
                stack[sp][i][j] = neighbors[ind][i][j];
              }
            }
            sp--;
            // stack.push(neighbors[ind]);
            visited[neighbors[ind][0][0]][neighbors[ind][0][1]][neighbors[ind][0][2]] = true;
            visited[neighbors[ind][1][0]][neighbors[ind][1][1]][neighbors[ind][1][2]] = true;
            for(byte i = 0; i < 2; i++) {
              for(byte j = 0; j < 3; j++) {
                stack[sp][i][j] = removed[i][j];
              }
            }
            sp++;
        }
        stopCondition = (sp == 0);
    }
    setGoal(maze);


}



int main() {
    byte maze[6][5][5];
    for (auto &i: maze) {
        for (auto &j: i) {
            for (byte &k: j) {
                k = 0; // wall
            }
        }
    }

    // Array<Array<Array<Array<Array<Array<byte, 3>, 2>, 4>, 5>, 5>, 6> adjMat{};
    byte adjMat[6][5][5][4][2][3];
    for (byte faceNum = 0; faceNum < 6; faceNum++) {
        for (byte i = 0; i < 5; i++) {
            for (byte j = 0; j < 5; j++) {
                byte *leftNeighbor = getLeft(faceNum, i, j);
                byte *leftNeighbor2 = getLeft(leftNeighbor[0], leftNeighbor[1], leftNeighbor[2]);
                for(byte k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][0][0][k] = leftNeighbor[k];
                  adjMat[faceNum][i][j][0][1][k] = leftNeighbor2[k];
                }
                

                byte *upNeighbor = getUp(faceNum, i, j);
                byte *upNeighbor2 = getUp(upNeighbor[0], upNeighbor[1], upNeighbor[2]);
                for(byte k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][1][0][k] = upNeighbor[k];
                  adjMat[faceNum][i][j][1][1][k] = upNeighbor2[k];
                }

                byte *rightNeighbor = getRight(faceNum, i, j);
                byte *rightNeighbor2 = getRight(rightNeighbor[0], rightNeighbor[1], rightNeighbor[2]);
                for(byte k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][2][0][k] = rightNeighbor[k];
                  adjMat[faceNum][i][j][2][1][k] = rightNeighbor2[k];
                }

                byte *downNeighbor = getDown(faceNum, i, j);
                byte *downNeighbor2 = getDown(downNeighbor[0], downNeighbor[1], downNeighbor[2]);
                for(byte k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][3][0][k] = downNeighbor[k];
                  adjMat[faceNum][i][j][3][1][k] = downNeighbor2[k];
                }
            }
        }
    }

    dfs(maze, adjMat);
    for(byte i = 0; i < 6; i++) {
      for(byte j = 0; j < 5; j++) {
        for(byte k = 0; k < 5; k++) {
          Serial.print(maze[i][j][k]);
          Serial.print(" ");
        }
        Serial.println("");
      }
      Serial.println("");
    }



}



