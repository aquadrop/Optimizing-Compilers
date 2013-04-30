//
//  DataFlow.cpp
//  A3
//
//  Created by Jianwei Xu on 13/02/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#include "DataFlow.h"

DataFlow::DataFlow()
{
    
}

DataFlow::DataFlow(Graph* cfg, char* proc_name)
{
    this->inlist = cfg->getInlist();
    this->graph = cfg->getGraph().first;
    this->antigraph = cfg->getGraph().second;
    this->proc_name = proc_name;
    
    this->nblk = cfg->get_Nblk();
    
    this->line_hash_map = cfg->getInlistLineHash();
    this->anti_line_hash_map = cfg->get_Anti_Line_Hash();
    this->belongtowhichblock = cfg->getBelongtoWhichBlock();
    this->ninstr = cfg->get_ninstr();
    /***debug****
    cout<<graph[1].data.ninstr<<endl;
    simple_instr *s = this->inlist;
    while(s)
    {
        cout<<simple_op_name(s->opcode)<<endl;
        s = s->next;
    }
     */

}


/***********find variable list**************/
/***********find variable list**************/
/***********find variable list**************/
void DataFlow::find_Variable_List()
{
    simple_instr* s = this->inlist;
    this->nvar = 0;
    while(s)
    {
        /**skip pseudo register**/
        //cout<<simple_op_name(s->opcode)<<endl;
        switch (s->opcode)
        {
                
            case LOAD_OP:
            {
                this->fill_Hash_Table(s->u.base.dst);
                this->fill_Hash_Table(s->u.base.src1);
                break;
            }
                
            case STR_OP:
            {
                this->fill_Hash_Table(s->u.base.src1);
                this->fill_Hash_Table(s->u.base.src2);
                break;
            }
                
            case MCPY_OP:
            {
                this->fill_Hash_Table(s->u.base.src1);
                this->fill_Hash_Table(s->u.base.src2);
                break;
            }
                
            case LDC_OP:
            {
                this->fill_Hash_Table(s->u.ldc.dst);
                break;
            }
                
            case JMP_OP:
            {
                break;
            }
                
            case BTRUE_OP:
            case BFALSE_OP:
            {
                this->fill_Hash_Table(s->u.bj.src);
                break;
            }
                
            case CALL_OP:
            {
                unsigned n, nargs;
                
                if (s->u.call.dst != NO_REGISTER)
                {
                    this->fill_Hash_Table(s->u.call.dst);
                }
                //this->fill_Hash_Table(s->u.call.proc);
                
                nargs = s->u.call.nargs;
                if (nargs != 0)
                {
                    for (n = 0; n < nargs; n++)
                    {
                        this->fill_Hash_Table(s->u.call.args[n]);
                    }
                }
                break;
            }
                
            case MBR_OP:
            {
                unsigned n, ntargets;
                
                this->fill_Hash_Table(s->u.mbr.src);
                break;
            }
                
            case LABEL_OP:
            {
                break;
            }
                
            case RET_OP:
            {
                if (s->u.base.src1 != NO_REGISTER)
                {
                    this->fill_Hash_Table(s->u.base.src1);
                }
                break;
            }
                
            case CVT_OP:
            case CPY_OP:
            case NEG_OP:
            case NOT_OP:
            {
                /* unary base instructions */
                this->fill_Hash_Table(s->u.base.dst);
                this->fill_Hash_Table(s->u.base.src1);
                break;
            }
                
            default:
            {
                /* binary base instructions */
                this->fill_Hash_Table(s->u.base.dst);
                this->fill_Hash_Table(s->u.base.src1);
                this->fill_Hash_Table(s->u.base.src2);
            }
        }

        /***************/
        s = s->next;
        /***************/
    }
    
    
    
}

void DataFlow::fill_Hash_Table(simple_reg *r)
{
    if (!this->valid_Reg(r)) return;
    int var_reg = this->retrive_Var_Register(r);
    if (this->anti_var_list.find(var_reg)!=anti_var_list.end()) return;
    this->nvar++;
    this->anti_var_list[var_reg] = this->nvar;
    var_list.push_back(var_reg);
}



int DataFlow::retrive_Var_Register (simple_reg *r)
{
    return r->num;
}

bool DataFlow::valid_Reg(simple_reg* r)
{
    if (r->kind == PSEUDO_REG) return true;//what we want is pseudo reg
    else return false;
}


void DataFlow::print_Variable_List()
{
    cout<<"variables "<<this->proc_name<<" "<<this->nvar<<endl;
    for (int i = 1; i<=this->nvar; i++)
    {
        cout<<"var "<<i<<" "<<TAG<<var_list[i-1]<<endl;
    }
}

/***********find variable list ends**************/
/***********find variable list ends**************/
/***********find variable list ends**************/


/***********find live variables DEF sets**********/
/***********find live variables DEF sets**********/
/***********find live variables DEF sets**********/

