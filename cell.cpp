#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>
#include <algorithm>
#include <vector>


class Cell::Impl {
public:
    using Value = std::variant<std::string, double, FormulaError>;

    explicit Impl(std::string text) 
        : value_(text) 
    {}

    virtual ~Impl() = default;

    virtual std::string GetText() const = 0;

    virtual Value GetValue(SheetInterface& sheet) const = 0;

    virtual std::vector<Position> GetReferencedCells() const = 0;

protected:
    std::string value_;
};

class Cell::EmptyImpl : public Impl {
public:
    explicit EmptyImpl()
        : Impl("")
    {}

    std::string GetText() const override {
        return value_;
    }

    Value GetValue(SheetInterface&) const override {
        return value_;
    }


    std::vector<Position> GetReferencedCells() const {
        return {};
    }

};

class Cell::TextImpl : public Impl {
public:
    explicit TextImpl(std::string& text)
        : Cell::Impl(std::move(text)) {}

    std::string GetText() const override {
        return value_;
    }


    Value GetValue(SheetInterface&) const override {
        if (value_[0] == ESCAPE_SIGN) {
            return value_.substr(1);
        }
        return value_;
    }

    std::vector<Position> GetReferencedCells() const {
        return {};
    }


};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(SheetInterface& sheet, std::string text)
        : Impl(text)
        , formula_(ParseFormula(text)) 
    {}

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    Value GetValue(SheetInterface& sheet) const override {
        const auto& value = formula_->Evaluate(sheet);
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }
        if (std::holds_alternative<FormulaError>(value)) {
            return std::get<FormulaError>(value);
        }
        return "";
    }

    std::vector<Position> GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }
private:
    std::unique_ptr<FormulaInterface> formula_;
};


Cell::Cell(Sheet& sheet) 
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>())
{}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
         impl_ = std::make_unique<EmptyImpl>();
    }
    else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        try {
            impl_ = std::make_unique<FormulaImpl>(sheet_, text.substr(1));
        }
        catch (...) {
            throw FormulaException("Formula sytnaxis error");
        }
      
    }
    else {    
        impl_ = std::make_unique<TextImpl>(text);
    }

}

void Cell::Clear() {
    Set("");
}

void Cell::InvalidateCache() {
    cached_value_.reset();
}

Cell::Value Cell::GetValue() const {
    if (!cached_value_) {
        cached_value_ = impl_->GetValue(sheet_);
    }

    return *cached_value_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}




