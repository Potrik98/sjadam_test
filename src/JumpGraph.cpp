#include "JumpGraph.h"

#include <stack>
#include <stdio.h>
#include <sstream>

namespace sjadam {
    static const std::pair<int, int> all_directions[] = {
            {-1, -1},
            {-1, 0},
            {-1, 1},
            {0,  -1},
            {0,  1},
            {1,  -1},
            {1,  0},
            {1,  1}};
    static const std::pair<int, int> some_directions[] = {
            {1, 1},
            {1, 0},
            {1, -1},
            {0, 1}};

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
    void remove_connections_over(const lczero::BoardSquare& square,
                                 std::array<Node, 64>& nodes) {
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
     * @param complete_board the bit board of all the pieces
     * @param nodes the nodes of the graph to add the connections to
     */
    void add_connections_over(const lczero::BoardSquare& square,
                              const lczero::BitBoard& complete_board,
                              std::array<Node, 64>& nodes) {
        for (const auto& direction : some_directions) {
            const int s1_row = square.row() + direction.first;
            const int s1_col = square.col() + direction.second;
            const int s2_row = square.row() - direction.first;
            const int s2_col = square.col() - direction.second;
            if (!lczero::BoardSquare::IsValid(s1_row, s1_col)) continue;
            if (!lczero::BoardSquare::IsValid(s2_row, s2_col)) continue;
            const std::uint8_t s1 = lczero::BoardSquare(s1_row, s1_col).as_int();
            const std::uint8_t s2 = lczero::BoardSquare(s2_row, s2_col).as_int();
            if (!complete_board.get(s2)) nodes[s1].connect(nodes[s2]);
            if (!complete_board.get(s1)) nodes[s2].connect(nodes[s1]);
        }
    }

    JumpGraph::JumpGraph(lczero::BitBoard* our_board,
                         lczero::BitBoard* their_board) {
        init_nodes();
        this->our_board = our_board;
        this->their_board = their_board;
        const lczero::BitBoard complete_board = *our_board + *their_board;
        for (lczero::BoardSquare square : *our_board)
            add_connections_over(square, complete_board, (*our_nodes));
        for (lczero::BoardSquare square : *their_board)
            add_connections_over(square, complete_board, (*their_nodes));
    }

    void JumpGraph::init_nodes() {
        for (std::uint8_t i = 0; i < 64; ++i) {
            _our_nodes[i] = Node(i);
            _their_nodes[i] = Node(i);
        }
        our_nodes = &_our_nodes;
        their_nodes = &_their_nodes;
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
                (*our_nodes)[from_square.as_int()].connect((*our_nodes)[square.as_int()]);
            }
            if (their_board->get(over_row, over_col)) {
                (*their_nodes)[from_square.as_int()].connect((*their_nodes)[square.as_int()]);
            }
        }
    }

    /**
     * Remove all connections jumping to a square, for both sides
     * Useful for removing connections when a square has been occupied
     * @param square square to jump to
     */
    void JumpGraph::remove_connections_to(const lczero::BoardSquare& square) {
        for (const auto& direction : all_directions) {
            const int from_row = square.row() + 2 * direction.first;
            const int from_col = square.col() + 2 * direction.second;
            if (!lczero::BoardSquare::IsValid(from_row, from_col)) continue;
            const lczero::BoardSquare from_square(from_row, from_col);
            (*our_nodes)[from_square.as_int()].disconnect(square.as_int());
            (*their_nodes)[from_square.as_int()].disconnect(square.as_int());
        }
    }

    /**
     * Move one of our pieces from one square to another
     * @param from move from this square
     * @param to move to this square
     */
    void JumpGraph::move(const lczero::BoardSquare& from,
                         const lczero::BoardSquare& to) {
        remove_connections_over(from, *our_nodes);
        const lczero::BitBoard complete_board = *our_board + *their_board;
        add_connections_over(to, complete_board, *our_nodes);
        add_connections_to(from);
        remove_connections_to(to);
    }

    /**
     * Get the set of all pairs of destination squares
     * with the corresponding set of source squares.
     * All sets of destination squares will be disjunct.
     * @return list of source - destination squares pairs.
     */
    std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>>
    JumpGraph::get_source_and_destination_squares() {
        std::array<int, 64> graphs{0};
        int graph_counter = 0;
        std::vector<std::list<lczero::BoardSquare>> sources;
        std::vector<std::list<lczero::BoardSquare>> destinations;
        for (lczero::BoardSquare square : *our_board) {
            const std::list<const Node*>& neighbours = (*our_nodes)[square.as_int()].get_neighbours();
            for (const Node* n : neighbours) {
                if (graphs[n->get_square()] == 0) {
                    // This graph has not been visited
                    ++graph_counter;
                    std::list<lczero::BoardSquare> squares;
                    std::stack<const Node*> stack;
                    stack.push(n);
                    while (!stack.empty()) {
                        const Node* top = stack.top();
                        stack.pop();
                        if (graphs[top->get_square()] != 0) continue;
                        squares.emplace_back(top->get_square());
                        graphs[top->get_square()] = graph_counter;
                        for (const Node* nn : top->get_neighbours()) { stack.push(nn); }
                        for (const Node* tn : (*their_nodes)[top->get_square()].get_neighbours()) {
                            // One jump over their piece is allowed,
                            // so add the squares connected to this one
                            // through one jump, if the square
                            // isn't part of any of our graphs
                            // through another node, because then
                            // it will be visited when searching
                            // the graph through another node.
                            if (!(*our_nodes)[tn->get_square()].get_neighbours().empty()) {
                                // The node has neighbours, thus it is
                                // part of one of our graphs.
                                // The square is empty,
                                // (you cannot jump to an occupied square)
                                // so all outgoing connections
                                // has a corresponding incoming connection
                                // from another node in one of our graphs.
                                continue;
                            }
                            squares.emplace_back(tn->get_square());
                        }
                    }
                    destinations.emplace_back(squares); // Add the destinations for this graph
                    std::list<lczero::BoardSquare> this_source;
                    this_source.emplace_back(square); // Add this square as the first source square
                    sources.emplace_back(this_source);
                } else {
                    // This graph has already been visited
                    sources[graphs[n->get_square()] - 1].emplace_back(square); // Add this square as a source
                };
            }
        }
        std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>> result;
        for (int i = 0; i < graph_counter; ++i) {
            result.emplace_back(std::make_pair(sources[i], destinations[i]));
        }
        return result;
    }

    void JumpGraph::flip() {
        // Should swap pointers
        std::swap(our_nodes, their_nodes);
        std::swap(our_board, their_board);
    }
}