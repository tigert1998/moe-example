#include <vector>

#include "mcts.h"
#include "minimax_tree.h"

void User(uint32_t who, const Chessboard &chessboard,
          Chessboard *output_chessboard) {
  for (int x = 0; x < CHESSBOARD_SIZE; x++) {
    for (int y = 0; y < CHESSBOARD_SIZE; y++) {
      if (chessboard.Place(who, x, y, output_chessboard)) {
        goto user_input_tag;
      }
    }
  }
  printf("No available user input. Skipping.\n");
  *output_chessboard = chessboard;
  return;

user_input_tag:
  while (1) {
    int x, y;
    scanf("%d%d", &x, &y);
    if (chessboard.Place(who, x, y, output_chessboard)) return;
  }
}

void MiniMaxAI(uint32_t who, const Chessboard &chessboard,
               Chessboard *output_chessboard) {
  MinimaxTree t(chessboard, who);
  auto [x, y] = t.Search(8);
  if (x == -1 || y == -1)
    *output_chessboard = chessboard;
  else {
    int32_t placed = chessboard.Place(who, x, y, output_chessboard);
    if (!placed) {
      fprintf(stderr, "AI fails to place a stone at (%d, %d)\n", x, y);
      fflush(stderr);
      exit(1);
    }
  }
}

void MCTSAI(uint32_t who, const Chessboard &chessboard,
            Chessboard *output_chessboard) {
  MCTS t(chessboard, who);
  auto [x, y] = t.Search(10000, 1 / std::pow(2, 0.5));
  if (x == -1 || y == -1)
    *output_chessboard = chessboard;
  else {
    int32_t placed = chessboard.Place(who, x, y, output_chessboard);
    if (!placed) {
      fprintf(stderr, "AI fails to place a stone at (%d, %d)\n", x, y);
      fflush(stderr);
      exit(1);
    }
  }
}

int main() {
  Chessboard chessboard;
  chessboard.Set(1, 3, 3);
  chessboard.Set(0, 3, 4);
  chessboard.Set(0, 4, 3);
  chessboard.Set(1, 4, 4);
  chessboard.Debug();

  int32_t starts_from;
  scanf("%d", &starts_from);

  typedef void (*PlayerFunc)(uint32_t, const Chessboard &, Chessboard *);
  const PlayerFunc players[2] = {&MiniMaxAI, &MCTSAI};

  int32_t i = starts_from, who = 0;
  while (1) {
    Chessboard next_chessboard;
    players[i](who, chessboard, &next_chessboard);
    i = i ^ 1;
    who = who ^ 1;
    chessboard = next_chessboard;
    chessboard.Debug();

    int32_t winner = chessboard.GetWinner();
    if (winner != -1) {
      if (winner == -2)
        printf("tie\n");
      else
        printf("Winner is %c\n", std::vector{'o', 'x'}[who]);
      break;
    }
  }

  return 0;
}