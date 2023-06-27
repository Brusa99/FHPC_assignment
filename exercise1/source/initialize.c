#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>

#include "initialize.h"
#include "read_write.h"

void initialize(char *fname, int k)
{
	char random;
    // create dynamic array
    char *grid = (char *)malloc(k*k * sizeof(char));

    // fill dynamic array in shared parallel
	#pragma omp parallel
	{
		// set seed
		int myid = omp_get_thread_num();
		unsigned int my_seed = myid*myid + 2*myid + 3;

		#pragma omp for schedule(static, k)
		for (int i=0; i<k*k; i++)
		{
			random = (char)(rand_r(&my_seed)%2); //generate random number, considers it modulo 2, cast it in char; 
			grid[i] = random;
		}
	}

    // write to pgm image
	printf("writing image to %s\n", fname);
    write_pgm_image((void *) grid, k, fname);
	
	free(grid);
    return;
}

/* old picture:

		grid[i] = 0;
		// make drawing (note that k>=100)
		if (i == 20*k+21 | i == 21*k+21 | i == 21*k + 23 | i == 23*k+21 | i == 23*k+22 | i == 23*k+23 | i == 24*k+23)
			grid[i] = 1;
*/