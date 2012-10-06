
#ifndef DD2380_DIRECTION_H
#define DD2380_DIRECTION_H

#include "maze.h"

class Direction {
    Maze::position delta;
public:
    Direction(Maze::position const& delta);
    Maze::position operator() (Maze::position const& pos) const;
    Direction backwards() const;
    Direction right() const;
    Direction left() const;
};

static const Direction up(Maze::position(0, -1));
static const Direction right(Maze::position(1, 0));
static const Direction down(Maze::position(0, 1));
static const Direction left(Maze::position(-1, 0));

#endif
