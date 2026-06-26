#pragma once
#include "Constants.hpp"
#include <vector>

using Cells = std::vector<std::pair<int,int>>;

// One player's 10×10 grid — all logic in this header, no .cpp needed
class Board {
    Cell g[GRID][GRID]{};   // default-init to Cell::Water (0)
public:

    Cell get(int r, int c) const { return g[r][c]; }

    // All cells a ship of `len` would cover starting at (r,c)
    Cells cells(int r, int c, int len, bool horiz) const {
        Cells out;
        for (int i = 0; i < len; i++)
            out.push_back({r + (horiz ? 0 : i), c + (horiz ? i : 0)});
        return out;
    }

    // Can the ship fit here? (in bounds + all Water)
    bool canPlace(const Cells& cs) const {
        for (auto [r,c] : cs)
            if (r<0||r>=GRID||c<0||c>=GRID||g[r][c]!=Cell::Water) return false;
        return true;
    }

    void place(const Cells& cs)  { for (auto [r,c] : cs) g[r][c] = Cell::Ship; }

    bool hasShips() const {
        for (auto& row : g) for (auto v : row) if (v==Cell::Ship) return true;
        return false;
    }

    // Fire at (r,c) — returns true on a hit
    bool fire(int r, int c) {
        bool hit = g[r][c] == Cell::Ship;
        g[r][c]  = hit ? Cell::Hit : Cell::Miss;
        return hit;
    }

    void reset() { for (auto& row : g) for (auto& v : row) v = Cell::Water; }
};
