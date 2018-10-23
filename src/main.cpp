#include <iostream>
#include "chess/bitboard.h"
#include "chess/board.h"
#include "JumpNetwork.h"

void print_source_dest_sq(const std::list<std::pair<std::list<lczero::BoardSquare>, std::list<lczero::BoardSquare>>>& sd) {
    for (auto p : sd) {
        printf("{ ");
        for (auto sq : p.first) {
            printf("%s ", sq.as_string().c_str());
        }
        printf("} -> { ");
        for (auto sq : p.second) {
            printf("%s ", sq.as_string().c_str());
        }
        printf("}\n");
    }
}

void print_moves(lczero::ChessBoard& chessBoard) {
    lczero::MoveList moveList = chessBoard.GeneratePseudolegalMoves();
    std::list<std::string> list;
    for (auto m : moveList) {
        list.emplace_back(m.as_string());
    }
    list.sort();
    list.unique();
    std::cout << "Generated " << list.size() << " moves:" << std::endl;
    for (const auto& s : list) {
        std::cout << s << std::endl;
    }
}

int main() {
    lczero::ChessBoard chessBoard;
    chessBoard.SetFromFen(lczero::ChessBoard::kStartingFen);

    chessBoard.ApplyMove(lczero::Move(11, 19)); //d2d3
    chessBoard.Mirror();
    chessBoard.ApplyMove(lczero::Move(11, 19)); //d7d6
    chessBoard.Mirror();
    chessBoard.ApplyMove(lczero::Move(12, 28)); //e2e4
    chessBoard.Mirror();
    chessBoard.ApplyMove(lczero::Move(12, 20)); //e7e6
    chessBoard.Mirror();
    lczero::BitBoard o = chessBoard.ours();
    lczero::BitBoard t = chessBoard.theirs();
    print_moves(chessBoard);
    print_source_dest_sq(sjadam::get_source_and_destination_squares(o, t));
    chessBoard.ApplyMove(lczero::Move("c1d8"));
    std::cout << std::endl << chessBoard.DebugString() << std::endl;

    return 0;
}
