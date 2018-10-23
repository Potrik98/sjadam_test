#pragma once

#include <array>
#include <vector>
#include <list>
#include "chess/bitboard.h"

namespace sjadam {
    /**
     * Get the set of all pairs of destination squares
     * with the corresponding set of source squares.
     * All sets of destination squares will be disjunct.
     * @return list of source - destination squares pairs.
     */
    std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>>
    get_source_and_destination_squares(const lczero::BitBoard& our_board,
                                       const lczero::BitBoard& their_board);
}