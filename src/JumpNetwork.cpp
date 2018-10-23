#include "JumpNetwork.h"

#include <stack>

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

    static inline std::list<lczero::BoardSquare> get_neighbours(const lczero::BoardSquare& source,
                                                                const lczero::BitBoard& our_board,
                                                                const lczero::BitBoard& complete_board) {
        std::list<lczero::BoardSquare> result;
        for (const auto& d : all_directions) {
            const int to_row = source.row() + 2 * d.first;
            const int to_col = source.col() + 2 * d.second;
            if (!lczero::BoardSquare::IsValid(to_row, to_col)) continue;
            const int over_row = source.row() + d.first;
            const int over_col = source.col() + d.second;
            if (our_board.get(over_row, over_col)) {
                const lczero::BoardSquare to_square(to_row, to_col);
                if (!complete_board.get(to_square)) {
                    result.push_back(to_square);
                }
            }
        }
        return result;
    }

    /**
     * Get the set of all pairs of destination squares
     * with the corresponding set of source squares.
     * All sets of destination squares will be disjunct.
     * @return list of source - destination squares pairs.
     */
    std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>>
    get_source_and_destination_squares(const lczero::BitBoard& our_board,
                                       const lczero::BitBoard& their_board) {
        std::array<int, 64> graphs{0};
        int graph_counter = 0;
        std::vector<std::list<lczero::BoardSquare>> sources;
        std::vector<std::list<lczero::BoardSquare>> destinations;
        const lczero::BitBoard complete_board = our_board + their_board;
        for (const lczero::BoardSquare& source_square : our_board) {
            for (const lczero::BoardSquare& graph_square : get_neighbours(source_square, our_board, complete_board)) {
                if (graphs[graph_square.as_int()] == 0) {
                    // This graph has not been visited
                    ++graph_counter;
                    std::list<lczero::BoardSquare> squares;
                    std::stack<lczero::BoardSquare> stack;
                    stack.push(graph_square);
                    while (!stack.empty()) {
                        const lczero::BoardSquare current_square = stack.top();
                        stack.pop();
                        squares.emplace_back(current_square);
                        graphs[current_square.as_int()] = graph_counter;
                        for (const lczero::BoardSquare& neighbour : get_neighbours(
                                current_square, our_board, complete_board)) {
                            if (graphs[neighbour.as_int()] == 0) {
                                stack.push(neighbour);
                            }
                        }
                        for (const lczero::BoardSquare& other_neighbour : get_neighbours(
                                current_square, their_board, complete_board)) {
                            // One jump over their piece is allowed,
                            // so add the squares connected to this one
                            // through one jump, if the square
                            // isn't part of any of our graphs
                            // through another node, because then
                            // it will be visited when searching
                            // the graph through another node.
                            squares.push_back(other_neighbour);
                        }
                    }
                    destinations.push_back(squares); // Add the destinations for this graph
                    std::list<lczero::BoardSquare> sources_for_this_graph;
                    sources_for_this_graph.push_back(source_square); // Add this square as the first source square
                    sources.push_back(sources_for_this_graph);
                } else {
                    // This graph has already been visited
                    sources[graphs[graph_square.as_int()] - 1].push_back(source_square); // Add this square as a source
                };
            }
        }
        std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>> result;
        for (int i = 0; i < graph_counter; ++i) {
            result.emplace_back(std::make_pair(sources[i], destinations[i]));
        }
        return result;
    }
}
