#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);

    void Clear();

    Value GetValue() const override;

    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCache();

private:
   
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;

    mutable std::optional<Value> cached_value_;
};