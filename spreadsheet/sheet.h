#pragma once
#include <iostream>
#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <vector>
#include <set>

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
    const std::set<Position>& GetCellsInput(){
        return cells_posit_;
    }
	// Можете дополнить ваш класс нужными полями и методами      
    
private:
	// Можете дополнить ваш класс нужными полями и методами   
    std::vector<std::vector<std::unique_ptr<Cell>>> sheet_;
    std::set<Position> cells_posit_;
    std::map<Position,std::vector<Position>> cell_dependencies_; 
};