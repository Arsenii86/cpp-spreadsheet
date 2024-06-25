#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
  return output <<fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        :ast_(ParseFormulaAST(expression)) {
            
    } catch (const std::exception& exc) {
       throw FormulaException ("Uncorrect formula");
    } 
    
    Value Evaluate(const SheetInterface& sheet ) const override{
        try {  double temp = ast_.Execute(sheet);
           return temp;
        } catch (FormulaError& FE ) {            
            return FormulaError (FE.GetCategory());
        }
    } 
    
    std::string GetExpression() const override {
      std::stringstream out;
      ast_.PrintFormula(out);
      return out.str();          
    } 
    
    std::vector<Position> GetReferencedCells() const{
        std::vector<Position> ref_cell (begin(ast_.GetCells()), end(ast_.GetCells()));
        std::sort(ref_cell.begin(), ref_cell.end());
        auto last = std::unique(ref_cell.begin(), ref_cell.end());
        ref_cell.erase(last, ref_cell.end());
        return ref_cell;
    }
    
    

private:
    FormulaAST ast_;    
};
}  // namespace

////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}