#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

using namespace std::literals;

//****Cell********************************************************
Cell::Cell(Sheet& sheet)
	: impl_(std::make_unique<EmptyImpl>())
	, sheet_(sheet)
{}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	if (text == impl_->GetText()) {
		return;
	}
	if (text.size() > 1 && text.front() == FORMULA_SIGN) {
		std::unique_ptr<Impl> impl;
		impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);

		FindCyclicDependencies(impl->GetReferencedCells(), this);
		impl_ = std::move(impl);
	}
	else if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
	}

	//Очищаем зависимости
	for (auto cell : dependent_cells_) {
		cell->dependent_cells_for_cache.erase(this);
	}
	dependent_cells_.clear();

	for (auto& cell : impl_->GetReferencedCells()) {
		auto c = sheet_.GetCell(cell);
		if (c == nullptr) {
			sheet_.SetCell(cell, "");
			c = sheet_.GetCell(cell);
		}

		dependent_cells_.insert(dynamic_cast<Cell*>(c));
		dynamic_cast<Cell*>(c)->SetDependentCell(this);
	}
	InvalidateCache();
}

void Cell::SetDependentCell(Cell* cell) {
	dependent_cells_for_cache.insert(dynamic_cast<Cell*>(cell));
}

void Cell::Clear() {
	Set("");
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

void Cell::UpdateCache() {
	return impl_->UpdateCache();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

void Cell::FindCyclicDependencies(std::vector<Position> cells, Cell* this_cell) {
	for (auto cell : cells) {
		CellInterface* c = sheet_.GetCell(cell);
		if (c == this_cell) {
			throw CircularDependencyException("");
		}
		if (c != nullptr) {
			FindCyclicDependencies(sheet_.GetCell(cell)->GetReferencedCells(), this_cell);
		}
	}
}

void Cell::InvalidateCache() {
	for (auto& c : dependent_cells_for_cache) {
		dynamic_cast<Cell*>(c)->UpdateCache();
		c->InvalidateCache();
	}
}

std::unordered_set<Cell*> Cell::GetDependentCell() {
	return dependent_cells_for_cache;
}

//********************************************************
std::vector<Position> Cell::Impl::GetReferencedCells() const {
	return {};
}

void Cell::Impl::UpdateCache() {
}

//****EmptyImpl********************************************************

CellInterface::Value Cell::EmptyImpl::GetValue() const {
	return {};
}

std::string Cell::EmptyImpl::GetText() const {
	return {};
}

//****TextImpl********************************************************

Cell::TextImpl::TextImpl(std::string text) : text_(text) {
}

CellInterface::Value Cell::TextImpl::GetValue() const {
	if (text_.size() > 0 && text_.front() == ESCAPE_SIGN) {
		return text_.substr(1);
	}
	else {
		return text_;
	}
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

//****FormulaImpl********************************************************

Cell::FormulaImpl::FormulaImpl(std::string formula, const SheetInterface& sheet)
	: formula_(ParseFormula(formula.substr(1)))
	, sheet_(sheet)
{
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
	if (!cache_) {
		cache_ = formula_->Evaluate(sheet_);
	}
	
	if (std::holds_alternative<double>(cache_.value())) {
		return std::get<double>(cache_.value());
	}
	else {
		return std::get<FormulaError>(cache_.value());
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::UpdateCache() {
	cache_.reset();
}