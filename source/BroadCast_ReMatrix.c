/**********************************************************************
  BroadCast_ReMatrix.c:

     BroadCast_ReMatrix.c is a subroutine to broadcast a matrix "Mat"
     which is distributed by row in each processor.

  Log of BroadCast_ReMatrix.c:

     26/Nov/2004  Released by T.Ozaki

***********************************************************************/

#include <stdio.h> 
#include <math.h>
#include <stdlib.h>
#include "openmx_common.h"
#include "mpi.h"


void BroadCast_ReMatrix(MPI_Comm MPI_Curret_Comm_WD, 
                        double **Mat, int n, int *is1, int *ie1, int myid, int numprocs, 
                        MPI_Status *stat_send,
                        MPI_Request *request_send,
                        MPI_Request *request_recv)
{
  int i,j,k,num,ID,tag=999;
  int k0,k1,num0,num1;
  double *Mat1;

  MPI_Status stat;
  MPI_Request request;

  /*********************************************
     elemements are stored from 1 to n in Mat. 
  **********************************************/

  Mat1 = (double*)malloc(sizeof(double)*(n+1)*(n+1));

  for (i=is1[myid]; i<=ie1[myid]; i++){
    for (j=1; j<=n; j++){
      k = (i-1)*n + j - 1;
      Mat1[k] = Mat[i][j];
    }
  }

  /* sending */

  k0 = (is1[myid]-1)*n;
  if (k0<0) k0 = 0;  
  num0 = (ie1[myid] - is1[myid] + 1)*n;
  if (num0<0) num0 = 0;

  for (ID=0; ID<numprocs; ID++){
    MPI_Isend(&Mat1[k0], num0, MPI_DOUBLE, ID, tag, MPI_Curret_Comm_WD, &request_send[ID]);
  }

  /* receiving */

  for (ID=0; ID<numprocs; ID++){
    k1 = (is1[ID]-1)*n;
    if (k1<0) k1 = 0;
    num1 = (ie1[ID] - is1[ID] + 1)*n;
    if (num1<0) num1 = 0;
    MPI_Irecv(&Mat1[k1], num1, MPI_DOUBLE, ID, tag, MPI_Curret_Comm_WD, &request_recv[ID]);
  }

  /* waitall */

  MPI_Waitall(numprocs,request_recv,stat_send);
  MPI_Waitall(numprocs,request_send,stat_send);

  for (ID=0; ID<numprocs; ID++){
    for (i=is1[ID]; i<=ie1[ID]; i++){
      for (j=1; j<=n; j++){
	k = (i-1)*n + j - 1;
	Mat[i][j] = Mat1[k];
      }
    }
  }

  free(Mat1);
}
