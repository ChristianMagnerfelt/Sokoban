#include "tile.h"

#include <limits>

Tile::Tile():
    type(Floor),
    source_displacement(std::numeric_limits<std::size_t>::max()),
    target_displacement(std::numeric_limits<std::size_t>::max()),
    walkable(true),
    mark(0)
{}

void Tile::setType(Type value) {
	walkable = (value == Obstacle)?false:true;
	type = value;
}
