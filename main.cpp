#include "common.h"
#include "test_runner_p.h"
#include "formula.h"
#include "sheet.h"

#include <string_view>
#include <string>
#include <iostream>

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit([&](const auto& x) {output << x;}, value);
    return output;
}

void TestValue() {
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "simple_text");
        ASSERT_EQUAL(sheet->GetCell({0,0})->GetText(), "simple_text");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "simple_text");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "'");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "'");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "'apostroph");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "'apostroph");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "apostroph");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "'=1+2");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "'=1+2");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "=1+2");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "=");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "=");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "=");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "1+2");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "1+2");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "1+2");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell(Position{ 0, 0 }, "1+2");
        ASSERT_EQUAL(sheet->GetCell({ 0,0 })->GetText(), "1+2");
        ASSERT_EQUAL(std::get<std::string>(sheet->GetCell({ 0,0 })->GetValue()), "1+2");
    }
  
}


void TestErrorPosition() {
    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell(Position{ -1, -1 }, "");
        }
        catch (const InvalidPositionException&) {}
    }
    {
        auto sheet = CreateSheet();
        try {
            sheet->ClearCell(Position{ Position::MAX_ROWS, 3 });
        }
        catch (const InvalidPositionException&) {}
    }
   
}



void TestFormulaException() {
    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "1");
            sheet->SetCell("B21"_pos, "=A1++");
            ASSERT(false);
        } catch (FormulaException& e) {}
    }
    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=A1");
            ASSERT(false);
        } catch (CircularDependencyException& e) {}
    }
    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=A1+1");
            sheet->SetCell("B3"_pos, "=B3+10");
            ASSERT(false);
        } catch (CircularDependencyException& e) {}
    }
}

void TestSheetSize() {
    {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("C5"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=C5+2");

        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 5, 3 }));
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B2"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=1/0");
        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "#DIV/0!\t\nmeow\t3\n");
    }
    {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("C5"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=C5+2");
        sheet->ClearCell("C5"_pos);
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 1 }));
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestErrorPosition);
    RUN_TEST(tr, TestValue);
    RUN_TEST(tr, TestFormulaException);
    RUN_TEST(tr, TestSheetSize);

    return 0;
}