void DataFlow::find_Live_DEF()
{
    /*initialize the live_DEF matrix*/
    this->live_DEF = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->live_DEF[i] = new bool[this->nvar];
        for (int j = 0; j<this->nvar; j++)
            this->live_DEF[i][j] = false;
    }
    
    /*find dst-pseudo register, place true*/
    simple_instr *s;
    for (int i = 1; i<=this->nblk; i++)
    {
        s = this->graph[i].data.leader;
        //cout<<this->graph[i].data.ninstr<<" ";
        for (int j = 0; j<this->graph[i].data.ninstr; j++)
        {
            //cout<<i<<" "<<j<<" "<<this->nblk<<" "<<this->graph[i].data.ninstr<<" "<<
            //simple_op_name(s->opcode)<<endl;
            switch (s->opcode)
            {
                case LOAD_OP:
                case CVT_OP:
                case CPY_OP:
                case NEG_OP:
                case NOT_OP:
                {
                    this->fill_Live_DEF(i,s->u.base.dst);
                    s = s->next;
                    break;
                }
                case LDC_OP:
                {
                    this->fill_Live_DEF(i,s->u.ldc.dst);
                    s = s->next;
                    break;
                }
                case CALL_OP:
                {
                    if (s->u.call.dst != NO_REGISTER)
                    {
                        this->fill_Live_DEF(i,s->u.call.dst);
                    }
                    s = s->next;
                    break;
                }
                    
                case STR_OP:
                case MCPY_OP:
                case JMP_OP:
                case BTRUE_OP:
                case BFALSE_OP:
                case MBR_OP:
                case LABEL_OP:
                {
                    s = s->next;
                    break;
                }
                case RET_OP:
                    break;
                default:
                {
                    /*be careful the rear of inlist will fall into default*/
                    this->fill_Live_DEF(i,s->u.base.dst);
                    s = s->next;
                    break;
                }
            }
        }
    }
}

void DataFlow::fill_Live_DEF(int i, simple_reg* r)
{
    if (!this->valid_Reg(r))
        return;
    int index = anti_var_list[this->retrive_Var_Register(r)]-1;
    //cout<<"Register: "<<this->retrive_Var_Register(r)<<endl;
    this->live_DEF[i][index] = true;
}

void DataFlow::print_Live_DEF()
{
    cout<<"def_lv_sets "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"def_lv "<<i<<" ";
        for (int j = 0; j<this->nvar; j++)
            cout<<this->live_DEF[i][j];
        cout<<endl;
    }
}

/***********find live variables DEF sets ends**********/
/***********find live variables DEF sets ends**********/
/***********find live variables DEF sets ends**********/


/***********find live variables USE **********/
/***********find live variables USE **********/
/***********find live variables USE **********/

void DataFlow::find_Live_USE()
{
    /*if a variable is used, it will remain used*/
    int **live_USE_int;
    
    this->live_USE = new bool*[this->nblk+2];
    live_USE_int = new int*[this->nblk+2];

    for (int i = 0; i<this->nblk+2; i++)
    {
        this->live_USE[i] = new bool[this->nvar];
        live_USE_int[i] = new int[this->nvar];
        for (int j = 0; j<this->nvar; j++)
        {
            this->live_USE[i][j] = false;
            live_USE_int[i][j] = -1;
        }
        
    }
    
    /**three states, -1, 0, 1, only -1 status can be altered**/
    /** check right then check left**/
    simple_instr *s;
    
    for (int i = 1; i<=this->nblk; i++)
    {
        s = this->graph[i].data.leader;
        for (int j = 0; j<this->graph[i].data.ninstr; j++)
        {
            switch (s->opcode)
            {
                    
                case LOAD_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.dst, false);
                    s = s->next;
                    break;
                }
                    
                case STR_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src2, true);
                    s = s->next;
                    break;
                }
                    
                case MCPY_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src2, true);
                    s = s->next;
                    break;
                }
                    
                case LDC_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.ldc.dst, false);
                    s = s->next;
                    break;
                }
                    
                case JMP_OP:
                {
                    //cout<<"block "<<i<<endl;
                    s = s->next;
                    break;
                }
                    
                case BTRUE_OP:
                case BFALSE_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.bj.src, true);
                    s = s->next;
                    break;
                }
                    
                case CALL_OP:
                {
                    //cout<<"block "<<i<<endl;
                    unsigned n, nargs;
                    
                    if (s->u.call.dst != NO_REGISTER)
                    {
                        this->fill_Live_USE(i, live_USE_int, s->u.call.dst,false);
                    }
                    //this->fill_Hash_Table(s->u.call.proc);
                    
                    nargs = s->u.call.nargs;
                    if (nargs != 0)
                    {
                        for (n = 0; n < nargs; n++)
                        {
                            this->fill_Live_USE(i, live_USE_int, s->u.call.args[n], true);
                        }
                    }
                    s = s->next;
                    break;
                }
                    
                case MBR_OP:
                {
                    //cout<<"block "<<i<<endl;
                    this->fill_Live_USE(i, live_USE_int, s->u.mbr.src, true);
                    s = s->next;
                    break;
                }
                    
                case LABEL_OP:
                {
                    //cout<<"block "<<i<<endl;
                    s = s->next;
                    break;
                }
                    
                case RET_OP:
                {
                    //cout<<"block "<<i<<endl;
                    if (s->u.base.src1 != NO_REGISTER)
                    {
                        this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    }
                    break;
                }
                    
                case CVT_OP:
                case CPY_OP:
                case NEG_OP:
                case NOT_OP:
                {
                    //cout<<"block "<<i<<endl;
                    /* unary base instructions */
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.dst, false);
                    s = s->next;
                    break;
                }
                    
                default:
                {
                    //cout<<"block "<<i<<endl;
                    /* binary base instructions */
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src1, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.src2, true);
                    this->fill_Live_USE(i, live_USE_int, s->u.base.dst, false);
                    s = s->next;
                }
            }

        }
    }

    
    /*finalize live_USE to 0, 1*/
    for (int i = 0; i<this->nblk+2; i++)
    {
        for (int j = 0; j<this->nvar; j++)
            if (live_USE_int[i][j] == 1)
                this->live_USE[i][j] = true;
    }
    
    
}

