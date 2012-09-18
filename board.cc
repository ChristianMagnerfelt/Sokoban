#include "board.h"

#include <iostream>
#include <algorithm>

void Board::addRowToBoard(std::string && row)
{
	board.emplace_back(row);
	positionIDs.emplace_back(std::vector<int>(row.length());
}
void Board::printBoard() const
{
	std::for_each(boardContainer.begin(),boardContainer.end(),[](const std::string & row)
	{
		std::cout << row << std::endl;	
	});
}
void Board::constructIDMatrix()
{
	size_t count = 0;
	std::for_each(positionIDs.begin(), positionIDs.end(), [&count](std::vector<int> & vec){
		std::generate(vec.begin(), vec.end(), [&count]{ return count++; });
	});
}
