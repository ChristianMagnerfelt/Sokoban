#include "board.h"

#include <iostream>
#include <algorithm>
#include <vector>

// Static member initialization
std::vector<std::vector<unsigned int>> Board::positionIDs = std::vector<std::vector<unsigned int>>();
std::vector<std::string> Board::board = std::vector<std::string>();
std::vector<Board::Position> Board::goals = std::vector<Board::Position>();

void Board::addRowToBoard(std::string && row)
{
	// Extract boxes and player position, then replace with empty space
	for(size_t i = 0; i < row.size(); ++i)
	{
		if(row[i] == '$')
		{
			boxes.emplace_back(Position(i, board.size()));
			row[i] = ' ';
		}
		else if(row[i] == '*')
		{
			boxes.emplace_back(Position(i, board.size()));
			goals.emplace_back(Position(i, board.size()));
			row[i] = ' ';
		}
		else if(row[i] == '@')
		{
			playerPosition = Position(i, board.size());
			row[i] = ' ';
		}
		else if(row[i] == '+')
		{
			playerPosition = Position(i, board.size());
			goals.emplace_back(Position(i, board.size()));
			row[i] = ' ';
		}
		else if(row[i] == '.')
		{
			goals.emplace_back(Position(i, board.size()));
			row[i] = ' ';
		}
	}

	// Add row to board
	board.emplace_back(row);

	// Add ID row to ID matrix
	Board::positionIDs.emplace_back(std::vector<unsigned int>(row.length()));
}
void Board::printBoard() const
{
	std::for_each(board.begin(),board.end(),[](const std::string & row)
	{
		std::cout << row << std::endl;	
	});
}
void Board::printIDMatrix() const 
{
	std::for_each(Board::positionIDs.begin(), Board::positionIDs.end(), [](const std::vector<unsigned int> & vec)
	{
		std::for_each(vec.begin(), vec.end(), [](const unsigned int value)
		{
			std::cout << value;
		});
		std::cout << std::endl;
	});


}
void Board::printBoxes() const
{
	std::cout << "Boxes : ";
	std::for_each(boxes.begin(), boxes.end(), [](const Position & pos)
	{
		std::cout << "(" << pos.x << "," << pos.y << ")";
	});
	std::cout << std::endl;
}
void Board::printGoals() const
{
	std::cout << "Goals : ";
	std::for_each(goals.begin(), goals.end(), [](const Position & pos)
	{
		std::cout << "(" << pos.x << "," << pos.y << ")";
	});
	std::cout << std::endl;
}
void Board::printPlayerPosition() const
{
	std::cout << "Player position : (" << playerPosition.x << "," << playerPosition.y << ")" << std::endl;
}

void Board::initialize()
{
	size_t count = 0;
	std::for_each(Board::positionIDs.begin(), Board::positionIDs.end(), [&count](std::vector<unsigned int> & vec){
		std::generate(vec.begin(), vec.end(), [&count]{ return count++; });
	});

	visitedMatrix = board;
}