void DataFlow::fill_Live_USE(int i, int** live_USE_int, simple_reg* r, bool used)
{
    /*
    cout<<"---------------- "<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        for (int j = 0; j<this->nvar; j++)
        {
            cout<<live_USE_int[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<"---------------- "<<endl;
     */

    /*only -1 state can be altered*/
    if (!this->valid_Reg(r))
    {
        //cout<<"not pseudo "<<i<<endl;
        return;
    }
    
    int index = anti_var_list[this->retrive_Var_Register(r)]-1;
    if (live_USE_int[i][index]!=-1)
    {
        //cout<<"already fixed "<<i<<endl;
        return;
    }
    
    //cout<<"Register: "<<this->retrive_Var_Register(r)<<endl;
    //cout<<"going to alter "<<i<<" "<<index<<" "<<(int)used<<endl;
    *(*(live_USE_int+i)+index) = (int)used;
}

void DataFlow::print_Live_USE()
{
    cout<<"use_lv_sets "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"use_lv "<<i<<" ";
        for (int j = 0; j<this->nvar; j++)
            cout<<this->live_USE[i][j];
        cout<<endl;
    }

}

/***********find live variables USE ends**********/
/***********find live variables USE ends**********/
/***********find live variables USE ends**********/

/***********find live**********/
/***********find live**********/
/***********find live**********/

/*Set of live variables at the end of block*/
void DataFlow::find_Live()
{
    this->livein = new bool*[this->nblk+2];
    this->liveout = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->livein[i] = new bool[this->nvar];
        this->liveout[i] = new bool[this->nvar];
        for (int j = 0; j<this->nvar; j++)
        {
            this->livein[i][j] = this->live_USE[i][j];
            this->liveout[i][j] = false;
        }
    }
    
    this->backward_Anyway_Solver(this->livein, this->live_USE, this->liveout,
                                 this->live_DEF, this->nblk+2, this->nvar);
}

//in, out should have already been initialized
void DataFlow::backward_Anyway_Solver(bool** in, bool**gen,
                                      bool**out, bool**minus, int rows, int cols)
{
    int I = rows;
    int J = cols;
    
    bool *oldin = new bool[J];
    bool changed = true;
    int i = 0;
    while (changed)
    {
        //cout<<"***********"<<i++<<endl;
        
        
        changed = false;
        
        /*skip entry and exit*/
        for (int i = 1; i<=I-2; i++)
        {
            this->copy_Vector(oldin, in, i, J);
            this->union_Suc_Vector(out, in, i, J);
            this->formula_Vector(in, gen, out, minus, i, J);
            if (!changed)
            {
                if (compare_Vector(in, oldin, i, J))
                {
                    changed = true;
                }

            }
                        
            
//            cout<<"True of False: "<<changed<<endl;
//            /*print livein*/
//            for (int k = 0; k<this->nblk+2; k++)
//            {
//                cout<<"livein "<<i<<" "<<k<<" ";
//                for (int j = 0; j<this->nvar; j++)
//                {
//                    cout<<in[k][j]<<" ";
//                }
//                cout<<endl;
//            }
//            
//            /*print livein*/
//            for (int k = 0; k<this->nblk+2; k++)
//            {
//                cout<<"liveout "<<i<<" "<<k<<" ";
//                for (int j = 0; j<this->nvar; j++)
//                {
//                    cout<<out[k][j]<<" ";
//                }
//                cout<<endl;
//            }
//            
//            cout<<"oldin "<<i<<" ";
//            for (int j = 0; j<this->nvar; j++)
//            {
//                cout<<oldin[j]<<" ";
//            }
//            cout<<endl;
            

        }
    }
    
    /*deal with entry*/
    this->union_Suc_Vector(out, in, 0, J);
}


void DataFlow::union_Suc_Vector(bool** left, bool** right, int whichblock, int cols)
{
    int J = cols;
    
    /*
     cout<<"---------------- "<<endl;
     for (int i = 0; i<this->nblk+2; i++)
     {
     for (int j = 0; j<this->nvar; j++)
     {
     cout<<in[i][j]<<" ";
     }
     cout<<endl;
     }
     cout<<"---------------- "<<endl;
    */
    
    for (int j = 0; j<J; j++)
        left[whichblock][j] = false;
    int suc = -1;
    ArcNode *p;
    p = this->graph[whichblock].firstarc;
    while(p)
    {
        suc = p->adjvex;
        //cout<<suc<<" "<<endl;
        for (int j = 0; j<J; j++)
        {
            if (left[whichblock][j]) continue;
            if (right[suc][j])
            {
                //cout<<"hi"<<endl;
                left[whichblock][j] = true;
            }
        }
        p = p->nextarc;
    }
    
//    for (int j = 0; j<J; j++)
//        cout<<out[index][j]<<" ";
//    
//    cout<<endl;
}


