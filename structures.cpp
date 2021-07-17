#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };


FormulaError::FormulaError(FormulaError::Category category) 
    : category_(category) 
{}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case FormulaError::Category::Ref: 
        return "#REF!"; break;
    case FormulaError::Category::Value: 
        return "#VALUE!"; break;
    case FormulaError::Category::Div0: 
        return "#DIV0!"; break;
    case FormulaError::Category::Unknown:
        return "#Unknown"; break;
    }
    throw std::runtime_error("Unknown Error");
}




bool Position::operator==(const Position rhs) const {
    return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
	return  row < rhs.row || col < rhs.col;
}

bool Position::IsValid() const {
    if ((row >= 0 && row < MAX_ROWS)
        && (col >= 0 && col < MAX_COLS)) {
        return true;
    }
    return false;
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return {};
    }
    std::string cols = [this]() {
        auto tmp = this->col + 1;
        std::string res;
        while (tmp) {
            auto ch = (--tmp) % LETTERS;
            res.push_back(static_cast<char>(ch + 65));
            tmp = tmp / LETTERS;
        }
        std::reverse(res.begin(), res.end());
        return res;
    }();
    return cols + std::to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
    Position res{ -1,-1 };
    size_t sep{ 0 };
    for (; sep < str.size(); sep++) {
        if (std::isdigit(str[sep]))
            break;
        if (!std::isalpha(str[sep]) || !std::isupper(str[sep]))
            return res;
    }
    for (auto i = sep; i < str.size(); i++) {
        if (!std::isdigit(str[i]))
            return res;
    }

    res.col = [str, sep]() {
        auto cols = str.substr(0, sep);
        int res{ 0 };
        for (int weight = 1; cols.size(); weight *= LETTERS) {
            char c = cols[cols.size() - 1];
            res += (c - 64) * weight;
            cols.remove_suffix(1);
        }
        return res - 1;
    }();

    str.remove_prefix(sep);
    res.row = std::atoi(str.data()) - 1;

    return res;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}