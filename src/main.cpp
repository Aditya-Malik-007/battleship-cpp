#include "Board.hpp"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <cmath>

// ── Window size & board positions on screen ─────────────────────────────────
constexpr unsigned WW = 1120, WH = 660;
constexpr float SX = 280, SY = 90;     // setup board origin
constexpr float MX = 60,  MY = 110;    // "my fleet" board (battle)
constexpr float EX = 590, EY = 110;    // "enemy" board (battle)

// ── All game state in one place ─────────────────────────────────────────────
enum class Screen { Home, Setup, Handoff, Battle, Win };

struct State {
    Screen screen      = Screen::Home;
    Board  boards[2];
    int    setupPlayer = 0, shipIdx = 0;
    bool   horiz       = true;
    int    battlePlayer= 0;
    std::string statusMsg, winTitle, setupMsg;
    std::optional<std::pair<int,int>> hover;
};

// ── Global font (simpler than passing it everywhere) ────────────────────────
sf::Font font;

// ── Pixel coords → grid cell (nullopt if in a gap or out of bounds) ─────────
std::optional<std::pair<int,int>> toCell(float px, float py) {
    float step = CELL + GAP;
    if (std::fmod(px,step) >= CELL || std::fmod(py,step) >= CELL) return {};
    int r = (int)(py/step), c = (int)(px/step);
    if (r<0||r>=GRID||c<0||c>=GRID) return {};
    return {{r, c}};
}

// ═══════════════════════════════════════════════════════════════════════════
//  DRAW HELPERS
// ═══════════════════════════════════════════════════════════════════════════

void drawText(sf::RenderWindow& w, const std::string& s,
              float x, float y, unsigned sz, sf::Color col, bool center=false) {
    sf::Text t(font, s, sz);
    t.setFillColor(col);
    if (center) x -= t.getLocalBounds().size.x / 2.f;
    t.setPosition({x, y});
    w.draw(t);
}

// Returns true if the mouse is hovering over the button
bool drawButton(sf::RenderWindow& w, const std::string& label,
                float x, float y, float bw, float bh,
                sf::Vector2i mouse, bool active=false) {
    bool hov = sf::FloatRect({x,y},{bw,bh}).contains(sf::Vector2f(mouse));
    sf::RectangleShape box({bw,bh});
    box.setPosition({x,y});
    box.setFillColor(active ? C::BtnAct : (hov ? C::BtnHov : C::BtnNorm));
    box.setOutlineThickness(1.5f);
    box.setOutlineColor(hov || active ? C::Gold : C::Grid);
    w.draw(box);
    sf::Text t(font, label, 18);
    auto b = t.getLocalBounds();
    t.setPosition({x + (bw-b.size.x)/2.f, y + (bh-b.size.y)/2.f - 2.f});
    t.setFillColor(C::Text);
    w.draw(t);
    return hov;
}

// Draw a full 10×10 board. hide=true makes ships invisible (enemy board).
// highlight cells get a coloured overlay (green if ok, red if not).
void drawBoard(sf::RenderWindow& w, const Board& b,
               float ox, float oy, bool hide,
               const Cells& highlight={}, bool hlOk=true) {

    for (int r = 0; r < GRID; r++) for (int c = 0; c < GRID; c++) {
        float x = ox + c*(CELL+GAP), y = oy + r*(CELL+GAP);
        Cell v = b.get(r,c);
        if (v == Cell::Ship && hide) v = Cell::Water;

        // Base cell colour
        sf::Color base;
        switch (v) {
            case Cell::Water: base = C::Water; break;
            case Cell::Ship:  base = C::Ship;  break;
            case Cell::Hit:   base = C::Hit;   break;
            case Cell::Miss:  base = C::Miss;  break;
        }

        // Blend hover overlay if this cell is highlighted
        bool hl = false;
        for (auto [hr,hc] : highlight) if (hr==r && hc==c) { hl=true; break; }
        if (hl) {
            auto ov = hlOk ? C::HoverOk : C::HoverBad;
            base.r = (uint8_t)std::min(255, base.r + ov.r*ov.a/255);
            base.g = (uint8_t)std::min(255, base.g + ov.g*ov.a/255);
            base.b = (uint8_t)std::min(255, base.b + ov.b*ov.a/255);
        }

        sf::RectangleShape cell({CELL,CELL});
        cell.setPosition({x,y});
        cell.setFillColor(base);
        cell.setOutlineThickness(1.f);
        cell.setOutlineColor(C::Grid);
        w.draw(cell);

        // Hit = X cross
        if (v == Cell::Hit) {
            float p = 9.f;
            sf::Vertex xs[4] = {
                {{x+p,        y+p       }, C::HitX},
                {{x+CELL-p,   y+CELL-p  }, C::HitX},
                {{x+CELL-p,   y+p       }, C::HitX},
                {{x+p,        y+CELL-p  }, C::HitX}
            };
            w.draw(xs, 2, sf::PrimitiveType::Lines);
            w.draw(xs+2, 2, sf::PrimitiveType::Lines);
        }
        // Miss = small dot
        if (v == Cell::Miss) {
            sf::CircleShape dot(4.f);
            dot.setFillColor(C::MissDot);
            dot.setOrigin({4.f,4.f});
            dot.setPosition({x+CELL/2, y+CELL/2});
            w.draw(dot);
        }
    }
}

