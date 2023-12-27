#ifndef MINIMAX_TREE_H_
#define MINIMAX_TREE_H_

#include <stdint.h>

#include <memory>

#include "chessboard.h"
#include "config.h"

class MinimaxTree;

class MinimaxTreeNode {
  friend class MinimaxTree;

 public:
  inline MinimaxTreeNode(const Chessboard &chessboard, uint32_t who)
      : chessboard_(chessboard), who_(who) {}

  inline int32_t Search(uint32_t depth, int32_t alpha, bool skip) {
    if (depth <= 0) return estimated_value_ = EstimatedValue();
    int32_t ret = std::numeric_limits<int32_t>::min();
    Chessboard output_chessboard;
    bool has_child = false;
    for (int x = 0; x < CHESSBOARD_SIZE; x++) {
      for (int y = 0; y < CHESSBOARD_SIZE; y++) {
        if (!chessboard_.Place(who_, x, y, &output_chessboard)) continue;
        has_child = true;
        childs_[Index(x, y)].reset(
            new MinimaxTreeNode(output_chessboard, who_ ^ 1));
        int32_t child_value =
            childs_[Index(x, y)]->Search(depth - 1, ret, false);
        ret = std::max(ret, -child_value);
        if (-ret < alpha) return estimated_value_ = ret;  // pruned
      }
    }
    if (has_child) {
      return estimated_value_ = ret;
    } else if (skip) {
      return estimated_value_ = EstimatedValue();
    } else {
      pass_.reset(new MinimaxTreeNode(chessboard_, who_ ^ 1));
      // do not decrement depth intentionally
      int32_t child_value = pass_->Search(depth, ret, true);
      ret = std::max(ret, -child_value);
      return estimated_value_ = ret;
    }
  }

  inline int32_t EstimatedValue() {
    int32_t ret = 0;
    for (int x = 0; x < CHESSBOARD_SIZE; x++)
      for (int y = 0; y < CHESSBOARD_SIZE; y++) {
        ret += chessboard_.At(who_, x, y);
      }
    for (int x = 0; x < CHESSBOARD_SIZE; x++)
      for (int y = 0; y < CHESSBOARD_SIZE; y++) {
        ret -= chessboard_.At(who_ ^ 1, x, y);
      }
    return ret;
  }

 private:
  Chessboard chessboard_;
  MinimaxTreeNode *father_;
  uint32_t who_;
  int32_t estimated_value_;
  std::unique_ptr<MinimaxTreeNode> childs_[CHESSBOARD_SIZE * CHESSBOARD_SIZE];
  std::unique_ptr<MinimaxTreeNode> pass_;

  inline int Index(int x, int y) { return x * CHESSBOARD_SIZE + y; }
};

class MinimaxTree {
 public:
  inline MinimaxTree(const Chessboard &chessboard, uint32_t who) {
    root_.reset(new MinimaxTreeNode(chessboard, who));
  }

  inline std::pair<uint32_t, uint32_t> Search(int depth) {
    root_->Search(depth, std::numeric_limits<int32_t>::min(), false);
    for (auto x = 0; x < CHESSBOARD_SIZE; x++)
      for (auto y = 0; y < CHESSBOARD_SIZE; y++) {
        auto child = root_->childs_[root_->Index(x, y)].get();
        if (child == nullptr) continue;
        if (root_->estimated_value_ == -child->estimated_value_) return {x, y};
      }
    return {-1, -1};
  }

 private:
  std::unique_ptr<MinimaxTreeNode> root_;
};

#endif