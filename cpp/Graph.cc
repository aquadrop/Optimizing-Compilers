//
//  Graph.cpp
//  A1
//
//  Created by Jianwei Xu on 19/01/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#include "Graph.h"

Graph::Graph()
{
    
}

Graph::Graph(simple_instr *inlist, char *proc_name)
{
    this->inlist = inlist;
    this->proc_name = proc_name;
}

void Graph::init_Hash()
{
    /*initiate hash_map for label instructions*/
    simple_instr *p;
    p = inlist;
    int count = 0;
    while(p)
    {
        count++;
        if (p->opcode == LABEL_OP)
        {
            label_hash_map[p->u.label.lab->name] = count;
        }
        line_hash_map[(int)p] = count;
        anti_line_hash_map[count] = (int)p;
        //cout<<"origin: "<<simple_op_name(p->opcode)<<endl;
        p = p->next;
    }
    this->ninstr = count;
    this->leader_index = new bool[this->ninstr+2];//0 to be the entry;ninstr+1 to be the exit
    for (int i = 0; i<this->ninstr+2; i++)
    {
        this->leader_index[i] = false;
    }
    /*
     cout<<(int)p<<" "<<this->label_hash_map[(int)p]<<" "<<count<<" ";
     if (p->opcode == LABEL_OP)
     cout<<"label "<<p->u.label.lab->name<<endl;
     else cout<<endl;
     
     */
}

void Graph::find_Leaders()
{
    int index = 1;
    simple_instr *p = this->inlist->next;
    leader_index[0] = true;
    leader_index[1] = true;//first is always leader
    this->nblk = 0;
    int count = 0;
    while(p)
    {
        index++;
        //cout<<index<<endl;
        /*any label starts a new block*/
        if (p->opcode == LABEL_OP)
        {
            leader_index[index] = true;
        }
        if (p->prev->opcode == JMP_OP||p->prev->opcode == BTRUE_OP||p->prev->opcode == BFALSE_OP||p->prev->opcode == MBR_OP)
        {
            leader_index[index] = true;
        }
        if (p->opcode == JMP_OP||p->opcode == BTRUE_OP||p->opcode == BFALSE_OP)
        {
            int target_index = this->label_hash_map[p->u.bj.target->name];
            //cout<<target_index<<endl;
            leader_index[target_index] = true;
        }
        if (p->opcode == MBR_OP)
        {
            int target_index = -1;
            int ntargets = p->u.mbr.ntargets;
            if (ntargets == 0)
            {
                target_index = this->label_hash_map[p->u.mbr.deflab->name];
                leader_index[target_index] = true;
            }
            else
            {
                for (int n = 0; n<ntargets; n++)
                {
                    target_index = this->label_hash_map[p->u.mbr.targets[n]->name];
                    leader_index[target_index] = true;
                }
                target_index = this->label_hash_map[p->u.mbr.deflab->name];
                leader_index[target_index] = true;
            }
        }
        p = p->next;
    }
    count = 0;
    for (int i = 1; i<=this->ninstr; i++)
    {
        if (leader_index[i])
            count++;
    }
    this->nblk = count;
    leader_index[this->ninstr+1] = true;
    //cout<<this->nblk;
    //    for (int i = 0; i<this->ninstr+1; i++)
    //    {
    //        cout<<i<<" "<<leader_index[i]<<endl;
    //    }
}

void Graph::init_Graph()
{
    //
    this->graph = new VNode[this->nblk+2];
    int length = this->nblk+2;
    //cout<<length<<endl;
    //    for (int i = 0; i<this->nblk+2; i++)
    //    {
    //        this->graph[i].data = new VertexType[1];
    //        cout<<i<<endl;
    //    }
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->graph[i].data.leader = NULL;
        this->graph[i].data.tail = NULL;
        this->graph[i].firstarc = NULL;
        this->graph[i].data.ninstr = 0;
        this->graph[i].data.blkindex = i;
        this->graph[i].data.nsuccessor = 0;
        this->graph[i].data.npredecessor = 0;
    }
    /*
     
     nested allocate
     struct VNode *graph[length];
     this->graph = graph;
     
     cout<<this->nblk<<endl;
     
     */
}

void Graph::init_AntiGraph()
{
    int length = this->nblk+2;
    this->antigraph = new VNode[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->antigraph[i].data.leader = NULL;
        this->antigraph[i].data.tail = NULL;
        this->antigraph[i].firstarc = NULL;
        this->antigraph[i].data.ninstr = 0;
        this->antigraph[i].data.blkindex = i;
        this->antigraph[i].data.nsuccessor = 0;
        this->antigraph[i].data.npredecessor = 0;
    }
}

