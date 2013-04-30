//
//  Common.cpp
//  Project
//
//  Created by Jianwei Xu on 15/04/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#include "Common.h"



void Common::print_instruction(simple_instr* i)
{
    int count = 0;
    while (i) {
        printf("%d ",++count);
        fprint_instr(stdout, i);
        i = i->next;
    }

}