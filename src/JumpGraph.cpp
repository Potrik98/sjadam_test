#include "JumpGraph.h"

#include <stack>
#include <stdio.h>
#include <sstream>

namespace sjadam {
    void Node::connect(Node* other) {
        printf("connecting %d to %d \n", square, other->square);
        neighbours.push_back(other);
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

    /**
     * Remove all connections jumping over this square
     * @param square the square to remove all connections over
     * @param nodes the graph to remove the connections from
     */
    void remove_connections_over(const lczero::BoardSquare &square,
                                 std::array<Node, 64> &nodes) {
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

    /**
     * Add all connections jumping over this square
     * @param square the square being jumped over
     * @param board the bit board of the color of the piece
     * @param nodes the nodes of the graph to add the connections to
     */
    void add_connections_over(const lczero::BoardSquare &square,
                              const lczero::BitBoard &board,
                              std::array<Node, 64> &nodes) {
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
            if (!board.get(s2)) nodes[s1].connect(nodes[s2]);
            if (!board.get(s1)) nodes[s2].connect(nodes[s1]);
            if (lczero::BoardSquare::IsValidCoord(cp1)
                && lczero::BoardSquare::IsValidCoord(cm1)) {
                s1 = lczero::BoardSquare(rm1, cm1).as_int();
                s2 = lczero::BoardSquare(rp1, cp1).as_int();
                if (!board.get(s2)) nodes[s1].connect(nodes[s2]);
                if (!board.get(s1)) nodes[s2].connect(nodes[s1]);

                s1 = lczero::BoardSquare(rm1, cp1).as_int();
                s2 = lczero::BoardSquare(rp1, cm1).as_int();
                if (!board.get(s2)) nodes[s1].connect(nodes[s2]);
                if (!board.get(s1)) nodes[s2].connect(nodes[s1]);
            }
        }
        if (lczero::BoardSquare::IsValidCoord(cp1)
            && lczero::BoardSquare::IsValidCoord(cm1)) {
            uint8_t s1 = lczero::BoardSquare(row, cp1).as_int();
            uint8_t s2 = lczero::BoardSquare(row, cm1).as_int();
            if (!board.get(s2)) nodes[s1].connect(nodes[s2]);
            if (!board.get(s1)) nodes[s2].connect(nodes[s1]);
        }
    }

    /**
     * Initialize the graph connections of all possible jumps on this board
     * @param board the bit board of the pieces of this color
     * @param nodes the nodes of the graph of this color
     */
    void init_graph(const lczero::BitBoard& board,
                    std::array<Node, 64>& nodes) {
        for (lczero::BoardSquare square : board) {
            add_connections_over(square, board, nodes);
        }
    }

    JumpGraph::JumpGraph(lczero::BitBoard* our_board,
                         lczero::BitBoard* their_board) {
        init_nodes();
        this->our_board = our_board;
        this->their_board = their_board;
        init_graph(*our_board, our_nodes);
        init_graph(*their_board, their_nodes);
    }

    void JumpGraph::init_nodes() {
        for (std::uint8_t i = 0; i < 64; ++i) {
            our_nodes[i] = Node(i);
            their_nodes[i] = Node(i);
        }
    }

    /**
     * Move one of our pieces from one square to another
     * @param from move from this square
     * @param to move to this square
     */
    void JumpGraph::move(const lczero::BoardSquare& from,
                         const lczero::BoardSquare& to) {
        remove_connections_over(from, our_nodes);
        add_connections_over(to, *our_board, our_nodes);
    }
}