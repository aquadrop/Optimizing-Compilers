//
//  LICM.h
//  Project
//
//  Created by Jianwei Xu on 07/04/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#ifndef __Project__LICM__
#define __Project__LICM__

#include <iostream>
#include "Graph.h"
#include "DataFlow.h"
#include <algorithm>
#include <queue>

class LICM
{
    VNode* graph;
    VNode* antigraph;
    simple_instr* inlist;
    char* proc_name;
    int nblk;
    int *belongtowhichblock;
    
    __gnu_cxx::hash_map<int, int>line_hash_map;
     __gnu_cxx::hash_map<int, int>anti_line_hash_map;
    
    //definition index to instruction line; definition index starts from one;
    __gnu_cxx::hash_map<int, int> anti_definition_map;
    //instruction line to definition index;
    __gnu_cxx::hash_map<int, int> definition_map;
    bool** rd_out;
    bool** rd_in;
    int ndefinition;

    //does the block belong to the loop
    __gnu_cxx::hash_map<int, bool>*is_inside_loop;
    LoopG *loop_graph;
    int nloop;
    
    //global loop_invariants, will be moved to preheader when examing EACH loops
    int ninstr;//entry and exit instr 0 and ** is not included, real instructions start from 1;
    bool **loop_invariants;
    
    
    //code motion module
    Dominator* proper_dominator_graph;
    vector<vector<int> > loop_member;
    
    LoopG** use_def_chain;
    vector<int> instruction_list;
    
    bool* move_list;
    
public:
    LICM();
    LICM(Graph* cfg, char* proc_name);
    LICM(Graph* cfg, DataFlow* dfa, char* proc_name);
    void initialize_Is_Inside_Loop();
    void initialize_loop_member();
    
    
    bool search_Reaching_Definitions(simple_reg* operand, int loop_index, int block_index);
    void find_Constant_Loop_Invariants(int loop_index);//**main
    
    /***/
    bool is_Operand_Loop_Invariant(simple_reg* operand, int loop_index);
    int is_Operand_Duplicate_In_Loop(simple_reg*, bool* detailed_rd, int loop_index);
    bool is_Operand_Definition_Outside_Loop(simple_reg*, int loop_index);
    bool is_Operand_Definition_Invariant(simple_reg*, int loop_index);
    void find_Extended_Loop_Invariants(int loop_index);//**main
    /***/
    
    void find_Loop_Invariants(int loop_index);
    void find_Loop_Invariants();
    
    
    
    
    bool check_domi(int down, int up);
    bool def_Block_Dominate_Exit(simple_instr*, int loop_index);
    bool is_definition_duplicate(simple_reg*, int loop_index);
    //the use of the definition can be moved
    bool is_instr_movable(simple_instr*, int loop_index, int block_index);
    //3. all uses of v in L can only be reached by the definition of v in s
    bool is_single_reaching_definition(simple_reg* v, int v_block_index, int loop_index);
    
    void grow_instr_vector(int t);
    void BFS(LoopG G, int root);
    
    
    void replace_reg(simple_reg* orig, simple_reg* tar, int block_index);
    void replace_reg(simple_reg* orig, simple_reg* tar, vector<simple_instr*> list);
    vector<simple_instr*> consolidate_list(simple_reg* temp, int loop_index);
    void consolidate_preheader(int p_index, int loop_index);
    void move_instr_preheader(simple_instr*s, int block_index, int loop_index);
    bool rd_inside_move_list(simple_reg*, bool*, int);
    void consolidate_move_list(int loop_index);
    void build_move_list(int loop_index);//instruction hash
    
    void code_Motion(int loop_index);
    void code_Motion();
    void print_Code_Motion();
    
    //finalize all the loop invariants to the preheaders
    void find_LICM();
    
    void print_Loop_Invariants();
    
    
    
};

































#endif /* defined(__Project__LICM__) */

