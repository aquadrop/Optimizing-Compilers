//
//  DataFlow.h
//  A3
//
//  Created by Jianwei Xu on 13/02/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#ifndef __A3__DataFlow__
#define __A3__DataFlow__

#include <iostream>
#include "Graph.h"

typedef struct
{
    simple_reg *src1;
    simple_op opcode;
    simple_reg *src2;
}Expression;

class DataFlow : public Graph
{
    VNode* graph;
    VNode* antigraph;
    simple_instr* inlist;
    char* proc_name;
    int nblk;
    int ninstr;
    
    

    
    vector<int> var_list;//number to string
    __gnu_cxx::hash_map<int, int>anti_var_list;//string to number
    int nvar;
    static const char TAG = 'r';
    
    bool **live_DEF;
    bool **live_USE;
    bool **liveout;
    bool **livein;
    
    //available expressions module
    __gnu_cxx::hash_map<int, int>line_hash_map;
    __gnu_cxx::hash_map<int, int>anti_line_hash_map;
    int *belongtowhichblock;
    __gnu_cxx::hash_map<int, int>expressions_map;//instruction line to expression index
    std::vector<int> expressions_list;
    std::vector<int> full_expressions_list;
    std::vector<Expression> expressions_vector;//use to eliminate duplicates
    int nexpression;
    bool **evaluations;
    bool **kill_set;
    bool **avail_exprs_out;
    bool **avail_exprs_in;
    
    //reach definition module
    /*
     * 
     *
     */
    
    /**anti_definition_map and definition_map is constant, when established, they shouldn't be altered**/
    
    __gnu_cxx::hash_map<int, int> anti_definition_map;//definition index to instruction line;
    
    //for this we do not consider kill
    __gnu_cxx::hash_map<int, int> definition_map; //instruction line to definition index;
    //defintion starts at 1;
    
    int ndefinition;
    
    bool** rd_gen_set;
    bool** rd_kill_set;
    bool** rd_out;
    bool** rd_in;
    
    //ndomi -1 for that scr2 is null, only happens for src2 or use_def_chain[1];
    //0 for src1, 1 for src2;
    LoopG **use_def_chain;
    
    
    
    
    
public:
    typedef Graph super;
    DataFlow();
    DataFlow(super*, char*);
    
    
    void find_Variable_List();
    void print_Variable_List();
    
    
    void find_Live_DEF();
    void print_Live_DEF();
    
    void find_Live_USE();
    void print_Live_USE();
    
    void find_Live();
    void print_Live();
    
    void print_Live_Info();
    
    //available expressions module
    void find_Expressions();
    void print_Expressions();
    
    void find_Evaluations();
    void print_Evaluations();
    
    void find_Kill_Set();
    void print_Kill_Set();
    
    void find_Avail_Expressions();
    void print_Avail_Expressions();
    
    void print_Avail_Expressions_Info();
    
    //reaching definition module
    int is_Definition(simple_instr*);
    void fill_RD_Gen_Hash(simple_instr*);
    void find_Definitions();
    void find_RD_Gen_Set();
    void find_RD_Kill_Set();
    void fill_RD_Kill_Hash(simple_instr*);
    
    void find_Reaching_Definitions();
    
    void print_RD_Gen_Set();
    void print_RD_Kill_Set();
    void print_RD();
    void print_RD_Info();
    __gnu_cxx::hash_map<int, int>get_anti_definition_map();
    __gnu_cxx::hash_map<int, int>get_definition_map();
    bool** get_rd_out();
    bool** get_rd_in();
    int get_ndefinition();
    
    //build use-def chains
    //hash map from instruction line to definition instruction line
    void build_Use_Def_Chain();
    void grow_Use_Def_Chain(simple_reg* r, bool* detailed_rd, int instruction_line, int type);
    void print_Use_Def_Chain();
    //type o for src1, type 1 for src2
    LoopG** get_Use_Def_Chain();
    
    
private:
   
    
    /*find variable list*/
    void fill_Hash_Table(simple_reg*);
    int retrive_Var_Register(simple_reg*);
    bool valid_Reg(simple_reg*);
    
    /*find live variable DEF*/
    void fill_Live_DEF(int, simple_reg*);

    /*find live variable USE*/
    void fill_Live_USE(int, int** ,simple_reg*, bool);

    /*find live*/
    
    
    
    /*available expressions*/
    void fill_Expression_Vector(Expression);
    int check_Duplicate_Expression(Expression);//-1 means no find
    bool compare_Expression(Expression, Expression);

    //share
    void copy_Vector(bool* left, bool** right, int whichblock, int cols);
    void intersect_Vector(bool** left, bool** right, int whichblock, int cols);
    void union_Suc_Vector(bool**left, bool**right, int whichblock, int cols);
    void union_Pred_Vector(bool**left, bool**right, int whichblock, int cols);
    void formula_Vector(bool** result, bool** gen, bool** put, bool** minus, int whichblock, int cols);
    bool compare_Vector(bool** left, bool* right, int whichblock, int cols);

    
    //share
    //bool key_Exist_Hash();

protected:
    void backward_Anyway_Solver(bool** in, bool**gen, bool**out, bool**minus, int rows, int cols);
    void forward_Allway_Solver(bool** out, bool** gen, bool** in, bool** minus, int rows, int cols);
    void forward_Anyway_Solver(bool** out, bool** gen, bool** in, bool** minus, int rows, int cols);
    
};
#endif /* defined(__A3__DataFlow__) */
