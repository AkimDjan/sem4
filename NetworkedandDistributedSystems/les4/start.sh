#!/bin/bash

#PBS -l walltime=00:01:00,nodes=3:ppn=2
#PBS -N christian_mpi
#PBS -q batch

cd "$PBS_O_WORKDIR"
mpirun --hostfile "$PBS_NODEFILE" -n 1 ./server : -n 5 ./client
