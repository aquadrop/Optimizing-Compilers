//
//  LICM.cpp
//  Project
//
//  Created by Jianwei Xu on 07/04/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#include "LICM.h"
#include <suif.h>
#include <suif_copyright.h>

static alist nr_stat;


LICM::LICM()
{
    
}

LICM::LICM(Graph* cfg, char* proc_name)
{
    this->inlist = cfg->getInlist();
    this->graph = cfg->getGraph().first;
    this->antigraph = cfg->getGraph().second;
    this->proc_name = proc_name;
    
    this->nblk = cfg->get_Nblk();
    
    this->loop_graph = cfg->get_Loop_Graph();
    this->nloop = cfg->get_nloop();
    
    this->ninstr = cfg->get_ninstr();
    
}

LICM::LICM(Graph* cfg, DataFlow* dfa, char* proc_name)
{
    this->inlist = cfg->getInlist();
    this->line_hash_map = cfg->getInlistLineHash();
    this->anti_line_hash_map = cfg->get_Anti_Line_Hash();
    
    this->graph = cfg->getGraph().first;
    this->antigraph = cfg->getGraph().second;
    this->proc_name = proc_name;
    this->belongtowhichblock = cfg->getBelongtoWhichBlock();
    
    
    this->nblk = cfg->get_Nblk();
    
    this->loop_graph = cfg->get_Loop_Graph();
    this->nloop = cfg->get_nloop();
    
    this->ninstr = cfg->get_ninstr();
    
    //reaching definition part
    this->ndefinition = dfa->get_ndefinition();
    this->definition_map = dfa->get_definition_map();
    this->anti_definition_map = dfa->get_anti_definition_map();
    this->rd_out = dfa->get_rd_out();
    this->rd_in = dfa->get_rd_in();
    
    this->initialize_Is_Inside_Loop();
    
    this->proper_dominator_graph = cfg->get_proper_dominator_graph();
    
    this->initialize_loop_member();
    
    this->use_def_chain = dfa->get_Use_Def_Chain();
    
}

void LICM::initialize_loop_member()
{
    for (int i = 0; i<this->nloop; i++)
    {
        vector<int> member;
        ArcNode *p = this->loop_graph[i].firstarc;
        while (p) {
            member.push_back(p->adjvex);
            p = p->nextarc;
        }
        std::sort(member.begin(), member.end());
        this->loop_member.push_back(member);
    }
}

void LICM::initialize_Is_Inside_Loop()
{
    //initilize to false
    this->is_inside_loop = new __gnu_cxx::hash_map<int, bool>[this->nloop];
    for (int l = 0; l<this->nloop; l++)
    {
        for (int i = 0; i<this->nblk+2; i++)
        {
            this->is_inside_loop[l][i] = false;
        }
    }
    for (int i = 0; i<this->nloop; i++)
    {
        ArcNode *p = this->loop_graph[i].firstarc;
        while (p) {
            this->is_inside_loop[i][p->adjvex] = true;
            p = p->nextarc;
        }
    }
}

//has reaching definition outside of the loop;
//check whether the rd_in for the block has the definition for this simple_reg*
bool LICM::search_Reaching_Definitions(simple_reg *operand, int loop_index, int block_index)
{
    //cout<<"reaching "<<loop_index<<" "<<block_index<<endl;
    bool is = true;
    for (int i = 0; i<ndefinition; i++)
    {
        if (!this->rd_in[block_index][i])
            continue;
        
        //definition starts from 1
        int instruction_line = this->anti_definition_map[i+1];
        
        int whichblock = this->belongtowhichblock[instruction_line];
        //cout<<"here "<<block_index<<" "<<instruction_line<<" "<<whichblock<<endl;
        
        simple_instr* checkpoint = (simple_instr*) this->anti_line_hash_map[instruction_line];
        //now check the operand is or not equal to dst
        //cout<<"here"<<instruction_line<<endl;
        if (checkpoint->u.base.dst == operand)
            if (is_inside_loop[loop_index][whichblock])
                return false;
    }
    return true;
}

void LICM::find_Constant_Loop_Invariants(int loop_index)
{
    //iterater through blocks of the current loop//loop linkedlist starts from 0
    
    ArcNode *p = this->loop_graph[loop_index].firstarc;
    simple_instr* s = NULL;
    int instruction_line = -1;
    /*
    simple_instr* _s = this->inlist;
    while (_s) {
        cout<<simple_op_name(_s->opcode)<<" "<<this->line_hash_map[(int)_s]<<endl;
        _s = _s->next;
    }
     */
//    int count = 0;
//    cout<<"hello"<<endl;
//    for (int j = 1; j<=this->ninstr; j++)
//    {
//        if (this->loop_invariants[1][j])
//            cout<<"Hello"<<++count<<" "<<j<<endl;
//    }
    for (std::vector<int>::iterator it = this->loop_member[loop_index].begin(); it!=this->loop_member[loop_index].end(); ++it)
    {
        int block_index = *it;
        
            //cout<<"next "<<block_index<<endl;
        s = this->graph[block_index].data.leader;
        //check operands of the instruction
        //first perform constant mark and reaching definitions mark
        for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
        {
            int instruction_line = this->line_hash_map[(int)s];

//            cout<<simple_op_name(s->opcode)<<" "<<i<<" "<<this->graph[block_index].data.ninstr<<" "<<block_index<<endl;
            switch (s->opcode)
            {
                    //load constant, must be put into loop_invariant
                case LDC_OP:
                {
                    if (s->u.ldc.value.format == IMMED_SYMBOL)
                    {
                        break;
                    }
                    if (s->u.ldc.value.format == IMMED_INT||s->u.ldc.value.format == IMMED_FLOAT)
                        this->loop_invariants[loop_index][instruction_line] = true;
                    
                    //now we ignore IMMED_SYMBOL
                    
                    //cout<<i<<instruction_line<<" "<<simple_op_name(s->opcode)<<endl;
                    break;
                    
                }
                    //one operand
                case LOAD_OP:
                case CPY_OP:
                case CVT_OP:
                case NEG_OP:
                case NOT_OP:
                {
                    if (this->search_Reaching_Definitions(s->u.base.src1, loop_index, block_index))
                    {
                        //cout<<"reaching"<<endl;
                        this->loop_invariants[loop_index][instruction_line] = true;
                        //cout<<instruction_line<<" "<<simple_op_name(s->opcode);
                    }
                    break;
                }
                //cpy src2 to src1:
                case STR_OP:
                {
                    if (this->search_Reaching_Definitions(s->u.base.src2, loop_index, block_index))
                    {
                        //cout<<"reaching"<<endl;
                        this->loop_invariants[loop_index][instruction_line] = true;
                        //cout<<instruction_line<<" "<<simple_op_name(s->opcode);
                    }

                    break;
                }
                    //two operands:
                case ADD_OP:
                case SUB_OP:
                case MUL_OP:
                case DIV_OP:
                case REM_OP:
                case MOD_OP:
                case AND_OP:
                case IOR_OP:
                case XOR_OP:
                case ASR_OP:
                case LSR_OP:
                case LSL_OP:
                case ROT_OP:
                case SEQ_OP:
                case SNE_OP:
                case SL_OP:
                case SLE_OP:
                {
                    if (this->search_Reaching_Definitions(s->u.base.src1, loop_index, block_index))
                        if (this->search_Reaching_Definitions(s->u.base.src2, loop_index, block_index))
                            this->loop_invariants[loop_index][instruction_line] = true;
                            
                            break;
                }
                default:
                    break;
            }
            s = s->next;
        }
        
        p = p->nextarc;
    }
}

