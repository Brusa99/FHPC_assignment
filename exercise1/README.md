# Exercise 1

The exercise consist in implementing _Conway's game of life_, parallelizing it with _OpenMP_ and _MPI_, and then measure its performance.  
More details on the implementation can be found in `../report.pdf`.  

## Compilation

The `Makefile` takes care of the compilation. It uses the flags `-O3 -march=native` to optimize the code, the flags `-Wall -Wextra` for warnings and `-fopenmp` to enable _OpenMP_.  
To compile the file it is required to have _OpenMPI_ installed on your system. On orfeo you can run:

```
module load architecture/AMD
module load openMPI/4.1.4/gnu/12.2.1
make
```

## Running

The executable is stored in `main.x`. It can run in two modes: _initialize_ and _run_.  

### Initialize

To initialize a random pbm image of side _K_ and save it to _NAME_ run:

```
./main.x -i -k $K -f $NAME
```

Note that this process does not require _MPI_.

### Running

To run the game for 100 iterations, starting from the _NAME_ image, on _P_ processes, run:

```
mpirun -np $P ./main.x -r -f $NAME
```

To properly parallelize the game make sure to export _OMP_ enviroment variables correctly as to work with the hybrid _MPI_ implementation.  
Optional flags include:

- `-n $N` to change the number of iterations to _N_.  
- `-s $S` to save a dump (named `snapshotXXXXXX.pbm`) every _S_ iterations. Defaults to 0, meaning only at the end.  
- `-e $E` where $E \in \{0,1\}$. Changes the evolution type: 1 stands for static (default), 0 for ordered.

## Contents

- `Makefile` used to compile the code.  
- `source/` source directory. The C code is stored here.  
    - `main.c` contains the main. Reads arguments and calls one of the modes.  
    - `run.c` code for the running modes.  
    - `initialize.c` code for the initialize mode.  
    - `read_write.c` code for reading and writing pbm images (not done by me).  
- `headers/` headers directory. To include if you decide to compile manually.  
    - `initialize.h`
    - `run.h` 
    - `read_write.h`
- `objects/` objects directory. The makefile will store here the objects file, to link them in the final step.  
    - `.gitignore` this file is here to make git track the directory.  
- `*.sh` bash scripts to gather data on orfeo. They should be modified opportunely with desired parameters (size, steps, places, etc..) for use.
- `data/` here the data used for the report is stored.  
