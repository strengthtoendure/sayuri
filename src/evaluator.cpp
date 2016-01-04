/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file evaluator.cpp
 * @author Hironori Ishibashi
 * @brief 評価関数クラスの実装。
 */

#include "evaluator.h"

#include <iostream>
#include "common.h"
#include "chess_engine.h"
#include "params.h"
#include "cache.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static変数 //
  // ========== //
  constexpr Bitboard Evaluator::START_POSITION[NUM_SIDES][NUM_PIECE_TYPES];
  constexpr Bitboard Evaluator::NOT_START_POSITION[NUM_SIDES][NUM_PIECE_TYPES];
  Bitboard Evaluator::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::iso_pawn_mask_[NUM_SQUARES];
  constexpr Bitboard Evaluator::PAWN_SHIELD_MASK[NUM_SIDES][NUM_SQUARES];
  constexpr Bitboard Evaluator::WEAK_SQUARE_MASK[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::pin_back_table_[NUM_SQUARES][0xff + 1][NUM_ROTS];

  // ================ //
  // テンプレート部品 //
  // ================ //
  // 評価用ビットボードを作成するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct GenBitboards {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {}
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, PAWN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      // 通常の動き。
      pawn_moves = engine.GetPawnStep(SIDE, square);

      // 攻撃。
      attacks = Util::GetPawnAttack(SIDE, square);

      // アンパッサン。
      if (engine.basic_st_.en_passant_square_
      && (SIDE == engine.basic_st_.to_move_)) {
        en_passant =
        Util::SQUARE[engine.basic_st_.en_passant_square_][R0] & attacks;
      }
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KNIGHT> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKnightMove(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetBishopAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetRookAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetQueenAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKingMove(square);
    }
  };

  // 駒の配置価値を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalPosition {
    static void F(Evaluator& evaluator, Square square) {}
  };
  template<PieceType TYPE>
  struct CalPosition<WHITE, TYPE> {
    static void F(Evaluator& evaluator, Square square) {
      evaluator.score_ +=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][square]
      + evaluator.cache_ptr_->ending_position_cache_[TYPE][square];
    }
  };
  template<PieceType TYPE>
  struct CalPosition<BLACK, TYPE> {
    static void F(Evaluator& evaluator, Square square) {
      evaluator.score_ -=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][Util::FLIP[square]]
      + evaluator.cache_ptr_->ending_position_cache_[TYPE][Util::FLIP[square]];
    }
  };

  // 駒の機動力を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalMobility {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st, Bitboard attacks,
    Bitboard pawn_moves, Bitboard en_passant) {
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[TYPE]
      [Util::CountBits(attacks & ~(basic_st.side_pieces_[SIDE]))];
    }
  };
  template<Side SIDE>
  struct CalMobility<SIDE, PAWN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st, Bitboard attacks,
    Bitboard pawn_moves, Bitboard en_passant) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[PAWN]
      [Util::CountBits((attacks & basic_st.side_pieces_[ENEMY_SIDE])
      | pawn_moves | en_passant)];
    }
  };

  // 各駒専用の価値を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalSpecial {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {}
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, PAWN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // パスポーンを計算。
      if (!(basic_st.position_[ENEMY_SIDE][PAWN]
      & Evaluator::pass_pawn_mask_[SIDE][square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->pass_pawn_cache_;

        // 守られたパスポーン。
        if (basic_st.position_[SIDE][PAWN]
        & Util::GetPawnAttack(ENEMY_SIDE, square)) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->protected_pass_pawn_cache_;
        }
      }

      // ダブルポーンを計算。
      if ((basic_st.position_[SIDE][PAWN]
      & Util::FYLE[Util::SquareToFyle(square)] & ~Util::SQUARE[square][R0])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->double_pawn_cache_;
      }

      // 孤立ポーンを計算。
      if (!(basic_st.position_[SIDE][PAWN]
      & Evaluator::iso_pawn_mask_[square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->iso_pawn_cache_;
      }

      // ポーンの盾を計算。
      if ((Util::SQUARE[square][R0]
      & Evaluator::PAWN_SHIELD_MASK[SIDE][basic_st.king_[SIDE]])) {
        if (SIDE == WHITE) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_[square];
        } else {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_[Util::FLIP[square]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, BISHOP> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // バッドビショップを計算。
      if ((Util::SQUARE[square][R0] & Util::SQCOLOR[WHITE])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (basic_st.position_[SIDE][PAWN] & Util::SQCOLOR[WHITE])];
      } else {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (basic_st.position_[SIDE][PAWN] & Util::SQCOLOR[BLACK])];
      }

      // ピンを計算。
      // ピンのターゲットと裏駒を作成。
      Bitboard pin_target = attacks & basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R45] >> Util::MAGIC_SHIFT[square][R45])
      & Util::MAGIC_MASK[square][R45]][R45]
      | Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R135] >> Util::MAGIC_SHIFT[square][R135])
      & Util::MAGIC_MASK[square][R135]][R135])
      & basic_st.side_pieces_[ENEMY_SIDE];

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & pin_target)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[BISHOP]
          [basic_st.piece_board_[Util::GetSquare(between & pin_target)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, ROOK> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // オープンファイルとセミオープンファイルを計算。
      Bitboard rook_fyle = Util::FYLE[Util::SquareToFyle(square)];
      if (!(basic_st.position_[SIDE][PAWN] & rook_fyle)) {
        // セミオープン。
        evaluator.score_ +=
        SIGN * evaluator.cache_ptr_->rook_semiopen_fyle_cache_;
        if (!(basic_st.position_[ENEMY_SIDE][PAWN] & rook_fyle)) {
          // オープン。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->rook_open_fyle_cache_;
        }
      }

      // ピンを計算。
      // ピンのターゲットと裏駒を作成。
      Bitboard pin_target = attacks & basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R0] >> Util::MAGIC_SHIFT[square][R0])
      & Util::MAGIC_MASK[square][R0]][R0]
      | Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R90] >> Util::MAGIC_SHIFT[square][R90])
      & Util::MAGIC_MASK[square][R90]][R90])
      & basic_st.side_pieces_[ENEMY_SIDE];

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & pin_target)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[ROOK]
          [basic_st.piece_board_[Util::GetSquare(between & pin_target)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, QUEEN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // クイーンの早過ぎる始動を計算。
      if (!(Util::SQUARE[square][R0]
      & Evaluator::START_POSITION[SIDE][QUEEN])) {
        evaluator.score_ +=
        SIGN * evaluator.cache_ptr_->early_queen_starting_cache_
        [Util::CountBits((basic_st.position_[SIDE][KNIGHT]
        & Evaluator::START_POSITION[SIDE][KNIGHT])
        | (basic_st.position_[SIDE][BISHOP]
        & Evaluator::START_POSITION[SIDE][BISHOP]))];
      }

      // ピンを計算。
      // ピンのターゲットと裏駒を作成。
      Bitboard pin_target =
      attacks & basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R0] >> Util::MAGIC_SHIFT[square][R0])
      & Util::MAGIC_MASK[square][R0]][R0]
      | Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R45] >> Util::MAGIC_SHIFT[square][R45])
      & Util::MAGIC_MASK[square][R45]][R45]
      | Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R90] >> Util::MAGIC_SHIFT[square][R90])
      & Util::MAGIC_MASK[square][R90]][R90]
      | Evaluator::pin_back_table_[square]
      [(basic_st.blocker_[R135] >> Util::MAGIC_SHIFT[square][R135])
      & Util::MAGIC_MASK[square][R135]][R135])
      & basic_st.side_pieces_[ENEMY_SIDE];

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & pin_target)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[QUEEN]
          [basic_st.piece_board_[Util::GetSquare(between & pin_target)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, KING> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // --- キング周りの弱いマスを計算 --- //
      int value = 0;

      // 弱いマス。
      Bitboard weak = (~(basic_st.position_[SIDE][PAWN]))
      & Evaluator::WEAK_SQUARE_MASK[SIDE][square];

      // 相手の白マスビショップに対する弱いマス。
      if ((basic_st.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[WHITE])) {
        value += Util::CountBits(weak & Util::SQCOLOR[WHITE]);
      }

      // 相手の黒マスビショップに対する弱いマス。
      if ((basic_st.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value += Util::CountBits(weak & Util::SQCOLOR[BLACK]);
      }

      // 評価値にする。
      evaluator.score_ +=
      SIGN * evaluator.cache_ptr_->weak_square_cache_[value];

      // --- キャスリングを計算する --- //
      constexpr Castling RIGHTS_MASK =
      SIDE == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (basic_st.has_castled_[SIDE]) {
        // キャスリングした。
        evaluator.score_ += SIGN * evaluator.cache_ptr_->castling_cache_;
      } else {
        if (!(basic_st.castling_rights_ & RIGHTS_MASK)) {
          // キャスリングの権利を放棄した。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->abandoned_castling_cache_;
        }
      }
    }
  };

  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  Evaluator::Evaluator(const ChessEngine& engine)
  : engine_ptr_(&engine), score_(0), cache_ptr_(nullptr) {}

  // コピーコンストラクタ。
  Evaluator::Evaluator(const Evaluator& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {}

  // ムーブコンストラクタ。
  Evaluator::Evaluator(Evaluator&& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {}

  // コピー代入演算子。
  Evaluator& Evaluator::operator=(const Evaluator& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  // ムーブ代入演算子。
  Evaluator& Evaluator::operator=(Evaluator&& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  // ======================= //
  // Evaluatorクラスの初期化 //
  // ======================= //
  // static変数の初期化。
  void Evaluator::InitEvaluator() {
    // pass_pawn_mask_[][]を初期化する。
    InitPassPawnMask();
    // iso_pawn_mask_[]を初期化する。
    InitIsoPawnMask();
    // pin_back_table_[][][][][]を初期化する。
    InitPinBackTable();
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // 現在の局面の評価値を計算する。
  int Evaluator::Evaluate(int material) {
    // 準備。
    const ChessEngine::BasicStruct& basic_st = engine_ptr_->basic_st_;
    // 初期化。
    cache_ptr_ = &(engine_ptr_->shared_st_ptr_->cache_.eval_cache_
    [Util::CountBits(basic_st.blocker_[R0])]);
    score_ = 0;

    const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES] =
    basic_st.position_;

    // --- 全体計算 --- //
    // 駒の展開。
    for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
      score_ += cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(basic_st.position_[WHITE][piece_type]
      & Evaluator::NOT_START_POSITION[WHITE][piece_type])]

      - cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(basic_st.position_[BLACK][piece_type]
      & Evaluator::NOT_START_POSITION[BLACK][piece_type])];
    }

    // ビショップペア。
    if ((position[WHITE][BISHOP] & Util::SQCOLOR[WHITE])
    && (position[WHITE][BISHOP] & Util::SQCOLOR[BLACK])) {
      score_ += cache_ptr_->bishop_pair_cache_;
    }
    if ((position[BLACK][BISHOP] & Util::SQCOLOR[WHITE])
    && (position[BLACK][BISHOP] & Util::SQCOLOR[BLACK])) {
      score_ -= cache_ptr_->bishop_pair_cache_;
    }

    // ルークペア。
    if ((position[WHITE][ROOK] & (position[WHITE][ROOK] - 1))) {
      score_ += cache_ptr_->rook_pair_cache_;
    }
    if ((position[BLACK][ROOK] & (position[BLACK][ROOK] - 1))) {
      score_ -= cache_ptr_->rook_pair_cache_;
    }

    // 各駒毎に価値を計算する。
    // 白のポーン。
    for (Bitboard pieces = position[WHITE][PAWN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, PAWN>(piece_square);
    }

    // 白のナイト。
    for (Bitboard pieces = position[WHITE][KNIGHT]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, KNIGHT>(piece_square);
    }

    // 白のビショップ。
    for (Bitboard pieces = position[WHITE][BISHOP]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, BISHOP>(piece_square);
    }

    // 白のルーク。
    for (Bitboard pieces = position[WHITE][ROOK]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, ROOK>(piece_square);
    }

    // 白のクイーン。
    for (Bitboard pieces = position[WHITE][QUEEN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, QUEEN>(piece_square);
    }

    // 白のキング。
    CalValue<WHITE, KING>(basic_st.king_[WHITE]);

    // 黒のポーン。
    for (Bitboard pieces = position[BLACK][PAWN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, PAWN>(piece_square);
    }

    // 黒のナイト。
    for (Bitboard pieces = position[BLACK][KNIGHT]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, KNIGHT>(piece_square);
    }

    // 黒のビショップ。
    for (Bitboard pieces = position[BLACK][BISHOP]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, BISHOP>(piece_square);
    }

    // 黒のルーク。
    for (Bitboard pieces = position[BLACK][ROOK]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, ROOK>(piece_square);
    }

    // 黒のクイーン。
    for (Bitboard pieces = position[BLACK][QUEEN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, QUEEN>(piece_square);
    }

    // 黒のキング。
    CalValue<BLACK, KING>(basic_st.king_[BLACK]);

    // 256で割る。
    score_ >>= 8;

    // 手番に合わせて符号を変えて返す。
    return basic_st.to_move_ == WHITE ? (material + score_)
    : (material - score_);
  }

  // ================== //
  // 価値を計算する関数 //
  // ================== //
  // 各駒の価値を計算する。
  template<Side SIDE, PieceType TYPE>
  void Evaluator::CalValue(Square piece_square) {
    constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
    constexpr int SIGN = SIDE == WHITE ? 1 : -1;

    // 準備。
    const ChessEngine::BasicStruct& basic_st = engine_ptr_->basic_st_;

    // 利き筋を作る。
    Bitboard attacks = 0;
    Bitboard pawn_moves = 0;
    Bitboard en_passant = 0;
    GenBitboards<SIDE, TYPE>::F(*this, *engine_ptr_, piece_square, attacks,
    pawn_moves, en_passant);

    // --- 全駒共通 --- //
    // オープニング、エンディング時の駒の配置を計算。
    CalPosition<SIDE, TYPE>::F(*this, piece_square);

    // 機動力を計算。
    CalMobility<SIDE, TYPE>::F
    (*this, basic_st, attacks, pawn_moves, en_passant);

    // センターコントロールを計算。
    score_ += SIGN * cache_ptr_->center_control_cache_[TYPE]
    [Util::CountBits(attacks & CENTER_MASK)];

    // スウィートセンターのコントロールを計算。
    score_ += SIGN * cache_ptr_->sweet_center_control_cache_[TYPE]
    [Util::CountBits(attacks & SWEET_CENTER_MASK)];

    // 敵への攻撃を計算。
    {
      Bitboard attacked = attacks & (basic_st.side_pieces_[ENEMY_SIDE]);

      for (; attacked; NEXT_BITBOARD(attacked)) {
        score_ += SIGN * cache_ptr_->attack_cache_[TYPE]
        [basic_st.piece_board_[Util::GetSquare(attacked)]];
      }

      if (en_passant) {
        score_ += SIGN * cache_ptr_->attack_cache_[PAWN][PAWN];
      }
    }

    // 味方への防御を計算。
    {
      Bitboard defensed = attacks & (basic_st.side_pieces_[SIDE]);

      for (; defensed; NEXT_BITBOARD(defensed)) {
        score_ += SIGN * cache_ptr_->defense_cache_[TYPE]
        [basic_st.piece_board_[Util::GetSquare(defensed)]];
      }
    }

    // 相手キング周辺への攻撃を計算。
    score_ += SIGN * cache_ptr_->attack_around_king_cache_[TYPE]
    [Util::CountBits(attacks & Util::GetKingMove(basic_st.king_[ENEMY_SIDE]))];

    // 各駒専用の価値を計算。
    CalSpecial<SIDE, TYPE>::F(*this, basic_st, piece_square, attacks);
  }

  // 実体化。
  template void Evaluator::CalValue<WHITE, PAWN>(Square);
  template void Evaluator::CalValue<WHITE, KNIGHT>(Square);
  template void Evaluator::CalValue<WHITE, BISHOP>(Square);
  template void Evaluator::CalValue<WHITE, ROOK>(Square);
  template void Evaluator::CalValue<WHITE, QUEEN>(Square);
  template void Evaluator::CalValue<WHITE, KING>(Square);
  template void Evaluator::CalValue<BLACK, PAWN>(Square);
  template void Evaluator::CalValue<BLACK, KNIGHT>(Square);
  template void Evaluator::CalValue<BLACK, BISHOP>(Square);
  template void Evaluator::CalValue<BLACK, ROOK>(Square);
  template void Evaluator::CalValue<BLACK, QUEEN>(Square);
  template void Evaluator::CalValue<BLACK, KING>(Square);

  // ======================== //
  // その他のプライベート関数 //
  // ======================== //
  // pass_pawn_mask_[][]を初期化する。
  void Evaluator::InitPassPawnMask() {
    // マスクを作って初期化する。
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
        Bitboard mask = 0;
        if (side == NO_SIDE) {  // どちらのサイドでもなければ0。
          pass_pawn_mask_[side][square] = 0;
        } else {
          // 自分のファイルと隣のファイルのマスクを作る。
          Fyle fyle = Util::SquareToFyle(square);
          mask |= Util::FYLE[fyle];
          if (fyle == FYLE_A) {  // aファイルのときはbファイルが隣り。
            mask |= Util::FYLE[fyle + 1];
          } else if (fyle == FYLE_H) {  // hファイルのときはgファイルが隣り。
            mask |= Util::FYLE[fyle - 1];
          } else {  // それ以外のときは両隣。
            mask |= Util::FYLE[fyle + 1];
            mask |= Util::FYLE[fyle - 1];
          }

          // 自分の位置より手前のランクは消す。
          if (side == WHITE) {
            Bitboard temp = (Util::SQUARE[square][R0] - 1)
            | Util::RANK[Util::SquareToRank(square)];
            mask &= ~temp;
          } else {
            Bitboard temp = ~(Util::SQUARE[square][R0] - 1)
            | Util::RANK[Util::SquareToRank(square)];
            mask &= ~temp;
          }

          // マスクをセット。
          pass_pawn_mask_[side][square] = mask;
        }
      }
    }
  }

  // iso_pawn_mask_[]を初期化する。
  void Evaluator::InitIsoPawnMask() {
    FOR_SQUARES(square) {
      Fyle fyle = Util::SquareToFyle(square);
      if (fyle == FYLE_A) {
        iso_pawn_mask_[square] = Util::FYLE[fyle + 1];
      } else if (fyle == FYLE_H) {
        iso_pawn_mask_[square] = Util::FYLE[fyle - 1];
      } else {
        iso_pawn_mask_[square] =
        Util::FYLE[fyle + 1] | Util::FYLE[fyle - 1];
      }
    }
  }

  // pin_back_table_[][][][][]を初期化する。
  void Evaluator::InitPinBackTable() {
    // 初期化。
    INIT_ARRAY(pin_back_table_);

    FOR_SQUARES(square) {
      for (Bitboard pattern = 0; pattern <= 0xff; ++pattern) {
        // 各角度のポイント。
        Bitboard point[NUM_ROTS] {
          (Util::SQUARE[square][R0] >> Util::MAGIC_SHIFT[square][R0])
          & Util::MAGIC_MASK[square][R0],
          (Util::SQUARE[square][R45] >> Util::MAGIC_SHIFT[square][R45])
          & Util::MAGIC_MASK[square][R45],
          (Util::SQUARE[square][R90] >> Util::MAGIC_SHIFT[square][R90])
          & Util::MAGIC_MASK[square][R90],
          (Util::SQUARE[square][R135] >> Util::MAGIC_SHIFT[square][R135])
          & Util::MAGIC_MASK[square][R135]
        };
        Bitboard temp[NUM_ROTS] {0, 0, 0, 0};
        bool find_target[NUM_ROTS] {false, false, false, false};

        // 先ず、左。
        COPY_ARRAY(temp, point);
        find_target[R0] = false;
        find_target[R45] = false;
        find_target[R90] = false;
        find_target[R135] = false;
        for (int i = 0; i < 8; ++i) {
          // シフト。
          temp[R0] = (temp[R0] << 1) & Util::MAGIC_MASK[square][R0];
          temp[R45] = (temp[R45] << 1) & Util::MAGIC_MASK[square][R45];
          temp[R90] = (temp[R90] << 1) & Util::MAGIC_MASK[square][R90];
          temp[R135] = (temp[R135] << 1) & Util::MAGIC_MASK[square][R135];

          // 0度。
          if (temp[R0]) {
            if (!find_target[R0]) {
              // ターゲットを見つける。
              if ((temp[R0] & pattern)) {
                find_target[R0] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R0] & pattern)) {
                pin_back_table_[square][pattern][R0] |=
                temp[R0] << Util::MAGIC_SHIFT[square][R0];

                // もう必要ない。
                temp[R0] = 0;
              }
            }
          }
          // 45度。
          if (temp[R45]) {
            if (!find_target[R45]) {
              // ターゲットを見つける。
              if ((temp[R45] & pattern)) {
                find_target[R45] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R45] & pattern)) {
                pin_back_table_[square][pattern][R45] |=
                Util::SQUARE[Util::R_ROT45[Util::GetSquare(temp[R45]
                << Util::MAGIC_SHIFT[square][R45])]][R0];

                // もう必要ない。
                temp[R45] = 0;
              }
            }
          }
          // 90度。
          if (temp[R90]) {
            if (!find_target[R90]) {
              // ターゲットを見つける。
              if ((temp[R90] & pattern)) {
                find_target[R90] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R90] & pattern)) {
                pin_back_table_[square][pattern][R90] |=
                Util::SQUARE[Util::R_ROT90[Util::GetSquare(temp[R90]
                << Util::MAGIC_SHIFT[square][R90])]][R0];

                // もう必要ない。
                temp[R90] = 0;
              }
            }
          }
          // 135度。
          if (temp[R135]) {
            if (!find_target[R135]) {
              // ターゲットを見つける。
              if ((temp[R135] & pattern)) {
                find_target[R135] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R135] & pattern)) {
                pin_back_table_[square][pattern][R135] |=
                Util::SQUARE[Util::R_ROT135[Util::GetSquare(temp[R135]
                << Util::MAGIC_SHIFT[square][R135])]][R0];

                // もう必要ない。
                temp[R135] = 0;
              }
            }
          }
        }

        // 次、右。
        COPY_ARRAY(temp, point);
        find_target[R0] = false;
        find_target[R45] = false;
        find_target[R90] = false;
        find_target[R135] = false;
        for (int i = 0; i < 8; ++i) {
          // シフト。
          temp[R0] = (temp[R0] >> 1) & Util::MAGIC_MASK[square][R0];
          temp[R45] = (temp[R45] >> 1) & Util::MAGIC_MASK[square][R45];
          temp[R90] = (temp[R90] >> 1) & Util::MAGIC_MASK[square][R90];
          temp[R135] = (temp[R135] >> 1) & Util::MAGIC_MASK[square][R135];

          // 0度。
          if (temp[R0]) {
            if (!find_target[R0]) {
              // ターゲットを見つける。
              if ((temp[R0] & pattern)) {
                find_target[R0] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R0] & pattern)) {
                pin_back_table_[square][pattern][R0] |=
                Util::SQUARE[Util::GetSquare(temp[R0]
                << Util::MAGIC_SHIFT[square][R0])][R0];

                // もう必要ない。
                temp[R0] = 0;
              }
            }
          }
          // 45度。
          if (temp[R45]) {
            if (!find_target[R45]) {
              // ターゲットを見つける。
              if ((temp[R45] & pattern)) {
                find_target[R45] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R45] & pattern)) {
                pin_back_table_[square][pattern][R45] |=
                Util::SQUARE[Util::R_ROT45[Util::GetSquare(temp[R45]
                << Util::MAGIC_SHIFT[square][R45])]][R0];

                // もう必要ない。
                temp[R45] = 0;
              }
            }
          }
          // 90度。
          if (temp[R90]) {
            if (!find_target[R90]) {
              // ターゲットを見つける。
              if ((temp[R90] & pattern)) {
                find_target[R90] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R90] & pattern)) {
                pin_back_table_[square][pattern][R90] |=
                Util::SQUARE[Util::R_ROT90[Util::GetSquare(temp[R90]
                << Util::MAGIC_SHIFT[square][R90])]][R0];

                // もう必要ない。
                temp[R90] = 0;
              }
            }
          }
          // 135度。
          if (temp[R135]) {
            if (!find_target[R135]) {
              // ターゲットを見つける。
              if ((temp[R135] & pattern)) {
                find_target[R135] = true;
              }
            } else {
              // バックを見つける。
              if ((temp[R135] & pattern)) {
                pin_back_table_[square][pattern][R135] |=
                Util::SQUARE[Util::R_ROT135[Util::GetSquare(temp[R135]
                << Util::MAGIC_SHIFT[square][R135])]][R0];

                // もう必要ない。
                temp[R135] = 0;
              }
            }
          }
        }
      }
    }
  }
}  // namespace Sayuri