void drawShipList(sf::RenderWindow& w, int cur) {
    for (int i = 0; i < (int)SHIPS.size(); i++) {
        std::string s = (i < cur ? "✓ " : i == cur ? "▶ " : "  ")
                        + SHIPS[i].name + " [" + std::to_string(SHIPS[i].len) + "]";
        sf::Color col = i < cur ? C::DoneGrn : i == cur ? C::Gold : C::TextDim;
        drawText(w, s, 60, 110 + i*30, 17, col);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  RENDER SCREENS
// ═══════════════════════════════════════════════════════════════════════════

void renderHome(sf::RenderWindow& w, sf::Vector2i m) {
    drawText  (w, "BATTLESHIP",          WW/2.f, 160, 62, C::Gold, true);
    drawText  (w, "2-Player Naval Combat",WW/2.f, 240, 22, C::TextDim, true);
    drawButton(w, "START GAME", WW/2.f-110, 310, 220, 52, m);
    drawText  (w, "H / V keys toggle ship direction during setup",
               WW/2.f, 545, 14, C::TextDim, true);
}

void renderSetup(sf::RenderWindow& w, State& s, sf::Vector2i m) {
    std::string title = "PLAYER " + std::to_string(s.setupPlayer+1)
                      + "  —  PLACE SHIPS  (" + std::to_string(s.shipIdx+1)
                      + " / " + std::to_string(SHIPS.size()) + ")";
    drawText  (w, title, WW/2.f, 14, 22, C::Gold, true);
    drawButton(w, "HORIZONTAL", SX,     50, 160, 38, m,  s.horiz);
    drawButton(w, "VERTICAL",   SX+170, 50, 160, 38, m, !s.horiz);
    drawShipList(w, s.shipIdx);

    // Preview cells under the cursor
    Cells hl; bool ok = true;
    if (s.hover) {
        auto [r,c] = *s.hover;
        hl = s.boards[s.setupPlayer].cells(r, c, SHIPS[s.shipIdx].len, s.horiz);
        ok = s.boards[s.setupPlayer].canPlace(hl);
    }
    drawBoard(w, s.boards[s.setupPlayer], SX, SY, false, hl, ok);
    drawText(w, s.setupMsg, SX, SY+BOARD+12, 16, sf::Color(200,80,80));
}

void renderHandoff(sf::RenderWindow& w, sf::Vector2i m) {
    drawText  (w, "PLAYER 1 READY",           WW/2.f, 160, 42, C::Gold, true);
    drawText  (w, "Pass the device to Player 2",WW/2.f, 230, 22, C::TextDim, true);
    drawButton(w, "CONTINUE", WW/2.f-110, 310, 220, 52, m);
}

void renderBattle(sf::RenderWindow& w, State& s, sf::Vector2i m) {
    drawText(w, "PLAYER "+std::to_string(s.battlePlayer+1)+"  —  FIRE",
             WW/2.f, 14, 26, C::Gold, true);
    sf::Color sc = (s.statusMsg=="HIT!") ? sf::Color(220,80,80) : C::TextDim;
    drawText(w, s.statusMsg, WW/2.f, 50, 20, sc, true);

    drawText (w, "YOUR FLEET",   MX+BOARD/2, 80, 16, C::TextDim, true);
    drawBoard(w, s.boards[s.battlePlayer], MX, MY, false);

    drawText (w, "ENEMY WATERS", EX+BOARD/2, 80, 16, C::TextDim, true);
    Cells hl; bool ok = true;
    if (s.hover) {
        auto [r,c] = *s.hover;
        Cell v = s.boards[1-s.battlePlayer].get(r,c);
        ok = (v != Cell::Hit && v != Cell::Miss);
        hl.push_back({r,c});
    }
    drawBoard(w, s.boards[1-s.battlePlayer], EX, EY, true, hl, ok);
}

void renderWin(sf::RenderWindow& w, State& s, sf::Vector2i m) {
    drawText  (w, s.winTitle,          WW/2.f, 160, 54, C::Gold, true);
    drawText  (w, "All enemy ships sunk!", WW/2.f, 250, 24, C::TextDim, true);
    drawButton(w, "PLAY AGAIN", WW/2.f-110, 360, 220, 52, m);
}

// ═══════════════════════════════════════════════════════════════════════════
//  EVENT HANDLER  (one function, dispatches by screen)
// ═══════════════════════════════════════════════════════════════════════════

void handleEvent(State& s, const sf::Event& ev) {
    auto mf = [&]() -> sf::Vector2f {       // helper: mouse as floats
        if (auto* mm = ev.getIf<sf::Event::MouseMoved>())
            return sf::Vector2f(mm->position);
        if (auto* mb = ev.getIf<sf::Event::MouseButtonReleased>())
            return sf::Vector2f(mb->position);
        return {};
    };
    bool clicked = ev.is<sf::Event::MouseButtonReleased>() &&
                   ev.getIf<sf::Event::MouseButtonReleased>()->button
                   == sf::Mouse::Button::Left;

    // ── Update hover on mouse move ──────────────────────────────────────────
    if (auto* mm = ev.getIf<sf::Event::MouseMoved>()) {
        float ox = (s.screen==Screen::Setup) ? SX : EX;
        float oy = (s.screen==Screen::Setup) ? SY : EY;
        float rx = mm->position.x - ox, ry = mm->position.y - oy;
        s.hover = (rx>=0 && ry>=0) ? toCell(rx,ry) : std::nullopt;
    }
    if (ev.is<sf::Event::MouseLeft>()) s.hover = {};

    // ── Per-screen logic ────────────────────────────────────────────────────
    if (s.screen == Screen::Home) {
        if (clicked && sf::FloatRect({WW/2.f-110,310},{220,52}).contains(mf())) {
            s = State{};                   // reset everything
            s.screen = Screen::Setup;
        }
    }
    else if (s.screen == Screen::Setup) {
        // H/V keyboard shortcuts
        if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::H) s.horiz = true;
            if (kp->code == sf::Keyboard::Key::V) s.horiz = false;
        }
        if (clicked) {
            // Direction buttons
            if (sf::FloatRect({SX,    50},{160,38}).contains(mf())) { s.horiz=true;  return; }
            if (sf::FloatRect({SX+170,50},{160,38}).contains(mf())) { s.horiz=false; return; }
            if (!s.hover) return;

            auto [r,c] = *s.hover;
            auto hl = s.boards[s.setupPlayer].cells(r, c, SHIPS[s.shipIdx].len, s.horiz);
            if (!s.boards[s.setupPlayer].canPlace(hl)) {
                s.setupMsg = "Invalid — try somewhere else."; return;
            }
            s.setupMsg = "";
            s.boards[s.setupPlayer].place(hl);

            if (++s.shipIdx == (int)SHIPS.size()) {       // all ships placed
                if (s.setupPlayer == 0) {
                    s.screen = Screen::Handoff;
                } else {
                    s.battlePlayer = 0;
                    s.screen = Screen::Battle;
                }
            }
        }
    }
    else if (s.screen == Screen::Handoff) {
        if (clicked && sf::FloatRect({WW/2.f-110,310},{220,52}).contains(mf())) {
            s.setupPlayer=1; s.shipIdx=0; s.horiz=true;
            s.setupMsg=""; s.hover={};
            s.screen = Screen::Setup;
        }
    }
    else if (s.screen == Screen::Battle) {
        if (clicked && s.hover) {
            auto [r,c] = *s.hover;
            Board& enemy = s.boards[1-s.battlePlayer];

            if (enemy.get(r,c)==Cell::Hit || enemy.get(r,c)==Cell::Miss) {
                s.statusMsg = "Already fired there."; return;
            }
            bool hit = enemy.fire(r,c);
            if (hit && !enemy.hasShips()) {
                s.winTitle = "PLAYER "+std::to_string(s.battlePlayer+1)+" WINS!";
                s.screen = Screen::Win;
                return;
            }
            s.statusMsg    = hit ? "HIT!" : "MISS";
            if (!hit) s.battlePlayer ^= 1;   // switch turns on a miss
        }
    }
    else if (s.screen == Screen::Win) {
        if (clicked && sf::FloatRect({WW/2.f-110,360},{220,52}).contains(mf())) {
            s = State{};   // go home
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
    sf::RenderWindow window(sf::VideoMode({WW,WH}), "Battleship",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // Load any available system font
    for (const char* p : {"C:/Windows/Fonts/consola.ttf",
                          "C:/Windows/Fonts/cour.ttf",
                          "C:/Windows/Fonts/arial.ttf"})
        if (font.openFromFile(p)) break;

    State state;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();
            handleEvent(state, *ev);
        }

        auto mouse = sf::Mouse::getPosition(window);
        window.clear(C::Bg);

        switch (state.screen) {
            case Screen::Home:    renderHome   (window, mouse);        break;
            case Screen::Setup:   renderSetup  (window, state, mouse); break;
            case Screen::Handoff: renderHandoff(window, mouse);        break;
            case Screen::Battle:  renderBattle (window, state, mouse); break;
            case Screen::Win:     renderWin    (window, state, mouse); break;
        }

        window.display();
    }
}
