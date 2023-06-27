#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="blas_size"
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
export OMP_NUM_THREADS=12

echo m,k,n,time,GFLOPS >> ~/scratch/blas_size_double_thin.csv


for m in 2000 3000 4000 5000 6000 7000 8000 9000 10000 11000 12000 13000 14000 15000 16000 17000 18000 19000 20000
do	
	for j in {1..10}
	do
		../gemm_oblas.x $m $m $m >> ~/scratch/blas_size_double_thin.csv
	done
done
