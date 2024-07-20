#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet()
    : printable_size_({0, 0})
{}

void Sheet::SetCell(const Position& pos, const std::string& text) {    
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid position ("s + std::to_string(pos.row) + " , "s + std::to_string(pos.col) + ")"s);
    } 
    
    if(sheet_data_.count(pos) && sheet_data_.at(pos)->GetText() == text){
        return;
    }
    
    std::unique_ptr<Cell> new_cell(std::make_unique<Cell>(*this)); 
    new_cell->Set(text);  
    std::unordered_set<Position, Cell::PositionHasher> visited;  
    if(new_cell->HaveCycleRef(pos, pos, visited)){
        throw CircularDependencyException("Circular Dependency found with ("s + std::to_string(pos.row) + " , "s + std::to_string(pos.col) + ")"s);
    }    

    MaybeIncreaseSize(pos);    
    if(sheet_data_.count(pos)){        
        sheet_data_.at(pos)->ClearCash();
        ClearBackRefFromOtherCells(pos);
        AddBackRefFromOldToNewCell(pos, new_cell);      
    }
    AddNewCellToForwRefCells(pos, new_cell);   
    sheet_data_[pos] = std::move(new_cell);
}


const CellInterface* Sheet::GetCell(const Position& pos) const { 
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid position ("s + std::to_string(pos.row) + " , "s + std::to_string(pos.col) + ")"s);
    }    
    return sheet_data_.count(pos) ? sheet_data_.at(pos).get() : nullptr;
}

CellInterface* Sheet::GetCell(const Position& pos) { // поменять
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid position ("s + std::to_string(pos.row) + " , "s + std::to_string(pos.col) + ")"s);
    } 
    return sheet_data_.count(pos) ? sheet_data_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(const Position& pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid position ("s + std::to_string(pos.row) + " , "s + std::to_string(pos.col) + ")"s);
    }
    if(pos.row <= printable_size_.rows && pos.col <= printable_size_.cols && 
           sheet_data_.count(pos)){        
        sheet_data_.at(pos)->ClearCash();
        ClearBackRefFromOtherCells(pos);        
        sheet_data_.erase(pos);        
        MaybeReduceSize();   
    }
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintCells(std::ostream& output, 
                         const std::function<void(const CellInterface&)>& printCell) const {
    for(int j = 0; j < printable_size_.rows; ++j){
        for(int i = 0; i < printable_size_.cols; ++i) {
            if(sheet_data_.count(Position{j, i})){
                if(!sheet_data_.at(Position{j, i})->IsEmptyCell()){
                    printCell(*sheet_data_.at(Position{j, i}));
                }
            }
            if(i + 1 != printable_size_.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }    
}

void Sheet::PrintValues(std::ostream& output) const {
    auto printCellValue = [&output](const CellInterface& cell){
        output << cell.GetValue();
    };
    PrintCells(output, printCellValue);
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto printCellText = [&output](const CellInterface& cell){
        output << cell.GetText();
    };
    PrintCells(output, printCellText);
}

void Sheet::MaybeIncreaseSize(const Position& pos) {
    if(pos.row + 1 > printable_size_.rows) {                
        printable_size_.rows = pos.row + 1;     
    }  
    if(pos.col + 1 > printable_size_.cols){
        printable_size_.cols = pos.col + 1;
    } 
}

void Sheet::MaybeReduceSize() {
    int new_row = 0, new_col = 0;
    for(int i = 0; i < printable_size_.rows; ++i){
        for(int j = 0; j < printable_size_.cols; ++j){
            if(sheet_data_.count(Position{i, j})){
                if(!sheet_data_.at(Position{j, i})->IsEmptyCell()){
                    if(i + 1 > new_row){
                        new_row = i + 1;
                    }
                    if(j + 1 > new_col){
                        new_col = j + 1;
                    } 
                }
            }
        }
    }
    printable_size_.rows = new_row;
    printable_size_.cols = new_col; 
}

void Sheet::ClearBackRefFromOtherCells(const Position& pos) {
    for(const auto& pos_ref : sheet_data_.at(pos)->GetReferencedCells()){
        sheet_data_.at(pos_ref)->DeleteReference(pos); 
    }
}

void Sheet::AddBackRefFromOldToNewCell(const Position& pos, std::unique_ptr<Cell>& cell) {
    for(const auto& pos_ref : sheet_data_.at(pos)->GetBackReferencedCells()) {
        cell->AddBackReference(pos_ref);
    }
}

void Sheet::AddNewCellToForwRefCells(const Position& pos, std::unique_ptr<Cell>& cell) {
    for(const auto& pos_ref : cell->GetReferencedCells()){
        if(!sheet_data_.count(pos_ref)){
            sheet_data_[pos_ref] = std::make_unique<Cell>(*this);
        }
        sheet_data_.at(pos_ref)->AddBackReference(pos);
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}