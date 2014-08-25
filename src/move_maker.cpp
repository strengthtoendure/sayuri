/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file move_maker.cpp
 * @author Hironori Ishibashi
 * @brief 候補手を生成するクラスの実装。
 */

#include "move_maker.h"

#include <iostream>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include <utility>
#include "common.h"
#include "chess_engine.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  //コンストラクタ。
  MoveMaker::MoveMaker(const ChessEngine& engine) :
  engine_ptr_(&engine),
  last_(0),
  max_(0),
  history_max_(1) {}

  // コピーコンストラクタ。
  MoveMaker::MoveMaker(const MoveMaker& maker) :
  engine_ptr_(maker.engine_ptr_),
  last_(maker.last_),
  max_(maker.max_),
  history_max_(maker.history_max_) {
    for (std::size_t i = 0; i < (MAX_CANDIDATES + 1); i++) {
      move_stack_[i] = maker.move_stack_[i];
    }
  }

  // ムーブコンストラクタ。
  MoveMaker::MoveMaker(MoveMaker&& maker) :
  engine_ptr_(maker.engine_ptr_),
  last_(maker.last_),
  max_(maker.max_),
  history_max_(maker.history_max_) {
    for (std::size_t i = 0; i < (MAX_CANDIDATES + 1); i++) {
      move_stack_[i] = maker.move_stack_[i];
    }
  }

  // コピー代入演算子。
  MoveMaker& MoveMaker::operator=
  (const MoveMaker& maker) {
    engine_ptr_ = maker.engine_ptr_;
    history_max_ = maker.history_max_;
    last_ = maker.last_;
    max_ = maker.max_;
    for (std::size_t i = 0; i < (MAX_CANDIDATES + 1); i++) {
      move_stack_[i] = maker.move_stack_[i];
    }
    return *this;
  }

  // ムーブ代入演算子。
  MoveMaker& MoveMaker::operator=
  (MoveMaker&& maker) {
    engine_ptr_ = maker.engine_ptr_;
    history_max_ = maker.history_max_;
    last_ = maker.last_;
    max_ = maker.max_;
    for (std::size_t i = 0; i < (MAX_CANDIDATES + 1); i++) {
      move_stack_[i] = maker.move_stack_[i];
    }
    return *this;
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // スタックに候補手を生成する。
  template<GenMoveType Type> int MoveMaker::GenMoves(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2) {
    // 初期化。
    last_ = max_ = 0;
    history_max_ = 1;

    GenMovesCore<Type>(prev_best, iid_move, killer_1, killer_2);
    
    max_ = last_;
    return last_;
  }
  // インスタンス化。
  template int MoveMaker::GenMoves<GenMoveType::NON_CAPTURE>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2);
  template int MoveMaker::GenMoves<GenMoveType::CAPTURE>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2);
  template <>
  int MoveMaker::GenMoves<GenMoveType::ALL>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2) {
    // 初期化。
    last_ = max_ = 0;
    history_max_ = 1;

    GenMovesCore<GenMoveType::NON_CAPTURE>(prev_best, iid_move, killer_1,
    killer_2);

    GenMovesCore<GenMoveType::CAPTURE>(prev_best, iid_move, killer_1,
    killer_2);

    max_ = last_;
    return last_;
  }

  // 次の候補手を取り出す。
  Move MoveMaker::PickMove() {
    std::unique_lock<std::mutex> lock(mutex_);

    MoveSlot slot;

    // 手がなければ何もしない。
    if (!last_) {
      return 0;
    }

    // とりあえず最後の手をポップ。
    last_--;
    slot = move_stack_[last_];

    // 一番高い手を探し、スワップ。
    for (int i = last_ - 1; i >= 0; i--) {
      if (move_stack_[i].score_ > slot.score_) {
        std::swap(slot, move_stack_[i]);
      }
    }

    return slot.move_;
  }

  // スタックに候補手を生成する。 (内部用)
  template<GenMoveType Type>
  void MoveMaker::GenMovesCore(Move prev_best, Move iid_move,
  Move killer_1, Move killer_2) {
    // サイド。
    Side side = engine_ptr_->to_move();
    Side enemy_side = OPPOSITE_SIDE(side);

    // 生成開始時のポインタ。
    std::size_t start = last_;

    // ナイト、ビショップ、ルーク、クイーンの候補手を作る。
    for (Piece piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
      Bitboard pieces = engine_ptr_->position()[side][piece_type];

      for (; pieces; NEXT(pieces)) {
        Square from = Util::GetSquare(pieces);

        // 各ピースの動き。
        Bitboard move_bitboard;
        switch (piece_type) {
          case KNIGHT:
            move_bitboard = Util::GetKnightMove(from);
            break;
          case BISHOP:
            move_bitboard = engine_ptr_->GetBishopAttack(from);
            break;
          case ROOK:
            move_bitboard = engine_ptr_->GetRookAttack(from);
            break;
          case QUEEN:
            move_bitboard = engine_ptr_->GetQueenAttack(from);
            break;
          default:
            throw SayuriError
            ("駒の種類が不正です。 in MoveMaker::GenMoveCore()");
            break;
        }

        // 展開するタイプによって候補手を選り分ける。。
        if (Type == GenMoveType::NON_CAPTURE) {
          // キャプチャーじゃない手。
          move_bitboard &= ~(engine_ptr_->blocker_0());
        } else {
          // キャプチャーの手。
          move_bitboard &= engine_ptr_->side_pieces()[enemy_side];
        }

        for (; move_bitboard; NEXT(move_bitboard)) {
          // 手を作る。
          Move move = 0;
          SET_FROM(move, from);
          Square to = Util::GetSquare(move_bitboard);
          SET_TO(move, to);
          SET_MOVE_TYPE(move, NORMAL);

          // ヒストリーの最大値を更新。
          if (Type == GenMoveType::NON_CAPTURE) {
            if (engine_ptr_->history()[side][from][to] > history_max_) {
              history_max_ = engine_ptr_->history()[side][from][to];
            }
          }

          // スタックに登録。
          move_stack_[last_].move_ = move;
          last_++;
        }
      }
    }

    // ポーンの動きを作る。
    Bitboard pieces = engine_ptr_->position()[side][PAWN];
    for (; pieces; NEXT(pieces)) {
      Square from = Util::GetSquare(pieces);

      Bitboard move_bitboard = 0;
      if (Type == GenMoveType::NON_CAPTURE) {
        // キャプチャーじゃない手。
        move_bitboard = engine_ptr_->GetPawnStep(side, from);
      } else {
        // キャプチャーの手。
        move_bitboard = Util::GetPawnAttack(from, side)
        & engine_ptr_->side_pieces()[enemy_side];
        // アンパッサンがある場合。
        if (engine_ptr_->en_passant_square()) {
          move_bitboard |= Util::SQUARE[engine_ptr_->en_passant_square()]
          & Util::GetPawnAttack(from, side);
        }
      } 

      for (; move_bitboard; NEXT(move_bitboard)) {
        // 手を作る。
        Move move = 0;
        SET_FROM(move, from);
        Square to = Util::GetSquare(move_bitboard);
        SET_TO(move, to);

        // ヒストリーの最大値を更新。
        if (Type == GenMoveType::NON_CAPTURE) {
          if (engine_ptr_->history()[side][from][to] > history_max_) {
            history_max_ = engine_ptr_->history()[side][from][to];
          }
        }

        if (engine_ptr_->en_passant_square()
        && (to == engine_ptr_->en_passant_square())) {
          SET_MOVE_TYPE(move, EN_PASSANT);
        } else {
          SET_MOVE_TYPE(move, NORMAL);
        }

        if (((side == WHITE) && (Util::SQUARE_TO_RANK[to] == RANK_8))
        || ((side == BLACK) && (Util::SQUARE_TO_RANK[to] == RANK_1))) {
          // 昇格を設定。
          for (Piece piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
            SET_PROMOTION(move, piece_type);
            move_stack_[last_].move_ = move;
            last_++;
          }
        } else {
          // 昇格しない場合。
          move_stack_[last_].move_ = move;
          last_++;
        }
      }
    }

    // キングの動きを作る。
    Square from = engine_ptr_->king()[side];
    Bitboard move_bitboard = Util::GetKingMove(from);
    if (Type == GenMoveType::NON_CAPTURE) {
      // キャプチャーじゃない手。
      move_bitboard &= ~(engine_ptr_->blocker_0());
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::SQUARE[G1];
        }
        if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::SQUARE[C1];
        }
      } else {
        if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::SQUARE[G8];
        }
        if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
          move_bitboard |= Util::SQUARE[C8];
        }
      }
    } else {
      // キャプチャーの手。
      move_bitboard &= engine_ptr_->side_pieces()[enemy_side];
    }
    for (; move_bitboard; NEXT(move_bitboard)) {
      Move move = 0;
      SET_FROM(move, from);
      Square to = Util::GetSquare(move_bitboard);
      SET_TO(move, to);

      // ヒストリーの最大値を更新。
      if (Type == GenMoveType::NON_CAPTURE) {
        if (engine_ptr_->history()[side][from][to] > history_max_) {
          history_max_ = engine_ptr_->history()[side][from][to];
        }
      }

      if ((side == WHITE) && (from == E1) && (to == G1)) {
        // 白のショートキャスリング。
        SET_MOVE_TYPE(move, CASTLE_WS);
      } else if ((side == WHITE) && (from == E1) && (to == C1)) {
        // 白のロングキャスリング。
        SET_MOVE_TYPE(move, CASTLE_WL);
      } else if ((side == BLACK) && (from == E8) && (to == G8)) {
        // 黒のショートキャスリング。
        SET_MOVE_TYPE(move, CASTLE_BS);
      } else if ((side == BLACK) && (from == E8) && (to == C8)) {
        // 黒のロングキャスリング。
        SET_MOVE_TYPE(move, CASTLE_BL);
      } else {
        // キャスリングじゃない手。
        SET_MOVE_TYPE(move, NORMAL);
      }

      move_stack_[last_].move_ = move;
      last_++;
    }

    ScoreMoves<Type>(start, prev_best, iid_move, killer_1, killer_2, side);
  }
  // 実体化。
  template void MoveMaker::GenMovesCore<GenMoveType::NON_CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);
  template void MoveMaker::GenMovesCore<GenMoveType::CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);

  // 候補手に点数をつける。
  template<GenMoveType Type>
  void MoveMaker::ScoreMoves(std::size_t start, Move prev_best, Move iid_move,
  Move killer_1, Move killer_2, Side side) {
    // --- 評価値の定義 --- //
    // ヒストリーの点数の最大値のビット桁数。 (つまり、最大値は 0x200 )
    constexpr int MAX_HISTORY_SCORE_SHIFT = 9;
    // キラームーブの点数。
    constexpr std::int64_t KILLER_2_MOVE_SCORE = 0x400LL;
    constexpr std::int64_t KILLER_1_MOVE_SCORE = KILLER_2_MOVE_SCORE << 1;
    // 駒を取る手のビットシフト。
    constexpr int CAPTURE_SCORE_SHIFT = 13;
    // 相手キングをチェックする手の点数。
    constexpr std::int64_t CHECKING_MOVE_SCORE = 1LL << 32;
    // IIDで得た最善手の点数。
    constexpr std::int64_t IID_MOVE_SCORE = CHECKING_MOVE_SCORE << 1;
    // 前回の繰り返しでトランスポジションテーブルから得た最善手の点数。
    constexpr std::int64_t BEST_MOVE_SCORE = IID_MOVE_SCORE << 1;
    // 悪い取る手の点数。
    constexpr std::int64_t BAD_CAPTURE_SCORE = -1LL;

    Bitboard enemy_king_bb =
    Util::SQUARE[engine_ptr_->king()[OPPOSITE_SIDE(side)]];
    for (std::size_t i = start; i < last_; i++) {
      // 手の情報を得る。
      Square from = GET_FROM(move_stack_[i].move_);
      Square to = GET_TO(move_stack_[i].move_);

      // 相手キングをチェックする手かどうか調べる。
      bool is_checking_move = false;
      switch (engine_ptr_->piece_board()[from]) {
        case PAWN:
          if ((enemy_king_bb & Util::GetPawnAttack(to, side))) {
            is_checking_move = true;
          }
          break;
        case KNIGHT:
          if ((enemy_king_bb & Util::GetKnightMove(to))) {
            is_checking_move = true;
          }
          break;
        case BISHOP:
          if ((enemy_king_bb & engine_ptr_->GetBishopAttack(to))) {
            is_checking_move = true;
          }
          break;
        case ROOK:
          if ((enemy_king_bb & engine_ptr_->GetRookAttack(to))) {
            is_checking_move = true;
          }
          break;
        case QUEEN:
          if ((enemy_king_bb & engine_ptr_->GetQueenAttack(to))) {
            is_checking_move = true;
          }
          break;
        default:
          // 何もしない。
          break;
      }

      // 特殊な手の点数をつける。
      if (EQUAL_MOVE(move_stack_[i].move_, prev_best)) {
        // 前回の最善手。
        move_stack_[i].score_ = BEST_MOVE_SCORE;
      } else if (EQUAL_MOVE(move_stack_[i].move_, iid_move)) {
        // IIDムーブ。
        move_stack_[i].score_ = IID_MOVE_SCORE;
      } else if (is_checking_move) {
        // 相手キングをチェックする手。
        move_stack_[i].score_ = CHECKING_MOVE_SCORE;
      } else if (EQUAL_MOVE(move_stack_[i].move_, killer_1)) {
        // キラームーブ。
        move_stack_[i].score_ = KILLER_1_MOVE_SCORE;
      } else if (EQUAL_MOVE(move_stack_[i].move_, killer_2)) {
        // キラームーブ。
        move_stack_[i].score_ = KILLER_2_MOVE_SCORE;
      } else {
        // その他の手を各候補手のタイプに分ける。
        if ((Type == GenMoveType::CAPTURE)
        || (move_stack_[i].move_ & PROMOTION_MASK)) {
          // SEEで点数をつけていく。
          std::int64_t temp =
          (engine_ptr_->SEE(move_stack_[i].move_)) << CAPTURE_SCORE_SHIFT;
          move_stack_[i].score_ = MAX(temp, BAD_CAPTURE_SCORE);
        } else {
          // ヒストリーを使って点数をつけていく。
          move_stack_[i].score_ = (engine_ptr_->history()[side][from][to]
          << MAX_HISTORY_SCORE_SHIFT) / history_max_;
        }
      }
    }
  }
  // 実体化。
  template void MoveMaker::ScoreMoves<GenMoveType::NON_CAPTURE>
  (std::size_t start, Move best_move, Move iid_move, Move killer_1,
  Move killer_2, Side side);
  template void MoveMaker::ScoreMoves<GenMoveType::CAPTURE>
  (std::size_t start, Move best_move, Move iid_move, Move killer_1,
  Move killer_2, Side side);
}  // namespace Sayuri