void Graph::build_Blocks()
{
    simple_instr *p;
    int blk = 1;
    int index = 1;
    p = this->inlist;
    int blkninstr = 1;
    /*build blocks*/
    this->belongtowhichblock = new int[this->ninstr+1];
    for (int i = 0; i<=this->ninstr; i++)
    {
        belongtowhichblock[i] = -2;
    }
    while(p)
    {
        if (leader_index[index])
        {
            this->graph[blk].data.leader = p;
            //cout<<(int)this->graph[blk-1].data.tail<<endl;
            belongtowhichblock[index] = blk;
            this->graph[blk].data.blkindex = blk;
            
            blkninstr = 1;
        }
        if (leader_index[index+1])
        {
            this->graph[blk].data.tail = p;
            belongtowhichblock[index] = blk;
            this->graph[blk].data.ninstr = blkninstr;
            blk++;
        }
        blkninstr++;
        index++;
        p = p->next;
    }
    
    //fill complete belongtowhichblock
    for (int i = 1; i<=this->ninstr; i++)
    {
        if (belongtowhichblock[i]==-2) belongtowhichblock[i] = belongtowhichblock[i-1];
    }
    
    /*check*/
    p = this->inlist;
    for (int i = 1; i<=this->ninstr; i++)
    {
        //cout<<i<<" "<<leader_index[i]<<" ";
        if (p->opcode == LABEL_OP)
        {
            int target_line = this->label_hash_map[p->u.label.lab->name];
            int target_block = belongtowhichblock[target_line];
            //cout<<" Hi: "<<target_line<<" "<<target_block<<endl;
        }
        //else cout<<endl;
        p = p->next;
    }
    //cout<<endl;
    /*
     if (belongtowhichblock[i]>=0)
     {
     cout<<" "<<this->graph[belongtowhichblock[i]].data.ninstr<<" "<<this->line_hash_map[(int)this->graph[belongtowhichblock[i]].data.leader]<<" "<<this->line_hash_map[(int)this->graph[belongtowhichblock[i]].data.tail]<<endl;
     }
     
     */
    
}

void Graph::enter_Node(int blk_index, int target_block)
{
    ArcNode *entry = new ArcNode;
    /*front insert*/
    entry->adjvex = target_block;
    entry->nextarc = graph[blk_index].firstarc;
    graph[blk_index].firstarc = entry;
    graph[blk_index].data.nsuccessor++;
    
    /*antigraph*/
    ArcNode *leave = new ArcNode;
    leave->adjvex = blk_index;
    leave->nextarc = antigraph[target_block].firstarc;
    antigraph[target_block].firstarc = leave;
    antigraph[target_block].data.npredecessor++;
}

void Graph::link_Blocks()
{
    ArcNode *entry = new ArcNode;
    entry->adjvex = 1;
    entry->nextarc = NULL;
    graph[0].firstarc = entry;
    graph[0].data.nsuccessor = 1;
    
    ArcNode *leave = new ArcNode;
    leave->adjvex = 0;
    leave->nextarc = NULL;
    antigraph[1].firstarc = leave;
    antigraph[1].data.npredecessor = 1;
    for (int i = 1; i<=this->nblk; i++)
    {
        simple_instr *p = graph[i].data.tail;
        //cout<<i<<" "<<(int)p<<endl;
        if (p->opcode == JMP_OP)
        {
            int target_index = this->label_hash_map[p->u.bj.target->name];
            int target_block = belongtowhichblock[target_index];
            //cout<<"Jump: "<<this->line_hash_map[(int)p]<<" "<<" "<<target_index<<" "<<endl;
            this->enter_Node(i,target_block);
            continue;
            
        }
        
        if (p->opcode == BTRUE_OP||p->opcode == BFALSE_OP)
        {
            int target_index = this->label_hash_map[p->u.bj.target->name];
            int target_block = belongtowhichblock[target_index];
            //cout<<"If: "<<this->line_hash_map[(int)p]<<" "<<" "<<target_index<<" "<<endl;
            this->enter_Node(i,target_block);
            
            //cout<<"If Natural: "<<i<<endl;
            target_block = i+1;
            this->enter_Node(i,target_block);
            
            continue;
            
        }
        
        
        if (p->opcode == MBR_OP)
        {
            int target_index = -1;
            int target_block = -1;
            int ntargets = p->u.mbr.ntargets;
            if (ntargets == 0)
            {
                //cout<<"Default MBR: "<<i<<endl;
                target_index = this->label_hash_map[p->u.mbr.deflab->name];
                target_block = belongtowhichblock[target_index];
                this->enter_Node(i,target_block);
            }
            else
            {
                //cout<<"MBR: "<<i<<endl;
                for (int n = 0; n<ntargets; n++)
                {
                    target_index = this->label_hash_map[p->u.mbr.targets[n]->name];
                    target_block = belongtowhichblock[target_index];
                    this->enter_Node(i,target_block);
                }
                target_index = this->label_hash_map[p->u.mbr.deflab->name];
                target_block = belongtowhichblock[target_index];
                this->enter_Node(i,target_block);
            }
            
            continue;
        }
        /*return function, successor would be exit*/
        if(p->opcode == RET_OP)
        {
            int target_block = this->nblk+1;
            this->enter_Node(i,target_block);
            continue;
        }
        /*the natural successor, not jump, not branch*/
        
        {
            //cout<<"Natural: "<<i<<endl;
            int target_block = i+1;
            this->enter_Node(i,target_block);
        }
        
    }
    
   
    /*
     simple_instr* p = this->inlist;
     while (p)
     {
     if (p->opcode == LABEL_OP)
     cout<<"label check: "<<p->u.label.lab->name<<endl;
     p = p->next;
     }

     */
    
}

void Graph::build_Graph()
{
    this->init_Hash();
    /*find leaders*/
    this->find_Leaders();
    /*initiate and build the graph*/
    this->init_Graph();
    this->init_AntiGraph();
    /*block and instruction start from 1*/
    this->build_Blocks();
    
    /*build graph*/
    this->link_Blocks();
    /*graph build complete*/
}

