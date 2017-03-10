#ifndef STSOLVER_H
#define STSOLVER_H
#include "Structures/State.h"
#include "../lpsolve/lp_lib.h"
#include <memory>

namespace PetriEngine {
    class STSolver {
    
    struct STVariable {
        STVariable(int c, REAL v){
            colno=c;
            value=v;
        }
        int colno;
        REAL value;
    };
    
    struct place_t {
        uint32_t pre, post;
    };
        
    public:
        STSolver(const PetriNet& net);
        virtual ~STSolver();
        int CreateFormula();
        int Solve();
        void PrintResult();
        int getResult(){
            return _ret;
        }
        
    private:        
        std::string VarName(uint32_t index);
        void MakeConstraint(std::vector<STVariable> constraint, int constr_type, REAL rh);
        void CreateSiphonConstraints();
        void CreateStepConstraints(uint32_t i);
        void CreatePostVarDefinitions(uint32_t i);
        void constructPrePost();
        void SanityCheckVariables();
        
        std::unique_ptr<place_t[]> _places;
        std::unique_ptr<uint32_t> _transitions;
        const PetriNet& _net;
        const MarkVal* _m0;
        lprec* _lp;
        uint32_t _siphonDepth;
        uint32_t _nPlaceVariables;
        uint32_t _nCol;
        int _ret = 0;        
    };
}
#endif /* STSOLVER_H */

