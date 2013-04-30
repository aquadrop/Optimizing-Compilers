/*  Print Simple Instructions */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <suif_copyright.h>

#include <assert.h>
#include <stdio.h>
extern "C"{
#include <simple.h>
}
extern "C"{
#include "print.h"
}



/*  Print out a simple_instr.  There is nothing magic about the
    output format.  I tried to make it look like some sort of combination
    of C and a generic assembly language so it would be familiar.  */

void
fprint_instr (FILE *fd, simple_instr *s)
{
    switch (s->opcode) {

	case LOAD_OP: {
	    fprintf(fd, "\t%s ", simple_op_name(s->opcode));
	    fprint_type(fd, s->type);
	    fprintf(fd, "\t");
	    fprint_reg(fd, s->u.base.dst);
	    fprintf(fd, " = *");
	    fprint_reg(fd, s->u.base.src1);
	    fprintf(fd, "\n");
	    break;
	}

	case STR_OP: {
	    fprintf(fd, "\t%s\t\t*", simple_op_name(s->opcode));
	    fprint_reg(fd, s->u.base.src1);
	    fprintf(fd, " = ");
	    fprint_reg(fd, s->u.base.src2);
	    fprintf(fd, "\n");
	    break;
	}

	case MCPY_OP: {
	    fprintf(fd, "\t%s\t\t*", simple_op_name(s->opcode));
	    fprint_reg(fd, s->u.base.src1);
	    fprintf(fd, " = *");
	    fprint_reg(fd, s->u.base.src2);
	    fprintf(fd, "\n");
	    break;
	}

	case LDC_OP: {
	    fprintf(fd, "\t%s ", simple_op_name(s->opcode));
	    fprint_type(fd, s->type);
	    fprintf(fd, "\t");
	    fprint_reg(fd, s->u.ldc.dst);
	    fprintf(fd, " = ");
	    fprint_immed(fd, &s->u.ldc.value);
	    fprintf(fd, "\n");
	    break;
	}

	case JMP_OP: {
	    fprintf(fd, "\t%s\t\t%s\n",
		    simple_op_name(s->opcode),
		    s->u.bj.target->name);
	    break;
	}

	case BTRUE_OP:
	case BFALSE_OP: {
	    fprintf(fd, "\t%s\t\t", simple_op_name(s->opcode));
	    fprint_reg(fd, s->u.bj.src);
	    fprintf(fd, ", %s\n", s->u.bj.target->name);
	    break;
	}

	case CALL_OP: {
	    unsigned n, nargs;

	    fprintf(fd, "\t%s ", simple_op_name(s->opcode));
	    fprint_type(fd, s->type);
	    fprintf(fd, "\t");
	    if (s->u.call.dst != NO_REGISTER) {
		fprint_reg(fd, s->u.call.dst);
		fprintf(fd, " = ");
	    }
	    fprintf(fd, "*");
	    fprint_reg(fd, s->u.call.proc);
	    fprintf(fd, " (");

	    /* print the list of arguments */
	    nargs = s->u.call.nargs;
	    if (nargs != 0) {
		for (n = 0; n < nargs - 1; n++) {
		    fprint_reg(fd, s->u.call.args[n]);
		    fprintf(fd, ", ");
		}
		fprint_reg(fd, s->u.call.args[nargs - 1]);
	    }
	    fprintf(fd, ")\n");
	    break;
	}

	case MBR_OP: {
	    unsigned n, ntargets;

	    fprintf(fd, "\t%s\t\t", simple_op_name(s->opcode));
	    fprint_reg(fd, s->u.mbr.src);

	    ntargets = s->u.mbr.ntargets;
	    if (ntargets == 0) {
		fprintf(fd, " ( ), %s\n", s->u.mbr.deflab->name);
	    } else {
		fprintf(fd, " (%d to %d), %s, (",
			s->u.mbr.offset,
			s->u.mbr.offset + ntargets - 1,
			s->u.mbr.deflab->name);

		/* print the list of branch targets */
		for (n = 0; n < ntargets - 1; n++) {
		    fprintf(fd, "%s, ", s->u.mbr.targets[n]->name);
		}
		fprintf(fd, "%s)\n", s->u.mbr.targets[ntargets - 1]->name);
	    }
	    break;
	}

	case LABEL_OP: {
	    fprintf(fd, "%s:\n", s->u.label.lab->name);
	    break;
	}

	case RET_OP: {
	    fprintf(fd, "\t%s\t\t", simple_op_name(s->opcode));
	    if (s->u.base.src1 != NO_REGISTER) {
		fprint_reg(fd, s->u.base.src1);
	    }
	    fprintf(fd, "\n");
	    break;
	}

	case CVT_OP:
	case CPY_OP:
	case NEG_OP:
	case NOT_OP: {
	    /* unary base instructions */
	    fprintf(fd, "\t%s ", simple_op_name(s->opcode));
	    fprint_type(fd, s->type);
	    fprintf(fd, "\t");
	    fprint_reg(fd, s->u.base.dst);
	    fprintf(fd, " = ");
	    fprint_reg(fd, s->u.base.src1);
	    fprintf(fd, "\n");
	    break;
	}

	default: {
	    /* binary base instructions */
	    assert(simple_op_format(s->opcode) == BASE_FORM);
	    fprintf(fd, "\t%s ", simple_op_name(s->opcode));
	    fprint_type(fd, s->type);
	    fprintf(fd, "\t");
	    fprint_reg(fd, s->u.base.dst);
	    fprintf(fd, " = ");
	    fprint_reg(fd, s->u.base.src1);
	    fprintf(fd, ", ");
	    fprint_reg(fd, s->u.base.src2);
	    fprintf(fd, "\n");
	}
    }
}



