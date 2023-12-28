#ifndef MCTS_H_
#define MCTS_H_

#include <memory>
#include <random>
#include <thread>

#include "chessboard.h"

class MCTS;

class MCTSNode {
  friend class MCTS;

 private:
  Chessboard chessboard_;
  uint32_t who_;
  MCTSNode *father_;
  std::unique_ptr<MCTSNode> childs_[CHESSBOARD_SIZE * CHESSBOARD_SIZE];
  std::unique_ptr<MCTSNode> pass_;
  uint32_t n_;
  double sigma_q_;

 public:
  inline MCTSNode(const Chessboard &chessboard, uint32_t who, MCTSNode *father)
      : chessboard_(chessboard),
        who_(who),
        father_(father),
        n_(0),
        sigma_q_(0) {}

  inline double Simulate() {
    constexpr int SIMULATE_TIMES = 10;

    double ans = 0;
    for (int i = 0; i < SIMULATE_TIMES; i++) {
      ans += SimulateOnce();
    }
    return ans / SIMULATE_TIMES;
  }

  inline bool Expand(int32_t x, int32_t y) {
    if (x == -1 || y == -1) {
      if (pass_ != nullptr) return false;
      pass_.reset(new MCTSNode(chessboard_, who_ ^ 1, this));
    } else {
      uint32_t idx = x * CHESSBOARD_SIZE + y;
      if (childs_[idx] != nullptr) return false;
      Chessboard output_chessboard;
      chessboard_.Place(who_, x, y, &output_chessboard);
      childs_[idx].reset(new MCTSNode(output_chessboard, who_ ^ 1, this));
    }
    return true;
  }

  inline void Backup(double delta_q) {
    MCTSNode *node = this;
    while (node != nullptr) {
      node->n_ += 1;
      node->sigma_q_ += delta_q;

      node = node->father_;
      delta_q = -delta_q;
    }
  }

  inline std::pair<int32_t, int32_t> Select(double c) {
    bool has_child = false;

    double max_uct = std::numeric_limits<double>::lowest();
    std::pair<int32_t, int32_t> ans;
    for (int x = 0; x < CHESSBOARD_SIZE; x++)
      for (int y = 0; y < CHESSBOARD_SIZE; y++) {
        if (!chessboard_.Place(who_, x, y, nullptr)) continue;
        has_child = true;

        double uct = 0;
        MCTSNode *child = childs_[x * CHESSBOARD_SIZE + y].get();
        if (child != nullptr) {
          uct = -child->sigma_q_ / child->n_ +
                c * std::pow(2 * std::log(this->n_) / child->n_, 0.5);
        } else {
          uct = c * std::pow(2 * std::log(this->n_), 0.5);
        }

        if (uct > max_uct) {
          max_uct = uct;
          ans.first = x;
          ans.second = y;
        }
      }

    if (!has_child) {
      return {-1, -1};
    } else {
      return ans;
    }
  }

  inline int32_t SimulateOnce() {
    static thread_local std::mt19937 engine(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));

    std::pair<int32_t, int32_t> candidates[CHESSBOARD_SIZE * CHESSBOARD_SIZE];
    Chessboard current_chessboard = chessboard_;
    int32_t player = who_;

    while (1) {
      if (int32_t winner = current_chessboard.GetWinner(); winner != -1) {
        switch (winner) {
          case 0:
          case 1:
            return winner == who_ ? 1 : -1;
          default:
            return 0;
        }
      }

      uint32_t candidates_cnt = 0;
      for (int x = 0; x < CHESSBOARD_SIZE; x++)
        for (int y = 0; y < CHESSBOARD_SIZE; y++) {
          if (current_chessboard.Place(player, x, y, nullptr)) {
            candidates[candidates_cnt++] = {x, y};
          }
        }

      if (candidates_cnt > 0) {
        int candidate_idx =
            std::uniform_int_distribution<>(0, candidates_cnt - 1)(engine);

        Chessboard output_chessboard;
        current_chessboard.Place(player, candidates[candidate_idx].first,
                                 candidates[candidate_idx].second,
                                 &output_chessboard);
        current_chessboard = output_chessboard;
      }

      player ^= 1;
    }
  }
};

class MCTS {
 private:
  std::unique_ptr<MCTSNode> root_;

 public:
  inline MCTS(const Chessboard &chessboard, uint32_t who) {
    root_.reset(new MCTSNode(chessboard, who, nullptr));
  }

  inline std::pair<int32_t, int32_t> Search(int32_t num_runs, double c) {
    for (int i = 0; i < num_runs; i++) RunOnce(c);
    return root_->Select(0);
  }

  inline void RunOnce(double c) {
    MCTSNode *node = root_.get();

    while (1) {
      auto [x, y] = node->Select(c);
      bool expanded = node->Expand(x, y);
      MCTSNode *next_node;
      if (x == -1 || y == -1)
        next_node = node->pass_.get();
      else
        next_node = node->childs_[x * CHESSBOARD_SIZE + y].get();
      if (expanded) {
        // new node
        double delta_q = next_node->Simulate();
        next_node->Backup(delta_q);
        return;
      }

      node = next_node;
    }
  }
};

#endif