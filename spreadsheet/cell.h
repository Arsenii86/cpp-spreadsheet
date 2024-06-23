#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
  public:    
    Cell(Sheet& sheet);
    ~Cell()=default;

    void Set(std::string text);
    void Clear();
    void SetTempValue(CellInterface::Value val);    
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    void Validate(bool valid);
    
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;        
    bool is_valid_;
    CellInterface::Value temp_;
};

    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.
class Cell::Impl{
    public:
        virtual CellInterface::Value GetValue([[maybe_unused]] const SheetInterface& sheet)const=0;
        virtual std::string GetText()const=0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual void Clear()=0;
        virtual ~Impl() = default;
};

class Cell::EmptyImpl: public Cell::Impl{
        std::string empty_value_={};
    public:
        EmptyImpl(){};
        CellInterface::Value GetValue([[maybe_unused]] const SheetInterface& sheet) const{
            return empty_value_;
        }
        std::string GetText() const{
            return empty_value_;
        }
    
        std::vector<Position> GetReferencedCells() const{
            return {};
            }
    
        void Clear(){ 
            empty_value_.clear();
        }
        ~EmptyImpl()=default;
    
 };

class Cell::TextImpl: public Cell::Impl{
        std::string value_;
    public:
        TextImpl(const std::string& text):value_(text){};
    
        CellInterface::Value GetValue([[maybe_unused]] const SheetInterface& sheet) const{
            if (value_[0]==ESCAPE_SIGN){
               return std::string(value_.begin()+1,value_.end()); 
            }
            return value_;
        }
    
        std::vector<Position> GetReferencedCells() const{
            return {};
        }
    
        std::string GetText() const{
            return value_;
        }
    
        void Clear(){
            value_.clear();
        };
        ~TextImpl()=default;
    
 };

class Cell::FormulaImpl: public Cell::Impl{
        std::unique_ptr<FormulaInterface> formula_uniq_ptr;    
    public:
        FormulaImpl(std::string formula_text):
            formula_uniq_ptr(std::move(ParseFormula(formula_text))){};
    
        CellInterface::Value GetValue([[maybe_unused]] const SheetInterface& sheet) const{            
            auto value = formula_uniq_ptr->Evaluate(sheet);
            if (std::holds_alternative<double>(value)) return std::get<double>(value);
            else return std::get<FormulaError>(value);
        }
    
        std::string GetText() const{
            return '='+formula_uniq_ptr->GetExpression();
        }
    
        std::vector<Position> GetReferencedCells() const{
            return formula_uniq_ptr -> GetReferencedCells(); 
        }
        void Clear(){
            formula_uniq_ptr.reset();
        };    
    
        ~FormulaImpl()=default;
    
    
};
