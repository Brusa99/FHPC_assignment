#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="blis_core"
#SBATCH --get-user-env
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --cpus-per-task=64
#SBATCH --exclusive
#SBATCH --time=02:00:00

module load architecture/AMD
module load mkl
module load openBLAS/0.3.21-omp

export LD_LIBRARY_PATH=/u/dssc/sbrusa00/myblis/lib:$LD_LIBRARY_PATH

export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export BLIS_NUM_THREADS=1

m=10000

echo cores,m,k,n,time,GFLOPS >> ~/scratch/blis_core_double.csv

for i in {1..64}
do
	export export BLIS_NUM_THREADS=$i
	for j in {1..5}
	do
		echo -n $i, >> ~/scratch/blis_core_double.csv
		../gemm_blis.x $m $m $m >> ~/scratch/blis_core_double.csv
	done
done
