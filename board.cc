#include "board.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <unordered_set>

// Static member initialization
std::unordered_set<std::string> Board::states = std::unordered_set<std::string>();
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
	std::cout << "Player position is : (" << playerPosition.x << "," << playerPosition.y << ")" << std::endl;
}

void Board::printVisitedMatrix() const
{
	std::for_each(visitedMatrix.begin(), visitedMatrix.end(), [](const std::string & row)
	{
		std::cout << row << std::endl;
	});
}

void Board::initialize()
{
	size_t count = 0;
	std::for_each(Board::positionIDs.begin(), Board::positionIDs.end(), [&count](std::vector<unsigned int> & vec){
		std::generate(vec.begin(), vec.end(), [&count]{ return count++; });
	});

	visitedMatrix = board;
}

Board::Position Board::getPlayerPosition()
{
	return playerPosition;
}

void Board::flood()
{
	std::deque<Board::Position> deque;
	deque.emplace_back(getPlayerPosition());
	while(!deque.empty())
	{
		Position pos = deque.front();
		deque.pop_front();
		checkPosition(Position(pos.x - 1, pos.y), 'L', deque);
		checkPosition(Position(pos.x + 1, pos.y), 'R', deque);
		checkPosition(Position(pos.x, pos.y - 1), 'D', deque);
		checkPosition(Position(pos.x, pos.y + 1), 'U', deque);
	}
}

void Board::checkPosition(Position && pos, const char c, std::deque<Board::Position> & deque)
{
	if(visitedMatrix[pos.y][pos.x] == ' ' && !hasBox(pos))
	{
		visitedMatrix[pos.y][pos.x] = c;
		deque.emplace_back(pos);
	}
}

bool Board::hasBox(Position & pos)
{
	auto it = std::find(boxes.begin(), boxes.end(), pos);
	if(it != boxes.end())
		return true;
	else
		return false;
}
void Board::findPossiblePushes()
{
/*
	std::for_each(boxes.begin(), boxes.end(), [](Position & box){
		if(visitedMatrix[pos.y][pos.x - 1] != '#' &&  visitedMatrix[pos.y][pos.x + 1] != '#')
		{
			Position(pos.x + 1, pos.y) right;
			Position(pos.x - 1, pos.y) left;

			// Check if it's possible to push right
			if(visitedMatrix[pos.y][pos.x - 1] != ' ' && 
			std::find(boxes.begin(), boxes.end(), right) == boxes.end())
			{
				pushes.insert(std::pair<Board::Position, char>(left, 'R');
			}

			// Check if it's possible to push left
			if(visitedMatrix[pos.y][pos.x + 1] != ' ' &&
			std::find(boxes.begin(), boxes.end(), left) == boxes.end())
			{
				pushes.insert(std::pair<Board::Position, char>(right, 'L');			
			}
		}
		if(visitedMatrix[pos.y - 1][pos.x] != '#' &&  visitedMatrix[pos.y + 1][pos.x] != '#')
		{
			Position(pos.x, pos.y + 1) right;
			Position(pos.x, pos.y - 1) left;

			// Check if it's possible to push right
			if(visitedMatrix[pos.y - 1][pos.x] != ' ' && 
			std::find(boxes.begin(), boxes.end(), right) == boxes.end())
			{
				pushes.insert(std::pair<Board::Position, char>(left, 'R');
			}

			// Check if it's possible to push left
			if(visitedMatrix[pos.y + 1][pos.x] != ' ' &&
			std::find(boxes.begin(), boxes.end(), left) == boxes.end())
			{
				pushes.insert(std::pair<Board::Position, char>(right, 'L');			
			}
		}
	});*/
}
