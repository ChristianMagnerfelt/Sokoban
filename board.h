#pragma once 

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
/*!
 *	\details	At the beginning of the game, rows are added to the board in order to create a complete sokoban board.
 *			When the board is complete the ID matrix have to be calculated in order to support hashing of already visisted states.
 *			The hash is produced by concatenating each ID value for every brick on the board. Because every position has an unique ID, two
 *			different position can never have the same hash number;
 *	\author		Christian Magnerfelt
 *	\date		2012.09.18
 */
class Board
{
	public:
		struct Position
		{
			size_t x;
			size_t y;	
		}
		typedef std::vector<std::string> BoardContainer;
		typedef std::vector<std::vector<size_t>> IDMatrix;

		void addRowToBoard(std::string &&);
		void constructIDMatrix();
		void printBoard() const;

		const std::string operator[](size_t pos){ return board[pos]; }

		size_t idAt(size_t row, size_t col){ return positionIDs[row][col]; }
	private:
		BoardContainer board;
		static const IDMatrix positionIDs;
};

#endif
