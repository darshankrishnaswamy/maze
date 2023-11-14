#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 150

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
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


byte * getLeft(byte face, byte i, byte j) {
    if(j > 0) {
        byte *out = new byte[3] {face, i, j+1};
        return out;
    }
    byte left[6][3] = {
      {3, i, 4},
      {0, i, 4},
      {1, i, 4},
      {2, i, 4},
      {3, 0, i},
      {3, 4, 4-i}
    };
    byte *out = new byte[3] {left[face][0], left[face][1], left[face][2]};
    return out;
}

byte * getRight(byte face, byte i, byte j) {
    if(j < 4) {
      byte *out = new byte[3] {face, i, j+1};
      return out;
    }
    byte right[6][3] = {
              {1, i, 0},
              {2, i, 0},
              {3, i, 0},
              {0, i, 0},
              {1, 0, 4-i},
              {1, 4, i}
      };
    byte *out = new byte[3] {right[face][0], right[face][1], right[face][2]};
    return out;
}

byte * getUp(byte face, byte i, byte j) {
    if(i > 0) {
      byte *out = new byte[3] {face, i-1, j};
      return out;
    }
    byte up[6][3] = {
            {4, 4, j},
            {4, 4-j, 4},
            {4, 0, 4-j},
            {4, j, 0},
            {2, 0, 4-j},
            {0, 4, j}
    };
    byte *out = new byte[3] {up[face][0], up[face][1], up[face][2]};
    return out;

}

byte * getDown(byte face, byte i, byte j) {
    if(i < 4) {
      byte *out = new byte[3] {face, i+1, j};
      return out; 
    }
    byte down[6][3] = {
            {5, 0, j},
            {5, j, 4},
            {5, 4, 4-j},
            {5, 4-j, 0},
            {0, 0, j},
            {2, 4, 4-j}
    };
    byte *out = new byte[3] {down[face][0], down[face][1], down[face][2]};
    return out;
  
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
            byte (&adjMat)[6][5][5][4][3]) {
    byte stack[150][3];
    int sp = 0;
    for(int i = 0; i < 60; i++) {
      for(int j = 0; j < 3; j++) {
        stack[i][j] = 0;
      }
    }
    // Array<Array<Array<byte, 3>, 2>> stack;
    bool visited[6][5][5];
    for(int i = 0; i < 6; i++) {
      for(int j = 0; j < 5; j++) {
        for(int k = 0; k < 5; k++) {
          visited[i][j][k] = false;
        }
      }
    }
    maze[0][2][2] = 2;
    visited[0][2][2] = true;
    int dir1 = (random(2)) * 2 - 1;
    int dir2 = (random(2)) * 2 - 1;
    byte initial[3] = {0, 0, 0};
    initial[0] |= 0;
    initial[0] |= (0 << 4);
    initial[1] |= 2;
    initial[1] |= (2 << 4);
    initial[2] |= 2;
    initial[2] |= (2 << 4);
    // byte initial[2][3] = {{0, 2, 2}, {0, 2, 2}};
    initial[dir1+1] += dir2;
    initial[dir1+1] += (dir2 * 2) << 4;
      for(int j = 0; j < 3; j++) {
        stack[sp][j] = initial[j];
      }
    sp += 1;
    visited[initial[0] % 16][initial[1] % 16][initial[2] % 16] = true;
    visited[initial[0] >> 4][initial[1] >> 4][initial[2] >> 4] = true;
    bool stopCondition = false;
    while(!stopCondition) {
        byte removed[3];
        
        for(int j = 0; j < 3; j++) {
          removed[j] = stack[sp-1][j];
        }
        // stack.pop_back();
        sp -= 1;
        maze[removed[0] % 16][removed[1] % 16][removed[2] % 16] = 1;
        maze[removed[0] >> 4][removed[1] >> 4][removed[2] >> 4] = 1;
        auto neighbors = adjMat[removed[0] >> 4][removed[1] >> 4][removed[2] >> 4];
        int validInds[4];
        int numValid = 0;
        for(int i = 0; i < 4; i++) {
            auto neighbor2 = neighbors[i];
            if(!visited[neighbor2[0] >> 4][neighbor2[1] >> 4][neighbor2[2] >> 4] && !visited[neighbor2[0] % 16][neighbor2[1] % 16][neighbor2[2] % 16]) {
                validInds[numValid] = i;
                numValid++;
            }
        }
        if(numValid) {
            int ind = validInds[random(numValid)];
            for(int j = 0; j < 3; j++) {
              stack[sp][j] = neighbors[ind][j];
            
            }
            sp++;
            // stack.push(neighbors[ind]);
            visited[neighbors[ind][0] % 16][neighbors[ind][1] % 16][neighbors[ind][2] % 16] = true;
            visited[neighbors[ind][0] >> 4][neighbors[ind][1] >> 4][neighbors[ind][2] >> 4] = true;
            for(int j = 0; j < 3; j++) {
              stack[sp][j] = removed[j];
            }
            
            sp++;
        }
        stopCondition = (maze[2][2][2] != 0);
    }
    setGoal(maze);


}



