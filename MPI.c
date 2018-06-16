#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MASTER 0
#define WORKTAG 1
#define DIETAG 2
#define n 1000 //number of nodes


void showDistances(int **dist);

int main(int argc, char** argv) 
{

int i, j, k;
int **dist;
int my_rank,                /* rank of process */
     num_procs,             /* number of processes */
     slice,
     remain=0;
     
MPI_Status status;
/* Start up MPI */
MPI_Init(&argc, &argv);
/* Find out number of process */
MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
/* Find out process rank */
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

remain= n%(num_procs-1);
slice= (n-remain)/(num_procs-1);



//Initiate the necessary memory with malloc()
int *distData;
dist = (int**)malloc(n*sizeof(int*));
distData =(int*)malloc(n*n*sizeof(int));
for(i=0; i<n; i++)
	dist[i] =&(distData[i * n]);
	
time_t start, end;
//use current time
time(&start);
//to generate "random" numbers with rand()
srand(42);

/* master code */
if (my_rank == MASTER) {
  double t1,t2;
  int disable=0,t=3;
  int result[t];
	//Initiate the dist with random values from 0-99
	for(i=0; i<n; i++)
		for(j=0; j<n; j++)
			if(i==j)
				dist[i][j] = 0;
			else
				dist[i][j] = rand()%100;
			
	//Print initial distances
	showDistances(dist);

	t1 = MPI_Wtime();

	for(i=1;i<num_procs;i++)
		MPI_Send(&(dist[0][0]),n*n,MPI_INT,i,WORKTAG,MPI_COMM_WORLD); //send the array dist in every machine
		//send takes the first argument [0][0] and the size of the array n*n and fills the array appropriately
	
	do {
		 MPI_Recv(&result,t,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		 if (status.MPI_TAG == DIETAG) //if received DIETAG(0) means that every other slaves has done and finish the whole process
		    disable++;
		 else
		    if (dist[result[1]][result[2]]>result[0]) //do a final check in results. Works like a reduce function
		        dist[result[1]][result[2]]=result[0];
	} while (disable < num_procs-1);	
			
	t2 = MPI_Wtime();
	//print the final distances
	showDistances(dist);
	printf("Total Elapsed Time %f sec\n", difftime(t2, t1));
	
}
/* workers code*/
else{

    int i, j, k,t=3;
    int out[t];
    MPI_Recv(&(dist[0][0]),n*n,MPI_INT,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    if(my_rank+1!=num_procs)
        remain=0;
    for (k = slice*(my_rank-1); k < slice*(my_rank-1)+slice+remain; ++k) // slice the k alongside the ranks(slaves) 
        for (i = 0; i < n; ++i)																			//use of remain for the last slice
            for (j = 0; j < n; ++j)
                /* If i and j are different nodes and if
                    the paths between i and k and between
                    k and j exist, do */
                if ((dist[i][k] * dist[k][j] != 0) && (i != j))
                    /* See if you can't get a shorter path
                        between i and j by interspacing
                        k somewhere along the current
                        path */
                    if ((dist[i][k] + dist[k][j] < dist[i][j]) || (dist[i][j] == 0)){
                        dist[i][j] = dist[i][k] + dist[k][j];
                        out[0]=dist[i][j];
                        out[1]=i;
                        out[2]=j;
                        MPI_Send(&out,t,MPI_INT,MASTER,0,MPI_COMM_WORLD); //send back to master the calculated distance
                    }
    MPI_Send(0,0,MPI_INT,MASTER,DIETAG,MPI_COMM_WORLD);
}

/* Shut down MPI */
MPI_Finalize();

free(distData);
free(dist);

return 0;
}


//Print distance function
void showDistances(int **dist) {

int i, j;
printf("     ");
for(i=0; i<n; ++i)
	printf("N%d   ", i);
printf("\n");
for(i=0; i<n; ++i) {
	printf("N%d", i);
	for(j=0; j<n; ++j)
		printf("%5d", dist[i][j]);
	printf("\n");
}
printf("\n");
}	
