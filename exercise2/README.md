# Exercise 2

This exercise consinst on measuring the performance of 3 math libraries (BLIS, OpenBLAS and MKL) on EPYC and THIN nodes on the orfeo cluster.

## Contents

- `dgemm.c` is the original file provided to run the benchmark.  
- `gemm.c` is a modified version, it prints size of the matrices, elapsed time and GFLOPS in a csv format.  
- `Makefile` file used to compile the code. It creates 3 executables (one per library). To change precision use the flags **USE_FLOAT** or **USE_DOUBLE**.  
- `scripts/` contains the scripts used to gather the data. They should be opportunely modified to match the desired output.  _i.e.:_ --partition=EPYC to --partition=THIN, ../gemm_oblas.x to ../gemm_mkl.x  
- 'data/` contains the `.csv` files used to produce the plots. Data gathering details are explained in the report.  