void DataFlow::print_Live()
{
    cout<<"live_variables "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"live_out "<<i<<" ";
        for (int j = 0; j<this->nvar; j++)
            cout<<this->liveout[i][j];
        cout<<endl;
    }

}




/***********find live ends**********/
/***********find live ends**********/
/***********find live ends**********/




/*print*/
void DataFlow::print_Live_Info()
{
    this->print_Variable_List();
    cout<<endl;
    this->print_Live_DEF();
    cout<<endl;
    this->print_Live_USE();
    cout<<endl;
    this->print_Live();
    cout<<endl;
    
    
}


/***********find expressions starts**********/
/***********find expressions starts**********/
/***********find expressions starts**********/

void DataFlow::find_Expressions()
{
    //cout<<(NULL==NULL);
    simple_instr *s = this->inlist;
    //expressions have one or two non temp registers and one*****
    //destination register which is NOT PSEUDO_REG*****
    while (s)
    {
        switch (s->opcode) {
            //these ops has two register sources.
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
                //1. get rid of constant operands; get rid of temporary registers
                //2. get rid of duplicate expressions
                if (s->u.base.src1->kind!=TEMP_REG&&s->u.base.src2->kind!=TEMP_REG)
                {
                    //cout<<"find_Expression found "<<endl;
                    Expression expression;
                    expression.src1 = s->u.base.src1;
                    expression.src2 = s->u.base.src2;
                    expression.opcode = s->opcode;
                    int inlist_line = this->line_hash_map[(int)s];//remember my instr starting from 1
                    int duplicatewhere = check_Duplicate_Expression(expression);
                    if(duplicatewhere==-1)
                    {
                        //cout<<"find_Expression Pured "<<endl;
                        this->expressions_vector.push_back(expression);
                        this->expressions_list.push_back(inlist_line);
                        expressions_map[inlist_line] = expressions_vector.size();
                    }
                    else
                        expressions_map[inlist_line] = duplicatewhere+1;
                    
                }
                    
                break;
            }
            //these ops has only one register source which is stored in src1
            case LOAD_OP:
            case CPY_OP:
            case CVT_OP:
            case NEG_OP:
            case NOT_OP:
            {
                if (s->u.base.src1->kind!=TEMP_REG&& s->u.base.dst->kind!=PSEUDO_REG)
                {
                    Expression expression;
                    expression.src1 = s->u.base.src1;
                    expression.src2 = NULL;
                    expression.opcode = s->opcode;
                    int inlist_line = this->line_hash_map[(int)s];//remember my instr starting from 1
                    int duplicatewhere = check_Duplicate_Expression(expression);
                    if(duplicatewhere==-1)
                    {
                        //cout<<"find_Expression Pured "<<endl;
                        this->expressions_vector.push_back(expression);
                        this->expressions_list.push_back(inlist_line);
                        expressions_map[inlist_line] = expressions_vector.size();
                    }
                    else
                        expressions_map[inlist_line] = duplicatewhere+1;

                }
                break;
            }
            
                
                
            default:
                break;
        }
        s = s->next;
    }
    //cout<<"find_Expression Complete "<<endl;
    this->nexpression = expressions_list.size();
}

//false, key exists; true key does not exist
//has checked duplicates before
void DataFlow::fill_Expression_Vector(Expression expression)
{
    
}
//true for has duplicate, false for has no duplicate
int DataFlow::check_Duplicate_Expression(Expression expression)
{
    for (int i = 0; i<this->expressions_vector.size(); i++)
    {
        if (compare_Expression(expression, this->expressions_vector[i]))
        {
            return i;
        }
    }
    return -1;
}

bool DataFlow::compare_Expression(Expression left, Expression right)
{
    if (left.opcode!=right.opcode) return false;
    if (left.src1!=right.src1) return false;
    if (left.src2 == NULL&&right.src2!=NULL) return false;
    if (left.src2!=NULL&&right.src2==NULL) return false;
    if (left.src2!=right.src2) return false;
    return true;
}

void DataFlow::print_Expressions()
{
    cout<<"expressions"<<" "<<this->proc_name<<" "<<this->expressions_list.size()<<endl;
    for (int i = 0; i<this->expressions_list.size(); i++)
    {
        cout<<"expr"<<" "<<i+1<<" "<<this->expressions_list[i]-1<<endl;
    }
}


/***********find expressions ends**********/
/***********find expressions ends**********/
/***********find expressions ends**********/

