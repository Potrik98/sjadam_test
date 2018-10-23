#pragma once

#include <list>
#include <algorithm>
#include <memory>
#include <array>
#include <cstdint>
#include <experimental/array>

#include "chess/bitboard.h"

namespace sjadam {
    class JumpGraph {
    private:
        bool* our_connections;
        bool* their_connections;
        lczero::BitBoard* our_board;
        lczero::BitBoard* their_board;
    public:
        JumpGraph();
        ~JumpGraph();

        void set_bit_boards(lczero::BitBoard* our_board,
                            lczero::BitBoard* their_board);

        /**
         * Move one of our pieces from one square to another
         * @param from move from this square
         * @param to move to this square
         */
        void move(const lczero::BoardSquare& from,
                  const lczero::BoardSquare& to);

        /**
         * Flip the sides of the graph
         */
        void flip();

        /**
         * Get the set of all pairs of destination squares
         * with the corresponding set of source squares.
         * All sets of destination squares will be disjunct.
         * @return list of source - destination squares pairs.
         */
        std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>>
        get_source_and_destination_squares() const;

    private:
        void add_connections_to(const lczero::BoardSquare& square);

        void remove_connections_to(const lczero::BoardSquare& square);

        void connect_ours(const std::uint8_t& sq1,
                          const std::uint8_t& sq2);

        void connect_theirs(const std::uint8_t& sq1,
                            const std::uint8_t& sq2);

        void disconnect_ours(const std::uint8_t& sq1,
                             const std::uint8_t& sq2);

        void disconnect_theirs(const std::uint8_t& sq1,
                               const std::uint8_t& sq2);


    };
}