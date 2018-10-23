#include "JumpGraph.h"

#include <stack>
#include <stdio.h>
#include <sstream>
#include <cublas_v2.h>

namespace sjadam {
    static const std::pair<int, int> all_directions[8] = {
            {-1, -1},
            {-1, 0},
            {-1, 1},
            {0,  -1},
            {0,  1},
            {1,  -1},
            {1,  0},
            {1,  1}};
    static const std::pair<int, int> some_directions[4] = {
            {1, 1},
            {1, 0},
            {1, -1},
            {0, 1}};

    std::uint8_t mirror_square(const std::uint8_t& square) {
        return static_cast<uint8_t>(square ^ 0b111000);
    }

    void connect(const std::uint8_t& sq1,
                 const std::uint8_t& sq2,
                 bool* connections) {
        connections[sq1 * 64 + sq2] = true;
    }

    void disconnect(const std::uint8_t& sq1,
                    const uint8_t& sq2,
                    bool* connections) {
        connections[sq1 * 64 + sq2] = false;
    }

    void JumpGraph::connect_ours(const std::uint8_t& sq1, const std::uint8_t& sq2) {
        connect(sq1, sq2, our_connections);
    }

    void JumpGraph::connect_theirs(const std::uint8_t& sq1, const std::uint8_t& sq2) {
        connect(mirror_square(sq1), mirror_square(sq2), their_connections);
    }

    void JumpGraph::disconnect_ours(const std::uint8_t& sq1, const std::uint8_t& sq2) {
        disconnect(sq1, sq2, our_connections);
    }

    void JumpGraph::disconnect_theirs(const std::uint8_t& sq1, const std::uint8_t& sq2) {
        disconnect(mirror_square(sq1), mirror_square(sq2), their_connections);
    }

    /**
     * Remove all connections jumping over this square
     * @param square the square to remove all connections over
     * @param nodes the graph to remove the connections from
     */
    void remove_connections_over(const lczero::BoardSquare& square,
                                 bool* connections) {
        for (const auto& direction : some_directions) {
            const int s1_row = square.row() + direction.first;
            const int s1_col = square.col() + direction.second;
            const int s2_row = square.row() - direction.first;
            const int s2_col = square.col() - direction.second;
            if (!lczero::BoardSquare::IsValid(s1_row, s1_col)) continue;
            if (!lczero::BoardSquare::IsValid(s2_row, s2_col)) continue;
            const std::uint8_t s1 = lczero::BoardSquare(s1_row, s1_col).as_int();
            const std::uint8_t s2 = lczero::BoardSquare(s2_row, s2_col).as_int();
            disconnect(s1, s2, connections);
            disconnect(s2, s1, connections);
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
                              bool* connections) {
        for (const auto& direction : some_directions) {
            const int s1_row = square.row() + direction.first;
            const int s1_col = square.col() + direction.second;
            const int s2_row = square.row() - direction.first;
            const int s2_col = square.col() - direction.second;
            if (!lczero::BoardSquare::IsValid(s1_row, s1_col)) continue;
            if (!lczero::BoardSquare::IsValid(s2_row, s2_col)) continue;
            const std::uint8_t s1 = lczero::BoardSquare(s1_row, s1_col).as_int();
            const std::uint8_t s2 = lczero::BoardSquare(s2_row, s2_col).as_int();
            if (!complete_board.get(s2)) connect(s1, s2, connections);
            if (!complete_board.get(s1)) connect(s2, s1, connections);
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
                connect_ours(from_square.as_int(), square.as_int());
            } else if (their_board->get(over_row, over_col)) {
                connect_theirs(from_square.as_int(), square.as_int());
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
            disconnect_ours(from_square.as_int(), square.as_int());
            disconnect_theirs(from_square.as_int(), square.as_int());
        }
    }

    /**
     * Move one of our pieces from one square to another
     * @param from move from this square
     * @param to move to this square
     */
    void JumpGraph::move(const lczero::BoardSquare& from,
                         const lczero::BoardSquare& to) {
        remove_connections_over(from, our_connections);
        const lczero::BitBoard complete_board = *our_board + *their_board;
        add_connections_over(to, complete_board, our_connections);
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
            const int base_index = square.as_int() * 64;
            for (std::uint8_t sq = 0; sq < 64; ++sq) {
                if (!our_connections[base_index + sq]) continue;
                if (graphs[sq] == 0) {
                    // This graph has not been visited
                    ++graph_counter;
                    std::list<lczero::BoardSquare> squares;
                    std::stack<std::uint8_t> stack;
                    stack.push(sq);
                    while (!stack.empty()) {
                        const std::uint8_t top_sq = stack.top();
                        stack.pop();
                        if (graphs[top_sq] != 0) continue;
                        squares.emplace_back(top_sq);
                        graphs[top_sq] = graph_counter;
                        int bi = 64 * top_sq;
                        for (std::uint8_t n = 0; n < 64; ++n) {
                            if (our_connections[bi + n]) {
                                stack.push(n);
                            }
                        }
                        const uint8_t mirror = mirror_square(top_sq);
                        bi = 64 * mirror;
                        for (std::uint8_t n = 0; n < 64; ++n) {
                            // One jump over their piece is allowed,
                            // so add the squares connected to this one
                            // through one jump, if the square
                            // isn't part of any of our graphs
                            // through another node, because then
                            // it will be visited when searching
                            // the graph through another node.
                            if (their_connections[bi + n]) {
                                squares.emplace_back(mirror_square(n));
                            }
                        }
                    }
                    destinations.emplace_back(squares); // Add the destinations for this graph
                    std::list<lczero::BoardSquare> this_source;
                    this_source.emplace_back(square); // Add this square as the first source square
                    sources.emplace_back(this_source);
                } else {
                    // This graph has already been visited
                    sources[graphs[sq] - 1].emplace_back(square); // Add this square as a source
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
        std::swap(our_connections, their_connections);
        std::swap(our_board, their_board);
    }

    void JumpGraph::set_bit_boards(lczero::BitBoard* our_board, lczero::BitBoard* their_board) {
        this->our_board = our_board;
        this->their_board = their_board;
        lczero::BitBoard complete_board = *our_board + *their_board;
        for (lczero::BoardSquare square : *our_board)
            add_connections_over(square, complete_board, our_connections);
        their_board->Mirror();
        complete_board.Mirror();
        for (lczero::BoardSquare square : *their_board)
            add_connections_over(square, complete_board, their_connections);
        their_board->Mirror();
    }

    JumpGraph::JumpGraph() {
        our_connections = static_cast<bool*>(malloc(64 * 64 * sizeof(bool)));
        their_connections = static_cast<bool*>(malloc(64 * 64 * sizeof(bool)));
        printf("Allocated connections, ours: %x, theirs: %x\n", our_connections, their_connections);
    }

    JumpGraph::~JumpGraph() {
        printf("Freeing connections, ours: %x, theirs: %x\n", our_connections, their_connections);
        free(our_connections);
        free(their_connections);
    }
}