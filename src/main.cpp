#include <iostream>
#include "chess/bitboard.h"
#include "chess/board.h"
#include "JumpGraph.h"

void print(sjadam::JumpGraph& graph) {
    auto sd = graph.get_source_and_destination_squares();
    for (auto p : sd) {
        auto s = p.first;
        auto d = p.second;
        std::cout << "{ ";
        for (auto sq : s) {
            std::cout << (int) sq.as_int() << " ";
        }
        std::cout << "} -> { ";
        for (auto sq : d) {
            std::cout << (int) sq.as_int() << " ";
        }
        std::cout << "}" << std::endl;
    }
}

int main() {
    lczero::ChessBoard chessBoard;
    chessBoard.SetFromFen(lczero::ChessBoard::kStartingFen);
    lczero::BitBoard white = chessBoard.ours();
    lczero::BitBoard black = chessBoard.theirs();
    sjadam::JumpGraph graph;
    graph.set_bit_boards(&white, &black);

    print(graph);

    lczero::BoardSquare from(10);
    lczero::BoardSquare to(18);
    white.reset(from);
    white.set(to);
    graph.move(from, to);

    print(graph);
    graph.flip();
    black.Mirror();
    white.Mirror();
    black.reset(10);
    black.set(18);
    graph.move(10, 18);
    black.reset(11);
    black.set(27);
    graph.move(11, 27);
    print(graph);

    return 0;
}
