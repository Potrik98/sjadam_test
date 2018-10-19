#include "JumpGraph.h"

#include <stack>
#include <stdio.h>
#include <sstream>

namespace sjadam {
    static const std::pair<int, int> all_directions[] = {
            {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    static const std::pair<int, int> some_directions[] = {
            {1, 1}, {1, 0}, {1, -1}, {0, 1}};

    void Node::connect(const Node* other) {
        printf("connecting %d to %d \n", square, other->square);
        neighbours.push_back(other);
    }

    void Node::disconnect(const Node* other) {
        printf("disconnecting %d from %d \n", other->square, square);
        neighbours.remove_if([other](const Node* node) {
            return *other == *node;
        });
    }

    void Node::disconnect(const std::uint8_t& other_square) {
        printf("disconnecting %d from %d \n", other_square, square);
        neighbours.remove_if([other_square](const Node* node) {
            return other_square == node->square;
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
        for (const auto& direction : some_directions) {
            const int s1_row = square.row() + direction.first;
            const int s1_col = square.col() + direction.second;
            const int s2_row = square.row() - direction.first;
            const int s2_col = square.col() - direction.second;
            if (!lczero::BoardSquare::IsValid(s1_row, s1_col)) continue;
            if (!lczero::BoardSquare::IsValid(s2_row, s2_col)) continue;
            const std::uint8_t s1 = lczero::BoardSquare(s1_row, s1_col).as_int();
            const std::uint8_t s2 = lczero::BoardSquare(s2_row, s2_col).as_int();
            nodes[s1].disconnect(s2);
            nodes[s2].disconnect(s1);
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
        for (const auto& direction : some_directions) {
            const int s1_row = square.row() + direction.first;
            const int s1_col = square.col() + direction.second;
            const int s2_row = square.row() - direction.first;
            const int s2_col = square.col() - direction.second;
            if (!lczero::BoardSquare::IsValid(s1_row, s1_col)) continue;
            if (!lczero::BoardSquare::IsValid(s2_row, s2_col)) continue;
            const std::uint8_t s1 = lczero::BoardSquare(s1_row, s1_col).as_int();
            const std::uint8_t s2 = lczero::BoardSquare(s2_row, s2_col).as_int();
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
     * Add all connections jumping to a square, for both sides
     * Useful for adding connections to a newly vacated square
     * @param square square to jump to
     */
    void JumpGraph::add_connections_to(const lczero::BoardSquare& square) {
        for (const auto& direction : all_directions) {
            const int from_row = square.row() + 2 * direction.first;
            const int from_col = square.col() + 2 * direction.second;
            if (!lczero::BoardSquare::IsValid(from_row, from_col)) continue;
            const int over_row = square.row() + direction.first;
            const int over_col = square.col() + direction.second;
            const lczero::BoardSquare from_square(from_row, from_col);
            if (our_board->get(over_row, over_col)) {
                our_nodes[from_square.as_int()].connect(our_nodes[square.as_int()]);
            }
            if (their_board->get(over_row, over_col)) {
                their_nodes[from_square.as_int()].connect(their_nodes[square.as_int()]);
            }
        }
    }

    /**
     * Remove all connections jumping to a square, for both sides
     * Useful for removing connections when a square has been occupied
     * @param square square to jump to
     */
    void JumpGraph::remove_connections_to(const lczero::BoardSquare &square) {
        for (const auto& direction : all_directions) {
            const int from_row = square.row() + 2 * direction.first;
            const int from_col = square.col() + 2 * direction.second;
            if (!lczero::BoardSquare::IsValid(from_row, from_col)) continue;
            const lczero::BoardSquare from_square(from_row, from_col);
            our_nodes[from_square.as_int()].disconnect(square.as_int());
            their_nodes[from_square.as_int()].disconnect(square.as_int());
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
        add_connections_to(from);
        remove_connections_to(to);
    }
}