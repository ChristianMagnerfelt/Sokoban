#pragma once 

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <unordered_set>

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
		Board(){}
		struct Position
		{
			Position() : x(0), y(0){}
			Position(unsigned int newX, unsigned int newY) : x(newX), y(newY){}
			bool operator==(const Position & other)
			{ 
				if(x == other.x && y == other.y) return true;
				else return false;
			}
			unsigned int x;
			unsigned int y;	
		};

		void addRowToBoard(std::string && row);
		void initialize();

		void printBoard() const;
		void printIDMatrix() const;
		void printBoxes() const;
		void printGoals() const;
		void printPlayerPosition() const;
		void printVisitedMatrix() const;

		Position getPlayerPosition();
		void flood();
		void checkPosition(Position && pos, const char c, std::deque<Position> & deque);
		bool hasBox(Position & pos);
		void findPossiblePushes();

		const std::string operator[](size_t pos){ return board[pos]; }

		size_t idAt(size_t row, size_t col){ return positionIDs[row][col]; }
	private:
		Position playerPosition;
		std::vector<Position> boxes;
		std::vector<std::string> visitedMatrix;
		std::multimap<Position, char> pushes; 
		static std::unordered_set<std::string> states;
		static std::vector<Position> goals;
		static std::vector<std::string> board;
		static std::vector<std::vector<unsigned int>> positionIDs;
};
#endif
