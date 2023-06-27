#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="smpi"
#SBATCH --get-user-env
#SBATCH --partition=EPYC
#SBATCH --nodes=3
#SBATCH --exclusive
#SBATCH --time=02:00:00

date
pwd
hostname

module load architecture/AMD
module load openMPI/4.1.4/gnu/12.2.1

make

export OMP_PLACES=threads
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=1

k=10000

./main.x -i -k $k -f init$k.pbm

echo size,cores,time >> smpi$k.csv

for i in {1..128}
do
    for j in {1..5}
    do
        echo -n $k,$i, >> smpi$k.csv
        mpirun -np $i --map-by socket ./main.x -r -n 100 -f init$k.pbm >> smpi$k.csv
    done
done

make clean
make image