void Graph::print_Graph()
{
    cout<<"cfg "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"block "<<i<<endl;
        cout<<"\tinstrs "<<graph[i].data.ninstr;
        simple_instr *p;
        p = graph[i].data.leader;
        while(p)
        {
            int index = this->line_hash_map[(int)p];
            /*in print form, instr starts from 1*/
            cout<<" "<<index-1;
            if (p == graph[i].data.tail)
                break;
            p = p->next;
        }
        cout<<endl;
        /*print successors*/
        cout<<"\tsuccessors "<<graph[i].data.nsuccessor;
        int successorlist[graph[i].data.nsuccessor];
        int l = 0;
        ArcNode *q;
        q = graph[i].firstarc;
        while(q)
        {
            //cout<<" "<<q->adjvex;
            successorlist[l] = q->adjvex;
            q = q->nextarc;
            l++;
        }
        q_sort(successorlist,0,graph[i].data.nsuccessor-1);
        for (int j = 0; j<graph[i].data.nsuccessor; j++)
            cout<<" "<<successorlist[j];
        cout<<endl;
        
        
        /*print predecessors*/
        cout<<"\tpredecessors "<<antigraph[i].data.npredecessor;
        int predecessorlist[antigraph[i].data.npredecessor];
        q = this->antigraph[i].firstarc;
        l = 0;
        while(q)
        {
            //cout<<" "<<q->adjvex;
            predecessorlist[l] = q->adjvex;
            q = q->nextarc;
            l++;
        }
        q_sort(predecessorlist,0,antigraph[i].data.npredecessor-1);
        for (int j = 0; j<antigraph[i].data.npredecessor; j++)
            cout<<" "<<predecessorlist[j];
        cout<<endl;
    }
}



/*true for proper and false for general*/
void Graph::find_Proper_Domi(bool proper_general)
{
    /*memory allocate*/
    this->proper_domi = new bool*[this->nblk+2];
    
    this->proper_domi_graph = new Dominator[this->nblk+2];
    //this->direct_domi_graph = new Dominator[this->nblk+2];
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->proper_domi_graph[i].ndomi = 0;
        //   this->direct_domi_graph[i].ndomi = 0;
        
        this->proper_domi_graph[i].firstarc = NULL;
        //   this->direct_domi_graph[i].firstarc = NULL;
    }
    
    bool olddomi[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
    {
        proper_domi[i] = new bool[this->nblk+2];
        for (int j = 0; j<this->nblk+2; j++)
        {
            if (i==0)
            {
                proper_domi[i][j] = false;
            }
            else
            {
                proper_domi[i][j] = true;
            }
        }
    }
    proper_domi[0][0] = true;
    
    /*find Domi*/
    bool changed = true;
    ArcNode *p;
    while(changed)
    {
        changed = false;
        for (int i = 1; i<this->nblk+2; i++)//for each v except entry
        {
            
            
            /*copy to old*/
            for (int j = 0; j<this->nblk+2; j++)
            {
                olddomi[j] = proper_domi[i][j];
            }
            
            /*for every predecessor of v, calculate intersection*/
            p = this->antigraph[i].firstarc;
            int location = 0;
            if (p==NULL)
            {
                for (int j = 0; j<this->nblk+2; j++)
                    proper_domi[i][j] = false;
            }
            else
            {
                while(p)
                {
                    int predecessor_index = p->adjvex;
                    //cout<<"Hi: "<<i<<" "<<p->adjvex<<endl;
                    for (int j = 0; j<this->nblk+2; j++)
                    {
                        //cout<<"location: "<<location<<endl;
                        if (location==0)
                        {
                            proper_domi[i][j] = proper_domi[predecessor_index][j];
                            //cout<<"true: "<<proper_domi[i][j]<<endl;
                        }
                        
                        else
                        {
                            if (!proper_domi[i][j]) continue;
                            if (!proper_domi[predecessor_index][j]) 
                                proper_domi[i][j] = false;
                        }
                    }
                    p = p->nextarc;
                    location++;
                }
                
            }
            
            /*add v itself into dominator*/
            proper_domi[i][i] = true;
            /*if (DOM(v) != oldDOM) changed = true*/
            for (int j = 0; j<this->nblk+2; j++)
            {
                if (olddomi[j] != proper_domi[i][j])
                {
                    changed = true;
                    break;
                }
            }
        }
        
    }
    
    /*backup for while(changed)
     
     bool changed = true;
     ArcNode *p;
     while(changed)
     {
     changed = false;
     for (int i = 1; i<this->nblk+2; i++)//for each v except entry
     {
     
     
     //copy to old
    for (int j = 0; j<this->nblk+2; j++)
    {
        olddomi[j] = proper_domi[i][j];
    }
    
    //for every predecessor of v, calculate intersection
    p = this->antigraph[i].firstarc;
    int location = 0;
    if (p==NULL)
    {
        for (int j = 0; j<this->nblk+2; j++)
            proper_domi[i][j] = false;
    }
    else
    {
        while(p)
        {
            int predecessor_index = p->adjvex;
            //cout<<"Hi: "<<i<<" "<<p->adjvex<<endl;
            for (int j = 0; j<this->nblk+2; j++)
            {
                //cout<<"location: "<<location<<endl;
                if (location==0)
                {
                    proper_domi[i][j] = proper_domi[predecessor_index][j];
                    //cout<<"true: "<<proper_domi[i][j]<<endl;
                }
                
                else
                    proper_domi[i][j] = proper_domi[i][j]&&proper_domi[predecessor_index][j];
            }
            p = p->nextarc;
            location++;
        }
        
    }
    
    ///*add v itself into dominator
    proper_domi[i][i] = true;
   // /*if (DOM(v) != oldDOM) changed = true
    for (int j = 0; j<this->nblk+2; j++)
    {
        if (olddomi[j] != proper_domi[i][j])
        {
            changed = true;
            break;
        }
    }
}

}



     */














    //
    bool which = !proper_general;
    for (int i = 0; i<this->nblk+2; i++)
    {
        proper_domi[i][i] = which;
    }


    /*construct proper dominatior graph*/
    /*construct mirror for direct domi graph*/
    for (int i = 0; i<this->nblk+2; i++)
    {
        for (int j = 0; j<this->nblk+2; j++)
        {
            if (proper_domi[i][j])
            {
                ArcNode *node = new ArcNode;
                
                
                node->adjvex = j;
                
                
                node->nextarc = this->proper_domi_graph[i].firstarc;
                
                
                this->proper_domi_graph[i].firstarc = node;
                
                
                this->proper_domi_graph[i].ndomi++;
            }
            
        }
        
    }
    
    
        
    
    //cout<<endl;
    /*check
     cout<<endl<<"Check "<<endl;
     for (int i = 0; i<this->nblk+2; i++)
     {
     cout<<i<<" ";
     for (int j = 0; j<this->nblk+2; j++)
     {
     if (proper_domi[i][j])
     cout<<j;
     }
     cout<<endl;
     }
     for (int i = 0; i<this->nblk+2; i++)
     {
     cout<<i<<": ";
     ArcNode *p = this->proper_domi_graph[i].firstarc;
     while(p)
     {
     cout<<p->adjvex<<" ";
     p = p->nextarc;
     }
     cout<<endl;
     }
     
     
     
     */
}

