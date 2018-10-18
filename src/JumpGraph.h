#pragma once

#include <list>
#include <algorithm>
#include <array>
#include <cstdint>

#include "chess/bitboard.h"

namespace sjadam {
    class Node {
    private:
        std::list<Node*> neighbours;
        std::uint8_t square{};

    public:
        Node() = default;
        explicit Node(const std::uint8_t& square) {this->square = square;}

        bool operator==(const Node& other) const {
            return square == other.square;
        }

        std::uint8_t get_square() const { return square; }
        const std::list<Node*>& get_neighbours() const { return neighbours; }

        void connect(Node* other);
        void connect(Node& other) { this->connect(&other); }
        void connect_safe(Node* other);
        void connect_safe(Node& other) { this->connect_safe(&other); }
        void disconnect(Node* other);
        void disconnect(Node& other) { this->disconnect(&other); }
        bool is_connected_to(const Node* other);
    };

    class JumpGraph {
    private:
        std::array<Node, 64> white_nodes;
        std::array<Node, 64> black_nodes;
    public:
        JumpGraph();
        explicit JumpGraph(const lczero::BitBoard& white,
                           const lczero::BitBoard& black);
        void move(const lczero::BitBoard& bitBoard,
                  const lczero::BoardSquare& from,
                  const lczero::BoardSquare& to,
                  const std::uint8_t& color) {
            if (color == 0) move(bitBoard, from, to, white_nodes);
            else move(bitBoard, from, to, black_nodes);
        }

        void move(const lczero::BitBoard& bitBoard,
                  const lczero::BoardSquare& from,
                  const lczero::BoardSquare& to,
                  std::array<Node, 64>& nodes);

    private:
        void init_nodes();
    };
}