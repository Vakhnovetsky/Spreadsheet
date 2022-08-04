#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

FormulaError::FormulaError(Category category): category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_==rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref: 
            return "#REF!";
        case Category::Value: 
            return "#VALUE!";
        case Category::Div0: 
            return "#DIV/0!";
    }
    return {};
}

namespace {
    Formula::Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {
    }
    catch (std::exception& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }

    Formula::Value Formula::Evaluate(const SheetInterface& sheet) const {
        try {
            return ast_.Execute(sheet);
        }
        catch (const FormulaError& fe) {
            return fe;
        }
    }

    std::string Formula::GetExpression() const {
        std::stringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> Formula::GetReferencedCells() const {
        std::vector<Position> cells;
        for (Position pos: ast_.GetCells()) {
            if (pos.IsValid()) {
                cells.emplace_back(std::move(pos));
            }
        }

        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
        return cells;
    }
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}