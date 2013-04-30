/*  Functions to Print Simple SUIF Objects */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <suif_copyright.h>

#ifndef PRINT_H
#define PRINT_H

/*  local function declarations */
extern void fprint_instr(FILE *fd, simple_instr *s);
extern void fprint_type(FILE *fd, simple_type *t);
extern void fprint_immed(FILE *fd, simple_immed *v);
extern void fprint_reg(FILE *fd, simple_reg *r);

#endif /* PRINT_H */
