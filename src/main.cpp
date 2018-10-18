#include <iostream>
#include "chess/bitboard.h"
#include "chess/board.h"
#include "JumpGraph.h"

int main() {
    lczero::ChessBoard chessBoard;
    chessBoard.SetFromFen(lczero::ChessBoard::kStartingFen);
    lczero::BitBoard white = chessBoard.ours();
    lczero::BitBoard black = chessBoard.theirs();
    sjadam::JumpGraph graph(white, black);
    lczero::BoardSquare from(10);
    lczero::BoardSquare to(18);
    white.reset(from);
    white.set(to);
    graph.move(white, from, to, 0);
    return 0;
}
