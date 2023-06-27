#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="blas_core"
#SBATCH --get-user-env
#SBATCH --partition=THIN
#SBATCH --nodes=1
#SBATCH --cpus-per-task=12
#SBATCH --exclusive
#SBATCH --time=02:00:00

module load architecture/Intel
module load mkl
module load openBLAS/0.3.21-omp

export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=1

echo m,k,n,time,GFLOPS >> ~/scratch/blas_core_double_thin.csv


m=10000

for i in {1..12}
do
	export OMP_NUM_THREADS=$i
	for j in {1..5}
	do
		echo -n $i, >> ~/scratch/blas_core_double_thin.csv
		../gemm_oblas.x $m $m $m >> ~/scratch/blas_core_double_thin.csv
	done
done