/***********find evaluations starts**********/
/***********find evaluations starts**********/
/***********find evaluations starts**********/
void DataFlow::find_Evaluations()
{
    this->evaluations = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->evaluations[i] = new bool[this->nexpression];
        for (int j = 0; j<this->nexpression; j++)
            this->evaluations[i][j] = false;
    }
    
    
    //initialize bool matrix by grouping expressions to blocks
    //now calculate the available expressions at the exit of each block
    //if src of an expression is shown in dst of others, it is then killed
    __gnu_cxx::hash_map<int, int>::iterator iter;
    for (iter = this->expressions_map.begin(); iter != this->expressions_map.end(); iter++)
    {
        int line = iter->first;
        int block = this->belongtowhichblock[line];
        
        //now search down
        simple_instr *s = (simple_instr*) this->anti_line_hash_map[line];
        simple_instr *tail = this->graph[block].data.tail;
        
        simple_reg *src1 = s->u.base.src1;
        simple_reg *src2 = s->u.base.src2;
        
        int expression_index = expressions_map[line]-1;
        //cout<<"block "<<" "<<block<<" "<<line<<" "<<expression_index<<endl;
        this->evaluations[block][expression_index] = true;
        
        while(true)
        {
            //base form required
            if (simple_op_format(s->opcode)==BASE_FORM)
            {
                if (s->u.base.dst == src1||s->u.base.dst == src2)
                {
                    
                    this->evaluations[block][expression_index] = false;
                    break;
                }
            }
            
            
            if (s == tail) break;
            s = s->next;
        } 
        
        
        
        
                
        //simple_instr *s = (simple_instr*)this->anti_line_hash_map[line];
        //cout<<simple_op_name(s->opcode);
    }
    //cout<<"find_Evaluations Complete "<<endl;
        
    
    
    
    
}

void DataFlow::print_Evaluations()
{
    cout<<"eval_ae_sets"<<" "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"eval_ae"<<" "<<i<<" ";
        for (int j = 0; j<this->nexpression; j++)
            cout<<evaluations[i][j];
        cout<<endl;
    }
    
}
/***********find evaluations ends**********/
/***********find evaluations ends**********/
/***********find evaluations ends**********/



/***********find kill starts**********/
/***********find kill starts**********/
/***********find kill starts**********/

void DataFlow::find_Kill_Set()
{
    //initialize kill_set
    this->kill_set = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->kill_set[i] = new bool[this->nexpression];
        for (int j = 0; j<this->nexpression; j++)
        {
            this->kill_set[i][j] = false;
        }
    }
    
    __gnu_cxx::hash_map<int, int>::iterator iter;
    for (iter = this->expressions_map.begin(); iter != this->expressions_map.end(); iter++)
    {
        int line = iter->first;
        int index = expressions_map[line]-1;
        //int block = this->belongtowhichblock[line];
        
        //now search globally for all blocks
        simple_instr *victim = (simple_instr*) this->anti_line_hash_map[line];
        simple_reg *src1 = victim->u.base.src1;
        simple_reg *src2 = victim->u.base.src2;
        for (int i = 1; i<=this->nblk; i++)
        {
            //simple_instr *sample = this->graph[3].data.leader;
            simple_instr *s = this->graph[i].data.leader;
            simple_instr *tail = this->graph[i].data.tail;
            while (true)
            {
                if (simple_op_format(s->opcode)==BASE_FORM)
                {
                    //avoid null destination register, such as ret_op
                    if (s->u.base.dst)
                    {
                        if (s->u.base.dst == src1||s->u.base.dst == src2)
                            this->kill_set[i][index] = true;
                    }
                }
                if (s == tail) break;
                s =s->next;
            }
            //cout<<i<<" "<<this->nblk<<endl;
        }
        
        
        
    }
}

void DataFlow::print_Kill_Set()
{
    cout<<"kill_ae_sets"<<" "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"kill_ae"<<" "<<i<<" ";
        for (int j = 0; j<this->nexpression; j++)
            cout<<this->kill_set[i][j];
        cout<<endl;
    }
}
/***********find kill ends**********/
/***********find kill ends**********/
/***********find kill ends**********/


/***********find aval_expr starts**********/
/***********find aval_expr starts**********/
/***********find aval_expr starts**********/

void DataFlow::find_Avail_Expressions()
{
    //initialize out and in
    this->avail_exprs_out = new bool*[this->nblk+2];
    this->avail_exprs_in = new bool*[this->nblk+2];
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->avail_exprs_out[i] = new bool[this->nexpression];
        this->avail_exprs_in[i] = new bool[this->nexpression];
        for (int j = 0; j<this->nexpression; j++)
        {
            this->avail_exprs_out[i][j] = false;
            this->avail_exprs_out[i][j] = false;
        }
    }
    
    forward_Allway_Solver(this->avail_exprs_out, this->evaluations, this->avail_exprs_in,
                          this->kill_set, this->nblk+2, this->nexpression);
}

void DataFlow::forward_Allway_Solver(bool** out, bool** eval,
                                     bool** in, bool**kill, int rows, int cols)
{
    bool changed = true;
    bool *oldout = new bool[this->nexpression];
    while (changed)
    {
//        for (int i = 0; i<this->nblk+2; i++)
//        {
//            cout<<"avail_exprs_out"<<" "<<i<<" ";
//            for (int j = 0; j<this->nexpression; j++)
//                cout<<this->avail_exprs_out[i][j];
//            cout<<endl;
//        }
//        
//        cout<<"------------------------------------"<<endl;
        changed = false;
        for (int i = 1; i<=this->nblk; i++)
        {
            this->copy_Vector(oldout, out, i, cols);
            this->intersect_Vector(in, out, i,cols);
            this->formula_Vector(out, eval, in, kill, i, cols);
            if (!changed)
            {
                if (compare_Vector(out, oldout, i, cols))
                {
                    changed = true;
                }
                
            }
        }
    }
    //deal with exit block
    this->intersect_Vector(in, out, this->nblk+1, cols);
    this->copy_Vector(*(out+this->nblk+1), in, this->nblk+1, cols);
}

