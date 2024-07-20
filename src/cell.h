#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>
#include <optional>


class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(const std::string& text) override;
    void Clear();
    const Value GetValue() const override;
    
    struct PositionHasher {
        size_t operator()(const Position& pos) const {
            size_t row_h = int_hasher_(pos.row);
            size_t col_h = int_hasher_(pos.col);
            return row_h * 17 + col_h;
        }
        std::hash<int> int_hasher_;
    };
    
    const std::string GetText() const override;
    const std::vector<Position>& GetReferencedCells() const override;
    std::unordered_set<Position, PositionHasher> GetBackReferencedCells() const;
    bool HaveCash() const;
    bool IsEmptyCell() const;
    bool HaveCycleRef(const Position& pos_current, const Position& pos_visit, 
                      std::unordered_set<Position, PositionHasher>& visited) const;
    void AddBackReference(const Position& pos);
    void DeleteReference(const Position& pos);
    void ClearCash(); 
       
private:
    
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
        
    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;    
    mutable std::optional<Value> cash_;   
    std::vector<Position> forward_reference_;
    std::unordered_set<Position, PositionHasher> backward_reference_;
};

std::ostream& operator<<(std::ostream& output, CellInterface::Value var);
