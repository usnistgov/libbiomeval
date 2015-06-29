#!/bin/sh
#
# Script used to run the test_be_csv_mpi test program for the Evaluation
# framework MPI parallel processing classes.
#
##############################################################################
#
#
# Record store for the input.
#
INPUTCSV=test_data/test.prop

#
# Create the properties file for this run
#
# Logsheet URL is used by the framework for logging and is optional.
# Record Logsheet URL is defined and used by the application and is
# optional in the test_be_rs_mpi program.
#
# An example config file for rsyslogd, listening on a non-default port:
#
#       $ModLoad imtcp
#       # Provides TCP syslog reception
#       $InputTCPServerRun 2514
#       local0.info /home/wsalamon/sandbox/rsyslog/record.log
#       local1.debug /home/wsalamon/sandbox/rsyslog/debug.log
#
# Example of starting rsyslogd:
#	rsyslogd -i $PWD/rsyslog.pid -f rsyslog.conf
#
# NOTE: rsyslogd requires full path names for files.
#
PROPS=test_be_csv_mpi.props
cat > $PROPS << EOF
Input CSV = $INPUTCSV
Chunk Size = 1
Read Entire File = NO
Randomize Lines = NO
Workers Per Node = 2
Logsheet URL = file://mpi.log
Record Logsheet URL = file://record.log
#Logsheet URL = syslog://linc01b:2514
#Record Logsheet URL = syslog://linc01b:2514
EOF

#
# Where the program is run. The directory must exist on all the
# nodes, and this script must be started here.
#
DIR=$PWD

#
# Two forms of the nodes string, one for the script to copy all
# files out, one for the mpirun command.
#
NODES="localhost"
MPINODES="localhost"

#
# MPIPROCS must be >= 2
#
MPIPROCS=3
PROGRAM=test_be_csv_mpi
LIBS=""
CPFILES="$PROGRAM $PROPS $LIBS"

for n in $NODES; do
	echo $n;
	scp -p $CPFILES $n:$DIR;
done

#
# Export the dynamic library path to each MPI task
#
export DYLD_LIBRARY_PATH=../../lib
export LD_LIBRARY_PATH=../../lib

#
# Test reading from a file
#
EXECSTR="$PROGRAM"
time mpirun -x LD_LIBRARY_PATH -x DYLD_LIBRARY_PATH -mca btl tcp,self -H $MPINODES -np $MPIPROCS $EXECSTR

#
# Test reading from a buffer
#
cat > $PROPS << EOF
Input CSV = $INPUTCSV
Chunk Size = 1
Read Entire File = YES
Randomize Lines = YES
Workers Per Node = 2
Logsheet URL = file://mpi.log
Record Logsheet URL = file://record.log
#Logsheet URL = syslog://linc01b:2514
#Record Logsheet URL = syslog://linc01b:2514
EOF
time mpirun -x LD_LIBRARY_PATH -x DYLD_LIBRARY_PATH -mca btl tcp,self -H $MPINODES -np $MPIPROCS $EXECSTR

#
# TODO: Test delimiters
#