void Graph::find_Dir_Domi()
{
    /*initialize proper dominator list*/
    dir_domi = new int[this->nblk+2];
    this->find_Proper_Domi(true);
    
    /*make mirror of proper_domi_graph*/
    this->direct_domi_graph = new Dominator[this->nblk+2];
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        this->direct_domi_graph[i].ndomi = 0;
        this->direct_domi_graph[i].firstarc = NULL;
    }
    
    for (int i = 0; i<this->nblk+2; i++)
    {
        ArcNode *p = proper_domi_graph[i].firstarc;
        while(p)
        {
            ArcNode *node = new ArcNode;
            node->adjvex= p->adjvex;
            
            node->nextarc = this->direct_domi_graph[i].firstarc;
            
            this->direct_domi_graph[i].firstarc = node;
            p = p->nextarc;
        }
    }
    
    
    for (int i =1; i<this->nblk+2; i++)
    {
        //        ArcNode *head = direct_domi_graph[i].firstarc;
        ArcNode *s = direct_domi_graph[i].firstarc;
        ArcNode *t = s;
        ArcNode *x = s; //the prev of t;
        ArcNode *r;
        while(s)
        {
            t = direct_domi_graph[i].firstarc;
            while (t)
            {
                if (t == s)
                {
                    t = t->nextarc;
                    continue;
                }
                
                r = proper_domi_graph[s->adjvex].firstarc;
                
                bool tmove = true;
                while(r)
                {
                    if (r->adjvex == t->adjvex)
                    {
                        /*delete node of t*/
                        if (t == direct_domi_graph[i].firstarc)
                        {
                            direct_domi_graph[i].firstarc = t->nextarc;
                            //                            cout<<i<<" head delete "<<t->adjvex<<endl;
                            t->nextarc = NULL;
                            t = direct_domi_graph[i].firstarc;
                            //                            cout<<"where is t "<<t->adjvex<<endl;
                            //                            cout<<"where is s "<<s->adjvex<<endl;
                            
                            tmove = false;
                            break;
                        }
                        
                        else
                        {
                            x = s;
                            while(x->nextarc!=t) x = x->nextarc;
                            x->nextarc = t->nextarc;
                            //                            cout<<i<<" normal delete "<<t->adjvex<<endl;
                            t->nextarc = NULL;
                            t = x->nextarc;
                            //                            if (t)
                            //                                cout<<"where is t "<<t->adjvex<<endl;
                            //                            else
                            //                                cout<<"where is t "<<"NULL"<<endl;
                            //                            if (s)
                            //                                cout<<"where is s "<<s->adjvex<<endl;
                            //                            else
                            //                                cout<<"where is s "<<"NULL"<<endl;;
                            tmove = false;
                            break;
                        }
                    }
                    else
                    {
                        r = r->nextarc;
                        //cout<<endl;
                    }
                    
                }
                if (tmove)
                    t = t->nextarc;
            }
            s = s->nextarc;
        }
    }
    
    /*finale direct dominators*/
    this->dir_domi[0] = -1;
    for (int i =1; i<this->nblk+2; i++)
    {
        if (!direct_domi_graph[i].firstarc)
            this->dir_domi[i] = -1;
        else
            this->dir_domi[i] = direct_domi_graph[i].firstarc->adjvex;
    }
    
    
    /*
     for (int i =1; i<this->nblk+2; i++)
     {
     cout<<proper_domi_graph[i].firstarc->adjvex<<endl;
     }
     */
}



void Graph::print_Dir_Domi()
{
    cout<<"idominators "<<this->proc_name<<" "<<this->nblk+2<<endl;
    
    
    for (int i =0; i<this->nblk+2; i++)
    {
        cout<<"block "<<i<<endl;
        if (this->dir_domi[i]==-1)
        {
            cout<<"\tidom"<<endl;
        }
        else
            cout<<"\tidom "<<this->dir_domi[i]<<endl;
    }
    
}



