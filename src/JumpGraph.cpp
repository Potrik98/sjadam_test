#include "JumpGraph.h"

#include <stack>
#include <stdio.h>
#include <sstream>

namespace sjadam {
    void Node::connect(Node* other) {
        neighbours.push_back(other);
    }

    void Node::connect_safe(Node* other) {
        printf("Connecting %d to %d \n", square, other->square);
        if (!is_connected_to(other)) {
            neighbours.push_back(other);
        }
    }

    void Node::disconnect(Node* other) {
        printf("disconnecting %d from %d \n", other->square, square);
        neighbours.remove_if([other](const Node* node) {
            return *other == *node;
        });
    }

    bool Node::is_connected_to(const Node* other) {
        return std::any_of(
                std::begin(neighbours),
                std::end(neighbours),
                [other](const Node* node) {
                    return *other == *node;
                });
    }

    void remove_connections(const lczero::BitBoard& board,
                            const lczero::BoardSquare& square,
                            std::array<Node, 64>& nodes) {
        int row = square.row();
        int col = square.col();
        int rp1 = row + 1;
        int cp1 = col + 1;
        int rm1 = row - 1;
        int cm1 = col - 1;
        if (lczero::BoardSquare::IsValidCoord(rp1)
            && lczero::BoardSquare::IsValidCoord(rm1)) {
            uint8_t s1 = lczero::BoardSquare(rm1, col).as_int();
            uint8_t s2 = lczero::BoardSquare(rp1, col).as_int();
            nodes[s1].disconnect(nodes[s2]);
            nodes[s2].disconnect(nodes[s1]);
            if (lczero::BoardSquare::IsValidCoord(cp1)
                && lczero::BoardSquare::IsValidCoord(cm1)) {
                s1 = lczero::BoardSquare(rm1, cm1).as_int();
                s2 = lczero::BoardSquare(rp1, cp1).as_int();
                nodes[s1].disconnect(nodes[s2]);
                nodes[s2].disconnect(nodes[s1]);

                s1 = lczero::BoardSquare(rm1, cp1).as_int();
                s2 = lczero::BoardSquare(rp1, cm1).as_int();
                nodes[s1].disconnect(nodes[s2]);
                nodes[s2].disconnect(nodes[s1]);
            }
        }
        if (lczero::BoardSquare::IsValidCoord(cp1)
            && lczero::BoardSquare::IsValidCoord(cm1)) {
            uint8_t s1 = lczero::BoardSquare(row, cp1).as_int();
            uint8_t s2 = lczero::BoardSquare(row, cm1).as_int();
            nodes[s1].disconnect(nodes[s2]);
            nodes[s2].disconnect(nodes[s1]);
        }
    }

    void add_connections(const lczero::BitBoard& board,
                         const lczero::BoardSquare& square,
                         std::array<Node, 64>& nodes) {
        int row = square.row();
        int col = square.col();
        int rp1 = row + 1;
        int cp1 = col + 1;
        int rm1 = row - 1;
        int cm1 = col - 1;
        if (lczero::BoardSquare::IsValidCoord(rp1)
            && lczero::BoardSquare::IsValidCoord(rm1)) {
            uint8_t s1 = lczero::BoardSquare(rm1, col).as_int();
            uint8_t s2 = lczero::BoardSquare(rp1, col).as_int();
            if (!board.get(s2)) nodes[s1].connect_safe(nodes[s2]);
            if (!board.get(s1)) nodes[s2].connect_safe(nodes[s1]);
            if (lczero::BoardSquare::IsValidCoord(cp1)
                && lczero::BoardSquare::IsValidCoord(cm1)) {
                s1 = lczero::BoardSquare(rm1, cm1).as_int();
                s2 = lczero::BoardSquare(rp1, cp1).as_int();
                if (!board.get(s2)) nodes[s1].connect_safe(nodes[s2]);
                if (!board.get(s1)) nodes[s2].connect_safe(nodes[s1]);

                s1 = lczero::BoardSquare(rm1, cp1).as_int();
                s2 = lczero::BoardSquare(rp1, cm1).as_int();
                if (!board.get(s2)) nodes[s1].connect_safe(nodes[s2]);
                if (!board.get(s1)) nodes[s2].connect_safe(nodes[s1]);
            }
        }
        if (lczero::BoardSquare::IsValidCoord(cp1)
            && lczero::BoardSquare::IsValidCoord(cm1)) {
            uint8_t s1 = lczero::BoardSquare(row, cp1).as_int();
            uint8_t s2 = lczero::BoardSquare(row, cm1).as_int();
            if (!board.get(s2)) nodes[s1].connect_safe(nodes[s2]);
            if (!board.get(s1)) nodes[s2].connect_safe(nodes[s1]);
        }
    }

    void init_graph(const lczero::BitBoard& board,
                    std::array<Node, 64>& nodes) {
        for (lczero::BoardSquare square : board) {
            add_connections(board, square, nodes);
        }
    }

    JumpGraph::JumpGraph() {
        init_nodes();
    }

    JumpGraph::JumpGraph(const lczero::BitBoard& white,
                         const lczero::BitBoard& black) {
        init_nodes();
        init_graph(white, white_nodes);
        init_graph(black, black_nodes);
    }

    void JumpGraph::init_nodes() {
        for (std::uint8_t i = 0; i < 64; ++i) {
            white_nodes[i] = Node(i);
            black_nodes[i] = Node(i);
        }
    }

    void JumpGraph::move(const lczero::BitBoard& bitBoard,
                               const lczero::BoardSquare& from,
                               const lczero::BoardSquare& to,
                               std::array<Node, 64>& nodes) {
        remove_connections(bitBoard, from, nodes);
        add_connections(bitBoard, to, nodes);
    }
}