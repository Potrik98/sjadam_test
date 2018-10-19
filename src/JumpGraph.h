#pragma once

#include <list>
#include <algorithm>
#include <array>
#include <cstdint>

#include "chess/bitboard.h"

namespace sjadam {
    class Node {
    private:
        std::list<const Node*> neighbours;
        std::uint8_t square{};

    public:
        Node() = default;
        explicit Node(const std::uint8_t& square) {this->square = square;}

        bool operator==(const Node& other) const {
            return square == other.square;
        }

        std::uint8_t get_square() const { return square; }
        const std::list<const Node*>& get_neighbours() const { return neighbours; }

        void connect(const Node* other);
        void connect(const Node& other) { this->connect(&other); }
        void disconnect(const std::uint8_t& other_square);
        void disconnect(const Node* other);
        void disconnect(const Node& other) { this->disconnect(&other); }
        bool is_connected_to(const Node* other);
    };

    class JumpGraph {
    private:
        std::array<Node, 64> our_nodes;
        std::array<Node, 64> their_nodes;
        lczero::BitBoard* our_board;
        lczero::BitBoard* their_board;
    public:
        explicit JumpGraph(lczero::BitBoard* our_board,
                           lczero::BitBoard* their_board);

        /**
         * Move one of our pieces from one square to another
         * @param from move from this square
         * @param to move to this square
         */
        void move(const lczero::BoardSquare& from,
                  const lczero::BoardSquare& to);

    private:
        void init_nodes();
        void add_connections_to(const lczero::BoardSquare& square);
        void remove_connections_to(const lczero::BoardSquare &square);
    };
}