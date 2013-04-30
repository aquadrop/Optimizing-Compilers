#!/usr/bin/perl

use strict;
use warnings;

use Data::Dumper;
use File::Basename;

my $LAB_NUM = shift;
my $REF_NUM = shift;

my $ECE540_DIR = "/homes/x/xujianwe/Compilers";
my $ECE540_TESTCASES_DIR = "/homes/x/xujianwe/Compilers/A".$LAB_NUM."/test";
my $ECE540_TEST_OUT = "$ECE540_DIR/A".$LAB_NUM."/cpp/test_out";

my $ECE540_EXE = "$ECE540_DIR/A".$LAB_NUM."/cpp/hw".$LAB_NUM;
my $ECE540_EXE_DIR = dirname ($ECE540_EXE);

my $ECE540_REFERENCE_EXE = "/cad2/ece540s/reference/assign".$REF_NUM."Ref";

if (! -d $ECE540_DIR || 
    ! -d $ECE540_TESTCASES_DIR ||
    ! -d $ECE540_EXE_DIR ) {
    print STDERR "[ERROR]: lab${LAB_NUM} not found!\n";
    exit 1;
}

if (! -d "$ECE540_TEST_OUT") {
    mkdir "$ECE540_TEST_OUT";
}

if (! -d "$ECE540_TEST_OUT/".$LAB_NUM."/") {
    mkdir "$ECE540_TEST_OUT/".$LAB_NUM."/";
}

system("cd $ECE540_EXE_DIR; make;");

my @testcase = glob ("$ECE540_TESTCASES_DIR/*.tmp");
#print Dumper \@testcase;

if (scalar @testcase == 0) {
    print "[ERROR]: no testcases found!\n";
    exit 1;
}

print "Testcase \t Result \t Log \n";
print "-------- \t ------ \t --- \n";

foreach (@testcase) {
    my $test = basename ($_);
    system("$ECE540_EXE $_ > $ECE540_TEST_OUT/$LAB_NUM/$test.out");
    system("$ECE540_REFERENCE_EXE $_ > $ECE540_TEST_OUT/$LAB_NUM/$test.ref");
    
    my $diff = `diff  $ECE540_TEST_OUT/$LAB_NUM/$test.ref  $ECE540_TEST_OUT/$LAB_NUM/$test.out`;

    
    if ($diff) {
        print "$test   \t  FAIL \t $diff \n";
    }
    else {
        print "$test   \t  PASS \t\t - \n";
    }
}

exit 0;
