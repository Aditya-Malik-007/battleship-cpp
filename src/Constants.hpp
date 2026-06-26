#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

// Grid geometry
constexpr int   GRID  = 10;
constexpr float CELL  = 44.f;
constexpr float GAP   = 3.f;
constexpr float BOARD = GRID * CELL + (GRID - 1) * GAP;   // total px

// Cell state
enum class Cell { Water, Ship, Hit, Miss };

// Ship definitions
struct Ship { std::string name; int len; };
constexpr std::array<Ship, 5> SHIPS = {{
    {"Carrier",5}, {"Battleship",4}, {"Cruiser",3}, {"Submarine",3}, {"Destroyer",2}
}};

// Colors – short namespace so they read nicely
namespace C {
    inline const sf::Color Water  {13, 26, 46};
    inline const sf::Color Ship   {30, 96, 64};
    inline const sf::Color Hit    {128,32, 32};
    inline const sf::Color Miss   {26, 32, 48};
    inline const sf::Color Grid   {26, 42, 64};
    inline const sf::Color HoverOk {180,160,80, 115};
    inline const sf::Color HoverBad{160, 40,40, 115};
    inline const sf::Color HitX   {192, 96, 96};
    inline const sf::Color MissDot{48,  64, 80};
    inline const sf::Color Bg     {8,   14, 28};
    inline const sf::Color Text   {200,220,255};
    inline const sf::Color TextDim{100,130,180};
    inline const sf::Color Gold   {220,185, 80};
    inline const sf::Color BtnNorm{20,  50, 90};
    inline const sf::Color BtnHov {30,  80,140};
    inline const sf::Color BtnAct {40, 110,190};
    inline const sf::Color DoneGrn{30,  90, 60};
}