/*****************Assignment 2*****************/
void Graph::find_Backedges()
{
    simple_instr *before = this->inlist;
//    while(before)
//    {
//        cout<<simple_op_name(before->opcode)<<endl;
//        before = before->next;
//    }

    //cout<<"Hello: "<<this->nblk+2<<endl;
    /*generate general dominator adjacent list*/
    this->find_Proper_Domi(false);
    
    
//    this->backedge_pair.resize(this->nblk+2);
//    for (int i = 0; i<this->nblk+2; i++)
//    {
//        this->backedge_pair[i].resize(3);
//        this->backedge_pair[i][0] = i;
//        this->backedge_pair[i][1] = -1;
//        this->backedge_pair[i][2] = 0;
//        //determine whether this is loop head, 0 or 1, later will be changed to original index: i
//    }
    
    
    
    int checktail = -1;
    int checkhead = -1;
    ArcNode *head,*tail;
    bool markhead[this->nblk+2];
    for (int i = 0; i<this->nblk+2; i++)
        markhead[i] = false;
    //cout<<"--------"<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        head = this->proper_domi_graph[i].firstarc;
        while (head)
        {
            checkhead = head->adjvex;
            tail = graph[i].firstarc;
            while (tail)
            {
                //cout<<checkhead<<" "<<tail->adjvex;

                if (tail->adjvex == checkhead)//a back edge found!
                {
                    //cout<<" Found Loop"<<endl;
                    this->alter_Prelist(checkhead,i,markhead);
                }
                //else cout<<endl;
                tail = tail->nextarc;
            }
            head = head->nextarc;
        }
    }
    this->alter_Afterlist();
    /*check*/
        
        
    /*
     simple_instr *instr = this->inlist;
     while(instr)
     {
     cout<<simple_op_name(instr->opcode)<<endl;
     instr = instr->next;
     }
     

    
  
     */
}


void Graph::alter_Link_to_Preheader(int tail, int head)
{
    int target_block = -1;
    
    //boundary, entry and exit node has no instructions
    if (tail == 0) return;
    
    simple_instr* p = graph[tail].data.tail;
    
    
//    cout<<"leader: "<<simple_op_name(p->opcode)<<endl;
    simple_instr *q = this->graph[head].data.leader->prev;
    if (p->opcode == JMP_OP)
    {
        int target_index = this->label_hash_map[p->u.bj.target->name];
        int target_block = belongtowhichblock[target_index];
        if (target_block == head)
        {
//            cout<<"before: "<<p->u.bj.target->name<<endl;
            p->u.bj.target = q->u.label.lab;
//            cout<<"after: "<<p->u.bj.target->name<<endl;
            return;
        }
    }
    
    if (p->opcode == MBR_OP)
    {
        int target_index = -1;
        int target_block = -1;
        int ntargets = p->u.mbr.ntargets;
        if (ntargets == 0)
        {
            //cout<<"Default MBR: "<<i<<endl;
            target_index = this->label_hash_map[p->u.mbr.deflab->name];
            target_block = belongtowhichblock[target_index];
            if (target_block == head)
            {
                p->u.bj.target->name = q->u.label.lab->name;
                return;
            }
        }
        else
        {
            //cout<<"MBR: "<<i<<endl;
            for (int n = 0; n<ntargets; n++)
            {
                target_index = this->label_hash_map[p->u.mbr.targets[n]->name];
                target_block = belongtowhichblock[target_index];
                if (target_block == head)
                {
                    p->u.bj.target->name = q->u.label.lab->name;
                    return;
                }
            }
            target_index = this->label_hash_map[p->u.mbr.deflab->name];
            target_block = belongtowhichblock[target_index];
            if (target_block == head)
            {
                p->u.bj.target->name = q->u.label.lab->name;
                return;
            }
        }
    }
}

