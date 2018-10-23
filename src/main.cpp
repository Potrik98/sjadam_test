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
    lczero::MoveList moveList = chessBoard.GeneratePseudolegalMoves();
    printf("Generated %d moves:\n", moveList.size());
    std::list<std::string> list;
    for (auto m : moveList) {
        list.emplace_back(m.as_string());
    }
    list.sort();
    for (const auto& s : list) {
        std::cout << s << std::endl;
    }

    return 0;
}
