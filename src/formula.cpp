#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <iostream>


using namespace std::literals;

FormulaError::FormulaError(Category category)
    : category_(category)
{
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.GetCategory();
}

std::string_view FormulaError::ToString() const {
    std::string_view res;
    if(category_ == FormulaError::Category::Ref){
        res =  "#REF!"sv;
    }
    if(category_ == FormulaError::Category::Value){
        res =  "#VALUE!"sv;
    }
    if(category_ == FormulaError::Category::Div0){
        res =  "#DIV/0!"sv;
    }
    return res; 
}
    
std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression); 
        Value Evaluate(const SheetInterface& sheet) const override;
        std::string GetExpression() const override;
        std::vector<Position> GetReferencedCells() const override; 
    private:
        FormulaAST ast_;      
    };
        
    Formula::Formula(std::string expression) 
        : ast_(ParseFormulaAST(expression))
    { 
    }
        
    FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const {        
        auto GetValueFromSheet = [&sheet](const Position& pos) {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }
            CellInterface::Value result;
            if(sheet.GetCell(pos)){
                result = sheet.GetCell(pos)->GetValue();
            }
            else{
                result = 0.0;
            }
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
            }
            if (std::holds_alternative<std::string>(result)) { 
                std::string str = std::get<std::string>(result);
                if (str.empty()) {
                    return 0.0;
                }
                if (str.find_first_not_of("0123456789.") == std::string::npos) {
                    return std::stod(str);
                }
                throw FormulaError(FormulaError::Category::Value);
            }
            if (std::holds_alternative<FormulaError>(result)) {
                throw result;
            }
            return 0.0;
        };        
        try {
            return ast_.Execute(GetValueFromSheet);
        }
        catch(const FormulaError& fe){
            return fe;
        }
    }
        
    std::string Formula::GetExpression() const {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }
        
    std::vector<Position> Formula::GetReferencedCells() const {       
        std::vector<Position> res(ast_.GetCells().begin(), ast_.GetCells().end());
        auto last = std::unique(res.begin(), res.end());
        res.erase(last, res.end());
        return res; 
    }
          
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try{
        return std::make_unique<Formula>(std::move(expression));
    } catch (...) {
        throw FormulaException("");
   }
}