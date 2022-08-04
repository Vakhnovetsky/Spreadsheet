#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosition(pos);
        
    if (cells_.count(pos) == 0) {
        cells_[pos] = std::make_unique<Cell>(*this);
    }

    blocked_cells_.erase(pos);
    cells_.at(pos)->Set(std::move(text));

    if (size_.cols <= pos.col) {
        size_.cols = pos.col + 1;
    }
    if (size_.rows <= pos.row) {
        size_.rows = pos.row + 1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosition(pos);

    if (cells_.count(pos) > 0 && blocked_cells_.count(pos) == 0) {
        return cells_.at(pos).get();
    }
    else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPosition(pos);

    if (cells_.count(pos) > 0 && blocked_cells_.count(pos) == 0) {
        return cells_.at(pos).get();
    }
    else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    CheckPosition(pos);

    if (cells_.count(pos) > 0) {
        cells_.at(pos)->Set("");
    }
    blocked_cells_.insert(pos);

    CountSize();
}

void Sheet::CountSize() {
    if (cells_.size() > 0) {
        bool is_empty = true;
        int temp_col = 0;
        int temp_row = 0;
        for (const auto& [p, c]: cells_) {
            if (blocked_cells_.count(p) == 0) {
                is_empty = false;
                if (temp_col < p.col) {
                    temp_col = p.col;
                }
                if (temp_row < p.row) {
                    temp_row = p.row;
                }
            }
                
        }
        if (is_empty) {
            size_ = { 0, 0 };
        }
        else {
            size_ = { temp_row + 1, temp_col + 1 };
        }
    }
    else {
        size_ = { 0, 0 };
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < size_.rows; ++row) {
        for (int col = 0; col < size_.cols; ++col) {
            auto it = cells_.find({ row, col });
            if (it != cells_.end() && blocked_cells_.find({ row, col }) == blocked_cells_.end()) {
                ::CellInterface::Value res = it->second->GetValue();
               
                if (std::holds_alternative<std::string>(res)) {
                    output << std::get<std::string>(res);
                }
                else if(std::holds_alternative<FormulaError>(res)) {
                    output << std::get<FormulaError>(res);
                }
                else {
                    output << std::get<double>(res);
                }
            }
            if (col < size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < size_.rows; ++row) {
        for (int col = 0; col < size_.cols; ++col) {
            auto it = cells_.find({ row, col });
            if (it != cells_.end() && blocked_cells_.find({ row, col }) == blocked_cells_.end()) {
                output << it->second->GetText();
            }
            if (col < size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::CheckPosition(const Position& pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Cell position is not valid");
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}