#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <mpi.h>
#include <omp.h>

#include "read_write.h"
#include "run.h"

// set to 0 to disable the printing of the timiming
#define TIME 0

struct timespec diff(struct timespec start, struct timespec end)
{
        struct timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
                temp.tv_sec = end.tv_sec-start.tv_sec-1;
                temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
                temp.tv_sec = end.tv_sec-start.tv_sec;
                temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
}

int count_neighbours(int cell, int k, char *grid)
{
	// grid has lato = k
	// we want to evaluate the sum of the neighbours of cell

	int count = 0;

	int left = cell % k == 0 ? cell + k-1 : cell-1;
	int right = cell % k == k-1 ? cell -k +1 : cell+1;

	// row above cell
	count += (int)grid[left-k];
	count += (int)grid[cell-k];
	count += (int)grid[right-k];

	// cell row
	count += (int)grid[left];
	count += (int)grid[right];

	// row below cell
	count += (int)grid[left+k];
	count += (int)grid[cell+k];
	count += (int)grid[right+k];

	return count;
}

// if a single process is running the there is no row above the first row and no row below the last one
int count_neighbours_single(int cell, int k, const char *grid)
{
	// grid has lato = k
	// we want to evaluate the sum of the neighbours of cell

	int count = 0;

	int left = cell % k == 0 ? cell + k-1 : cell-1;
	int right = cell % k == k-1 ? cell -k +1 : cell+1;

	if (cell < k) //first row case
	{
		// row above cell is the last row
		count += (int)grid[left + k*k -k];
		count += (int)grid[cell + k*k -k];
		count += (int)grid[right + k*k -k];

		// cell row
		count += (int)grid[left];
		count += (int)grid[right];

		// row below cell
		count += (int)grid[left+k];
		count += (int)grid[cell+k];
		count += (int)grid[right+k];
	}
	else if (cell < k*k-k) //last row case
	{
		// row above cell
		count += (int)grid[left-k];
		count += (int)grid[cell-k];
		count += (int)grid[right-k];

		// cell row
		count += (int)grid[left];
		count += (int)grid[right];

		// row below cell is the first row
		count += (int)grid[right % k];
		count += (int)grid[cell % k];
		count += (int)grid[right % k];
	}
	else
	{
		// row above cell
		count += (int)grid[left-k];
		count += (int)grid[cell-k];
		count += (int)grid[right-k];

		// cell row
		count += (int)grid[left];
		count += (int)grid[right];

		// row below cell
		count += (int)grid[left+k];
		count += (int)grid[cell+k];
		count += (int)grid[right+k];
	}

	return count;
}


// in general:
// my_ indicates a variable private to the process
// net_ indicates it is shared

// STATIC EVOLUTION

void run(char* fname, int n, int s)
{
	int mpi_provided_thread_level;
	MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
	if (mpi_provided_thread_level < MPI_THREAD_FUNNELED)
		printf("[WARNING] mpi thread support is lower than demanded\n");

	//timing
    struct timespec begin, end;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &begin);

	MPI_Status status;
	MPI_Request request;
	
	int k, my_wl; // size of matrix, workload = local rows to process, maximum value of color scale in image
	char snap[24]; // will store snapshot_nnnnn
	void *ptr = NULL;

	int net_size, my_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &net_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