bool LICM::is_Operand_Loop_Invariant(simple_reg* operand, int loop_index)
{
    for (int i = 0; i<this->ninstr+2; i++)
    {
        //skip non-invariant instruction
        if (!this->loop_invariants[loop_index][i])
            continue;
        
        simple_instr* checkpoint = (simple_instr*) this->anti_line_hash_map[i];
        if (checkpoint->u.base.dst == operand)
            return true;
    }
    return false;
}
 
    
//only one definition from inside loop, if there are outside definition plus SINGLE inside, true
int LICM::is_Operand_Duplicate_In_Loop(simple_reg* operand, bool* detailed_rd, int loop_index)
{
    int which_instruction = -1;
    int inside_count = 0;
    int outside_count = 0;
    int block_index = 0;
    simple_instr* checkpoint = NULL;
    for (int i = 0; i<this->ndefinition; i++)
    {
        //check whether this definition is reaching definition
        if (!detailed_rd[i])
            continue;
        
        //check whether this definition is inside the loop
        int instruction_line = this->anti_definition_map[i+1];
        //cout<<"duplicate"<<instruction_line<<endl;
        
        block_index = this->belongtowhichblock[instruction_line];
        checkpoint = (simple_instr*) this->anti_line_hash_map[instruction_line];
        if (checkpoint->u.base.dst == operand)
        {
            //cout<<count++<<endl;
            if (!is_inside_loop[loop_index][block_index])
                outside_count++;
            else
            {
                inside_count++;
                which_instruction = instruction_line;
            }
            //cout<<"inside "<<checkpoint->u.base.dst->num<<" "<<operand->num<<" "<<instruction_line<<endl;
            
        }
        
    }
    //cout<<"count"<<count<<" "<<which_instruction<<"reg "<<operand->num<<endl;
    if (inside_count>1)
        return -1;
    
    else if (inside_count == 1)
        return which_instruction;
    //meaning the definition is outside the loop
    else
    {
        return -2;
    }
    
}
//there are two steps in for this function
    
/*
 *
 1. operands are all marked as loop-invariants (i.e., defined
 by a loop invariant operation), or
 2. operands have exactly one reaching definition from
 inside the loop, and that definition is in an instruction
 which is marked invariant
 
 
 */
void LICM::find_Extended_Loop_Invariants(int loop_index)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        ArcNode *p = this->loop_graph[loop_index].firstarc;
        
        //we sort the block_index