void Graph::alter_Afterlist()
{
    if (backedge_pair.empty()) return;
    
    
    /**very very important!!!!!!!!!!!!*/
    /**very very important!!!!!!!!!!!!*/
    /**move link from predecessors to preheader except backedge!!!!!!!!!!!!*/
    __gnu_cxx::hash_map<int, int>tailtohead;
    for (int i = 0; i<this->nblk+2; i++)
        tailtohead[i] = -1;
    for (int i = 0; i<backedge_pair.size(); i++)
        tailtohead[backedge_pair[i][1]] = backedge_pair[i][0];
    int dual = -1;
    for (int i = 0; i<backedge_pair.size(); i++)
    {
        bool doinsert = false;
        int head = backedge_pair[i][0];
        if (head == dual) continue;
        dual = head;
        ArcNode *p = this->antigraph[head].firstarc;
        while (p)
        {
            if (tailtohead[p->adjvex] ==-1)
            {
                
                this->alter_Link_to_Preheader(p->adjvex,head);
                // cout<<"nonloop predecessor: "<<" "<<head<<endl;
            }
            p = p->nextarc;
        }
    }
    
    
    /**very very important!!!!!!!!!!!!*/
    /**very very important!!!!!!!!!!!!*/
    
    
    
    /*additional function*/
    
//    for (int i = 0; i<backedge_pair.size(); i++)
//    {
//        cout<<"before backedges: "<<backedge_pair[i][0]<<" "<<backedge_pair[i][1]<<" "<<backedge_pair[i][2]<<endl;
//    }
    
    /*--------------------------------*/
    /*important!!! alter the block index as preheaders */
    int headaugment = 0;
    int tailaugment = 0;
    __gnu_cxx::hash_map<int, bool>headnodualhash,tailnodualhash;
    for (int i = 0; i<backedge_pair.size(); i++)
    {
        for (int k = 0; k<backedge_pair.size(); k++)
        {
            headnodualhash[backedge_pair[k][2]] = false;
            tailnodualhash[backedge_pair[k][2]] = false;
        }
        headaugment = 0;
        tailaugment = 0;
        for (int j = 0; j<backedge_pair.size(); j++)
        {
            if (backedge_pair[i][1]>=backedge_pair[j][2]&&!tailnodualhash[backedge_pair[j][2]])
            {
                tailaugment++;
                tailnodualhash[backedge_pair[j][2]] = true;
            }
            
            if (backedge_pair[i][0]>=backedge_pair[j][2]&&!headnodualhash[backedge_pair[j][2]])
            {
                headaugment++;
                headnodualhash[backedge_pair[j][2]] = true;
            }
            
        }
        backedge_pair[i][1]+=tailaugment;
        backedge_pair[i][0]+=headaugment;
        
    }
    /*-----------calculate nloop---------------------*/
    stack<int> container;
    container.push(backedge_pair[0][2]);
    for (int i = 1; i<backedge_pair.size(); i++)
    {
        if (backedge_pair[i][2]!=container.top())
            container.push(backedge_pair[i][2]);
    }
    
    this->nloop =(int)container.size();
    
    
    
//    for (int i = 0; i<backedge_pair.size(); i++)
//    {
//        cout<<"after backedges: "<<backedge_pair[i][0]<<" "<<backedge_pair[i][1]<<" "<<backedge_pair[i][2]<<endl;
//    }

    

}
/*
^
|
|
|
|
|
|
*/
void Graph::alter_Prelist(int head, int tail,bool* markhead=NULL)
{
    
    
    simple_instr *preheader_instr;
    simple_instr *p_instr;
    
    
    vector<int> row(3);
    row[0] = head;
    row[1] = tail;
    row[2] = head;
    
    this->backedge_pair.push_back(row);
    
    if (!markhead[head])
    {
        simple_instr* preheader_instr = new_instr(LABEL_OP, simple_type_void);
        preheader_instr->u.label.lab=new_label();
        preheader_instr->u.label.lab->name = "LOOP";
        this->insert_Instr(preheader_instr, head);
    }
    
    markhead[head] = true;
    
    
    
    /*
     simple_instr* p = this->inlist;
     while (p)
     {
     if (p->opcode == LABEL_OP)
     cout<<"label after: "<<p->u.label.lab->name<<endl;
     p = p->next;
     }
     */

       
}


/*insert before*/
void Graph::insert_Instr(simple_instr* preheader_instr, int block_index)
{
    simple_instr* p_instr = this->graph[block_index].data.leader;
    /*insert current*/
    if (block_index == 1) //insert before the first instruction
    {
        //cout<<"head insert "<<endl;
        preheader_instr->next = p_instr;
        p_instr->prev = preheader_instr;
        this->inlist = this->inlist->prev;
//        cout<<"inserted: "<<simple_op_name(this->inlist->opcode)<<endl;
//        cout<<"origin leader: "<<simple_op_name(this->inlist->next->opcode)<<endl;
    }
    else
    {
        preheader_instr->next = p_instr;
        p_instr->prev->next = preheader_instr;
        preheader_instr->prev = p_instr->prev;
        p_instr->prev = preheader_instr;
    }
    
}



/*rebuild graph according to newly updated inlist*/

void Graph::destruct_Graph(VNode* target)
{
    for (int i = 0; i<this->nblk+2; i++)
    {
        while(target[i].firstarc)
        {
            ArcNode *g = target[i].firstarc;
            target[i].firstarc = g->nextarc;
            g->nextarc = NULL;
            free(g);
        }
    }
    delete[] target;
}

void Graph::destruct_Graph(Dominator* target)
{
    for (int i = 0; i<this->nblk+2; i++)
    {
        while(target[i].firstarc)
        {
            ArcNode *g = target[i].firstarc;
            target[i].firstarc = g->nextarc;
            g->nextarc = NULL;
            free(g);
        }
    }
    delete[] target;
}

void Graph::rebuild_Graph()
{
    if (this->backedge_pair.empty()) return;
    this->destruct_Graph(this->graph);
    this->destruct_Graph(this->antigraph);
    this->destruct_Graph(this->proper_domi_graph);
    this->build_Graph();
    /****************************/
    /****************************/
    /****************************/
    //this->redirect_Graph();//we need to move nonloop linkin to preheader
    /****************************/
    /****************************/
    /****************************/
}




void Graph::build_Loop_Graph()
{
    this->find_Backedges();
    this->rebuild_Graph();
    this->find_Proper_Domi(true);
    if (this->backedge_pair.empty()) return;
    this->loop_graph = new LoopG[this->nloop];
    for (int i = 0; i<nloop; i++)
    {
        this->loop_graph[i].ndomi = 0;
        //   this->direct_domi_graph[i].ndomi = 0;
        
        this->loop_graph[i].firstarc = NULL;
        //   this->direct_domi_graph[i].firstarc = NULL;
    }

    
    int loop_index = -1;//to make sure the first be 0
    int dual = -1;
    bool isdual = false;
    //this->find_Loop_Members(0, 1,false);
    for (int pair_index = 0; pair_index<backedge_pair.size(); pair_index++)
    {
        if (backedge_pair[pair_index][2]!=dual)
        {
            isdual = false;
            loop_index++;
            dual = backedge_pair[pair_index][2];
        }
        else
        {
            isdual = true;
        }

        this->find_Loop_Members(loop_index, pair_index,isdual);
    }
    
}


