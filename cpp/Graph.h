//
//  Graph.h
//  A1
//
//  Created by Jianwei Xu on 19/01/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#ifndef __A1__Graph__
#define __A1__Graph__

#include <iostream>
#include <ext/hash_map>
extern "C" {
#include <simple.h>
}

#include <stack>
#include <vector>


using namespace std;

typedef struct VertexType
{
    simple_instr *leader;
    simple_instr *tail;
    int ninstr;//num of instr in the node 0 to be the entry;-1 to be exit
    int blkindex;
    int nsuccessor;
    int npredecessor;
}VertexType;

typedef struct ArcNode
{
    int adjvex;
    struct ArcNode *nextarc;
}ArcNode;

typedef struct VNode//head node for adjacency list
{
    VertexType data;
    ArcNode *firstarc;
}VNode;

typedef struct Dominator
{
    int ndomi;
    ArcNode *firstarc;
}Dominator,LoopG;

class Graph
{
    simple_instr *inlist;
    char *proc_name;
    __gnu_cxx::hash_map<char*, int>label_hash_map;
    __gnu_cxx::hash_map<int, int>line_hash_map;
    __gnu_cxx::hash_map<int, int>anti_line_hash_map;
    bool *leader_index;
    int *belongtowhichblock;
    int ninstr;
    int nblk;
    VNode *graph;
    VNode *antigraph;
    
    bool **proper_domi;
    Dominator *proper_domi_graph;
    Dominator *direct_domi_graph;
    int *dir_domi;
    
    vector<vector<int> > backedge_pair;
    stack<int> S;
    int nloop;
    LoopG *loop_graph;
    int *loop_tail;
    int *loop_head;
    
public:
    
    pair<VNode*, VNode*> getGraph();
    int get_Nblk();
    simple_instr* getInlist();
    __gnu_cxx::hash_map<int,int> getInlistLineHash();
    __gnu_cxx::hash_map<int, int> get_Anti_Line_Hash();
    int* getBelongtoWhichBlock();
    
    LoopG * get_Loop_Graph();
    int get_nloop();
    
    int get_ninstr();
    
    
    Dominator* get_proper_dominator_graph();
    
    Graph();
    Graph(simple_instr*, char*);
    
    /*basic module, graph*/
    void build_Graph();
    void print_Graph();
    
    
    void find_Dir_Domi();
    void print_Dir_Domi();
    
    
    /*module loop graph*/
    void build_Loop_Graph();
    void print_Loop_Graph();

private:
    /*basic module, graph*/
    void init_Hash();
    void find_Leaders();
    void init_Graph();
    void init_AntiGraph();
    void build_Blocks();
    void link_Blocks();
    void find_Proper_Domi(bool);
    
    /*module loop graph*/
    void alter_Prelist(int, int, bool*);
    void alter_Afterlist();
    
    void enter_Node(int,int);
    
    bool check_Domi(int, int);
    void insert_Instr(simple_instr*, int);//before
    void replace_Instr(simple_instr*, int);//before
    void insert_On_Stack(int,LoopG*);
    void destruct_Graph(VNode*);
    void destruct_Graph(Dominator*);
    void alter_Link_to_Preheader(int, int);

    void find_Backedges();
    void rebuild_Graph();
    void find_Loop_Members(int, int, bool);

protected:
    
    void q_sort(int*, int, int);

};





































#endif /* defined(__A1__Graph__) */
