# Final Assignement FHPC course 2022/2023

In this directory you can find my resolution of the final assignement for the _Foundation of High Performance Computing_ course at University of Trieste, relative to the 2022/2023 course.  
More details on each exercise files can be found inside each directory's `README.md`. More details on the implementation and on the results achieved can be found in the `report.pdf` document. The document is the rendered format of `report.ipynb`. 

## Exercise 1

The exercise consists of implementing _Conway's game of life_, parallelizing it through _OpenMP_ and _MPI_ and measuring its performance.  
The code for the game is written in C. Plots are generated by python code. I used bash scripts to gather the data on the _ORFEO_ cluster.  

## Exercise 2

The exercise consists of measuring performance of _OpenBLAS_, _MKL_ and _BLIS_ libraries on the _ORFEO_ cluster on both INTEL and AMD nodes.
I slightly modified the code `dgemm.c` provided by the professor  to print relevant information in a `.csv` fashion. Then I used bash scripts to measure how the libraries scale, changing matrix sizes and cores used.
