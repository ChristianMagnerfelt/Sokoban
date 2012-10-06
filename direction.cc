
#include "direction.h"

Direction::Direction(Maze::position const& delta): delta(delta) {}

Maze::position Direction::operator() (Maze::position const& pos) const {
    return pos + delta;
}

Direction Direction::backwards() const {
    return Direction(Maze::position(-delta.first, -delta.second));
}

Direction Direction::right() const {
    return Direction(Maze::position(-delta.second, delta.first));
}

Direction Direction::left() const {
    return Direction(Maze::position(delta.second, -delta.first));
}

