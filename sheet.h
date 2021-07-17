#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

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


    void IsPositionValid(Position& pos) const;

    bool CellHasCurcularDependency(Cell* cell, Position pos);
    void DeleteDependances(Position pos);
    void CreateDependances(Position pos);
    void InvalidateCacheStartingWith(Position pos);

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    Size print_size_;

    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> cell_dependants_;

};