//        std::vector<int> loop_block_vector;
//        while (p) {
//            loop_block_vector.push_back(p->adjvex);
//            p = p->nextarc;
//        }
//        std::sort(loop_block_vector.begin(), loop_block_vector.end());
        
        for (std::vector<int>::iterator it=loop_member[loop_index].begin(); it!=loop_member[loop_index].end(); ++it)
        {
            int block_index = *it;
            simple_instr* s = this->graph[block_index].data.leader;
            /***note that the definition index starts from 1**/
            //initialize the detailed_rd to rd_in
            bool detailed_rd[this->ndefinition];
            for (int i = 0; i<this->ndefinition; i++)
                detailed_rd[i] = this->rd_in[block_index][i];
            /****
             but definition index stores in the array starts from zero
             
             ready to use anti_definition_map to convert definition index to instruction line
             **********************************************/
            
            
            
            for (int i = 0;i<graph[block_index].data.ninstr; i++)
            {
                
                //skip already invariant marked instruction
                int instruction_line = this->line_hash_map[(int)s];
                if (this->loop_invariants[loop_index][instruction_line])
                {
                    s = s->next;
                    continue;
                }
                
                //**************operands are all marked as loop-invariants (i.e., defined
                //by a loop invariant operation)
                /*
                switch (s->opcode)
                {
                        //load constant, must be put into loop_invariant
                    case LDC_OP:
                    {
                        this->loop_invariants[loop_index][instruction_line] = true;
                        break;
                    }
                        //one operand
                    case LOAD_OP:
                    case CPY_OP:
                    case CVT_OP:
                    case NEG_OP:
                    case NOT_OP:
                    {
                        if (this->is_Operand_Loop_Invariant(s->u.base.src1, loop_index))
                        {
                            this->loop_invariants[loop_index][instruction_line] = true;
                            if (!changed)
                                changed = true;
                        }
                        
                        break;
                    }
                        //two operands:
                    case ADD_OP:
                    case SUB_OP:
                    case MUL_OP:
                    case DIV_OP:
                    case REM_OP:
                    case MOD_OP:
                    case AND_OP:
                    case IOR_OP:
                    case XOR_OP:
                    case ASR_OP:
                    case LSR_OP:
                    case LSL_OP:
                    case ROT_OP:
                    case SEQ_OP:
                    case SNE_OP:
                    case SL_OP:
                    case SLE_OP:
                    {
                        if (this->is_Operand_Loop_Invariant(s->u.base.src1, loop_index))
                            if (this->is_Operand_Loop_Invariant(s->u.base.src2, loop_index))
                            {
                                this->loop_invariants[loop_index][instruction_line] = true;
                                if (!changed)
                                    changed = true;
                            }
                                
                        break;
                    }
                    default:
                        break;
                }
                
                */
                //********operands have exactly one reaching definition from
                //inside the loop, and that definition is in an instruction
                //which is marked invariant
                
                //we need to examine reaching definitions against instruction one by one, starting
                //from rd_in of this block
                //Computes reaching definition bitvector for each instruction
                
                //1. first compute the reaching defintion bitvector for each instruction
               
                
                {
                    //no need to operate detailed_rd
                    //operands have exactly one reaching definition from
                    //inside the loop, and that definition is in an instruction
                    //which is marked invariant
                    //1.1.1 check duplicate WITHIN LOOP!!
                    
                    
                    switch (s->opcode)
                    {
                        //load constant, must be put into loop_invariant
                        case LDC_OP:
                        {
                            //this will never happen in this step 
                            //this->loop_invariants[loop_index][instruction_line] = true;
                            break;
                        }
                            //one operand
                        case LOAD_OP:
                        case CPY_OP:
                        case CVT_OP:
                        case NEG_OP:
                        case NOT_OP:
                        {
                            //cout<<instruction_line<<" single "<<simple_op_name(s->opcode)<<endl;
                            int condition_index = this->is_Operand_Duplicate_In_Loop(s->u.base.src1, detailed_rd, loop_index);
                            if (condition_index == -1)
                                break;
                            //1.1.2 if not duplicate, check whether it is loop_invariant
                            if (this->loop_invariants[loop_index][condition_index])
                            {
                                this->loop_invariants[loop_index][instruction_line] = true;
                                if (!changed)
                                    changed = true;
                            }
                            break;
                        }
                            
                        case STR_OP:
                        {
                            int condition_index = this->is_Operand_Duplicate_In_Loop(s->u.base.src2, detailed_rd, loop_index);
                            if (condition_index == -1)
                                break;
                            //1.1.2 if not duplicate, check whether it is loop_invariant
                            if (this->loop_invariants[loop_index][condition_index])
                            {
                                this->loop_invariants[loop_index][instruction_line] = true;
                                if (!changed)
                                    changed = true;
                            }
                            break;
                        }
                            //two operands:
                        case ADD_OP:
                        case SUB_OP:
                        case MUL_OP:
                        case DIV_OP:
                        case REM_OP:
                        case MOD_OP:
                        case AND_OP:
                        case IOR_OP:
                        case XOR_OP:
                        case ASR_OP:
                        case LSR_OP:
                        case LSL_OP:
                        case ROT_OP:
                        case SEQ_OP:
                        case SNE_OP:
                        case SL_OP:
                        case SLE_OP:
                        {
                            //cout<<instruction_line<<" double "<<simple_op_name(s->opcode)<<endl;
                            
                            //the case that oprand is from outside also counts
                            
                            
                            bool is_invariant = false;
                            int condition_index = this->is_Operand_Duplicate_In_Loop(s->u.base.src1, detailed_rd, loop_index);
                            if (condition_index != -1)
                            {
                                if (condition_index == -2 || this->loop_invariants[loop_index][condition_index])
                                {
                                    is_invariant = true;
                                }
                                else
                                    is_invariant = false;
                            }
                            else
                                is_invariant = false;
                            if (is_invariant)
                            {
                                condition_index = this->is_Operand_Duplicate_In_Loop(s->u.base.src2, detailed_rd, loop_index);
                                if (condition_index != -1)
                                {
                                    if (condition_index == -2 ||this->loop_invariants[loop_index][condition_index])
                                        is_invariant = true;
                                    else
                                        is_invariant = false;
                                }
                                else
                                    is_invariant = false;
                            }
                            if (!changed)
                                changed = is_invariant;
                            
                            this->loop_invariants[loop_index][instruction_line] = is_invariant;
                            break;
                        }
                        default:
                        break;
                    }

                    
                }
                 //2 is the current instruction a definition? if not, the detailed_rd remains the same
                // if is definition, the detailed_rd needs to be changed
                //this is done after operation
                if (this->definition_map.find(instruction_line) != definition_map.end())
                {
                    //alter the detailed_rd bitvector
                    simple_instr* current_instruction = (simple_instr*) this->anti_line_hash_map[instruction_line];
                    for (int i = 0; i<this->ndefinition; i++)
                    {
                        if (!detailed_rd[i])
                            continue;
                        
                        int checkpoint_instruction_line = this->anti_definition_map[i+1];
                        simple_instr* checkpoint = (simple_instr*) this->anti_line_hash_map[checkpoint_instruction_line];
                        
                        if (checkpoint->u.base.dst == current_instruction->u.base.dst)
                            detailed_rd[i] = false;
                        
                                               
                    }
                    //add the current instruction definition
                    int current_definition_index = this->definition_map[instruction_line];
                    detailed_rd[current_definition_index-1] = true;
                }
                
                /*
                //cout<<"detailed_rd"<<block_index<<" ";
                for (int i = 0; i<this->ndefinition; i++)
                {
                    
                    if (detailed_rd[i])
                        //cout<<this->anti_definition_map[i+1]<<" ";
                }
                //cout<<endl;
                 */
                //**********************************************************
                
                
                s = s->next;
            }
                       
            
            
            
            
            
            

            
            
        }
    }
}
    
