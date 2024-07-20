#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


class Cell::Impl {
public:        
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual bool IsEmpty() const = 0;
};


class Cell::EmptyImpl : public Impl {
public:
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsEmpty() const override;
};

class Cell::TextImpl : public Impl {
public:        
    TextImpl(std::string text);          
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsEmpty() const override;
private:
    std::string text_;
};


class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl(SheetInterface& sheet, std::string text);    
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsEmpty() const override;    
private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell(SheetInterface& sheet) 
    : sheet_(sheet) 
    , impl_(std::make_unique<EmptyImpl>())
{   
}

Cell::~Cell() = default;

void Cell::Set(const std::string& text) {
    if(text[0] == FORMULA_SIGN && text != std::to_string(FORMULA_SIGN)){ 
        impl_ = std::make_unique<FormulaImpl>(sheet_, text.substr(1)); 
        forward_reference_ = std::move(impl_->GetReferencedCells());
    }
    else if(text != ""){
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
    else {
        impl_ = std::make_unique<EmptyImpl>();
    }
}

void Cell::Clear() { 
    forward_reference_.clear();
    impl_ = std::make_unique<EmptyImpl>();
}

const CellInterface::Value Cell::GetValue() const {
    if(!cash_.has_value()){
        cash_ = impl_->GetValue(); 
    }
    return cash_.value();
}

const std::string Cell::GetText() const {
    return impl_->GetText();
}

const std::vector<Position>& Cell::GetReferencedCells() const {
    return forward_reference_;
}

std::unordered_set<Position, Cell::PositionHasher> Cell::GetBackReferencedCells() const {
    return backward_reference_;
}

bool Cell::HaveCash() const {
    return cash_.has_value();
}

bool Cell::IsEmptyCell() const {
    return impl_->IsEmpty();
}

bool Cell::HaveCycleRef(const Position& pos_current, const Position& pos_target, 
                        std::unordered_set<Position, PositionHasher>& visited) const {
    visited.insert(pos_current);      
    for(const auto& pos_ref : forward_reference_){      
        if(!visited.count(pos_ref)){
            auto cell = static_cast<Cell*>(sheet_.GetCell(pos_ref));
            if(cell){
                if(cell->HaveCycleRef(pos_ref, pos_target, visited)){
                    return true;
                }
            }   
        }
        else if(pos_ref == pos_target){
            return true;
        } 
    }
    return false;   
}

void Cell::AddBackReference(const Position& pos) {
    if(!backward_reference_.count(pos)){
        backward_reference_.insert(pos);
    }
}

void Cell::DeleteReference(const Position& pos) {
    backward_reference_.erase(pos);
}

void Cell::ClearCash() {
    cash_.reset();
    for(const auto& pos_ref : backward_reference_){
        auto cell = static_cast<Cell*>(sheet_.GetCell(pos_ref));
        if(cell && cell->HaveCash()){ 
            cell->ClearCash();    
        } 
    }
}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return 0.0;
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    std::vector<Position> res;
    return res;
}

bool Cell::EmptyImpl::IsEmpty() const {
    return true;
}

Cell::TextImpl::TextImpl(std::string text) 
    : text_(text)
{            
}
              
CellInterface::Value Cell::TextImpl::GetValue() const { 
    if(text_[0] == ESCAPE_SIGN){
        return text_.substr(1);
    }
        return text_;
}
        
std::string Cell::TextImpl::GetText() const { 
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    std::vector<Position> res;
    return res;
}

bool Cell::TextImpl::IsEmpty() const {
    return false;
}

Cell::FormulaImpl::FormulaImpl(SheetInterface& sheet, std::string text) 
        : sheet_(sheet) 
        , formula_(ParseFormula(text))
{
} 
               
CellInterface::Value Cell::FormulaImpl::GetValue() const { 
    FormulaInterface::Value tmp = formula_->Evaluate(sheet_); 
    if(std::holds_alternative<double>(tmp)){
        return std::get<double>(tmp);
    }
    else{
        return std::get<FormulaError>(tmp);
    }
}
        
std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::IsEmpty() const {
    return false;    
}

std::ostream& operator<<(std::ostream& output, CellInterface::Value var) {
    if(std::holds_alternative<double>(var)){
         output << std::get<double>(var);
    }
    if(std::holds_alternative<std::string>(var)){
        output << std::get<std::string>(var);
    }
    if(std::holds_alternative<FormulaError>(var)){
        output << std::get<FormulaError>(var);
    }
    return output;
}
