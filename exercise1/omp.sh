#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="omp"
#SBATCH --get-user-env
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=02:00:00

date
pwd
hostname

module load architecture/AMD
module load openMPI/4.1.4/gnu/12.2.1

make

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=64

k=1000

./main.x -i -k $k -f init$k.pbm

echo size,cores,time >> omp$k.csv

for i in {1..64}
do
    export OMP_NUM_THREADS=$i
    for j in {1..5}
    do
        echo -n $k,$i, >> omp$k.csv
        mpirun -np 1 -map-by node --bind-to socket ./main.x -r -n 100 -f init$k.pbm >> omp$k.csv
    done
done

make image
make clean

