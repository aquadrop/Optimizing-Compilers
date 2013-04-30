#!/usr/bin/perl

use strict;
use warnings;

use Data::Dumper;
use File::Basename;

my $TEST_CASE = shift;

my $ECE540_DIR = "/homes/x/xujianwe/Compilers";
my $ECE540_TESTCASES_DIR = "/homes/x/xujianwe/Compilers/Project/ptest";

my $ECE540_EXE = "$ECE540_DIR/Project/cpp/project";
my $ECE540_EXE_DIR = dirname ($ECE540_EXE);

my $ECE540_REFERENCE_EXE = "$ECE540_DIR/Project/cpp/hw2";

if (! -d $ECE540_DIR || 
    ! -d $ECE540_TESTCASES_DIR ||
    ! -d $ECE540_EXE_DIR ) {
    print STDERR "[ERROR]: related file not found!\n";
    exit 1;
}




#my @testcase = glob ("$ECE540_TESTCASES_DIR/*.tmp");
#print Dumper \@testcase;

#if (scalar @testcase == 0) {
 #   print "[ERROR]: no testcases found!\n";
 #   exit 1;
#}

my $TEST_CASE_NAME = $ECE540_TESTCASES_DIR."/".$TEST_CASE;
my $TEST_CASE_TMP = $ECE540_TESTCASES_DIR."/".$TEST_CASE.".tmp";
my $TEST_CASE_SPX = $ECE540_TESTCASES_DIR."/".$TEST_CASE.".spx";
my $TEST_CASE_SS = $ECE540_TESTCASES_DIR."/".$TEST_CASE.".ss";
my $TEST_CASE_SPX_C = $TEST_CASE_SPX.".c";	
#print "$TEST_CASE_SPX\n";
#clean $TESt_CASE."tmp" file


system("rm -f $TEST_CASE_SPX");
system("rm -f $TEST_CASE_SS");
system("rm -f $TEST_CASE_SPX_C");
system("$ECE540_REFERENCE_EXE $TEST_CASE_TMP $TEST_CASE_SPX");
system("ssbe $TEST_CASE_NAME");
#system("$ECE540_TESTCASES_DIR/evaluate-cost $TEST_CASE_SS>$time");
my $rtime = 0.0;
$rtime = qx($ECE540_TESTCASES_DIR/evaluate-cost $TEST_CASE_SS);
print "$rtime \n";

system("rm -f $TEST_CASE_SPX");
system("rm -f $TEST_CASE_SS");
system("rm -f $TEST_CASE_SPX_C");
system("$ECE540_EXE $TEST_CASE_TMP $TEST_CASE_SPX");
system("ssbe $TEST_CASE_NAME");
my $mtime = 0.0;
$mtime = qx($ECE540_TESTCASES_DIR/evaluate-cost $TEST_CASE_SS);
my $speedup = $rtime/$mtime;
#$rtime/$mtime;

printf "%.3f\n",$speedup; 
system("rm -f $TEST_CASE_SPX");
system("rm -f $TEST_CASE_SS");
system("rm -f $TEST_CASE_SPX_C");
exit 0;