// if only a process is present at runtime we can't run the code below, so we distinguish the two cases
if (net_size > 1){

	// determine neighbours
    int my_prev = my_rank == 0 ? net_size-1 : my_rank-1;
    int my_succ = my_rank == net_size-1 ? 0 : my_rank+1;

	// root process reads matrix
	if (my_rank == 0)
	{
		int maxval, ysize; // their values arent actually used
		read_pgm_image(&ptr, &maxval, &k, &ysize, fname);
	}
	
	//transmit size of matrix to all
	MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	// calculate workload = matrix size / network size
	my_wl = k / net_size;
	if (my_wl * net_size < k) // because we did integer division
		if (my_rank < k-(my_wl*net_size))
			my_wl++; // distribute remainder among processes
	
	// gather worload size in of all processes
	int net_wl[net_size]; //static allocation might generate error?
	MPI_Allgather(&my_wl, 1, MPI_INT, net_wl, 1, MPI_INT, MPI_COMM_WORLD);

	// for gatherv
	int *recv_counts = NULL;
	int *displs = NULL; // where to place in root process
	if (my_rank == 0)
	{
		recv_counts = (int *)malloc(net_size * sizeof(int));
		displs = (int *)malloc(net_size * sizeof(int));

		int count = 0;
		for (int i = 0; i < net_size; i++)
		{
			recv_counts[i] = net_wl[i] * k;
			displs[i] = count;
			count += net_wl[i] * k;
		}
		
	}

	// allocate matrixes to store old and new parts of grid and full grid
	// matrix has to store two extra rows which will not be computed by the process itself
	char *my_grid = (char *)malloc((2+my_wl)*k * sizeof(char));
	char *my_new_grid = (char *)malloc((2+my_wl)*k * sizeof(char));
	char *full_grid = NULL;

	// root fills the full_grid matrix ?do I need this?
	if (my_rank == 0)
	{
		full_grid = (char *)malloc(k*k * sizeof(char));
		char *ptr_char = (char *)ptr;
		//omp paralelize? reading/writing same places..
		for (int i = 0; i < k*k; i++)
		{
			full_grid[i] = ptr_char[i];
		}
	}

	/* debug print
	if (my_rank == 0)
	{
		for (int i = 1; i <= k*k; i++)
		{
			printf("%d ",full_grid[i-1]);
			if (i%k == 0)
				printf("\n");
		}
	}*/

	// root transmits the matrix parts
	if (my_rank == 0)
	{
		int start = 0; // from what point in the grid to start transmission
		for (int i=1; i<net_size-1; i++) //cycle through middle processes
		{
			start += net_wl[i-1]*k;
			MPI_Send(&full_grid[start-k], net_wl[i]*k + 2*k, MPI_BYTE, i, 0, MPI_COMM_WORLD);
		}
		// last process receives first row
		start += net_wl[net_size-2]*k;
		MPI_Send(&full_grid[start-k], net_wl[net_size-1]*k + k, MPI_BYTE, net_size-1, 0, MPI_COMM_WORLD);
		MPI_Send(&full_grid[0], k, MPI_BYTE, net_size-1, 1, MPI_COMM_WORLD); //tagged as last process receives two times

		// copy in personal memory
		for (int i = 0; i < k; i++) //first row
			my_grid[i] = full_grid[k*k-k + i];
		for (int i = 0; i < my_wl*k+k; i++)
			my_grid[i+k] = full_grid[i];
		
	}
	else // non root processes receive
	{
		if (my_rank < net_size-1){
			MPI_Recv(my_grid, my_wl*k + 2*k, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}else{
			MPI_Recv(my_grid, my_wl*k + k, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&my_grid[my_wl*k+k], k, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	// parallel region for the duration of the game
	#pragma omp parallel
	{
		#pragma omp barrier
		// run the game n times
		for (int iteration=1; iteration <= n; iteration++)
		{
			// write new matrix
			#pragma omp for schedule(static, k)
			for (int cell = k; cell < my_wl*k + k; cell++)
			{
				int count = count_neighbours(cell, k, my_grid);
				my_new_grid[cell] = count == 2 || count == 3 ? 1 : 0; 
			}

			#pragma omp master
			{
				/* debug print
				if (my_rank == 1)
				{
					printf("proc [%d], iteration=%d k=%d, wl=%d:\n", my_rank, iteration, k, my_wl);
					for (int i=1; i<=my_wl*k +k+k; i++)
					{
						printf("%d ", my_grid[i-1]);
						if (i%k == 0)
							printf("\n");
					}
					printf("\n");
				}
				*/

				// saving
				if (s!=0 && iteration%s == 0)
				{
					MPI_Barrier(MPI_COMM_WORLD);

					// gather matrix in root process
					MPI_Gatherv(&my_grid[k], my_wl*k, MPI_BYTE, full_grid, recv_counts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);

					// root process writes to file
					if(my_rank == 0)
					{
						sprintf(snap, "snapshot_%06d.pbm", iteration-1); // name of the image 
						write_pgm_image((void *)full_grid, k, snap);
					}
				}

				// transmit borders of domain to neighbour processes
				// we can avoid this part at the last process [not implemented]

				// send first proper row to previous process
				MPI_Isend(&my_new_grid[k], k, MPI_BYTE, my_prev, 1+iteration, MPI_COMM_WORLD, &request);
				// send last proper row to next process
				MPI_Isend(&my_new_grid[my_wl*k], k, MPI_BYTE, my_succ, 1+n+iteration, MPI_COMM_WORLD, &request);

				// receive first row (my_first == other_last)
				MPI_Recv(my_new_grid, k, MPI_BYTE, my_prev, 1+n+iteration, MPI_COMM_WORLD, &status);
				// receive last row
				MPI_Recv(&my_new_grid[my_wl*k+k], k, MPI_BYTE, my_succ, 1+iteration, MPI_COMM_WORLD, &status);

				// swap grids
				char *swap = my_grid;
				my_grid = my_new_grid;
				my_new_grid = swap;
			}//end master
		}//end for
	}//end parallel region

	// saving
	MPI_Barrier(MPI_COMM_WORLD);
	// gather matrix in root process
	MPI_Gatherv(&my_grid[k], my_wl*k, MPI_BYTE, full_grid, recv_counts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);

	// root process writes to file
	if(my_rank == 0)
	{
		sprintf(snap, "snapshot_%06d.pbm", n); // name of the image 
		write_pgm_image((void *)full_grid, k, snap);
	}

	//free(ptr); only root initialized it && mught be freeing it two times => seg fault
	free(my_grid);
	free(my_new_grid);
	free(full_grid);

	if (my_rank == 0)
	{
        //timing
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = (double)diff(begin,end).tv_sec + (double)diff(begin,end).tv_nsec / 1000000000.0;
        if (TIME)
            printf("%F\n", elapsed);
	}

	MPI_Finalize();

	return;

//!end parallel code
}else{ //serial case ~ mpirun -np 1 ...
	// read image
	int maxval, ysize; // their values arent actually used
	read_pgm_image(&ptr, &maxval, &k, &ysize, fname);

	// initialize matrix
	char *full_grid = (char *)malloc(k*k * sizeof(char));
	char *full_new_grid = (char *)malloc(k*k * sizeof(char));

	char *ptr_char = (char *)ptr;
	// could be paralellized
	for (int i = 0; i < k*k; i++)
	{
		full_grid[i] = ptr_char[i];
	}

	#pragma omp parallel
	{
		// run game
		for (int iteration=1; iteration <= n; iteration++)
		{
			// write new matrix
			#pragma omp for schedule(static,k)
			for (int cell = 0; cell < k*k; cell++)
			{
				int count = count_neighbours_single(cell, k, full_grid);
				full_new_grid[cell] = count == 2 || count == 3 ? 1 : 0;
			}
		
			#pragma omp master
			{		
				// saving
				if (s!=0 && iteration%s == 0)
				{
					sprintf(snap, "snapshot_%06d.pbm", iteration); // name of the image 
					write_pgm_image((void *)full_new_grid, k, snap);
				}

				// swap grids
				char *swap = full_grid;
				full_grid = full_new_grid;
				full_new_grid = swap;
			}//end omp master
		}
	}//end omp parallel

	// saving
	sprintf(snap, "snapshot_%06d.pbm", n); // name of the image 
	write_pgm_image((void *)full_new_grid, k, snap);

	// end program
	free(full_grid);
	free(full_new_grid);
	free(ptr); // free earlier?

    //timing
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (double)diff(begin,end).tv_sec + (double)diff(begin,end).tv_nsec / 1000000000.0;
    if (TIME) {printf("%F\n", elapsed);}

	MPI_Finalize();

	return;
}//!end serial code
}

// ORDERED EVOLUTION

void run_ordered(char* fname, int n, int s)
{
	int k; // size of matrix
	char snap[24]; // will store snapshot_nnnnn
	void *ptr = NULL;

	int maxval, ysize; // their values arent actually used
	read_pgm_image(&ptr, &maxval, &k, &ysize, fname);

	// initialize matrix
	char *grid = (char *)malloc(k*k * sizeof(char));

	char *ptr_char = (char *)ptr;
	for (int i = 0; i < k*k; i++)
	{
		grid[i] = ptr_char[i];
	}
	free(ptr);

	// run game
	for (int iteration=1; iteration <= n; iteration++)
	{
		// update matrix
		for (int cell=0; cell<k*k; cell++)
		{
			int count = count_neighbours_single(cell, k, grid);
			grid[cell] = count == 2 || count == 3 ? 1 : 0; 
		}

		// save
		if (s!=0 && iteration%s == 0)
		{
			sprintf(snap, "snapshot_%06d.pbm", iteration); // name of the image 
			write_pgm_image((void *)grid, k, snap);
		}
	}

	// save
	sprintf(snap, "snapshot_%06d.pbm", n); // name of the image 
	write_pgm_image((void *)grid, k, snap);

	free(grid);

	return;
}