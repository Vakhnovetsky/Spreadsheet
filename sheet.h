#pragma once

#include "cell.h"
#include "common.h"

#include <map>
#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
private:
    std::map<Position, std::unique_ptr<Cell>> cells_;
    Size size_;
    std::set<Position> blocked_cells_;

    void CountSize();
    void CheckPosition(const Position& pos) const;
};