void DataFlow::copy_Vector(bool* left, bool** right, int whichblock, int cols)
{
    for (int i = 0; i<cols; i++)
    {
        *(left+i) = *(*(right+whichblock)+i);
    }
}

void DataFlow::intersect_Vector(bool** left, bool** right, int whichblock, int cols)
{
    for (int i = 0; i<cols; i++)
    {
        left[whichblock][i] = true;
    }
    ArcNode *p = this->antigraph[whichblock].firstarc;
    while(p)
    {
        int pred = p->adjvex;
        for (int j = 0; j<cols; j++)
        {
            if (!left[whichblock][j]) continue;
            if (!right[pred][j])
            {
                left[whichblock][j] = false;
            }
        }
        p = p->nextarc;
    }
}

//result = x union [y-z]
void DataFlow::formula_Vector(bool**result, bool**gen, bool**put, bool** minus, int whichblock, int cols)
{
    for (int j = 0; j<cols; j++)
    {
        if (gen[whichblock][j]||put[whichblock][j]&&!minus[whichblock][j])
            result[whichblock][j] = true;
    }
}

bool DataFlow::compare_Vector(bool** left, bool* right, int whichblock, int cols)
{
    for (int j = 0; j<cols; j++)
    {
        if (left[whichblock][j]!=right[j])
        {
            //cout<<"true"<<whichblock<<endl;
            return true;
        }
    }
    return false;
}

void DataFlow::print_Avail_Expressions()
{
    cout<<"available_exprs"<<" "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"avail_exprs_out"<<" "<<i<<" ";
        for (int j = 0; j<this->nexpression; j++)
            cout<<this->avail_exprs_out[i][j];
        cout<<endl;
    }
}

void DataFlow::print_Avail_Expressions_Info()
{
    print_Expressions();
    cout<<endl;
    print_Evaluations();
    cout<<endl;
    print_Kill_Set();
    cout<<endl;
    print_Avail_Expressions();
    cout<<endl;
}


/***********find aval_expr ends**********/
/***********find aval_expr ends**********/
/***********find aval_expr ends**********/




/***********find rd starts**********/
/***********find rd starts**********/
/***********find rd starts**********/

//check dst has duplicate or not
//if does, return defintion index so that the  could be replaced
//if not, return -1;


//definitions are ops that have dst reg which is of base_form
void DataFlow::find_Definitions()
{
    simple_instr *s = this->inlist;
    this->ndefinition = 0;
    while (s)
    {
        //cout<<"op_name "<<simple_op_name(s->opcode)<<endl;
        switch (s->opcode)
        {
            /****specical ldc_form:**/
            case LDC_OP:
            case LOAD_OP:
            case CPY_OP:
            case CVT_OP:
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
                this->ndefinition++;
                int instruction_line = line_hash_map[(int)s];
                //cout<<"instruction_line"<<instruction_line<<" "<<simple_op_name(s->opcode)<<endl;
                this->definition_map[instruction_line] = ndefinition;
                this->anti_definition_map[ndefinition] = instruction_line;
                break;
            }
                
        }
        s = s->next;
    }
}

int DataFlow::is_Definition(simple_instr* s)
{
    int instruction_line = line_hash_map[(int)s];
    if (definition_map.find(instruction_line) == definition_map.end()) return -1;
    
    return definition_map[instruction_line];
}

//fill the rd_gen_set
void DataFlow::fill_RD_Gen_Hash(simple_instr* s)
{
    int instruction_line = line_hash_map[(int)s];
    int whichblock = this->belongtowhichblock[instruction_line];
    
    simple_instr* checkpoint = this->graph[whichblock].data.leader;
    
    int current_definition_index = is_Definition(s);
    if (current_definition_index == -1)
        return;
    this->rd_gen_set[whichblock][current_definition_index-1] = true;
    while(checkpoint!=s)
    {
        int checkpoint_definition_index = is_Definition(checkpoint);
        if (checkpoint_definition_index == -1)
        {
            checkpoint = checkpoint->next;
            continue;
        }
        else
        {
            
            

            //definition_index starts from 1
            if (this->rd_gen_set[whichblock][checkpoint_definition_index-1]&&checkpoint->u.base.dst == s->u.base.dst)
            {
                //the current instruction s is to replace the checkpoint
                //cout<<"which "<<checkpoint_definition_index<<" "<<current_definition_index
                //<<" "<<simple_op_name(s->opcode)<<endl;
                this->rd_gen_set[whichblock][checkpoint_definition_index-1] = false;
            }
            checkpoint =  checkpoint->next;
        }
    }
}
//find rd_gen_set for each block
void DataFlow::find_RD_Gen_Set()
{
    //initialize the set.
    this->rd_gen_set = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->rd_gen_set[i] = new bool[this->ndefinition];
        for (int j = 0; j< this->ndefinition; j++)
            this->rd_gen_set[i][j] = false;//definition index starts from 1; matrix index starts from 0
    }
    
    
    simple_instr *s = this->inlist;
    while (s) {
        this->fill_RD_Gen_Hash(s);
        s = s->next;
    }
        
    
}

