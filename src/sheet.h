#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <vector>

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet() = default;

    void SetCell(const Position& pos, const std::string& text) override;

    const CellInterface* GetCell(const Position& pos) const override;
    CellInterface* GetCell(const Position& pos) override;

    void ClearCell(const Position& pos) override;
    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
private:
    Size printable_size_;
    std::unordered_map<Position, std::unique_ptr<Cell>, Cell::PositionHasher> sheet_data_;    
    void PrintCells(std::ostream& output, 
                         const std::function<void(const CellInterface&)>& printCell) const;
    
    void MaybeIncreaseSize(const Position& pos);
    void MaybeReduceSize();
    
    void ClearBackRefFromOtherCells(const Position& pos);
    void AddBackRefFromOldToNewCell(const Position& pos, std::unique_ptr<Cell>& cell);
    void AddNewCellToForwRefCells(const Position& pos, std::unique_ptr<Cell>& cell); 
};