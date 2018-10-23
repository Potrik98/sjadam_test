/*
  This file is part of Leela Chess Zero.
  Copyright (C) 2018 The LCZero Authors

  Leela Chess is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Leela Chess is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Leela Chess.  If not, see <http://www.gnu.org/licenses/>.

  Additional permission under GNU GPL version 3 section 7

  If you modify this Program, or any covered work, by linking or
  combining it with NVIDIA Corporation's libraries from the NVIDIA CUDA
  Toolkit and the NVIDIA CUDA Deep Neural Network library (or a
  modified version of those libraries), containing parts covered by the
  terms of the respective license agreement, the licensors of this
  Program grant you additional permission to convey the resulting work.
*/

#include "bitboard.h"
#include "../utils/exception.h"

namespace lczero {

Move::Move(const std::string& str, bool black) {
  if (str.size() < 4) throw Exception("Bad move: " + str);
  SetFrom(BoardSquare(str.substr(0, 2), black));
  SetTo(BoardSquare(str.substr(2, 2), black));
  if (str.size() > 5) throw Exception("Bad move: " + str);
}

uint16_t Move::as_packed_int() const {
  return static_cast<uint16_t>(static_cast<int>(0) * 64 * 64 + from().as_int() * 64 +
                               to().as_int());
}

}  // namespace lczero