void DataFlow::print_RD_Gen_Set()
{
    cout<<"rd_gen_sets "<<this->proc_name<<" "<<this->nblk+2<<" "<<this->ndefinition<<endl;
    for (int i = 1; i<=this->nblk; i++)
    {
        cout<<"rd_gen"<<i<<" ";
        for (int j = 0; j<this->ndefinition; j++)
            if (this->rd_gen_set[i][j])
                cout<<this->anti_definition_map[j+1]<<" ";
        cout<<endl;
    }
}

//find kill in OTHER blocks
void DataFlow::fill_RD_Kill_Hash(simple_instr* s)
{
    int instruction_line = line_hash_map[(int)s];
    int whichblock = this->belongtowhichblock[instruction_line];
    if (is_Definition(s)==-1)
        return;
    for (int i = 1; i<=this->nblk; i++)
    {
        if (i==whichblock)
            continue;
        
        simple_instr* checkpoint = this->graph[i].data.leader;
        for (int j = 0; j<this->graph[i].data.ninstr; j++)
        {
            int checkpoint_definition_index = is_Definition(checkpoint);
            if (checkpoint_definition_index == -1)
            {
                checkpoint = checkpoint->next;
                continue;
            }
            else
            {
                //cout<<"kill "<<whichblock<<" "<<i<<" "<<simple_op_name(checkpoint->opcode)<<endl;
                if (checkpoint->u.base.dst == s->u.base.dst)
                    this->rd_kill_set[whichblock][checkpoint_definition_index-1] = true;
            }
            checkpoint = checkpoint->next;
        }
    }
}

//Set of definitions in other basic blocks that are killed in b
void DataFlow::find_RD_Kill_Set()
{
    this->rd_kill_set = new bool*[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->rd_kill_set[i] = new bool[this->ndefinition];
        for (int j = 0; j<this->ndefinition; j++)
            this->rd_kill_set[i][j] = false;
    }
    
    simple_instr* s = this->inlist;
    while (s) {
        fill_RD_Kill_Hash(s);
        s = s->next;
    }
}



void DataFlow::print_RD_Kill_Set()
{
    cout<<"rd_kill_set "<<this->proc_name<<" "<<this->nblk+2<<" "<<this->ndefinition<<endl;
    
    for (int i = 1; i<=this->nblk; i++)
    {
        cout<<"rd_kill"<<i<<" ";
        for (int j = 0; j<this->ndefinition; j++)
            if (this->rd_kill_set[i][j])
                cout<<this->anti_definition_map[j+1]<<" ";
        cout<<endl;
    }

}

void DataFlow::union_Pred_Vector(bool**left, bool**right, int whichblock, int cols)
{
    int J = cols;
    
    for (int j = 0; j<J; j++)
        left[whichblock][j] = false;
    int pred = -1;
    ArcNode *p;
    p = this->antigraph[whichblock].firstarc;
    while(p)
    {
        pred = p->adjvex;
        //cout<<suc<<" "<<endl;
        for (int j = 0; j<J; j++)
        {
            if (left[whichblock][j]) continue;
            if (right[pred][j])
            {
                //cout<<"hi"<<endl;
                left[whichblock][j] = true;
            }
        }
        p = p->nextarc;
    }
    
    //    for (int j = 0; j<J; j++)
    //        cout<<out[index][j]<<" ";
    //
    //    cout<<endl;

}

void DataFlow::forward_Anyway_Solver(bool** out, bool** gen, bool** in, bool** minus, int rows, int cols)
{
    int I = rows;
    int J = cols;
    
    bool changed = true;
    
    bool *oldout = new bool[J];
    
    while(changed)
    {
        changed = false;
        for (int i = 1; i<=I-2; i++)
        {
            this->copy_Vector(oldout, out, i, J);
            this->union_Pred_Vector(in, out, i, cols);
            this->formula_Vector(out, gen, in, minus, i, cols);
            
            if (!changed)
            {
                if (compare_Vector(out, oldout, i, cols))
                {
                    changed = true;
                }
                
            }
            
        }
    }
    //deal with exit node
    this->union_Pred_Vector(in, out, I-1, cols);
}

void DataFlow::find_Reaching_Definitions()
{
    this->find_Definitions();
    this->find_RD_Gen_Set();
    this->find_RD_Kill_Set();
    
    this->rd_in = new bool*[this->nblk+2];
    this->rd_out = new bool*[this->nblk+2];
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->rd_in[i] = new bool[this->ndefinition];
        this->rd_out[i] = new bool[this->ndefinition];
        for (int j = 0; j< this->ndefinition; j++)
        {
            this->rd_in[i][j] = false;
            this->rd_out[i][j] = false;
        }
    }
    
    forward_Anyway_Solver(this->rd_out, this->rd_gen_set, this->rd_in, this->rd_kill_set, this->nblk+2, this->ndefinition);
    
}

