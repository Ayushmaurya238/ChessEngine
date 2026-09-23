#include <SFML/Graphics.hpp>

#include "ChessEngine.h"
#include "MoveGenerator.h"
#include "SearchEngine.h"
#include "Move.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr unsigned WINDOW_W = 1220;
constexpr unsigned WINDOW_H = 790;
constexpr float BOARD_X = 34.f;
constexpr float BOARD_Y = 66.f;
constexpr float SQ = 82.f;
constexpr float BOARD_SIZE = SQ * 8.f;
constexpr float PANEL_X = BOARD_X + BOARD_SIZE + 28.f;
constexpr float PANEL_W = WINDOW_W - PANEL_X - 28.f;

const sf::Color BG(28, 31, 36);
const sf::Color PANEL(39, 43, 49);
const sf::Color PANEL_2(48, 53, 60);
const sf::Color LIGHT(238, 217, 181);
const sf::Color DARK(181, 136, 99);
const sf::Color ACCENT(94, 170, 255);
const sf::Color GREEN(95, 190, 125);
const sf::Color RED(220, 90, 90);
const sf::Color TEXT(235, 238, 242);
const sf::Color MUTED(158, 166, 177);

char32_t pieceGlyph(PieceType type, Color color) {
    if (color == Color::WHITE) {
        switch (type) {
            case PieceType::KING:   return U'\u2654';
            case PieceType::QUEEN:  return U'\u2655';
            case PieceType::ROOK:   return U'\u2656';
            case PieceType::BISHOP: return U'\u2657';
            case PieceType::KNIGHT: return U'\u2658';
            case PieceType::PAWN:   return U'\u2659';
            default: return U'\0';
        }
    }
    switch (type) {
        case PieceType::KING:   return U'\u265A';
        case PieceType::QUEEN:  return U'\u265B';
        case PieceType::ROOK:   return U'\u265C';
        case PieceType::BISHOP: return U'\u265D';
        case PieceType::KNIGHT: return U'\u265E';
        case PieceType::PAWN:   return U'\u265F';
        default: return U'\0';
    }
}

std::string sideName(Color c) {
    return c == Color::WHITE ? "White" : "Black";
}

std::string formatScore(int score) {
    double pawns = static_cast<double>(score) / 100.0;
    std::ostringstream os;
    os << std::showpos << std::fixed << std::setprecision(2) << pawns;
    return os.str();
}

class ChessGUI {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Font pieceFont;
    ChessEngine engine;

    std::vector<Move> selectedMoves;
    int selectedRow = -1;
    int selectedCol = -1;
    std::vector<std::string> history;

    bool thinking = false;
    std::atomic<bool> searchFinished{false};
    std::thread searchThread;
    std::mutex resultMutex;
    Move pendingBestMove;
    SearchEngine::SearchInfo pendingInfo;

    int engineDepth = 6;
    int lastEval = 0;
    SearchEngine::SearchInfo lastInfo;
    std::string status = "Your turn";
    std::string message = "Click a piece to begin";

public:
    ChessGUI()
        : window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Cpp Chess Engine") {
        window.setFramerateLimit(60);

        if (!font.openFromFile("assets/fonts/DejaVuSans.ttf")) {
            throw std::runtime_error("Could not load assets/fonts/DejaVuSans.ttf");
        }
        if (!pieceFont.openFromFile("assets/fonts/DejaVuSans.ttf")) {
            throw std::runtime_error("Could not load assets/fonts/DejaVuSans.ttf for chess pieces");
        }

        engine.newGame();
        updateStatus();
    }

    ~ChessGUI() {
        stopSearch();
    }

    void run() {
        while (window.isOpen()) {
            processSearchResult();
            processEvents();
            draw();
        }
    }