void generateMaze(byte (&maze)[6][5][5]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                maze[i][j][k] = 0; // wall
            }
        }
    }
  
    byte adjMat[6][5][5][4][3];
    for (int faceNum = 0; faceNum < 6; faceNum++) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                byte *leftNeighbor = getLeft(faceNum, i, j);
                byte *leftNeighbor2 = getLeft(leftNeighbor[0], leftNeighbor[1], leftNeighbor[2]);
                for(int k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][0][k] &= 0;
                  adjMat[faceNum][i][j][0][k] |= leftNeighbor[k];
                  adjMat[faceNum][i][j][0][k] |= (leftNeighbor2[k] << 4);
                }
                delete leftNeighbor;
                delete leftNeighbor2;                

                byte *upNeighbor = getUp(faceNum, i, j);
                byte *upNeighbor2 = getUp(upNeighbor[0], upNeighbor[1], upNeighbor[2]);
                for(int k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][1][k] &= 0;
                  adjMat[faceNum][i][j][1][k] |= upNeighbor[k];
                  adjMat[faceNum][i][j][1][k] |= (upNeighbor2[k] << 4);
                }
                delete upNeighbor;
                delete upNeighbor2;    

                byte *rightNeighbor = getRight(faceNum, i, j);
                byte *rightNeighbor2 = getRight(rightNeighbor[0], rightNeighbor[1], rightNeighbor[2]);
                for(int k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][2][k] &= 0;
                  adjMat[faceNum][i][j][2][k] |= rightNeighbor[k];
                  adjMat[faceNum][i][j][2][k] |= (rightNeighbor2[k] << 4);
                }
                delete rightNeighbor;
                delete rightNeighbor2;    
                byte *downNeighbor = getDown(faceNum, i, j);
                byte *downNeighbor2 = getDown(downNeighbor[0], downNeighbor[1], downNeighbor[2]);
                for(int k = 0; k < 3; k++) {
                  adjMat[faceNum][i][j][3][k] &= 0;
                  adjMat[faceNum][i][j][3][k] |= downNeighbor[k];
                  adjMat[faceNum][i][j][3][k] |= (downNeighbor2[k] << 4);
                }
                delete downNeighbor;
                delete downNeighbor2;    
            }
        }
    }

    dfs(maze, adjMat);
    


}

void printMaze(byte maze[6][5][5]) {
  for(int i = 0; i < 6; i++) {
      for(int j = 0; j < 5; j++) {
        for(int k = 0; k < 5; k++) {
          Serial.print(maze[i][j][k]);
          Serial.print(" ");
        }
        Serial.println("");
      }
      Serial.println("");
    }
}

void printArray(byte flatMaze[150]) {
for(int i = 0; i < 150; i++) {
    Serial.print(flatMaze[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  byte maze[6][5][5];
  generateMaze(maze);
  
  byte flatMaze[150];
  for(int i = 0; i < 6; i++) {
      for(int j = 0; j < 5; j++) {
        for(int k = 0; k < 5; k++) {
          flatMaze[25*i + 5*j + k] = maze[i][j][k];
        }
      }
  }
  

      // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
    // END of Trinket-specific code.
    uint32_t colors[4] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(85, 85, 85) };

    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(5); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, colors[flatMaze[i]]);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
  }
  }

void loop() {

}