void LICM::find_Loop_Invariants(int loop_index)
{
    this->find_Constant_Loop_Invariants(loop_index);
    ////cout<<"constant complete"<<endl;
    this->find_Extended_Loop_Invariants(loop_index);
                        
}
                        
                        
void LICM::find_Loop_Invariants()
{
    //cout<<endl;
    //cout<<"start "<<endl;
    this->loop_invariants = new bool*[this->nloop];//0 entry, real instructions start from 1
    for (int i = 0; i<this->nloop; i++)
    {
        this->loop_invariants[i] = new bool[this->ninstr+2];
        for (int j = 0; j<this->ninstr+2; j++)
        {
            this->loop_invariants[i][j] = false;
        }
        
    }
    
    for (int i = 0; i<this->nloop; i++)
        find_Loop_Invariants(i);
}

                        
void LICM::print_Loop_Invariants()
{
    cout<<"loop_invariants "<<this->proc_name<<" "<<this->nloop<<" "<<this->ninstr<<endl;
    for (int i = 0; i<this->nloop; i++)
    {
        //loop_graph starts from 0;print real instructions
        cout<<"loop"<<i<<" "<<endl;
        int count = 0;
        for (int j = 1; j<=this->ninstr; j++)
        {
            if (this->loop_invariants[i][j])
                cout<<"instr"<<++count<<" "<<j<<endl;
        }
        cout<<endl;
    }
}

bool LICM::check_domi(int down, int up)
{
    ArcNode *p = this->proper_dominator_graph[down].firstarc;
    while (p) {
        if (up == p->adjvex)
            return true;
        p = p->nextarc;
    }
    return false;
}

//1. BB s dominates all exits of L
bool LICM::def_Block_Dominate_Exit(simple_instr* s, int loop_index)
{
    int instruction_line = this->line_hash_map[(int)s];
    int block_index = this->belongtowhichblock[instruction_line];
    
    //iterater blocks of the loop to check its successors that are not in the loop;
    //exits
    
    
    bool satifify_first = false;
    for (std::vector<int>::iterator it = this->loop_member[loop_index].begin(); it!=loop_member[loop_index].end(); ++it)
    {
        int checkbox = *it;
        
        ArcNode *p = this->graph[checkbox].firstarc;
        while (p)
        {
            //successor not in loop, one exit
            if (!this->is_inside_loop[loop_index][p->adjvex])
                //not dominate the exit, return false immediately
                if (!check_domi(p->adjvex, block_index))
                    return false;
            p = p->nextarc;
        }
    }
    return true;
}

//2. v is not defined elsewhere in L &&
// true for duplicate, false for not duplicate
bool LICM::is_definition_duplicate(simple_reg* r, int loop_index)
{
    int count = 0;
    for (std::vector<int>::iterator it = this->loop_member[loop_index].begin(); it!=this->loop_member[loop_index].end(); ++it)
    {
        //cout<<"member "<<*it<<endl;
        simple_instr* s = this->graph[*it].data.leader;
        
        for (int i = 0; i<this->graph[*it].data.ninstr; i++)
        {
            if (s->u.base.dst == r)
            {
                count++;
                //cout<<*it<<this->line_hash_map[(int)s]<<" "<<r->num<<endl;
            }
            if (count>1) return true;
            s = s->next;
        }
    }
    return false;
}

//3.
bool LICM::is_single_reaching_definition(simple_reg* v, int v_block_index, int loop_index)
{
    //1.check use_def_chain with single node, then check whether src is v
    //2.check block of definition v dominates of the current block
    
    for (std::vector<int>::iterator it = this->loop_member[loop_index].begin(); it!=loop_member[loop_index].end(); ++it)
    {
        int block_index = *it;
        
        simple_instr* s = this->graph[block_index].data.leader;
        
        for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
        {
            int instruction_line = this->anti_line_hash_map[(int)s];
            ArcNode *p = this->use_def_chain[0][instruction_line].firstarc;
            if (p != NULL)
            {
                if (p->nextarc == NULL)//only one child node
                {
                    if (s->u.base.src1 == v)
                    {
                        //check domination, if one fails, return false
                        if (!check_domi(block_index, v_block_index))
                            return false;
                        
                    }
                }
            }
            p = this->use_def_chain[1][instruction_line].firstarc;
            if (p != NULL)
            {
                if (p->nextarc == NULL)//only one child node
                {
                    if (s->u.base.src2 == v)
                    {
                        //check domination, if one fails, return false
                        if (!check_domi(block_index, v_block_index))
                            return false;
                        
                    }
                }
            }
            s = s->next;
        }
    }
    
    
    return true;
}


//4. temperol register should reside in the same block, if move, check
//the use of it can be moved too
bool LICM::is_instr_movable(simple_instr* s, int loop_index, int block_index)
{
    simple_instr* checkpoint = s;
    int line = this->line_hash_map[(int)s];
    
    int step = 0;
    while (checkpoint!=this->graph[block_index].data.tail) {
        checkpoint = checkpoint->next;
        step++;
    }
    step++;
    
    checkpoint = s;
    for (int i = 0; i<step; i++)
    {
        //cout<<i<<endl;
        //don't check self
        if (checkpoint == s)
        {
            checkpoint = checkpoint->next;
            continue;
        }
        
        int instruction_line = this->line_hash_map[(int)checkpoint];
        
        switch (checkpoint->opcode)
        {
                //load constant, must be put into loop_invariant
            case LDC_OP:
            {
                //this will never happen in this step
                break;
            }
                //one operand
            case LOAD_OP:
            case CPY_OP:
            case CVT_OP:
            case NEG_OP:
            case NOT_OP:
            {
                
                if (checkpoint->u.base.src1 == s->u.base.dst)
                {
                    
                    if (move_list[instruction_line])
                        move_list[line] = true;
                    else
                    {
                        move_list[line] = false;
                        //cout<<"set move a"<<line<<endl;
                        return false;
                    }
                }
                break;
            }
            case STR_OP:
            {
                if (checkpoint->u.base.src2 == s->u.base.dst)
                {
                    
                    if (move_list[instruction_line])
                        move_list[line] = true;
                    else
                    {
                        move_list[line] = false;
                        //cout<<"set move a"<<line<<endl;
                        return false;
                    }
                }

                break;
            }
                //two operands:
            case ADD_OP:
            case SUB_OP:
            case MUL_OP:
            case DIV_OP:
            case REM_OP:
            case MOD_OP:
            case AND_OP:
            case IOR_OP:
            case XOR_OP:
            case ASR_OP:
            case LSR_OP:
            case LSL_OP:
            case ROT_OP:
            case SEQ_OP:
            case SNE_OP:
            case SL_OP:
            case SLE_OP:
            {
                
                if (checkpoint->u.base.src1 == s->u.base.dst)
                {
                        if (move_list[instruction_line])
                        move_list[line] = true;
                    else
                    {
                        move_list[line] = false;
                        return false;
                    }
                }
                if (checkpoint->u.base.src2 == s->u.base.dst)
                {
                    
                    if (move_list[instruction_line])
                        move_list[line] = true;
                    else
                    {
                        move_list[line] = false;
                        return false;
                    }
                }
                break;
            }
            default:
                break;
        }
        checkpoint = checkpoint->next;
    }

    return true;
    
}