private:
    void stopSearch() {
        if (searchThread.joinable()) searchThread.join();
        thinking = false;
    }

    void resetSelection() {
        selectedRow = selectedCol = -1;
        selectedMoves.clear();
    }

    void newGame() {
        stopSearch();
        engine.newGame();
        history.clear();
        resetSelection();
        lastEval = 0;
        lastInfo = {};
        message = "New game started";
        updateStatus();
    }

    void undo() {
        if (thinking) return;
        if (engine.undoMove()) {
            if (!history.empty()) history.pop_back();
            // If the engine is playing Black, undo both plies so the human gets the turn back.
            if (engine.getBoard().getSideToMove() == Color::BLACK && engine.getBoard().canUndo()) {
                engine.undoMove();
                if (!history.empty()) history.pop_back();
            }
            resetSelection();
            message = "Move undone";
            updateStatus();
        }
    }

    bool humanTurn() const {
        return !thinking && engine.getBoard().getSideToMove() == Color::WHITE;
    }

    void selectSquare(int row, int col) {
        if (!humanTurn()) return;

        auto legal = engine.legalMoves();
        Piece clicked = engine.getBoard().getPiece(row, col);

        if (selectedRow != -1) {
            std::vector<Move> candidates;
            for (const auto& m : selectedMoves) {
                if (m.toRow == row && m.toCol == col) candidates.push_back(m);
            }
            if (!candidates.empty()) {
                // Queen promotion is selected automatically for now.
                Move chosen = candidates.front();
                for (const auto& m : candidates) {
                    if (m.promotionPiece == PieceType::QUEEN) {
                        chosen = m;
                        break;
                    }
                }
                if (engine.makeMove(chosen)) {
                    history.push_back(moveToUCI(chosen));
                    resetSelection();
                    message = "Move played: " + moveToUCI(chosen);
                    updateStatus();
                    if (engine.getBoard().getSideToMove() == Color::BLACK) startEngineSearch();
                    return;
                }
            }
        }

        if (clicked.type != PieceType::NONE && clicked.color == Color::WHITE) {
            selectedRow = row;
            selectedCol = col;
            selectedMoves.clear();
            for (const auto& m : legal) {
                if (m.fromRow == row && m.fromCol == col) selectedMoves.push_back(m);
            }
            message = selectedMoves.empty() ? "No legal moves" : "Choose a highlighted square";
        } else {
            resetSelection();
            message = "Select a white piece";
        }
    }

    void startEngineSearch() {
        if (thinking) return;
        resetSelection();
        thinking = true;
        searchFinished.store(false);
        status = "Engine thinking...";
        message = "Searching at depth " + std::to_string(engineDepth);

        Board position = engine.getBoard();
        int depth = engineDepth;

        searchThread = std::thread([this, position, depth]() mutable {
            SearchEngine searcher;
            Move best = searcher.findBestMove(position, depth, 0);
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                pendingBestMove = best;
                pendingInfo = searcher.getInfo();
            }
            searchFinished.store(true);
        });
    }

    void processSearchResult() {
        if (!thinking || !searchFinished.load()) return;

        if (searchThread.joinable()) searchThread.join();

        Move best;
        SearchEngine::SearchInfo info;
        {
            std::lock_guard<std::mutex> lock(resultMutex);
            best = pendingBestMove;
            info = pendingInfo;
        }

        thinking = false;
        searchFinished.store(false);
        lastInfo = info;
        lastEval = info.score;

        if (best.fromRow >= 0 && engine.makeMove(best)) {
            history.push_back(moveToUCI(best));
            message = "Engine played: " + moveToUCI(best);
        } else {
            message = "Game over";
        }
        updateStatus();
    }

    void updateStatus() {
        auto moves = engine.legalMoves();
        Color side = engine.getBoard().getSideToMove();
        if (moves.empty()) {
            if (MoveGenerator::isInCheck(engine.getBoard(), side)) {
                status = side == Color::WHITE ? "Checkmate — Black wins" : "Checkmate — White wins";
            } else {
                status = "Stalemate — Draw";
            }
        } else if (MoveGenerator::isInCheck(engine.getBoard(), side)) {
            status = sideName(side) + " is in check";
        } else if (!thinking) {
            status = side == Color::WHITE ? "Your turn" : "Engine turn";
        }
    }

    void processEvents() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse->button != sf::Mouse::Button::Left) continue;
                handleMouse(mouse->position.x, mouse->position.y);
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::N) newGame();
                else if (key->code == sf::Keyboard::Key::U) undo();
                else if (key->code == sf::Keyboard::Key::Escape) resetSelection();
            }
        }
    }

    void handleMouse(int x, int y) {
        const float fx = static_cast<float>(x);
        const float fy = static_cast<float>(y);

        if (fx >= BOARD_X && fx < BOARD_X + BOARD_SIZE &&
            fy >= BOARD_Y && fy < BOARD_Y + BOARD_SIZE) {
            int col = static_cast<int>((fx - BOARD_X) / SQ);
            int row = static_cast<int>((fy - BOARD_Y) / SQ);
            selectSquare(row, col);
            return;
        }

        // New Game button
        if (fx >= PANEL_X && fx <= PANEL_X + PANEL_W && fy >= 88 && fy <= 136) {
            newGame();
            return;
        }

        // Undo button
        if (fx >= PANEL_X && fx <= PANEL_X + PANEL_W && fy >= 148 && fy <= 196) {
            undo();
            return;
        }

        // Difficulty/depth buttons
        if (fx >= PANEL_X && fx <= PANEL_X + PANEL_W && fy >= 260 && fy <= 310) {
            float rel = (fx - PANEL_X) / PANEL_W;
            engineDepth = rel < 0.34f ? 4 : (rel < 0.67f ? 6 : 8);
            message = "Engine depth set to " + std::to_string(engineDepth);
        }
    }

    void draw() {
        window.clear(BG);
        drawHeader();
        drawBoard();
        drawPanel();
        window.display();
    }

    void drawHeader() {
        sf::Text title(font, "CPP CHESS ENGINE", 28);
        title.setPosition({34.f, 20.f});
        title.setFillColor(TEXT);
        window.draw(title);

        sf::Text subtitle(font, "C++17  |  Alpha-Beta  |  Transposition Table", 15);
        subtitle.setPosition({305.f, 27.f});
        subtitle.setFillColor(MUTED);
        window.draw(subtitle);
    }

    void drawBoard() {
        auto legal = engine.legalMoves();
        int kingRow = -1, kingCol = -1;
        Color side = engine.getBoard().getSideToMove();
        if (MoveGenerator::isInCheck(engine.getBoard(), side)) {
            int k = engine.getBoard().findKing(side);
            if (k >= 0) { kingRow = k / 8; kingCol = k % 8; }
        }

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                sf::RectangleShape square({SQ, SQ});
                square.setPosition({BOARD_X + c * SQ, BOARD_Y + r * SQ});
                square.setFillColor(((r + c) & 1) ? DARK : LIGHT);
                window.draw(square);

                if (r == selectedRow && c == selectedCol) {
                    sf::RectangleShape selected({SQ, SQ});
                    selected.setPosition({BOARD_X + c * SQ, BOARD_Y + r * SQ});
                    selected.setFillColor(sf::Color(80, 155, 235, 105));
                    window.draw(selected);
                }

                bool destination = false;
                bool capture = false;
                for (const auto& m : selectedMoves) {
                    if (m.toRow == r && m.toCol == c) {
                        destination = true;
                        capture = m.isCapture || m.isEnPassant;
                        break;
                    }
                }
                if (destination) {
                    sf::CircleShape dot(capture ? 28.f : 11.f);
                    dot.setPosition({BOARD_X + c * SQ + (SQ - dot.getRadius() * 2.f) / 2.f,
                                     BOARD_Y + r * SQ + (SQ - dot.getRadius() * 2.f) / 2.f});
                    dot.setFillColor(capture ? sf::Color(220, 90, 90, 115)
                                             : sf::Color(40, 100, 60, 170));
                    window.draw(dot);
                }

                if (r == kingRow && c == kingCol) {
                    sf::RectangleShape check({SQ, SQ});
                    check.setPosition({BOARD_X + c * SQ, BOARD_Y + r * SQ});
                    check.setFillColor(sf::Color(220, 65, 65, 90));
                    window.draw(check);
                }

                Piece p = engine.getBoard().getPiece(r, c);
                if (p.type != PieceType::NONE) drawPiece(p, r, c);
            }
        }

        // Coordinate labels
        for (int c = 0; c < 8; ++c) {
            sf::Text t(font, std::string(1, char('a' + c)), 13);
            t.setPosition({BOARD_X + c * SQ + 4.f, BOARD_Y + BOARD_SIZE - 19.f});
            t.setFillColor(((7 + c) & 1) ? LIGHT : DARK);
            window.draw(t);
        }
        for (int r = 0; r < 8; ++r) {
            sf::Text t(font, std::string(1, char('8' - r)), 13);
            t.setPosition({BOARD_X + 4.f, BOARD_Y + r * SQ + 3.f});
            t.setFillColor((r & 1) ? LIGHT : DARK);
            window.draw(t);
        }
    }

    void drawPiece(const Piece& p, int row, int col) {
        sf::Text piece(pieceFont, pieceGlyph(p.type, p.color), 68);
        sf::FloatRect bounds = piece.getLocalBounds();
        float x = BOARD_X + col * SQ + (SQ - bounds.size.x) / 2.f - bounds.position.x;
        float y = BOARD_Y + row * SQ + (SQ - bounds.size.y) / 2.f - bounds.position.y - 3.f;
        piece.setPosition({x, y});
        piece.setFillColor(p.color == Color::WHITE ? sf::Color(250, 250, 250)
                                                   : sf::Color(25, 27, 30));
        piece.setOutlineColor(p.color == Color::WHITE ? sf::Color(35, 35, 35)
                                                      : sf::Color(235, 235, 235));
        piece.setOutlineThickness(1.5f);
        window.draw(piece);
    }

    void drawPanel() {
        sf::RectangleShape panel({PANEL_W, 690.f});
        panel.setPosition({PANEL_X, 66.f});
        panel.setFillColor(PANEL);
        panel.setOutlineColor(sf::Color(67, 72, 80));
        panel.setOutlineThickness(1.f);
        window.draw(panel);

        drawButton("NEW GAME", 88.f, ACCENT);
        drawButton("UNDO", 148.f, PANEL_2);

        sf::Text stateText(font, status, 20);
        stateText.setPosition({PANEL_X + 18.f, 222.f});
        stateText.setFillColor(status.find("Check") != std::string::npos ||
                               status.find("Checkmate") != std::string::npos ? RED : TEXT);
        window.draw(stateText);

        sf::Text depthTitle(font, "ENGINE DEPTH", 13);
        depthTitle.setPosition({PANEL_X + 18.f, 265.f});
        depthTitle.setFillColor(MUTED);
        window.draw(depthTitle);

        drawDepthButton("4", PANEL_X + 18.f, 288.f, engineDepth == 4);
        drawDepthButton("6", PANEL_X + 92.f, 288.f, engineDepth == 6);
        drawDepthButton("8", PANEL_X + 166.f, 288.f, engineDepth == 8);

        drawInfo();
        drawHistory();
        drawFooter();
    }

    void drawButton(const std::string& label, float y, sf::Color fill) {
        sf::RectangleShape button({PANEL_W - 36.f, 48.f});
        button.setPosition({PANEL_X + 18.f, y});
        button.setFillColor(fill);
        button.setOutlineColor(sf::Color(255, 255, 255, 25));
        button.setOutlineThickness(1.f);
        window.draw(button);

        sf::Text t(font, label, 16);
        sf::FloatRect b = t.getLocalBounds();
        t.setPosition({PANEL_X + 18.f + ((PANEL_W - 36.f) - b.size.x) / 2.f - b.position.x,
                       y + (48.f - b.size.y) / 2.f - b.position.y - 2.f});
        t.setFillColor(TEXT);
        window.draw(t);
    }

    void drawDepthButton(const std::string& label, float x, float y, bool active) {
        sf::RectangleShape b({58.f, 34.f});
        b.setPosition({x, y});
        b.setFillColor(active ? ACCENT : PANEL_2);
        window.draw(b);
        sf::Text t(font, label, 15);
        t.setPosition({x + 23.f, y + 6.f});
        t.setFillColor(TEXT);
        window.draw(t);
    }

    void drawInfo() {
        sf::Text heading(font, "SEARCH INFO", 13);
        heading.setPosition({PANEL_X + 18.f, 344.f});
        heading.setFillColor(MUTED);
        window.draw(heading);

        std::vector<std::pair<std::string, std::string>> rows = {
            {"Evaluation", formatScore(lastEval)},
            {"Depth", std::to_string(lastInfo.depth)},
            {"Nodes", std::to_string(lastInfo.nodes)},
            {"TT hits", std::to_string(lastInfo.ttHits)},
            {"Time", std::to_string(lastInfo.seconds).substr(0, 5) + " s"}
        };

        float y = 370.f;
        for (const auto& [label, value] : rows) {
            sf::Text l(font, label, 14);
            l.setPosition({PANEL_X + 18.f, y});
            l.setFillColor(MUTED);
            window.draw(l);

            sf::Text v(font, value, 14);
            v.setPosition({PANEL_X + PANEL_W - 110.f, y});
            v.setFillColor(TEXT);
            window.draw(v);
            y += 27.f;
        }
    }

    void drawHistory() {
        sf::Text heading(font, "MOVE HISTORY", 13);
        heading.setPosition({PANEL_X + 18.f, 520.f});
        heading.setFillColor(MUTED);
        window.draw(heading);

        float y = 548.f;
        int shown = std::min<int>(history.size(), 7);
        int start = static_cast<int>(history.size()) - shown;
        for (int i = start; i < static_cast<int>(history.size()); ++i) {
            int ply = i + 1;
            std::string text = std::to_string((ply + 1) / 2) + (ply & 1 ? ".  " : "... ") + history[i];
            sf::Text t(font, text, 14);
            t.setPosition({PANEL_X + 18.f, y});
            t.setFillColor(TEXT);
            window.draw(t);
            y += 25.f;
        }
    }

    void drawFooter() {
        sf::Text msg(font, message, 13);
        msg.setPosition({PANEL_X + 18.f, 712.f});
        msg.setFillColor(GREEN);
        window.draw(msg);

        sf::Text controls(font, "N: new game    U: undo    Esc: deselect", 12);
        controls.setPosition({34.f, 758.f});
        controls.setFillColor(MUTED);
        window.draw(controls);
    }
};

} // namespace

int main() {
    try {
        ChessGUI gui;
        gui.run();
    } catch (const std::exception& e) {
        return 1;
    }
    return 0;
}