void DataFlow::print_RD()
{
    cout<<"rd "<< this->proc_name<<" "<<this->nblk+2<<" "<<this->ndefinition<<endl;
    for (int i = 1; i<=this->nblk; i++)
    {
        cout<<"rd_out"<<i<<" ";
        for (int j = 0; j<this->ndefinition; j++)
        {
            if (rd_out[i][j])
                cout<<this->anti_definition_map[j+1]<<" ";
        }
        cout<<endl;
    }
    
    for (int i = 1; i<=this->nblk; i++)
    {
        cout<<"rd_in"<<i<<" ";
        for (int j = 0; j<this->ndefinition; j++)
            if (rd_in[i][j])
                cout<<this->anti_definition_map[j+1]<<" ";
        cout<<endl;
    }
}
void DataFlow::print_RD_Info()
{
    //print_RD_Gen_Set();
    //print_RD_Kill_Set();
    print_RD();
}

__gnu_cxx::hash_map<int, int> DataFlow::get_anti_definition_map()
{
    return this->anti_definition_map;
}

__gnu_cxx::hash_map<int, int> DataFlow::get_definition_map()
{
    return this->definition_map;
}

bool** DataFlow::get_rd_out()
{
    return this->rd_out;
}

bool** DataFlow::get_rd_in()
{
    return this->rd_in;
}

int DataFlow::get_ndefinition()
{
    return this->ndefinition;
}
/***********find rd ends**********/
/***********find rd ends**********/
/***********find rd ends**********/



/***********build ud chain starts**********/
/***********build ud chain starts**********/
/***********build ud chain starts**********/

//type = 0, 1
void DataFlow::grow_Use_Def_Chain(simple_reg* r, bool* detailed_rd, int instruction_line, int type)
{
    for (int i = 0; i<this->ndefinition; i++)
    {
        if (!detailed_rd[i])
            continue;
        int checkpoint_line = this->anti_definition_map[i+1];
        simple_instr* checkpoint = (simple_instr*) this->anti_line_hash_map[checkpoint_line];
        if (checkpoint->u.base.dst == r)
        {
            cout<<"in "<<instruction_line<<" "<<checkpoint_line<<endl;
            //insert node to use_def_chain[type]
            ArcNode *new_member = new ArcNode;
            new_member->adjvex = checkpoint_line;
            //cout<<"Here"<<endl;
            new_member->nextarc = this->use_def_chain[type][instruction_line].firstarc;
            
            this->use_def_chain[type][instruction_line].firstarc= new_member;
        }
    }
    
}


void DataFlow::build_Use_Def_Chain()
{
    if (this->rd_in == NULL)
    {
        cout<<"EMPTY REACHING DEFINITION"<<endl;
        exit(-1);
    }
    
    this->use_def_chain = new LoopG*[2];
    for (int i = 0; i<2; i++)
    {
        use_def_chain[i] = new LoopG[this->ninstr+2];
        for (int j = 0; j<this->ninstr+2; j++)
        {
            use_def_chain[i][j].ndomi = 0;
            use_def_chain[i][j].firstarc = NULL;
        }
        
    }
    bool detailed_rd[this->ndefinition];
    
    for (int b = 1; b<=this->nblk; b++)
    {
        simple_instr* s = this->graph[b].data.leader;
        
        //at the entry, copy the rd_in into detailed_rd
        for (int j = 0; j<this->ndefinition; j++)
            detailed_rd[j] = this->rd_in[b][j];
        
        for (int v = 0; v<this->graph[b].data.ninstr; v++)
        {
            int instruction_line = this->line_hash_map[(int)s];
//            cout<<"detailed_rd ";
//            for (int j = 0; j<this->ndefinition; j++)
//                cout<<detailed_rd[j]<<" ";
            cout<<endl;
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
                    this->grow_Use_Def_Chain(s->u.base.src1, detailed_rd, instruction_line, 0);
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
                    this->grow_Use_Def_Chain(s->u.base.src1, detailed_rd, instruction_line, 0);
                    this->grow_Use_Def_Chain(s->u.base.src2, detailed_rd, instruction_line, 1);
                    break;
                }
                default:
                    break;
            }
            
            
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
                //cout<<"current"<<current_definition_index;
            }
//            cout<<"detailed_rd"<<b<<" ";
//            for (int i = 0; i<this->ndefinition; i++)
//            {
//                if (detailed_rd[i])
//                    cout<<this->anti_definition_map[i+1]<<" ";
//            }
//            cout<<endl;
            s = s->next;
            //
        }
        
    }
    
    
}

void DataFlow::print_Use_Def_Chain()
{
    simple_instr* s = this->inlist;
    cout<<"src1 "<<endl;
    while (s) {
        int instruction_line = this->line_hash_map[(int)s];
        cout<<instruction_line<<" ";
        ArcNode *p = this->use_def_chain[0][instruction_line].firstarc;
        
        while (p) {
            cout<<p->adjvex<<" ";
            p = p->nextarc;
        }
        cout<<endl;
        
        s = s->next;
    }
    
    cout<<"src2 "<<endl;
    s = this->inlist;
    while (s) {
        int instruction_line = this->line_hash_map[(int)s];
        cout<<instruction_line<<" ";
        ArcNode *p = this->use_def_chain[1][instruction_line].firstarc;
        
        while (p) {
            cout<<p->adjvex<<" ";
            p = p->nextarc;
        }
        cout<<endl;
        
        s = s->next;
    }
    cout<<endl;
}

LoopG** DataFlow::get_Use_Def_Chain()
{
    return this->use_def_chain;
}
/***********build ud ends**********/
/***********build ud ends**********/
/***********build ud ends**********/










