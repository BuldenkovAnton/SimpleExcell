#include "formula.h"

#include "FormulaAST.h"

#include <forward_list>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression)
            : ast_( ParseFormulaAST(std::move(expression) ) )
        {}

        Value Evaluate(const SheetInterface& sheet) const override {   
            try {
                auto getValue = [&sheet](Position pos) {
                    CellInterface::Value value = sheet.GetCell(pos)->GetValue();
                    if (std::holds_alternative<double>(value)) {
                        return std::get<double>(value);
                    }
                    else if (std::holds_alternative<FormulaError>(value)) {
                        throw std::get<FormulaError>(value);
                    }
                    return std::stod(sheet.GetCell(pos)->GetText());  
                };

                return ast_.Execute(getValue);
            }
            catch (FormulaError error) {
                return error;
            }
        }


        std::string GetExpression() const override {
            std::stringstream result;
            ast_.PrintFormula(result);
            return result.str();
        }

        std::vector<Position> GetReferencedCells() const {
            std::forward_list<Position> cells = ast_.GetCells();
            return { cells.begin(), cells.end() };
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}