void LICM::grow_instr_vector(int t)
{
    simple_instr* s = this->graph[t].data.leader;
    for (int i = 0; i<this->graph[t].data.ninstr; i++)
    {
        this->instruction_list.push_back(this->line_hash_map[(int)s]);
        s = s->next;
    }
}


void LICM::BFS(LoopG G, int root)
{
    __gnu_cxx::hash_map<int, bool> mark;
    queue<int> Q;
    Q.push(root);
    mark[root] = true;
    
    while (!Q.empty()) {
        int t = Q.front();
        Q.pop();
        this->grow_instr_vector(t);
        
        //check for successors of block t
        ArcNode *p = G.firstarc;
        while (p)
        {
            if (mark.find(p->adjvex) == mark.end())
            {
                mark[p->adjvex] = true;
                Q.push(p->adjvex);
            }
            
            p = p->nextarc;
        }
    }
    
    
}


void LICM::replace_reg(simple_reg* orig, simple_reg* tar, int loop_index)
{
    for (std::vector<int>::iterator it = this->loop_member[loop_index].begin(); it!=loop_member[loop_index].end(); ++it)
    {
        int block_index = *it;
        simple_instr* s = this->graph[block_index].data.leader;
        
        for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
        {
            switch (s->opcode)
            {
                case LDC_OP:
                {
                    break;
                }
                    //one operand
                case LOAD_OP:
                case CPY_OP:
                case CVT_OP:
                case NEG_OP:
                case NOT_OP:
                {
                    if (s->u.base.src1 == orig)
                        s->u.base.src1 = tar;
                    break;
                }
                case STR_OP:
                {
                    if (s->u.base.src2 == orig)
                        s->u.base.src2 = tar;
                    break;
                }
                
                case ADD_OP:
                case SUB_OP:
                case MUL_OP:
                case DIV_OP:
                case REM_OP:
                case MOD_OP:
                case AND_OP:
                case IOR_OP:
                case XOR_OP:
                case ASR_OP:
                case LSR_OP:
                case LSL_OP:
                case ROT_OP:
                case SEQ_OP:
                case SNE_OP:
                case SL_OP:
                case SLE_OP:
                {
                    if (s->u.base.src1 == orig)
                        s->u.base.src1 = tar;
                    if (s->u.base.src2 == orig)
                        s->u.base.src2 = tar;
                    break;
                }
                    
                // SPECIAL
                case CALL_OP:
                {
                    unsigned n, nargs;
                    
                    /* print the list of arguments */
                    nargs = s->u.call.nargs;
                    if (nargs != 0) {
                        for (n = 0; n < nargs; n++) {
                            if (s->u.call.args[n] == orig)
                                s->u.call.args[n] = tar;
                            
                        }
                        
                    }
                    
                    
                    break;
                }
                //if & mbr
                case BTRUE_OP:
                case BFALSE_OP:
                {
                    
                    if (s->u.bj.src == orig)
                        s->u.bj.src = tar;
                    break;
                }
                case MBR_OP:
                {
                    if (s->u.mbr.src == orig)
                        s->u.mbr.src = tar;
                    
                    break;
                }

                case RET_OP:
                {
                    if (s->u.base.src1!=NULL)
                        if (s->u.base.src1 == orig)
                            s->u.base.src1 = tar;
                    break;
                }
                default:
                    break;
            }
            
            s = s->next;
        }
    }
}


void LICM::replace_reg(simple_reg* orig, simple_reg* tar, vector<simple_instr*> list)
{
    for (std::vector<simple_instr*>::iterator it = list.begin(); it!=list.end(); ++it)
    {
        simple_instr* s = *it;
        
        switch (s->opcode)
        {
            case LDC_OP:
            {
                break;
            }
                //one operand
            case LOAD_OP:
            case CPY_OP:
            case CVT_OP:
            case NEG_OP:
            case NOT_OP:
            {
                if (s->u.base.src1 == orig)
                    s->u.base.src1 = tar;
                break;
            }
            case STR_OP:
            {
                if (s->u.base.src2 == orig)
                    s->u.base.src2 = tar;
                break;
            }
                //two operands:
            case ADD_OP:
            case SUB_OP:
            case MUL_OP:
            case DIV_OP:
            case REM_OP:
            case MOD_OP:
            case AND_OP:
            case IOR_OP:
            case XOR_OP:
            case ASR_OP:
            case LSR_OP:
            case LSL_OP:
            case ROT_OP:
            case SEQ_OP:
            case SNE_OP:
            case SL_OP:
            case SLE_OP:
            {
                if (s->u.base.src1 == orig)
                    s->u.base.src1 = tar;
                if (s->u.base.src2 == orig)
                    s->u.base.src2 = tar;
                break;
            }
                
            //VERY SPECIAL
            case CALL_OP:
            {
                unsigned n, nargs;
                
                /* print the list of arguments */
                nargs = s->u.call.nargs;
                if (nargs != 0) {
                    for (n = 0; n < nargs; n++) {
                        if (s->u.call.args[n] == orig)
                            s->u.call.args[n] = tar;
                        
                    }
                    
                }
                
                
                break;
            }
            //if & mbr
            case BTRUE_OP:
            case BFALSE_OP:
            {
                
                if (s->u.bj.src == orig)
                    s->u.bj.src = tar;
                break;
            }

                
            case MBR_OP:
            {
                if (s->u.mbr.src == orig)
                    s->u.mbr.src = tar;
                
                break;
            }
                
            case RET_OP:
            {
                if (s->u.base.src1!=NULL)
                    if (s->u.base.src1 == orig)
                        s->u.base.src1 = tar;
                break;
            }
            default:
                break;
        }

        
    }
}


