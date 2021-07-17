#include "sheet.h"

#include "common.h"


#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>


using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    IsPositionValid(pos);

    auto cell = std::make_unique<Cell>(*this);
    cell->Set(text);


    if (CellHasCurcularDependency(cell.get(), pos)) {
        throw CircularDependencyException("circular dependenses");
    }

    cells_.resize(std::max(static_cast<size_t>(pos.row) + 1, cells_.size()));
    cells_[pos.row].resize(std::max(static_cast<size_t>(pos.col) + 1, cells_.at(pos.row).size()));

    cells_[pos.row][pos.col] = std::move(cell);

    DeleteDependances(pos);
    CreateDependances(pos);
    InvalidateCacheStartingWith(pos); 

    print_size_ = GetPrintableSize();
}


bool Sheet::CellHasCurcularDependency(Cell* cell, Position pos) {
    std::vector<Position> incoming = cell->GetReferencedCells();
    if (incoming.empty()) return false;

    auto changed_cell = GetCell(pos);

    std::stack<Position> graph_dependeces;
    std::vector<Position> processed;

    for (auto posisition : incoming) {
        graph_dependeces.push(posisition);
    }

    while (!graph_dependeces.empty()) {
        Position current = graph_dependeces.top();
        graph_dependeces.pop();
        processed.push_back(current);

        auto* cell_pos = GetCell(current);
        if (cell_pos == changed_cell) {
            return true;
        }

        if (cell_pos) {
            for (const auto& posisition : cell_pos->GetReferencedCells()) {
                if (std::count(incoming.begin(), incoming.end(), posisition) == 0) {
                    graph_dependeces.push(posisition);
                }
            }
        }
    }
    return false;
}

void Sheet::DeleteDependances(Position pos) {
    std::vector<Position> outcoming =  dynamic_cast<Cell*>(GetCell(pos))->GetReferencedCells();
    if (!outcoming.empty()) {
        for (auto position : outcoming) {
            auto it = cell_dependants_[position].find(pos);
            if (it != cell_dependants_[position].end()) {
                cell_dependants_[position].erase(it);
            }  
        }  
    }
}

void Sheet::CreateDependances(Position pos) {
    std::vector<Position> outcoming = dynamic_cast<Cell*>(GetCell(pos))->GetReferencedCells();
    if (!outcoming.empty()) {
        for (auto position : outcoming) {
            cell_dependants_[position].insert(pos);
        }
    }
}

void  Sheet::InvalidateCacheStartingWith(Position pos) {
    dynamic_cast<Cell*>(GetCell(pos))->InvalidateCache();

    if (!cell_dependants_[pos].empty()) {
        for (auto cell : cell_dependants_.at(pos)) {
            InvalidateCacheStartingWith(cell);
        }
    }
   
}



const CellInterface* Sheet::GetCell(Position pos) const {
    IsPositionValid(pos);

    if ((static_cast<size_t>(pos.row) >= cells_.size()) || (static_cast<size_t>(pos.col) >= cells_[pos.row].size())) {
        return nullptr;
    }
    return cells_.at(pos.row).at(pos.col).get();
 
}

CellInterface* Sheet::GetCell(Position pos) {
    IsPositionValid(pos);

    if ((static_cast<size_t>(pos.row) >= cells_.size()) ||  (static_cast<size_t>(pos.col) >= cells_[pos.row].size())) {
        return nullptr;
    }
    return cells_.at(pos.row).at(pos.col).get();
    
}

void Sheet::ClearCell(Position pos) {
    IsPositionValid(pos);

    if ((static_cast<size_t>(pos.row) >= cells_.size()) ||  (static_cast<size_t>(pos.col) >= cells_[pos.row].size())) {
        return ;
    }
    
    auto& cell = cells_.at(pos.row).at(pos.col);
    if (!cell || cell->GetText().empty()) {
        return;
    }
    cell.reset();

    print_size_ = GetPrintableSize();
}

Size Sheet::GetPrintableSize() const {
    Size size;
    for (int row = 0; static_cast<size_t>(row) < cells_.size(); ++row) {
        for (int col = cells_[row].size() - 1; col >= 0; --col) {
            if (cells_[row][col] && !cells_[row][col]->GetText().empty()) {
                size.rows = (size.rows > row + 1) ? size.rows : row + 1;
                size.cols = (size.cols > col + 1) ? size.cols : col + 1;
                break;
            }
        }
    }
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < print_size_.rows; ++row) {
        for (int col = 0; col < print_size_.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if ((size_t)col < cells_[row].size()) {
                if (const auto& cell = cells_.at(row).at(col)) {
                    std::visit([&](const auto& value) { output << value; }, cell->GetValue());
                }
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < print_size_.rows; ++row) {
        for (int col = 0; col < print_size_.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (static_cast<size_t>(col) < cells_[row].size()) {
                if (const auto& cell = cells_.at(row).at(col)) {
                    output << cell->GetText();
                }
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}


void Sheet::IsPositionValid(Position& pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is not valid"s);
    }
}
