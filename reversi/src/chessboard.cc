#include "chessboard.h"

#include <algorithm>
#include <memory>

#include "config.h"

int Chessboard::GetWinner() const {
  // -2: tie
  // -1: to be continued
  // 0, 1: winner is 0/1

  int tots[2] = {0, 0};
  for (int x = 0; x < CHESSBOARD_SIZE; x++) {
    for (int y = 0; y < CHESSBOARD_SIZE; y++) {
      for (int who : {0, 1}) {
        if (Place(who, x, y, nullptr)) {
          return -1;
        }
        tots[who] += data_[Index(who, x, y)];
      }
    }
  }

  if (tots[0] == tots[1]) return -2;
  return tots[1] > tots[0];
}

bool Chessboard::Place(int c, int x, int y,
                       Chessboard *output_chessboard) const {
  if (At(0, x, y) + At(1, x, y) >= 1) return false;
  for (int dir = 0; dir < 4; dir++)
    for (int mul : {-1, 1}) {
      int32_t i, new_x, new_y;
      for (i = 1; i <= 8; i++) {
        new_x = DIRS[dir][0] * mul * i + x;
        new_y = DIRS[dir][1] * mul * i + y;
        if (std::min(new_x, new_y) < 0 ||
            std::max(new_x, new_y) >= CHESSBOARD_SIZE)
          break;
        if (At(c ^ 1, new_x, new_y) == 0 || At(c, new_x, new_y) >= 1) break;
      }
      if (std::min(new_x, new_y) >= 0 &&
          std::max(new_x, new_y) < CHESSBOARD_SIZE && i >= 2 &&
          At(c, new_x, new_y))
        goto success_flag;
    }
  return false;

success_flag:
  if (output_chessboard == nullptr) {
    return true;
  }

  *output_chessboard = *this;
  output_chessboard->data_[Index(c, x, y)] = 1;

  for (int dir = 0; dir < 4; dir++)
    for (int mul : {-1, 1}) {
      int32_t i, new_x, new_y;
      for (i = 1; i <= 8; i++) {
        new_x = DIRS[dir][0] * mul * i + x;
        new_y = DIRS[dir][1] * mul * i + y;
        if (std::min(new_x, new_y) < 0 ||
            std::max(new_x, new_y) >= CHESSBOARD_SIZE)
          break;
        if (At(c ^ 1, new_x, new_y) == 0 || At(c, new_x, new_y) >= 1) break;
      }
      if (std::min(new_x, new_y) >= 0 &&
          std::max(new_x, new_y) < CHESSBOARD_SIZE && i >= 2 &&
          At(c, new_x, new_y)) {
        for (int j = 1; j <= i - 1; j++) {
          new_x = DIRS[dir][0] * mul * j + x;
          new_y = DIRS[dir][1] * mul * j + y;
          output_chessboard->data_[Index(c, new_x, new_y)] = 1;
          output_chessboard->data_[Index(c ^ 1, new_x, new_y)] = 0;
        }
      }
    }
  return true;
}

void Chessboard::SetMemory(char *ptr) {
  std::copy(ptr, ptr + 2 * CHESSBOARD_SIZE * CHESSBOARD_SIZE, data_);
}

void Chessboard::Debug() {
  printf("  ");
  for (int y = 0; y < CHESSBOARD_SIZE; y++) {
    printf("%d ", y);
  }
  printf("\n");
  for (int x = 0; x < CHESSBOARD_SIZE; x++) {
    printf("%d ", x);
    for (int y = 0; y < CHESSBOARD_SIZE; y++) {
      char c = '.';
      if (At(0, x, y) > 0) {
        c = 'x';
      } else if (At(1, x, y) > 0) {
        c = 'o';
      }
      printf("%c ", c);
    }
    printf("\n");
  }
  fflush(stdout);
}