std::vector<simple_instr*> LICM::consolidate_list(simple_reg *r, int loop_index)
{
    vector<simple_instr*> list;
    ArcNode *p = this->loop_graph[loop_index].firstarc;
    while (p) {
        int block_index = p->adjvex;
        simple_instr* s = this->graph[block_index].data.leader;
        
        for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
        {
            switch (s->opcode)
            {
                case LDC_OP:
                {
                    break;
                }
                    //one operand
                case LOAD_OP:
                case CPY_OP:
                case CVT_OP:
                case NEG_OP:
                case NOT_OP:
                {
                    if (s->u.base.src1 == r)
                        list.push_back(s);
                    break;
                }
                case STR_OP:
                {
                    if (s->u.base.src2 == r)
                        list.push_back(s);
                    break;
                }
                    //two operands:
                case ADD_OP:
                case SUB_OP:
                case MUL_OP:
                case DIV_OP:
                case REM_OP:
                case MOD_OP:
                case AND_OP:
                case IOR_OP:
                case XOR_OP:
                case ASR_OP:
                case LSR_OP:
                case LSL_OP:
                case ROT_OP:
                case SEQ_OP:
                case SNE_OP:
                case SL_OP:
                case SLE_OP:
                {
                    if (s->u.base.src1 == r||s->u.base.src2 == r)
                        list.push_back(s);
                    break;
                }
                    
                    //VERY SPECIAL
                case CALL_OP:
                {
                    unsigned n, nargs;
                    
                    /* print the list of arguments */
                    nargs = s->u.call.nargs;
                    if (nargs != 0) {
                        for (n = 0; n < nargs; n++) {
                            if (s->u.call.args[n] == r)
                                list.push_back(s);
                            
                        }
                        
                    }
                    
                    
                    break;
                }
                    //if & mbr
                case BTRUE_OP:
                case BFALSE_OP:
                {
                    
                    if (s->u.bj.src == r)
                         list.push_back(s);
                    break;
                }
                    
                    
                case MBR_OP:
                {
                    if (s->u.mbr.src == r)
                         list.push_back(s);
                    
                    break;
                }

                case RET_OP:
                {
                    if (s->u.base.src1!=NULL)
                        if (s->u.base.src1 == r)
                            list.push_back(s);
                    break;
                }
                default:
                    break;
            }
            
            s = s->next;

        }
        
        p = p->nextarc;
    }
    return list;
}

//if the block in the loop uses the TEMP_REG, we need to create a pseudo register in the preheader to
//replace it.
void LICM::consolidate_preheader(int p_index, int loop_index)
{
    //use this to encapsulate the original instructions of the preheader before modification
    vector<simple_instr*> ins;
    simple_instr* s = this->graph[p_index].data.leader;
    for (int i = 0; i<this->graph[p_index].data.ninstr; i++)
    {
        ins.push_back(s);
        s = s->next;
    }
    //this num of definitions to deal with
    for (std::vector<simple_instr*>::iterator it = ins.begin(); it!=ins.end(); ++it)
    {
        s = *it;
        //now look for TEMP_REG
        switch (s->opcode) {
            case LOAD_OP:
            case CPY_OP:
            case CVT_OP:
            case LDC_OP:
            case NEG_OP:
            case ADD_OP:
            case SUB_OP:
            case MUL_OP:
            case DIV_OP:
            case REM_OP:
            case MOD_OP:
            case NOT_OP:
            case AND_OP:
            case IOR_OP:
            case XOR_OP:
            case ASR_OP:
            case LSR_OP:
            case LSL_OP:
            case ROT_OP:
            case SEQ_OP:
            case SNE_OP:
            case SL_OP:
            case SLE_OP:
            {
                if (s->u.base.dst->kind == TEMP_REG)
                {
                    vector<simple_instr*> list = this->consolidate_list(s->u.base.dst, loop_index);
                    if (list.size()!=0)
                    {
                        
                        simple_instr* cpy_i = new_instr(CPY_OP, s->u.base.dst->var->type);
                        cpy_i->u.base.src1 = s->u.base.dst;
                        //createa a pseudo register
                        simple_reg* cpy_r = new_register(s->u.base.dst->var->type, PSEUDO_REG);
                        cpy_i->u.base.dst = cpy_r;
                        cout<<"replacing"<<list.size()<<endl;
                        this->replace_reg(s->u.base.dst, cpy_r, list);
                        
                        //insert
                        cpy_i->next = s->next;
                        cpy_i->prev = s;
                        s->next->prev = cpy_i;
                        s->next = cpy_i;
                        this->graph[p_index].data.ninstr++;
                    }
                    
                    
                    
                }
 
                break;
            }
            default:
                break;
        }
    }
}

void LICM::move_instr_preheader(simple_instr* s, int block_index, int loop_index)
{
    simple_instr* ref = this->graph[block_index].data.leader;
    ref = this->graph[block_index].data.leader;
//    cout<<"before "<<this->graph[block_index].data.ninstr<<endl;
//    for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
//    {
//        cout<<simple_op_name(ref->opcode)<<endl;
//        ref = ref->next;
//    }
    
    
    //remove s from original place
    //watch if s is the last instruction
    simple_instr* pre = s->prev;
    if (s->next == NULL)
    {
        pre->next = NULL;
    }
    
    else
    {
        pre->next = s->next;
        s->next->prev = pre;
    }

    //now move s to preheader the last instruction
    //move ref to tail of the block and insert
    ref = this->graph[block_index].data.leader;
    for (int i = 0; i<this->graph[block_index].data.ninstr-1; i++)
        ref = ref->next;
    s->next = ref->next;
    s->prev = ref;
    ref->next->prev = s;
    ref->next = s;
    
    this->graph[block_index].data.ninstr++;
    
//    //if s->u.base.dst is temperal register, we need to create a pseudo register and make a copy
//    //instruction, then replace all the temperal registers in that block
//    
//    if (s->u.base.dst->kind == TEMP_REG)
//    {
//        simple_instr* cpy_i = new_instr(CPY_OP, s->u.base.dst->var->type);
//        cpy_i->u.base.src1 = s->u.base.dst;
//        //createa a pseudo register
//        simple_reg* cpy_r = new_register(s->u.base.dst->var->type, PSEUDO_REG);
//        cpy_i->u.base.dst = cpy_r;
//        //insert cpy_i
//        //the tail is s
//        cpy_i->next = s->next;
//        cpy_i->prev = s;
//        s->next->prev = cpy_i;
//        s->next = cpy_i;
//        this->graph[block_index].data.ninstr++;
//        //now replace the src in source_block_index to cpy_r;
//        this->replace_reg(s->u.base.dst, cpy_r, loop_index);
//        
//    }
    
    
    
        
}