void Graph::find_Loop_Members(int loop_index, int pair_index, bool isdual)
{
    if (this->backedge_pair.empty()) return;
    int head = this->backedge_pair[pair_index][0];
    int tail = this->backedge_pair[pair_index][1];
    
    LoopG* shadow = new LoopG;
    shadow->ndomi = 0;
    shadow->firstarc = NULL;
    ArcNode *node = new ArcNode;//member
    node->adjvex = head;
    node->nextarc = shadow->firstarc;
    shadow->firstarc = node;
    shadow->ndomi = 1;

    
        
    this->insert_On_Stack(tail, shadow);
    //cout<<"Loop Member "<<head<<" "<<tail<<endl;
    int m = -1;
    ArcNode *pred;
    while(!S.empty())
    {
        m = this->S.top();
        this->S.pop();
        pred = this->antigraph[m].firstarc;
        while (pred)
        {
            //cout<<m<<" Stack: "<<pred->adjvex<<endl;
            insert_On_Stack(pred->adjvex,shadow);
            pred = pred->nextarc;
        }
    }
    
    if (!isdual)//not dual
    {
        //cout<<"****not dual****"<<endl;
        loop_graph[loop_index].firstarc = shadow->firstarc;
        loop_graph[loop_index].ndomi = shadow->ndomi;
        shadow->firstarc = NULL;
    }
    else//dual, do union
    {
        ArcNode *p = loop_graph[loop_index].firstarc;
        ArcNode *q;
        bool doinsert = true;
        while (shadow->firstarc)
        {
            q = shadow->firstarc;
            p = loop_graph[loop_index].firstarc;
            doinsert = true;
            while (p)
            {
                //cout<<"compare: "<<q->adjvex<<" "<<p->adjvex;
                if (q->adjvex == p->adjvex)
                {
                   // cout<<" delete"<<endl;
                    doinsert = false;
                    shadow->firstarc = q->nextarc;
                    q->nextarc = NULL;
                    break;
                }
                //cout<<endl;
                p = p->nextarc;
            }
            if (doinsert)
            {
                shadow->firstarc = q->nextarc;
                q->nextarc = loop_graph[loop_index].firstarc;
                loop_graph[loop_index].firstarc = q;
                loop_graph[loop_index].ndomi++;
            }
            
        }
        shadow->firstarc = NULL;
    }
    
    free(shadow);
    
    /*check*/
    
    
    /*
     
     ArcNode *p = loop_graph[loop_index].firstarc;
     while(p)
     {
     cout<<p->adjvex<<" ";
     p = p->nextarc;
     }
     cout<<endl;
     
     pred = this->antigraph[8].firstarc;
     cout<<"predecessor "<<endl;
     while (pred)
     {
     cout<<pred->adjvex<<" ";
     pred = pred->nextarc;
     }
     cout<<endl;
    */
}

void Graph::insert_On_Stack(int t, LoopG* shadow)
{
    ArcNode *p = shadow->firstarc;
    while(p)
    {
        //cout<<"Hi "<<t<<" "<<p->adjvex<<endl;
        if (t == p->adjvex) return;
        p = p->nextarc;
    }
    ArcNode *node = new ArcNode;
    node->adjvex = t;
    node->nextarc = shadow->firstarc;
    shadow->firstarc = node;
    shadow->ndomi++;
    
    /*push_back_stack*/
    this->S.push(t);
}





//replace before
void Graph::replace_Instr(simple_instr* preheader_instr, int block_index)
{
    simple_instr* p_instr = this->graph[block_index].data.leader->prev;
    if (block_index==1)
    {
        p_instr->prev->next = preheader_instr;
        p_instr->next->prev = preheader_instr;
        p_instr->next = NULL;
        free_instr(p_instr);

    }
    else
    {
        preheader_instr->prev = p_instr->prev;
        p_instr->prev->next = preheader_instr;
        preheader_instr->prev = p_instr->next;
        p_instr->next->prev = preheader_instr;
        
        p_instr->next = NULL;
        p_instr->prev = NULL;
        free_instr(p_instr);
    }
    

}

