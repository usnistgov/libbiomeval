#!/bin/sh
#
# Script used to run the test_be_rs_mpi test program for the Evaluation
# framework MPI parallel processing classes.
#
##############################################################################
#
#
# Record store for the input.
#
INPUTRS=test_data/AN2KRecordStore
CHKPATH=/tmp

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
PROPS=test_be_rs_mpi.props
cat > $PROPS << EOF
Input Record Store = $INPUTRS
Chunk Size = 2
Workers Per Node = 1
Logsheet URL = file://mpi.log
Record Logsheet URL = file://record.log
Checkpoint Path = $CHKPATH
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
PROGRAM=test_be_rs_mpi
LIBS=""
MPIHOSTFN=mpi.hosts
CPFILES="$PROGRAM $PROPS $LIBS"

for n in $NODES; do
	echo $n;
	scp -p $CPFILES $n:$DIR;
done

echo "$MPINODES slots=$MPIPROCS" > $MPIHOSTFN

#
# Export the dynamic library path to each MPI task
#
export DYLD_LIBRARY_PATH=../../lib
export LD_LIBRARY_PATH=../../lib

#
# Test with keys only.
#
echo "----------------------------------------------------------------------"
echo "Running with checkpoint save"
echo "To send the clean shutdown, run:"
echo
echo "kill -QUIT \`cat $CHKPATH/Distributor.chk | grep PID | cut -d= -f2\`"
echo
EXECSTR="$PROGRAM -c"
time mpirun -x LD_LIBRARY_PATH -x DYLD_LIBRARY_PATH -mca btl tcp,self --hostfile $MPIHOSTFN -np $MPIPROCS $EXECSTR

echo "----------------------------------------------------------------------"
echo "Running with checkpoint restore and save"
EXECSTR="$PROGRAM -c"
time mpirun -x LD_LIBRARY_PATH -x DYLD_LIBRARY_PATH -mca btl tcp,self --hostfile $MPIHOSTFN -np $MPIPROCS $EXECSTR

#
# Test with keys and values.
#
echo "----------------------------------------------------------------------"
echo "Running with keys and values"
EXECSTR="$PROGRAM -v"
#time mpirun -x LD_LIBRARY_PATH $MPIOPTS --hostfile $MPIHOSTFN -np $MPIPROCS $EXECSTR

rm -f $MPIHOSTFN
