/* Generated automatically by the SUIF makefiles. */

#include <suif.h>

extern void start_suif(int &argc, char *argv[])
  {
    LIBRARY(check, init_check, exit_check);
    LIBRARY(useful, init_useful, exit_useful);

    init_suif(argc, argv);
  }
