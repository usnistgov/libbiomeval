#!/bin/sh
#
# Script used to run the test_be_mpi test program for the Evaluation
# framework MPI parallel processing classes.
#
##############################################################################
#
#
# Record store for the input.
#
INPUTRS=test_data/AN2KRecordStore

#
# Create the properties file for this run
#
PROPS=test_be_mpi.props
cat > $PROPS << EOF
Input Record Store = $INPUTRS
Chunk Size = 4
Workers Per Node = 2
Logsheet URL = file://mpi.log
Record Logsheet URL = file://record.log
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
PROGRAM=test_be_mpi
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
# Test with keys only.
#
EXECSTR="$PROGRAM"
time mpirun -x LD_LIBRARY_PATH -x DYLD_LIBRARY_PATH -mca btl tcp,self -H $MPINODES -np $MPIPROCS $EXECSTR

#
# Test with keys and values; there just has to be one argument to the exec,
# it doesn't matter what it is.
#
EXECSTR="$PROGRAM values"
time mpirun -x LD_LIBRARY_PATH $MPIOPTS -H $MPINODES -np $MPIPROCS $EXECSTR
