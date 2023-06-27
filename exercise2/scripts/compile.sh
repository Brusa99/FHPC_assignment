cd ..

module load architecture/AMD
module load mkl
module load openBLAS/0.3.21-omp

export LD_LIBRARY_PATH=/u/dssc/sbrusa00/myblis/lib:$LD_LIBRARY_PATH

srun -n1 -p EPYC make cpu
