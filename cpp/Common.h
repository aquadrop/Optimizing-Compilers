//
//  Common.h
//  Project
//
//  Created by Jianwei Xu on 15/04/2013.
//  Copyright (c) 2013 Jianwei Xu. All rights reserved.
//

#ifndef __Project__Common__
#define __Project__Common__

#include <iostream>
#include <stdio.h>
extern "C" {
#include <simple.h>
}
extern "C"{
#include "print.h"
}
#include<vector>

class Common
{
public:
    static void print_instruction(simple_instr* i);
};



#endif /* defined(__Project__Common__) */
