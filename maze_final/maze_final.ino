#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 150

// m/s^2, how much corresponds to movement (scale of 0 to 9.8)
#define THRESHOLD 2

// These are directions, NOT face numbers
#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3

// user starting position, gets updated as the user moves
int userx = 4;
int usery = 2;
int userz = 2;

bool FINISHED = false;

byte officialMaze[6][5][5];


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;

uint32_t colors[4] = {strip.Color(150, 0, 0), strip.Color(96, 156, 48), strip.Color(0, 255, 0), strip.Color(255, 0, 255) };

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

// takes in a current location (face, i, j) -> maps to the location left of it in the same form (nextface, nexti, nextj)
// similar functions for right, up, down as well
byte * getLeft(byte face, byte i, byte j) {
    if(j > 0) {
        byte *out = new byte[3] {face, i, j-1};
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

/* Implementation of prims algo for maze generation. Not in use because it was making mazes with too many paths. 
   Also written using C++ stdlib so it doesn't compile for arduino which is why it's commented out. Can be adapted to work 
   Arduino like the DFS algorithm, but currently no need to do that.
*/

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
     * Try to make the goal state the middle position on the bottom face
     * If not possible, just make it some arbitrary location on the bottom face
     * If still not possible, just find something not on the top face
     * Don't think it ever ends up needing to do anything other than the first case,
     *  but the backup plan is there just in case
     */
    auto& bottomFace = maze[5];
    bool found = false;
    if(bottomFace[2][2]) {
        bottomFace[2][2] = 3;
        found = true;
    }
    if(!found) {
        for (auto& row : bottomFace) {
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
        byte remainingFaces[] = {1, 3, 0, 2};
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
    /**
    * Performs a DFS on the maze to generate the legal positions
    */
    byte stack[150][3];
    int sp = 0;
    for(int i = 0; i < 150; i++) {
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
    maze[4][2][2] = 2;
    visited[4][2][2] = true;
    randomSeed(analogRead(0));
    int dir1 = (random(2)) * 2 - 1;
    int dir2 = (random(2)) * 2 - 1;
    byte initial[3] = {0, 0, 0};
    initial[0] |= 4;
    initial[0] |= (4 << 4);
    initial[1] |= 2;
    initial[1] |= (2 << 4);
    initial[2] |= 2;
    initial[2] |= (2 << 4);
    initial[(dir1+1)/2+1] += dir2;
    initial[(dir1+1)/2+1] += (dir2 * 2) << 4;
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
        stopCondition = (maze[5][2][2] != 0);
    }
    setGoal(maze);


}



void generateMaze(byte (&maze)[6][5][5]) {
  /**
  * Generates the full maze. Creates the adjacency list, runs the DFS, and sets the start and goal positions.
  */
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


byte getDirection(byte face, float x, float y, float z) {
  /**
  * Uses the current face and the accelerometer readings to determine which direction to move in (or -1 if it's not tilted enough)
  * Can be converted to a transition matrix to make it look cleaner but would use too much memory, so had to use a lot of if statements.
  */

  // return 0 if left, 1 if up, 2 if right, 3 if down

  if(face == 4) {
    if(abs(x) > THRESHOLD && abs(y) > THRESHOLD) {
      if(abs(x) > abs(y)) y = 0;
      else x = 0;
    }
    if(x < -THRESHOLD) return RIGHT;
    else if (x > THRESHOLD) return LEFT;
    else if(y < -THRESHOLD) return DOWN;
    else if(y > THRESHOLD) return UP;
    else return -1;
  }
  else if(face == 5) {
    if(abs(x) > THRESHOLD && abs(y) > THRESHOLD) {
      if(abs(x) > abs(y)) y = 0;
      else x = 0;
    }
    if(x < -THRESHOLD) return RIGHT;
    else if (x > THRESHOLD) return LEFT;
    else if(y < -THRESHOLD) return UP;
    else if(y > THRESHOLD) return DOWN;
    else return -1;

  }
  if(face == 0) {
    if(abs(x) > THRESHOLD && abs(z) > THRESHOLD) {
      if(abs(x) > abs(z)) z = 0;
      else x = 0;
    }
    if(x < -THRESHOLD) return RIGHT;
    else if (x > THRESHOLD) return LEFT;
    else if(z < -THRESHOLD) return DOWN;
    else if(z > THRESHOLD) return UP;
    else return -1;
  }
  else if(face == 2) {
    if(abs(x) > THRESHOLD && abs(z) > THRESHOLD) {
      if(abs(x) > abs(z)) z = 0;
      else x = 0;
    }
    if(x < -THRESHOLD) return LEFT;
    else if (x > THRESHOLD) return RIGHT;
    else if(z < -THRESHOLD) return DOWN;
    else if(z > THRESHOLD) return UP;
    else return -1;
  }
  if(face == 3) {
    if(abs(y) > THRESHOLD && abs(z) > THRESHOLD) {
      if(abs(y) > abs(z)) z = 0;
      else y = 0;
    }
    if(z < -THRESHOLD) return DOWN;
    else if (z > THRESHOLD) return UP;
    else if(y < -THRESHOLD) return RIGHT;
    else if(y > THRESHOLD) return LEFT;
    else return -1;
  }
  else if(face == 1) {
    if(abs(y) > THRESHOLD && abs(z) > THRESHOLD) {
      if(abs(y) > abs(z)) z = 0;
      else y = 0;
    }
    if(z < -THRESHOLD) return DOWN;
    else if (z > THRESHOLD) return UP;
    else if(y < -THRESHOLD) return LEFT;
    else if(y > THRESHOLD) return RIGHT;
    else return -1;
  }
}

int indexMap(int face, int i, int j) {
  /**
  * Maps indices from the 6x5x5 array to the length-150 array used for the LEDs.
  */
  if(face == 4) {
    if(i%2 == 0) {
      return i*5+4-j;
    } else {
      return i*5+j;
    }
  }
  else if(face == 0) {
    if(i%2 == 0) {
      return 25 + i*5+j;
    } else {
      return 25 + i*5+4-j;
    }
  }
  else if(face == 1) {
    if(i%2 == 0) {
      return 50 + i*5+j;
    } else {
      return 50 + i*5+4-j;
    }
  }
  else if(face == 2) {
    if(i%2 == 0) {
      return 75 + (4-i)*5+j;
    } else {
      return 75 + (4-i)*5+4-j;
    }
  }
  else if(face == 3) {
    if(i%2 == 0) {
      return 100 + i*5+j;
    } else {
      return 100 + i*5+4-j;
    }
  }
  else {
    if(i%2 == 0) {
      return 125 + i*5+j;
    } else {
      return 125 + i*5+4-j;
    }
  }
  
}

void flatten(byte (&maze)[6][5][5], byte (&flatMaze)[150]) {
  for(int face = 0; face < 6; face++) {
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        flatMaze[indexMap(face, i, j)] = maze[face][i][j];
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  generateMaze(officialMaze);
  
  byte flatMaze[150];
  flatten(officialMaze, flatMaze);


  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

      // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
    // END of Trinket-specific code.

    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, colors[flatMaze[i]]);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
  }
  printMaze(officialMaze);
  delay(2000);
}

void loop() {
  if(!FINISHED) {
    sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      Serial.print("Temperature:");
      Serial.print(temp.temperature);
      Serial.print("\tx-acceleration:");
      Serial.print(a.acceleration.x);
      Serial.print("\ty-acceleration:");
      Serial.print(a.acceleration.y);
      Serial.print("\tz-acceleration:");
      Serial.print(a.acceleration.z);
      Serial.print("\tx-gyro:");
      Serial.print(g.gyro.x);
      Serial.print("\ty-gyro:");
      Serial.print(g.gyro.y);
      Serial.print("\tz-gyro:");
      Serial.println(g.gyro.z);

      int prevX = userx;
      int prevY = usery;
      int prevZ = userz;

      float ax = a.acceleration.x;
      float ay = a.acceleration.y;
      // experimentally determined based on slight tilt of accelerometer in cube
      float az = a.acceleration.z- 2.25;

      // player movement

      byte direction = getDirection(userx, ax, ay, az);
      byte *next;
      bool hasNext = false;
      if(direction == LEFT) {
        next = getLeft(userx, usery, userz);
        hasNext = true;
      }
      else if(direction == RIGHT) {
        next = getRight(userx, usery, userz);
        hasNext = true;
      }
      else if(direction == UP) {
        next = getUp(userx, usery, userz);
        hasNext = true;
      }
      else if(direction == DOWN) {
        next = getDown(userx, usery, userz);
        hasNext = true;
      }

      byte nextX = next[0];
      byte nextY = next[1];
      byte nextZ = next[2];

      delete next;

      if(hasNext && officialMaze[nextX][nextY][nextZ] == 1) {
        // if the user makes a legal move
        userx = nextX;
        usery = nextY;
        userz = nextZ;
        officialMaze[prevX][prevY][prevZ] = 1;
        officialMaze[userx][usery][userz] = 2;
        strip.setPixelColor(indexMap(prevX, prevY, prevZ), colors[1]);
        strip.setPixelColor(indexMap(nextX, nextY, nextZ), colors[2]);
        strip.show();
      }
      else if(hasNext && officialMaze[nextX][nextY][nextZ] == 3) {
        // if the user reaches the goal state
        FINISHED = true;
        for(int i = 0; i < 6; i++) {
          for(int j = 0; j < 5; j++) {
            for(int k = 0; k < 5; k++) {
              officialMaze[i][j][k] = 0;
            }
          }
        }
        for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
          strip.setPixelColor(i, colors[3]);         //  Set pixel's color (in RAM)
          strip.show();                          //  Update strip to match
          delay(20);
        }
      }
    }

    printMaze(officialMaze);

  delay(1250);
}
