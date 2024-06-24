#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include "sheet.h"

Cell::Cell(Sheet& sheet):sheet_(sheet){is_valid_ = false;};

void Cell::IsCyclicDependenced(const Sheet& sheet,
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

void Cell::Set(std::string text,[[maybe_unused]] Position& pos) {
    if(text.empty()){
        //создаем указатель на пустую текстовую ячейку
       // Clear();
        std::unique_ptr<EmptyImpl> empty_impl = std::make_unique<EmptyImpl>();
        impl_=std::move(empty_impl);       
    }    
    else if (text.size() > 1 && text[0] == FORMULA_SIGN){        
        //воспринимаем как фомулу и создаем указатель на формульную ячейку
        std::string formula(text.begin() + 1, text.end());
        std::unique_ptr<FormulaImpl> formula_impl =std::make_unique<FormulaImpl>(formula);
        impl_=std::move(formula_impl);
        
        std:: vector<Position> UpperNodeCells; 
        UpperNodeCells.push_back(pos);
        std::vector<Position> ReferencedCells = GetReferencedCells();
        if (ReferencedCells.size()==0){;} 
        else{ 
            for(const auto& cell:ReferencedCells){ 
                if (count(UpperNodeCells.begin(),UpperNodeCells.end(),cell)) {
                    throw CircularDependencyException("Циклическая зависимость");
                } 
                else IsCyclicDependenced(sheet_,cell,UpperNodeCells); 
            }
        }
    } 
    else{
        //воспринимаем как текст и создаем указатель на текстовую ячейку
        //Clear();       
        std::unique_ptr<TextImpl> text_impl = std::make_unique<TextImpl>(std::move(text));
        impl_=std::move(text_impl);        
    }
}


std::vector<Position> Cell::GetReferencedCells() const{
  return impl_->GetReferencedCells();    
}


void Cell::Clear() {   
    if(impl_!=nullptr){
        Position pos{};
        Set("",pos);
        //impl_->Clear();
        //impl_.reset();
    }
}

CellInterface::Value Cell::GetValue() const {
    if(is_valid_) return temp_;
    else  return impl_->GetValue(sheet_); 
}
 void Cell::SetTempValue(CellInterface::Value val){
     temp_ = val;    
 }; 

std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::IsReferenced() const{
    return is_valid_;
};

void Cell::Validate(bool valid){
    is_valid_ = valid;
};


    