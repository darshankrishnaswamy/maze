#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>

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

std::array<int, 3> getLeft(int face, int i, int j) {
    if(j > 0) {
        return {face, i, j-1};
    }
    std::array<std::array<int, 3>, 6> left {{
      {3, i, 4},
      {0, i, 4},
      {1, i, 4},
      {2, i, 4},
      {3, 0, i},
      {3, 4, 4-i}}};
    return left[face];

}

std::array<int, 3> getRight(int face, int i, int j) {
    if(j < 4) {
        return {face, i, j+1};
    }
    std::array<std::array<int, 3>, 6> right {{
              {1, i, 0},
              {2, i, 0},
              {3, i, 0},
              {0, i, 0},
              {1, 0, 4-i},
              {1, 4, i}
      }};
    return right[face];
}

std::array<int, 3> getUp(int face, int i, int j) {
    if(i > 0) {
        return {face, i-1, j};
    }
    std::array<std::array<int, 3>, 6> up {{
            {4, 4, j},
            {4, 4-j, 4},
            {4, 0, 4-j},
            {4, j, 0},
            {2, 0, 4-j},
            {0, 4, j}
    }};
    return up[face];

}

std::array<int, 3> getDown(int face, int i, int j) {
    if(i < 4) {
        return {face, i+1, j};
    }
    std::array<std::array<int, 3>, 6> down {{
            {5, 0, j},
            {5, j, 4},
            {5, 4, 4-j},
            {5, 4-j, 0},
            {0, 0, j},
            {2, 4, 4-j}}
    };
    return down[face];
}

int main() {
    std::array<std::array<std::array<int, 5>, 5>, 6> maze{};
    for (auto &i: maze) {
        for (auto &j: i) {
            for (int &k: j) {
                k = 0; // wall
            }
        }
    }

    std::array<std::array<std::array<std::array<std::array<std::array<int, 3>, 2>, 4>, 5>, 5>, 6> adjMat{};
    for (int faceNum = 0; faceNum < 6; faceNum++) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                std::array<int, 3> leftNeighbor = getLeft(faceNum, i, j);
                adjMat[faceNum][i][j][0][0] = leftNeighbor;
                adjMat[faceNum][i][j][0][1] = getLeft(leftNeighbor[0], leftNeighbor[1], leftNeighbor[2]);

                std::array<int, 3> upNeighbor = getUp(faceNum, i, j);
                adjMat[faceNum][i][j][1][0] = upNeighbor;
                adjMat[faceNum][i][j][1][1] = getUp(upNeighbor[0], upNeighbor[1], upNeighbor[2]);

                std::array<int, 3> rightNeighbor = getRight(faceNum, i, j);
                adjMat[faceNum][i][j][2][0] = rightNeighbor;
                adjMat[faceNum][i][j][2][1] = getRight(rightNeighbor[0], rightNeighbor[1], rightNeighbor[2]);

                std::array<int, 3> downNeighbor = getDown(faceNum, i, j);
                adjMat[faceNum][i][j][3][0] = downNeighbor;
                adjMat[faceNum][i][j][3][1] = getDown(downNeighbor[0], downNeighbor[1], downNeighbor[2]);



            }
        }
    }

    auto frontier = new std::vector<std::array<std::array<int, 3>, 2>>();
    // start point will always be [0, 2, 2] -> center of front face
    maze[0][2][2] = 1;

    frontier->push_back({{{0, 2, 1}, {0, 2, 0}}});
    frontier->push_back({{{0, 2, 3}, {0, 2, 4}}});
    frontier->push_back({{{0, 1, 2}, {0, 0, 2}}});
    frontier->push_back({{{0, 3, 2}, {0, 4, 2}}});

    bool stopCondition = false; // need to figure this out
    while (!stopCondition) {
        int index = std::rand() % frontier->size();
        std::array<std::array<int, 3>, 2> removed = (*frontier)[index];
        frontier->erase(frontier->begin() + index);
        std::array<int, 3> first = removed[0], second = removed[1];
        maze[first[0]][first[1]][first[2]] = 1;
        maze[second[0]][second[1]][second[2]] = 1;

        auto neighbors = adjMat[second[0]][second[1]][second[2]];
        for(auto neighbor : neighbors) {
            auto neighbor2 = neighbor[1];
            if (!maze[neighbor2[0]][neighbor2[1]][neighbor2[2]]) {
                frontier->push_back(neighbor);
            }
        }

        stopCondition = frontier->size() == 0;

    }

    // print the maze (for debugging purposes)
    for(auto face : maze) {
        for(auto row : face) {
            for(auto item : row) {
                std::cout << item << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

/*
 * Probably don't need any of this but keeping it just in case
 * //                // LEFT
//                if(j > 0) {
//                    auto loc = adjMat[faceNum][i][j][0];
//                    loc[0] = faceNum;
//                    loc[1] = i;
//                    loc[2] = j-1;
//                }
//                else {
//                    // work
//                    auto loc = adjMat[faceNum][i][j][0];
//                    auto next = getLeft(faceNum, i);
//                    loc[0] = next[0];
//                    loc[1] = next[1];
//                    loc[2] = next[2];
//                }
//
//                // UP
//                if(i > 0) {
//                    auto loc = adjMat[faceNum][i][j][1];
//                    loc[0] = faceNum;
//                    loc[1] = i-1;
//                    loc[2] = j;
//                }
//                else {
//                    auto loc = adjMat[faceNum][i][j][1];
//                    auto next = getUp(faceNum, i);
//                    loc[0] = next[0];
//                    loc[1] = next[1];
//                    loc[2] = next[2];
//                }
//
//                // RIGHT
//                if(j < 4) {
//                    auto loc = adjMat[faceNum][i][j][2];
//                    loc[0] = faceNum;
//                    loc[1] = i;
//                    loc[2] = j+1;
//                }
//                else {
//                    auto loc = adjMat[faceNum][i][j][2];
//                    auto next = getRight(faceNum, i);
//                    loc[0] = next[0];
//                    loc[1] = next[1];
//                    loc[2] = next[2];
//                }
//
//                // DOWN
//                if(i < 4) {
//                    auto loc = adjMat[faceNum][i][j][3];
//                    loc[0] = faceNum;
//                    loc[1] = i+1;
//                    loc[2] = j;
//                }
//                else {
//                    auto loc = adjMat[faceNum][i][j][3];
//                    auto next = getDown(faceNum, i);
//                    loc[0] = next[0];
//                    loc[1] = next[1];
//                    loc[2] = next[2];
//                }
//            }
        }
    }
 * */