void Graph::print_Loop_Graph()
{
    cout<<"loopcfg "<<this->proc_name<<" "<<this->nblk+2<<endl;
    for (int i = 0; i<this->nblk+2; i++)
    {
        cout<<"block "<<i<<endl;
        cout<<"\tinstrs "<<graph[i].data.ninstr;
        simple_instr *p;
        p = graph[i].data.leader;
        while(p)
        {
            int index = this->line_hash_map[(int)p];
            /*in print form, instr starts from 1*/
            cout<<" "<<index-1;
            if (p == graph[i].data.tail)
                break;
            p = p->next;
        }
        cout<<endl;
        /*print successors*/
        cout<<"\tsuccessors "<<graph[i].data.nsuccessor;
        int successorlist[graph[i].data.nsuccessor];
        int l = 0;
        ArcNode *q;
        q = graph[i].firstarc;
        while(q)
        {
            //cout<<" "<<q->adjvex;
            successorlist[l] = q->adjvex;
            q = q->nextarc;
            l++;
        }
        q_sort(successorlist,0,graph[i].data.nsuccessor-1);
        for (int j = 0; j<graph[i].data.nsuccessor; j++)
            cout<<" "<<successorlist[j];
        cout<<endl;
        
        
        /*print predecessors*/
        cout<<"\tpredecessors "<<antigraph[i].data.npredecessor;
        int predecessorlist[antigraph[i].data.npredecessor];
        q = this->antigraph[i].firstarc;
        l = 0;
        while(q)
        {
            //cout<<" "<<q->adjvex;
            predecessorlist[l] = q->adjvex;
            q = q->nextarc;
            l++;
        }
        q_sort(predecessorlist,0,antigraph[i].data.npredecessor-1);
        for (int j = 0; j<antigraph[i].data.npredecessor; j++)
            cout<<" "<<predecessorlist[j];
        cout<<endl;
    }
    /*print loopcfg*/
    cout<<"natural_loops "<<this->proc_name<<" "<<this->nloop;
    
    int tailgroup[this->nloop][this->nblk+5];
    //first for head, second for num, third for which loop
    
    
    int row = -1;
    int dual = -1;
    int col = 0;
    for (int i = 0; i<this->nloop; i++)
    {
        for (int j = 0; j<this->nblk+4; j++)
            tailgroup[i][j] = 0;
    }
    for (int i = 0; i<this->backedge_pair.size(); i++)
    {
        if (backedge_pair[i][0] != dual)
        {
            row++;
            tailgroup[row][0] = backedge_pair[i][0];
            tailgroup[row][2] = row;
            tailgroup[row][3] = backedge_pair[i][1];
            tailgroup[row][1]++;
            col = 4;
            dual = backedge_pair[i][0];
        }
        else
        {
            tailgroup[row][col] = backedge_pair[i][1];
            tailgroup[row][2] = row;
            tailgroup[row][1]++;
            col++;
        }
    }
    
    /**********************************************/
    /**********************************************/
    /**************sort back edge head****************/
    int a[this->nblk+5];
    for (int i = this->nloop; i>0; i--)
    {
        for (int j = 0; j<i-1; j++)
        {
            if (tailgroup[j][0]>tailgroup[j+1][0])
            {
                for (int k = 0; k<this->nblk+5; k++)
                {
                    a[k] = tailgroup[j+1][k];
                    tailgroup[j+1][k] = tailgroup[j][k];
                    tailgroup[j][k] = a[k];
                }
               
            }
        }
    }
    
    
    
    /**********************************************/
    /**********************************************/
    
    
    
    
//    for (int i = 0; i<this->nloop; i++)
//    {
//        cout<<endl;
//        for (int j = 0; j<this->nblk+4; j++)
//            cout<<" "<<tailgroup[i][j];
//        cout<<endl;
//    }
    
    for (int i= 0; i<this->nloop; i++)
    {
        this->q_sort(tailgroup[i],3,tailgroup[i][1]+2);
        cout<<endl<<"natloop "<<i<<endl;
        cout<<"\tbackedge";
        cout<<" "<<tailgroup[i][0];
        for (int j = 0; j<tailgroup[i][1]; j++)
            cout<<" "<<tailgroup[i][j+3];
        cout<<endl<<"\tpreheader "<<tailgroup[i][0]-1;
        
        cout<<endl<<"\tloop_blocks";
        
        ArcNode *p = loop_graph[tailgroup[i][2]].firstarc;
        int loop_block_list[loop_graph[tailgroup[i][2]].ndomi];
        int k = 0;
        while(p)
        {
            loop_block_list[k++] = p->adjvex;
            p = p->nextarc;
        }
        
        this->q_sort(loop_block_list,0,loop_graph[tailgroup[i][2]].ndomi-1);
        
        for (k = 0; k<loop_graph[tailgroup[i][2]].ndomi; k++)
            cout<<" "<<loop_block_list[k];
    }
    cout<<endl;
    
}



bool Graph::check_Domi(int head, int tail)
{
    ArcNode *p = proper_domi_graph[tail].firstarc;
    while(p)
    {
        if (p->adjvex == head)
            return true;
        p = p->nextarc;
    }
    return false;
}


void Graph::q_sort(int *numbers, int left, int right)
{
    int pivot, l_hold, r_hold;
    
    l_hold = left;
    r_hold = right;
    pivot = numbers[left];
    while (left < right)
    {
        while ((numbers[right] >= pivot) && (left < right))
            right--;
        if (left != right)
        {
            numbers[left] = numbers[right];
            left++;
        }
        while ((numbers[left] <= pivot) && (left < right))
            left++;
        if (left != right)
        {
            numbers[right] = numbers[left];
            right--;
        }
    }
    numbers[left] = pivot;
    pivot = left;
    left = l_hold;
    right = r_hold;
    if (left < pivot)
        q_sort(numbers, left, pivot-1);
    if (right > pivot)
        q_sort(numbers, pivot+1, right);
}


pair<VNode*, VNode*> Graph::getGraph()
{
    return make_pair(this->graph, this->antigraph);
}

simple_instr* Graph::getInlist()
{
    return this->inlist;
}


int Graph::get_Nblk()
{
    return this->nblk;
}

__gnu_cxx::hash_map<int,int> Graph::getInlistLineHash()
{
    return this->line_hash_map;
}

int* Graph::getBelongtoWhichBlock()
{
    return this->belongtowhichblock;
}

__gnu_cxx::hash_map<int,int> Graph::get_Anti_Line_Hash()
{
    return this->anti_line_hash_map;
}


LoopG* Graph::get_Loop_Graph()
{
    return this->loop_graph;
}

int Graph::get_nloop()
{
    return this->nloop;
}

int Graph::get_ninstr()
{
    return this->ninstr;
}

Dominator* Graph::get_proper_dominator_graph()
{
    return this->proper_domi_graph;
}
