/*  Print a simple type.  The format is (x.n) where x represents the base
    type and n is the number of bits (e.g. (s.32) for signed 32-bit type).  */

void
fprint_type (FILE *fd, simple_type *t)
{
    char tag;
    switch (t->base) {
	case VOID_TYPE: {
	    tag = 'v';
	    break;
	}
	case ADDRESS_TYPE: {
	    tag = 'a';
	    break;
	}
	case SIGNED_TYPE: {
	    tag = 's';
	    break;
	}
	case UNSIGNED_TYPE: {
	    tag = 'u';
	    break;
	}
	case FLOAT_TYPE: {
	    tag = 'f';
	    break;
	}
	case RECORD_TYPE: {
	    tag = 'r';
	    break;
	}
	default: {
	    simple_error("unknown simple_type_base");
	}
    }
    fprintf(fd, "(%c.%d)", tag, t->len);
}



/*  Print out an immediate value.  Strings are printed with quotation marks
    around them.  Nothing else very exciting....  */

void
fprint_immed (FILE *fd, simple_immed *v)
{
    switch (v->format) {
	case IMMED_INT: {
	    fprintf(fd, "%d", v->u.ival);
	    break;
	}
	case IMMED_FLOAT: {
	    fprintf(fd, "%f", v->u.fval);
	    break;
	}
	case IMMED_SYMBOL: {
	    fprintf(fd, "&%s ", v->u.s.symbol->name);
	    if (v->u.s.offset < 0) {
		fprintf(fd, "- %d", -v->u.s.offset);
	    } else {
		fprintf(fd, "+ %d", v->u.s.offset);
	    }
	    break;
	}
	default: {
	    simple_error("unknown immed_type");
	}
    }
}



void
fprint_reg (FILE *fd, simple_reg *r)
{
#ifdef USE_VAR_NAMES
    fprintf(fd, "%s", r->var->name);
#else
    switch (r->kind) {
	case MACHINE_REG: {
	    fprintf(fd, "m%d", r->num);
	    break;
	}
	case TEMP_REG: {
	    fprintf(fd, "t%d", r->num);
	    break;
	}
	case PSEUDO_REG: {
	    fprintf(fd, "r%d", r->num);
	    break;
	}
	default: {
	    simple_error("unknown register kind");
	}
    }
#endif
}