bool LICM::rd_inside_move_list(simple_reg* r, bool* detailed_rd, int loop_index)
{
    for (int i = 0; i<this->ndefinition; i++)
    {
        if (!detailed_rd[i])
            continue;
        int instruction_line = this->anti_definition_map[i+1];
        int block_index = this->belongtowhichblock[instruction_line];
        simple_instr* s = (simple_instr*) this->anti_line_hash_map[instruction_line];
        
        if (s->u.base.dst == r)
        {
            bool is_outside = !this->is_inside_loop[loop_index][block_index];
            
            //is in the move list OR is defined outside the loop
            if (!this->move_list[instruction_line]&&!is_outside)
                return false;
        }
    }
    return true;
}

//4.
void LICM::consolidate_move_list(int loop_index)
{
    bool changed = true;
    while(changed)
    {
        changed = false;
        for (std::vector<int>::iterator it=loop_member[loop_index].begin(); it!=loop_member[loop_index].end(); ++it)
        {
            int block_index = *it;
            simple_instr* s = this->graph[block_index].data.leader;
            /***note that the definition index starts from 1**/
            //initialize the detailed_rd to rd_in
            bool detailed_rd[this->ndefinition];
            for (int i = 0; i<this->ndefinition; i++)
                detailed_rd[i] = this->rd_in[block_index][i];
            
            for (int i = 0; i<this->graph[block_index].data.ninstr; i++)
            {
                int instruction_line = this->line_hash_map[(int)s];
                
                //only consider instructions of conditions 1.2.3
                if (!this->move_list[instruction_line])
                {
                    s = s->next;
                    continue;
                }
                
                switch (s->opcode)
                {
                        
                    case LDC_OP:
                    {
                        break;
                    }
                        //one operand
                    case LOAD_OP:
                    case CPY_OP:
                    case CVT_OP:
                    case NEG_OP:
                    case NOT_OP:
                    {
                        if (!rd_inside_move_list(s->u.base.src1, detailed_rd, loop_index))
                        {
                            move_list[instruction_line] = false;
                            changed = true;
                        }
                        break;
                    }
                    case STR_OP:
                    {
                        if (!rd_inside_move_list(s->u.base.src2, detailed_rd, loop_index))
                        {
                            move_list[instruction_line] = false;
                            changed = true;
                        }
 
                    }
                        //two operands:
                    case ADD_OP:
                    case SUB_OP:
                    case MUL_OP:
                    case DIV_OP:
                    case REM_OP:
                    case MOD_OP:
                    case AND_OP:
                    case IOR_OP:
                    case XOR_OP:
                    case ASR_OP:
                    case LSR_OP:
                    case LSL_OP:
                    case ROT_OP:
                    case SEQ_OP:
                    case SNE_OP:
                    case SL_OP:
                    case SLE_OP:
                    {
                        if (!rd_inside_move_list(s->u.base.src1, detailed_rd, loop_index)||!rd_inside_move_list(s->u.base.src2, detailed_rd, loop_index))
                        {
                            move_list[instruction_line] = false;
                            changed = true;
                        }
                        break;
                    }
                    default:
                        break;
                }
                
                
                
                //2 is the current instruction a definition? if not, the detailed_rd remains the same
                // if is definition, the detailed_rd needs to be changed
                //this is done after operation
                if (this->definition_map.find(instruction_line) != definition_map.end())
                {
                    //alter the detailed_rd bitvector
                    simple_instr* current_instruction = (simple_instr*) this->anti_line_hash_map[instruction_line];
                    for (int i = 0; i<this->ndefinition; i++)
                    {
                        if (!detailed_rd[i])
                            continue;
                        
                        int checkpoint_instruction_line = this->anti_definition_map[i+1];
                        simple_instr* checkpoint = (simple_instr*) this->anti_line_hash_map[checkpoint_instruction_line];
                        
                        if (checkpoint->u.base.dst == current_instruction->u.base.dst)
                            detailed_rd[i] = false;
                        
                        
                    }
                    //add the current instruction definition
                    int current_definition_index = this->definition_map[instruction_line];
                    detailed_rd[current_definition_index-1] = true;
                }
                
                /*
                 //cout<<"detailed_rd"<<block_index<<" ";
                 for (int i = 0; i<this->ndefinition; i++)
                 {
                 
                 if (detailed_rd[i])
                 //cout<<this->anti_definition_map[i+1]<<" ";
                 }
                 //cout<<endl;
                 */
                //**********************************************************
                
                
                s = s->next;
                
                
                
            }
        }
    }
    
}

void LICM::build_move_list(int loop_index)
{
    
    
    
    vector<int> member = this->loop_member[loop_index];
    BFS(loop_graph[loop_index], member[0]);

    //instruction list the the BFS loop list
    for (std::vector<int>::iterator it = this->instruction_list.begin(); it!=this->instruction_list.end(); ++it)
    {
        simple_instr* s= (simple_instr*)this->anti_line_hash_map[*it];
        //is loop invariant
        if (this->loop_invariants[loop_index][*it])
        {
            
            //satisfy 1.2.3 conditions
            //1.dominates exits
            
            if (def_Block_Dominate_Exit(s, loop_index))
            {
                
                //2. no duplicate
                if (!is_definition_duplicate(s->u.base.dst, loop_index))
                {
                    if (is_single_reaching_definition(s->u.base.dst, this->belongtowhichblock[*it], loop_index))
                    {
                                                
                        this->move_list[*it] = true;
                        
                                                
                    }
                }
            }
        }
    }

    //now consolidate move_list
    //
    cout<<"beforeloop"<<loop_index<<endl;
    for (int i = 1; i<=this->ninstr; i++)
    {
        if (this->move_list[i])
            cout<<"move_list "<<i<<endl;
    }

    this->consolidate_move_list(loop_index);

}

