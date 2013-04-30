#include <stdio.h>
extern "C" { 
#include <simple.h>
}
extern "C"{
#include "print.h"
}
#include "Graph.h"
#include "DataFlow.h"
#include "LICM.h"
#include "Common.h"
#include<vector>
//#include<hash_map>
// data structures you should consider using are vector and hash_map from the STL
// refer to the following link as a starting point if you are not familiar with them: 
// http://www.sgi.com/tech/stl/Vector.html
// http://www.sgi.com/tech/stl/hash_map.html 

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
    // build flow control graph 

    Graph *cfg = new Graph(inlist, proc_name);
    cfg->build_Graph();
    
    cfg->build_Loop_Graph();
    
    simple_instr *i;
    printf("\nProcedure %s:\n", proc_name);
    int count = 0;
    
    i = inlist;
    Common::print_instruction(i);
//    i = inlist;
//    while (i) {
//        printf("%d ",++count);
//        fprint_instr(stdout, i);
//        i = i->next;
//    }

    
    DataFlow *dfa = new DataFlow(cfg, proc_name);
//    dfa->find_Variable_List();
//    
//    dfa->find_Live_DEF();
//    
//    dfa->find_Live_USE();
//    
//    dfa->find_Live();
//    
//    dfa->print_Live_Info();
    
//    dfa->find_Expressions();
//    //dfa->print_Expressions();
//    dfa->find_Evaluations();
//    //dfa->print_Evaluations();
//    dfa->find_Kill_Set();
//    //dfa->print_Kill_Set();
//    dfa->find_Avail_Expressions();
//    //dfa->print_Avail_Expressions();
//    dfa->print_Avail_Expressions_Info();
    
    dfa->find_Reaching_Definitions();
    //dfa->print_RD_Info();
    
    dfa->build_Use_Def_Chain();
    //dfa->print_Use_Def_Chain();
    
    
    LICM *licm = new LICM(cfg, dfa, proc_name);
    licm->find_Loop_Invariants();
    licm->print_Loop_Invariants();
    
    licm->code_Motion();
    //licm->print_Code_Motion();
    i = inlist;
    printf("\nProcedure %s:\n", proc_name);
    Common::print_instruction(i);
    
    //cout<<"complete"<<endl;
    return inlist;
    

}
