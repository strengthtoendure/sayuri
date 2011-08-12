/* transposition_table.h: トランスポジションテーブル。
   copyright (c) 2011 石橋宏之利
 */

#include "transposition_table.h"

#include <iostream>
#include <list>
#include "chess_def.h"
#include "chess_util.h"
#include "chess_board.h"

namespace Misaki {
  /****************************************
   * トランスポジションテーブルのクラス。 *
   ****************************************/
  // コンストラクタ。
  TranspositionTable::TranspositionTable() :
  num_slots_(0) {
  }
  // テーブルに追加する。
  void TranspositionTable::Add(hash_key_t key, int level, int depth,
  side_t to_move, int upper_bound, int lower_bound, move_t best_move) {
    // テーブルに追加。
    int index = GetTableIndex(key);
    table_[index].Add(key, level, depth, to_move, upper_bound, lower_bound,
    best_move);

    // スロットの個数を増やす。
    num_slots_++;

    // スロットが最大値を越えていれば、スロットを一つ消す。
    int current_level;
    int highest_level = -1;
    int highest_level_list_index = 0;
    if (num_slots_ > MAX_SIZE) {
      for (int index = 0; index < NUM_LISTS; index++) {
        if (table_[index].GetSize() > 0) {
          current_level = table_[index].GetHighestLevel();
          if (current_level > highest_level) {
            highest_level = current_level;
            highest_level_list_index = index;
          }
        }
      }
      // スロットを消す。
      if (table_[highest_level_list_index].GetSize()) {
        table_[highest_level_list_index].DeleteOne();
        num_slots_--;
      }
    }
  }

  /******************************
   * スロットのリストのクラス。 *
   ******************************/
  TranspositionTableSlot&
  TranspositionTableSlotList::GetSameSlot(hash_key_t key, int level,
  int depth, side_t to_move) throw (bool) {
    std::list<TranspositionTableSlot>::iterator itr = slot_list_.begin();
    while (itr != slot_list_.end()) {
      // レベルが大きくなりすぎたらもうない。
      if (itr->level() > level) {
        throw false;
      }
      // 同じスロットを見つけたらそれを返す。
      if (itr->Equals(key, level, depth, to_move)) {
        return *itr;
      }
      ++itr;
    }
    throw false;
  }

  /**********************
   * スロットのクラス。 *
   **********************/
  // コンストラクタ。
  TranspositionTableSlot::TranspositionTableSlot(hash_key_t key, int level,
  int depth, side_t to_move, int upper_bound, int lower_bound,
  move_t best_move) :
  key_(key),
  level_(level),
  depth_(depth),
  to_move_(to_move),
  upper_bound_(upper_bound),
  lower_bound_(lower_bound),
  best_move_(best_move) {
  }
  // コピーコンストラクタ。
  TranspositionTableSlot::TranspositionTableSlot(
  const TranspositionTableSlot& slot) :
  key_(slot.key_),
  level_(slot.level_),
  depth_(slot.depth_),
  to_move_(slot.to_move_),
  upper_bound_(slot.upper_bound_),
  lower_bound_(slot.lower_bound_),
  best_move_(slot.best_move_) {
  }
  // 代入。
  TranspositionTableSlot&
  TranspositionTableSlot::operator=(const TranspositionTableSlot& slot) {
    key_ = slot.key_;
    level_ = slot.level_;
    depth_ = slot.depth_;
    to_move_ = slot.to_move_;
    upper_bound_ = slot.upper_bound_;
    lower_bound_ = slot.lower_bound_;
    best_move_ = slot.best_move_;

    return *this;
  }
}  // Misaki
