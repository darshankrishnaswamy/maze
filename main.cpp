#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stack>

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

void setGoal(std::array<std::array<std::array<int, 5>, 5>, 6>& maze);


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

void prims(std::array<std::array<std::array<int, 5>, 5>, 6>& maze,
           std::array<std::array<std::array<std::array<std::array<std::array<int, 3>, 2>, 4>, 5>, 5>, 6>& adjMat) {
    auto frontier = new std::vector<std::array<std::array<int, 3>, 2>>();
    // start point will always be [0, 2, 2] -> center of front face
    maze[0][2][2] = 1;

    frontier->push_back({{{0, 2, 1}, {0, 2, 0}}});
    frontier->push_back({{{0, 2, 3}, {0, 2, 4}}});
    frontier->push_back({{{0, 1, 2}, {0, 0, 2}}});
    frontier->push_back({{{0, 3, 2}, {0, 4, 2}}});

    bool stopCondition = false; // need to figure this out
    while (!stopCondition) {
        int index = std::rand() % frontier->size(); // better rng might be needed
        std::array<std::array<int, 3>, 2> removed = (*frontier)[index];
        frontier->erase(frontier->begin() + index);
        std::array<int, 3> first = removed[0], second = removed[1];
        maze[first[0]][first[1]][first[2]] = 1;
        maze[second[0]][second[1]][second[2]] = 1;

        auto neighbors = adjMat[second[0]][second[1]][second[2]];
        for(auto neighbor : neighbors) {
            auto neighbor2 = neighbor[1];
            if (!maze[neighbor2[0]][neighbor2[1]][neighbor2[2]] && std::find(frontier->begin(), frontier->end(), neighbor) == frontier->end()) {
                frontier->push_back(neighbor);
            }
        }
        stopCondition = frontier->empty();
    }
    setGoal(maze);
}

void dfs(std::array<std::array<std::array<int, 5>, 5>, 6>& maze,
           std::array<std::array<std::array<std::array<std::array<std::array<int, 3>, 2>, 4>, 5>, 5>, 6>& adjMat) {
    std::stack<std::array<std::array<int, 3>, 2>> stack;
    std::array<std::array<std::array<bool, 5>, 5>, 6> visited{};
    maze[0][2][2] = 2;
    visited[0][2][2] = true;
    int dir1 = (std::rand() % 2) * 2 - 1;
    int dir2 = (std::rand() % 2) * 2 - 1;
    std::array<std::array<int, 3>, 2> initial = {{{0, 2, 2}, {0, 2, 2}}};
    initial[0][dir1+1] += dir2;
    initial[1][dir1+1] += dir2 * 2;
    stack.push(initial);
    visited[initial[0][0]][initial[0][1]][initial[0][2]] = true;
    visited[initial[1][0]][initial[1][1]][initial[1][2]] = true;
    while(!stack.empty()) {
        std::array<std::array<int, 3>, 2> removed = stack.top();
        stack.pop();
        std::array<int, 3> first = removed[0], second = removed[1];
        maze[first[0]][first[1]][first[2]] = 1;
        maze[second[0]][second[1]][second[2]] = 1;
        auto neighbors = adjMat[second[0]][second[1]][second[2]];
        std::vector<int> validInds;
        for(int i = 0; i < 4; i++) {
            auto neighbor2 = neighbors[i][1];
            if(!visited[neighbor2[0]][neighbor2[1]][neighbor2[2]]) {
                validInds.push_back(i);
            }
        }
        if(!validInds.empty()) {
            int ind = validInds[std::rand() % validInds.size()];
            stack.push(neighbors[ind]);
            visited[neighbors[ind][0][0]][neighbors[ind][0][1]][neighbors[ind][0][2]] = true;
            visited[neighbors[ind][1][0]][neighbors[ind][1][1]][neighbors[ind][1][2]] = true;
            stack.push(removed);
        }
    }
    setGoal(maze);


}

void setGoal(std::array<std::array<std::array<int, 5>, 5>, 6>& maze) {
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
        std::array<int, 4> remainingFaces = {1, 3, 4, 5};
        for(int faceNum : remainingFaces) {
            auto face = maze[faceNum];
            for (auto& row : face) {
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
    }
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

    dfs(maze, adjMat);



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

