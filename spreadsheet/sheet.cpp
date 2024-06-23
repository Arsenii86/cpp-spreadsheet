#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) throw InvalidPositionException ("неверный номер ячейки");  
    std::unique_ptr<Cell> cell_ptr = std::move(std::make_unique<Cell>(*this));       
    cell_ptr -> Set(text);    
    std:: vector<Position> UpperNodeCells;
	UpperNodeCells.push_back(pos);
    std::vector<Position> ReferencedCells = cell_ptr -> GetReferencedCells(); 
    if (ReferencedCells.size()==0){;}
	else{
        for(const auto& cell:ReferencedCells){
            if (count(UpperNodeCells.begin(),UpperNodeCells.end(),cell)) {
                throw CircularDependencyException("Циклическая зависимость");
            }
            else IsCyclicDependenced(*this,cell,UpperNodeCells);
        }            	
    }    
    if(cells_posit_.count(pos))  {
        sheet_[pos.row][pos.col]->Clear();	
    }
    else { 
        cells_posit_.insert(pos);
        const auto print_size =  GetPrintableSize();       
        if (print_size.rows > static_cast<int>(sheet_.size())){
            for(int i=static_cast<int>(sheet_.size()); i < print_size.rows; i++ ){
               sheet_.push_back(std::vector<std::unique_ptr<Cell>>{}); 
            }         
        }
        for (int i = 0 ; i < static_cast<int>(sheet_.size());i++){
            if (print_size.cols > static_cast<int>(sheet_[i].size())) {
                for (int j = static_cast<int>(sheet_[i].size()) ; j < print_size.cols; j++){
                    sheet_[i].push_back(std::move(std::make_unique<Cell>(*this)));
                }                
            }
        }
    }    
    sheet_[pos.row][pos.col] = std::move(cell_ptr) ;
	if(cell_dependencies_.count(pos)){
		for (const auto& position:cell_dependencies_.at(pos)){
			reinterpret_cast<Cell*>(GetCell(position))->Validate(false);
		}
	}
    
    std::vector<Position> ReferencedCell = sheet_[pos.row][pos.col] -> GetReferencedCells();  
    for(const auto& position:ReferencedCell ){
        cell_dependencies_[position].push_back(pos);
    }  
}


const CellInterface* Sheet::GetCell(Position pos) const { 
    if(!pos.IsValid()) throw InvalidPositionException ("неверный номер ячейки"); 
    if(cells_posit_.count(pos) == 0){
       const auto print_size =  GetPrintableSize(); 
       if (pos.row < print_size.rows && pos.col < print_size.cols){
           sheet_[pos.row][pos.col] -> Set("");           
           return sheet_[pos.row][pos.col].get();
       } 
       return nullptr;
    } 
    else return sheet_[pos.row][pos.col].get();
    
}

CellInterface* Sheet::GetCell(Position pos) {  
    if(!pos.IsValid()) throw InvalidPositionException ("неверный номер ячейки"); 
    if(cells_posit_.count(pos) == 0){
        const auto print_size =  GetPrintableSize(); 
        if (pos.row < print_size.rows && pos.col < print_size.cols){
           sheet_[pos.row][pos.col] -> Set("");           
           return sheet_[pos.row][pos.col].get();
        }
         return nullptr;
    } 
    else return sheet_[pos.row][pos.col].get();    
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) throw InvalidPositionException ("неверный номер ячейки"); 
    if(cells_posit_.count(pos) == 0);
    else {
            if(cell_dependencies_.count(pos)){
                for (const auto& position:cell_dependencies_.at(pos)){
                    reinterpret_cast<Cell*>(GetCell(position))->Validate(false);
                }
            }
            cell_dependencies_.erase(pos);
            sheet_[pos.row][pos.col]->Clear();            
            cells_posit_.erase(pos);
        }   
}

Size Sheet::GetPrintableSize() const {
    int print_row = 0;
    int print_col = 0;
    if (cells_posit_.size() == 0) return {0,0};
    else{
        for(const auto& cell_pos:cells_posit_){
            if (print_row < cell_pos.row) print_row = cell_pos.row;
            if (print_col < cell_pos.col) print_col = cell_pos.col;
        } 
    }
    return Size{print_row+1, print_col+1};  
}

void Sheet::PrintValues(std::ostream& output) const {
    const auto sheet_print_size = GetPrintableSize();
    for (int i = 0; i < sheet_print_size.rows; i++){
       bool is_first = true;
       for (int j = 0; j < sheet_print_size.cols; j++){
           if (!is_first) output << '\t';
           is_first = false;
           if(cells_posit_.count(Position{i,j})){  
              const auto value = sheet_[i][j]-> GetValue();
              if (!(sheet_[i][j] -> IsReferenced())){
                  sheet_[i][j] -> SetTempValue(value);
                  sheet_[i][j] ->Validate(true);
              }
              if (std::holds_alternative<double>(value))  output<<std::get<double>(value); 
              else if (std::holds_alternative<std::string>(value))  output<<std::get<std::string>(value);
              else output<<std::get<FormulaError>(value); 
           }
       }
       output<<'\n'; 
    }
    
}
void Sheet::PrintTexts(std::ostream& output) const {
    const auto sheet_print_size = GetPrintableSize();
    for (int i = 0; i < sheet_print_size.rows; i++){
        bool is_first = true;
       for (int j = 0; j < sheet_print_size.cols; j++){
           if (!is_first) output << '\t';
           is_first = false;
           if(cells_posit_.count(Position{i,j})) output<<(sheet_[i][j]-> GetText());               
       }
       output<<'\n'; 
    }   
}

void Sheet::IsCyclicDependenced(const Sheet& sheet,
                                const Position& curent_cell, 
                               std:: vector<Position> UpperNodeCells){
	UpperNodeCells.push_back(curent_cell);
    if (sheet.GetCell(curent_cell)==nullptr) return;//возможно пределать
	std::vector<Position> ReferencedCells = (sheet.GetCell(curent_cell)) -> GetReferencedCells();
	if (ReferencedCells.size()==0){;}
	else{
		for(const auto& cell:ReferencedCells){
			if (count(UpperNodeCells.begin(),UpperNodeCells.end(),cell)) {
                throw CircularDependencyException("Циклическая зависимость");
            }
			else IsCyclicDependenced(sheet,cell,UpperNodeCells);
	    }
	}
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}