void LICM::code_Motion(int loop_index)
{
    //do the BFS search
    //check the loop invariant instructions
    //check satisfying the 1.2.3 conditions
    //move the instruction to the preheader
       
//    for (std::vector<int>::iterator it = instruction_list.begin(); it!=instruction_list.end(); ++it)
//    {
//        cout<<*it<<endl;
//    }
//    this->build_move_list(loop_index);
//    vector<int> member = this->loop_member[loop_index];
//    for (std::vector<int>::iterator it = this->instruction_list.begin(); it!=this->instruction_list.end(); ++it)
//    {
//        if (move_list[*it])
//        {
//            simple_instr* s = (simple_instr*)this->anti_line_hash_map[*it];
//            cout<<"moving instr"<<*it<<" "<<member[0]-1<<" "<<s->u.base.dst->num<<"  "<<simple_op_name(s->opcode)<<endl;
//            this->move_instr_preheader(s, member[0]-1);
//        }
//    }
    
    vector<int> member = this->loop_member[loop_index];
    
    this->move_list = new bool[this->ninstr+2];
    for (int i = 0; i<this->ninstr+2; i++)
        this->move_list[i] = false;
    this->build_move_list(loop_index);
    
    cout<<"after_loop"<<loop_index<<endl;
    for (int i = 1; i<=this->ninstr; i++)
    {
        if (this->move_list[i])
            cout<<"move_list "<<i<<endl;
    }
    //cout<<"Here"<<endl;
    int preheader_block_index = member[0]-1;
    for (int i = 1; i<=this->ninstr; i++)
    {
        
         if (this->move_list[i])
         {
             cout<<i<<endl;
             
             simple_instr* s = (simple_instr* )this->anti_line_hash_map[i];
             move_instr_preheader(s, preheader_block_index, loop_index);
             int current_block_index = this->belongtowhichblock[i];
             this->graph[current_block_index].data.ninstr--;
             
         }
    }
    
    this->consolidate_preheader(preheader_block_index, loop_index);
//    
//    BFS(loop_graph[loop_index], member[0]);
//    
//    //instruction list the the BFS loop list
//    for (std::vector<int>::iterator it = this->instruction_list.begin(); it!=this->instruction_list.end(); ++it)
//    {
//        simple_instr* s= (simple_instr*)this->anti_line_hash_map[*it];
//        //is loop invariant
//        if (this->loop_invariants[loop_index][*it])
//        {
//            
//            //satisfy 1.2.3 conditions
//            //1.dominates exits
//            
//            if (def_Block_Dominate_Exit(s, loop_index))
//            {
//                
//                //2. no duplicate
//                if (!is_definition_duplicate(s->u.base.dst, loop_index))
//                {
//                    if (is_single_reaching_definition(s->u.base.dst, this->belongtowhichblock[*it], loop_index))
//                    {
//                        //move the instruction to header
//                        
//                        //move_list[*it] = true;
//                        
//                        int preheader_block_index = member[0]-1;
//                        //
//                        //                        cout<<"reach instr"<<*it<<" "<<preheader_block_index<<" "<<s->u.base.dst->num<<"  "<<simple_op_name(s->opcode)<<endl;
//                        move_instr_preheader(s, preheader_block_index, this->belongtowhichblock[*it]);
//                        
//                    }
//                }
//            }
//        }
//    }

    
    
    
    
    
    //now use the instruction_list
        
    
}

void LICM::code_Motion()
{
    //code motion from inside to outside
    if (this->nloop == 0)
        return;
    vector<int> order;
    bool finish[this->nloop];
    for (int i = 0; i<this->nloop; i++)
    {
        finish[i] = false;
    }
    
     
    //order by the entry of the loop
    
    for (int i = 0; i<this->nloop; i++)
    {
        int front = 100000;
        int front_index = 0;
        
        for (int j = 0; j<this->nloop; j++)
        {
            if (finish[j])
                continue;
            vector<int>member = this->loop_member[j];
            if (front>member[0])
            {
                front = member[0];
                front_index = j;
            }
        }
        finish[front_index] = true;
        order.push_back(front_index);
    }
    
    for (int i = 0; i<this->nloop; i++)
    {
        vector<int>member = this->loop_member[order[i]];
        cout<<"loop_reverse_order"<<order[i]<<member[0]<<endl;
    }
    int pass = 0;
    for (pass = 0; pass<1; pass++)
    {
//        code_Motion(0);
        for (int i = this->nloop-1 ; i>=0; i--)
            code_Motion(order[i]);
    }
//        code_Motion(2);
//    code_Motion(0);
//    code_Motion(1);
//    code_Motion(0);
//    code_Motion(1);
    
    
//    simple_instr* s = this->inlist;
//    while (s) {
//        simple_reg* r = s->u.base.dst;
//        if (r)
//        {
//            if ((r != NO_REGISTER) && (r->kind == TEMP_REG)) {
//                
//                /* make sure that the register has been defined */
//                alist_e *a = nr_stat.search((void *)r->num);
//                if (!a) simple_error("temporary register used before defined");
//            }
//        }
//        s = s->next;
//    }
    
}


void LICM::print_Code_Motion()
{
//    cout<<endl<<"domi"<<endl;
//    ArcNode *p;
//    for (int i = 0; i<this->nblk+2; i++)
//    {
//        cout<<"blk"<<i<<" ";
//        p = this->proper_dominator_graph[i].firstarc;
//        while (p) {
//            cout<<p->adjvex<<" ";
//            p = p->nextarc;
//        }
//        cout<<endl;
//    }
    
    
}
    
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
