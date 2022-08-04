#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
	return this->col == rhs.col && this->row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
	return this->col < rhs.col || (this->col == rhs.col && this->row < rhs.row);
}

bool Position::IsValid() const {
	return row < MAX_ROWS && col < MAX_COLS && row >= 0 && col >= 0;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}

char GetSymbol(int num) {
	num += 64;
	if (num < 65) {
		num += 26;
	}
	if (num > 90) {
		num -= 26;
	}
	return num;
}

int GetNumber(char c) {
	return static_cast<int>(c - 64);
}

std::string Position::ToString() const {
	if (!IsValid()) {
		return "";
	}

	std::string res;

	int temp_col = col;

	int a = temp_col / (LETTERS * LETTERS);
	int aa = temp_col % (LETTERS * LETTERS);

	if (a > 0 && aa > 25) {
		res.push_back(GetSymbol(a));
		temp_col -= LETTERS * LETTERS * a;
	}

	int b = temp_col / LETTERS;
	if (b > 0) {
		res.push_back(GetSymbol(b));
		temp_col -= LETTERS * b;
	}
	res.push_back(GetSymbol(temp_col+1));

	res += std::to_string(row+1);

	return res;
}

Position Position::FromString(std::string_view str) {
	Position result;
	std::string temp_col, temp_row;
	int num_symbol = 0;
	bool symbol = true;

	for (char c : str) {
		if (std::isupper(c) || std::isdigit(c)) {
			if (symbol && std::isupper(c) && num_symbol < MAX_POS_LETTER_COUNT) {
				temp_col.push_back(c);
				++num_symbol;
			}
			else if(std::isdigit(c)) {
				temp_row.push_back(c);
				symbol = false;
			}
			else {
				return Position::NONE;
			}
		}
		else {
			return Position::NONE;
		}
	}

	if (temp_col.size() == 0 || temp_row.size() == 0 || temp_row.size() > 5) {
		return Position::NONE;
	}

	result.row = std::stoi(temp_row) - 1;

	if (temp_col.size() == 3) {
		result.col += LETTERS * LETTERS * GetNumber(temp_col.at(0));
	}

	if (temp_col.size() >= 2) {
		result.col += LETTERS * GetNumber(temp_col.at(temp_col.size()-2));
	}

	result.col += GetNumber(temp_col.at(temp_col.size() - 1));
	--result.col;

	return result;
}