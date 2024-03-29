/**********************************************************************
  Set_OLP_Kin.c:

     Set_OLP_Kin.c is a subroutine to calculate the overlap matrix
     and the matrix for the kinetic operator in momentum space.

  Log of Set_OLP_Kin.c:

     15/Oct./2002  Released by T.Ozaki

***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "openmx_common.h"
#include "mpi.h"
#include <omp.h>


double Set_OLP_Kin(double *****OLP, double *****H0)
{
  /****************************************************
          Evaluate overlap and kinetic integrals
                 in the momentum space
  ****************************************************/
  static int firsttime=1;
  int size_SumS0,size_TmpOLP;
  double time0;
  double TStime,TEtime;
  int numprocs,myid;
  int Mc_AN,Gc_AN,h_AN;
  int OneD_Nloop;
  int *OneD2Mc_AN,*OneD2h_AN;

  /* MPI */
  MPI_Comm_size(mpi_comm_level1,&numprocs);
  MPI_Comm_rank(mpi_comm_level1,&myid);

  dtime(&TStime);

  /****************************************************
   MPI_Barrier
  ****************************************************/

  MPI_Barrier(mpi_comm_level1);

  /* PrintMemory */

  if (firsttime) {

    size_SumS0  = (List_YOUSO[25]+1)*List_YOUSO[24]*(List_YOUSO[25]+1)*List_YOUSO[24];
    size_TmpOLP = (List_YOUSO[25]+1)*List_YOUSO[24]*(2*(List_YOUSO[25]+1)+1)*
                  (List_YOUSO[25]+1)*List_YOUSO[24]*(2*(List_YOUSO[25]+1)+1);
 
    PrintMemory("Set_OLP_Kin: SumS0",  sizeof(double)*size_SumS0,NULL);
    PrintMemory("Set_OLP_Kin: SumK0",  sizeof(double)*size_SumS0,NULL);
    PrintMemory("Set_OLP_Kin: SumSr0", sizeof(double)*size_SumS0,NULL);
    PrintMemory("Set_OLP_Kin: SumKr0", sizeof(double)*size_SumS0,NULL);
    PrintMemory("Set_OLP_Kin: TmpOLP", sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpOLPr",sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpOLPt",sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpOLPp",sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpKin", sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpKinr",sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpKint",sizeof(dcomplex)*size_TmpOLP,NULL);
    PrintMemory("Set_OLP_Kin: TmpKinp",sizeof(dcomplex)*size_TmpOLP,NULL);
    firsttime=0;
  }

  /* one-dimensionalize the Mc_AN and h_AN loops */

  OneD_Nloop = 0;
  for (Mc_AN=1; Mc_AN<=Matomnum; Mc_AN++){
    Gc_AN = M2G[Mc_AN];    
    for (h_AN=0; h_AN<=FNAN[Gc_AN]; h_AN++){
      OneD_Nloop++;
    }
  }  

  OneD2Mc_AN = (int*)malloc(sizeof(int)*(OneD_Nloop+1));
  OneD2h_AN = (int*)malloc(sizeof(int)*(OneD_Nloop+1));

  OneD_Nloop = 0;
  for (Mc_AN=1; Mc_AN<=Matomnum; Mc_AN++){
    Gc_AN = M2G[Mc_AN];    
    for (h_AN=0; h_AN<=FNAN[Gc_AN]; h_AN++){
      OneD2Mc_AN[OneD_Nloop] = Mc_AN; 
      OneD2h_AN[OneD_Nloop]  = h_AN; 
      OneD_Nloop++;
    }
  }

  /* OpenMP */

#pragma omp parallel shared(time_per_atom,H0,OLP,OLP_L,Comp2Real,OneD_Grid,List_YOUSO,Spe_Num_Basis,Spe_MaxL_Basis,PAO_Nkmax,atv,Gxyz,ncn,natn,WhatSpecies,M2G,OneD2h_AN,OneD2Mc_AN,OneD_Nloop) 
  {

    int Nloop;
    int OMPID,Nthrds,Nprocs;
    int Mc_AN,h_AN,Gc_AN,Cwan;
    int Gh_AN,Rnh,Hwan;
    int Ls,L0,Mul0,L1,Mul1,M0,M1;
    int Lmax_Four_Int;
    int i,j,k,l,m,p;
    int num0,num1; 

    double Stime_atom,Etime_atom;
    double dx,dy,dz;
    double S_coordinate[3];
    double theta,phi,h;
    double Bessel_Pro0,Bessel_Pro1;
    double tmp0,tmp1,tmp2,tmp3,tmp4;
    double siT,coT,siP,coP;
    double kmin,kmax,Sk,Dk,r; 
    double sj,sjp,coe0,coe1; 
    double Normk,Normk2;
    double gant,SH[2],dSHt[2],dSHp[2];
    double **SphB,**SphBp;
    double *tmp_SphB,*tmp_SphBp;
    double ****SumS0;
    double ****SumK0;
    double ****SumSr0;
    double ****SumKr0;

    dcomplex CsumS_Lx,CsumS_Ly,CsumS_Lz;
    dcomplex CsumS0,CsumSr,CsumSt,CsumSp;
    dcomplex CsumK0,CsumKr,CsumKt,CsumKp;
    dcomplex Ctmp0,Ctmp1,Ctmp2,Cpow;
    dcomplex CY,CYt,CYp,CY1,CYt1,CYp1;
    dcomplex ******TmpOLP;
    dcomplex ******TmpOLPr;
    dcomplex ******TmpOLPt;
    dcomplex ******TmpOLPp;
    dcomplex ******TmpKin;
    dcomplex ******TmpKinr;
    dcomplex ******TmpKint;
    dcomplex ******TmpKinp;

    dcomplex **CmatS0;
    dcomplex **CmatSr;
    dcomplex **CmatSt;
    dcomplex **CmatSp;
    dcomplex **CmatK0;
    dcomplex **CmatKr;
    dcomplex **CmatKt;
    dcomplex **CmatKp;

    /****************************************************************
                          allocation of arrays:
    ****************************************************************/
    
    TmpOLP = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpOLP[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpOLP[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpOLP[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpOLP[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpOLP[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpOLP[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpOLPr = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpOLPr[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpOLPr[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpOLPr[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpOLPr[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpOLPr[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpOLPr[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpOLPt = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpOLPt[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpOLPt[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpOLPt[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpOLPt[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpOLPt[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpOLPt[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpOLPp = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpOLPp[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpOLPp[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpOLPp[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpOLPp[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpOLPp[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpOLPp[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpKin = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpKin[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpKin[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpKin[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpKin[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpKin[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpKin[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpKinr = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpKinr[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpKinr[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpKinr[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpKinr[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpKinr[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpKinr[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpKint = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpKint[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpKint[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpKint[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpKint[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpKint[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpKint[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    TmpKinp = (dcomplex******)malloc(sizeof(dcomplex*****)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      TmpKinp[i] = (dcomplex*****)malloc(sizeof(dcomplex****)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	TmpKinp[i][j] = (dcomplex****)malloc(sizeof(dcomplex***)*(2*(List_YOUSO[25]+1)+1));
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  TmpKinp[i][j][k] = (dcomplex***)malloc(sizeof(dcomplex**)*(List_YOUSO[25]+1));
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    TmpKinp[i][j][k][l] = (dcomplex**)malloc(sizeof(dcomplex*)*List_YOUSO[24]);
	    for (m=0; m<List_YOUSO[24]; m++){
	      TmpKinp[i][j][k][l][m] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
	      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) TmpKinp[i][j][k][l][m][p] = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }

    SumS0 = (double****)malloc(sizeof(double***)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      SumS0[i] = (double***)malloc(sizeof(double**)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	SumS0[i][j] = (double**)malloc(sizeof(double*)*(List_YOUSO[25]+1));
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  SumS0[i][j][k] = (double*)malloc(sizeof(double)*List_YOUSO[24]);
	  for (m=0; m<List_YOUSO[24]; m++) SumS0[i][j][k][m] = 0.0;       
	}
      }
    }

    SumK0 = (double****)malloc(sizeof(double***)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      SumK0[i] = (double***)malloc(sizeof(double**)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	SumK0[i][j] = (double**)malloc(sizeof(double*)*(List_YOUSO[25]+1));
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  SumK0[i][j][k] = (double*)malloc(sizeof(double)*List_YOUSO[24]);
	  for (m=0; m<List_YOUSO[24]; m++) SumK0[i][j][k][m] = 0.0;
	}
      }
    }


    SumSr0 = (double****)malloc(sizeof(double***)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      SumSr0[i] = (double***)malloc(sizeof(double**)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	SumSr0[i][j] = (double**)malloc(sizeof(double*)*(List_YOUSO[25]+1));
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  SumSr0[i][j][k] = (double*)malloc(sizeof(double)*List_YOUSO[24]);
	  for (m=0; m<List_YOUSO[24]; m++) SumSr0[i][j][k][m] = 0.0;
	}
      }
    }

    SumKr0 = (double****)malloc(sizeof(double***)*(List_YOUSO[25]+1));
    for (i=0; i<(List_YOUSO[25]+1); i++){
      SumKr0[i] = (double***)malloc(sizeof(double**)*List_YOUSO[24]);
      for (j=0; j<List_YOUSO[24]; j++){
	SumKr0[i][j] = (double**)malloc(sizeof(double*)*(List_YOUSO[25]+1));
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  SumKr0[i][j][k] = (double*)malloc(sizeof(double)*List_YOUSO[24]);
	  for (m=0; m<List_YOUSO[24]; m++) SumKr0[i][j][k][m] = 0.0;
	}
      }
    }
    
    CmatS0 = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatS0[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatS0[i][p] = Complex(0.0,0.0);
    }

    CmatSr = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatSr[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatSr[i][p] = Complex(0.0,0.0);
    }

    CmatSt = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatSt[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatSt[i][p] = Complex(0.0,0.0);
    }

    CmatSp = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatSp[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatSp[i][p] = Complex(0.0,0.0);
    }

    CmatK0 = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatK0[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatK0[i][p] = Complex(0.0,0.0);
    }

    CmatKr = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatKr[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatKr[i][p] = Complex(0.0,0.0);
    }

    CmatKt = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatKt[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatKt[i][p] = Complex(0.0,0.0);
    }

    CmatKp = (dcomplex**)malloc(sizeof(dcomplex*)*(2*(List_YOUSO[25]+1)+1));
    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      CmatKp[i] = (dcomplex*)malloc(sizeof(dcomplex)*(2*(List_YOUSO[25]+1)+1));
      for (p=0; p<(2*(List_YOUSO[25]+1)+1); p++) CmatKp[i][p] = Complex(0.0,0.0);
    }

    /* get info. on OpenMP */ 

    OMPID = omp_get_thread_num();
    Nthrds = omp_get_num_threads();
    Nprocs = omp_get_num_procs();

    /* one-dimensionalized loop */

    for (Nloop=OMPID*OneD_Nloop/Nthrds; Nloop<(OMPID+1)*OneD_Nloop/Nthrds; Nloop++){

      dtime(&Stime_atom); 

      /* get Mc_AN and h_AN */

      Mc_AN = OneD2Mc_AN[Nloop];
      h_AN  = OneD2h_AN[Nloop];

      /* set data on Mc_AN */

      Gc_AN = M2G[Mc_AN];
      Cwan = WhatSpecies[Gc_AN];

      /* set data on h_AN */

      Gh_AN = natn[Gc_AN][h_AN];
      Rnh = ncn[Gc_AN][h_AN];
      Hwan = WhatSpecies[Gh_AN];

      dx = Gxyz[Gh_AN][1] + atv[Rnh][1] - Gxyz[Gc_AN][1]; 
      dy = Gxyz[Gh_AN][2] + atv[Rnh][2] - Gxyz[Gc_AN][2]; 
      dz = Gxyz[Gh_AN][3] + atv[Rnh][3] - Gxyz[Gc_AN][3];

      xyz2spherical(dx,dy,dz,0.0,0.0,0.0,S_coordinate); 
      r     = S_coordinate[0];
      theta = S_coordinate[1];
      phi   = S_coordinate[2];

      /* for empty atoms or finite elemens basis */
      if (r<1.0e-10) r = 1.0e-10;

      /* precalculation of sin and cos */

      siT = sin(theta);
      coT = cos(theta);
      siP = sin(phi);
      coP = cos(phi);

      /****************************************************
          Overlap and the derivative
              \int RL(k)*RL'(k)*jl(k*R) k^2 dk^3,
              \int RL(k)*RL'(k)*j'l(k*R) k^3 dk^3

          Kinetic 
              \int RL(k)*RL'(k)*jl(k*R) k^4 dk^3, 
              \int RL(k)*RL'(k)*j'l(k*R) k^5 dk^3 
      ****************************************************/

      kmin = Radial_kmin;
      kmax = PAO_Nkmax;
      Sk = kmax + kmin;
      Dk = kmax - kmin;

      for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
	for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){
	  for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
	    for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){
	      for (M0=-L0; M0<=L0; M0++){
		for (M1=-L1; M1<=L1; M1++){

                  TmpOLP[L0][Mul0][L0+M0][L1][Mul1][L1+M1]  = Complex(0.0,0.0);
                  TmpOLPr[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);
                  TmpOLPt[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);
                  TmpOLPp[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);

                  TmpKin[L0][Mul0][L0+M0][L1][Mul1][L1+M1]  = Complex(0.0,0.0);
                  TmpKinr[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);
                  TmpKint[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);
                  TmpKinp[L0][Mul0][L0+M0][L1][Mul1][L1+M1] = Complex(0.0,0.0);

		}
	      }
	    }
	  }
	}
      }

      if (Spe_MaxL_Basis[Cwan]<Spe_MaxL_Basis[Hwan])
        Lmax_Four_Int = 2*Spe_MaxL_Basis[Hwan];
      else 
        Lmax_Four_Int = 2*Spe_MaxL_Basis[Cwan];

      /* allocate SphB and SphBp */

      SphB = (double**)malloc(sizeof(double*)*(Lmax_Four_Int+3));
      for(l=0; l<(Lmax_Four_Int+3); l++){ 
        SphB[l] = (double*)malloc(sizeof(double)*(OneD_Grid+1));
      }

      SphBp = (double**)malloc(sizeof(double*)*(Lmax_Four_Int+3));
      for(l=0; l<(Lmax_Four_Int+3); l++){ 
        SphBp[l] = (double*)malloc(sizeof(double)*(OneD_Grid+1));
      }
      
      tmp_SphB  = (double*)malloc(sizeof(double)*(Lmax_Four_Int+3));
      tmp_SphBp = (double*)malloc(sizeof(double)*(Lmax_Four_Int+3));

      /* calculate SphB and SphBp */

      h = (kmax - kmin)/(double)OneD_Grid;

      for (i=0; i<=OneD_Grid; i++){
        Normk = kmin + (double)i*h;
        Spherical_Bessel(Normk*r,Lmax_Four_Int,tmp_SphB,tmp_SphBp);
        for(l=0; l<=Lmax_Four_Int; l++){ 
          SphB[l][i]  = tmp_SphB[l]; 
          SphBp[l][i] = tmp_SphBp[l]; 
	}
      }

      free(tmp_SphB);
      free(tmp_SphBp);

      /* l loop */

      for(l=0; l<=Lmax_Four_Int; l++){

        for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
	  for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){
            for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
	      for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){
                SumS0[L0][Mul0][L1][Mul1]  = 0.0;
                SumK0[L0][Mul0][L1][Mul1]  = 0.0;
                SumSr0[L0][Mul0][L1][Mul1] = 0.0;
                SumKr0[L0][Mul0][L1][Mul1] = 0.0;
	      }
	    }
	  }
	}

        h = (kmax - kmin)/(double)OneD_Grid;
	for (i=0; i<=OneD_Grid; i++){

          if (i==0 || i==OneD_Grid) coe0 = 0.50;
          else                      coe0 = 1.00;

	  Normk = kmin + (double)i*h;
          Normk2 = Normk*Normk;

          sj  =  SphB[l][i];
          sjp = SphBp[l][i];

	  for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
	    for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){

	      Bessel_Pro0 = RF_BesselF(Cwan,L0,Mul0,Normk);

              tmp0 = coe0*h*Normk2*Bessel_Pro0;
              tmp1 = tmp0*sj;
              tmp2 = tmp0*Normk*sjp;

	      for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
		for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){

                  Bessel_Pro1 = RF_BesselF(Hwan,L1,Mul1,Normk);

                  tmp3 = tmp1*Bessel_Pro1;
                  tmp4 = tmp2*Bessel_Pro1;

                  SumS0[L0][Mul0][L1][Mul1] += tmp3;
                  SumK0[L0][Mul0][L1][Mul1] += tmp3*Normk2;

                  SumSr0[L0][Mul0][L1][Mul1] += tmp4;
                  SumKr0[L0][Mul0][L1][Mul1] += tmp4*Normk2;
		}
	      }

	    }
	  }
	}

        if (h_AN==0){ 
	  for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
	    for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){
	      for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
		for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){
		  SumSr0[L0][Mul0][L1][Mul1] = 0.0;
		  SumKr0[L0][Mul0][L1][Mul1] = 0.0;
		}
	      }
	    }
	  }
	}

        /****************************************************
          For overlap and the derivative,
          sum_m 8*(-i)^{-L0+L1+1}*
                C_{L0,-M0,L1,M1,l,m}*Y_{lm}
                \int RL(k)*RL'(k)*jl(k*R) k^2 dk^3,

          For kinetic,
          sum_m 4*(-i)^{-L0+L1+1}*
                C_{L0,-M0,L1,M1,l,m}*
                \int RL(k)*RL'(k)*jl(k*R) k^4 dk^3,
        ****************************************************/

        for(m=-l; m<=l; m++){ 

          ComplexSH(l,m,theta,phi,SH,dSHt,dSHp);
          SH[1]   = -SH[1];
          dSHt[1] = -dSHt[1];
          dSHp[1] = -dSHp[1];

          CY  = Complex(SH[0],SH[1]);
          CYt = Complex(dSHt[0],dSHt[1]);
          CYp = Complex(dSHp[0],dSHp[1]);

          for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
            for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){
              for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
                for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){

                  Ls = -L0 + L1 + l;  

                  if (abs(L1-l)<=L0 && L0<=(L1+l) ){

		    Cpow = Im_pow(-1,Ls);
		    CY1  = Cmul(Cpow,CY);
		    CYt1 = Cmul(Cpow,CYt);
		    CYp1 = Cmul(Cpow,CYp);

		    for (M0=-L0; M0<=L0; M0++){

		      M1 = M0 - m;

                      if (abs(M1)<=L1){

                        gant = Gaunt(L0,M0,L1,M1,l,m);

                        /* S */ 

                        tmp0 = gant*SumS0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CY1,tmp0);
                        TmpOLP[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpOLP[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dS/dr */ 

                        tmp0 = gant*SumSr0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CY1,tmp0);
                        TmpOLPr[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpOLPr[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dS/dt */ 

                        tmp0 = gant*SumS0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CYt1,tmp0);
                        TmpOLPt[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpOLPt[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dS/dp */ 

                        tmp0 = gant*SumS0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CYp1,tmp0);
                        TmpOLPp[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpOLPp[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* K */ 

                        tmp0 = gant*SumK0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CY1,tmp0);
                        TmpKin[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpKin[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dK/dr */ 

                        tmp0 = gant*SumKr0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CY1,tmp0);
                        TmpKinr[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpKinr[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dK/dt */ 

                        tmp0 = gant*SumK0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CYt1,tmp0);
                        TmpKint[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpKint[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

                        /* dK/dp */ 

                        tmp0 = gant*SumK0[L0][Mul0][L1][Mul1];
                        Ctmp2 = CRmul(CYp1,tmp0);
                        TmpKinp[L0][Mul0][L0+M0][L1][Mul1][L1+M1] =
			  Cadd(TmpKinp[L0][Mul0][L0+M0][L1][Mul1][L1+M1],Ctmp2);

		      }
		    }
		  }
		}
	      }

	    }
	  }
        } 
      } /* l */

      /* free SphB and SphBp */

      for(l=0; l<(Lmax_Four_Int+3); l++){ 
        free(SphB[l]);
      }
      free(SphB);

      for(l=0; l<(Lmax_Four_Int+3); l++){ 
        free(SphBp[l]);
      }
      free(SphBp);

      /****************************************************
                         Complex to Real
      ****************************************************/

      num0 = 0;
      for (L0=0; L0<=Spe_MaxL_Basis[Cwan]; L0++){
	for (Mul0=0; Mul0<Spe_Num_Basis[Cwan][L0]; Mul0++){
          
          num1 = 0;
          for (L1=0; L1<=Spe_MaxL_Basis[Hwan]; L1++){
	    for (Mul1=0; Mul1<Spe_Num_Basis[Hwan][L1]; Mul1++){
              
    	      for (M0=-L0; M0<=L0; M0++){
		for (M1=-L1; M1<=L1; M1++){

                  CsumS0 = Complex(0.0,0.0);
                  CsumSr = Complex(0.0,0.0);
                  CsumSt = Complex(0.0,0.0);
                  CsumSp = Complex(0.0,0.0);

                  CsumK0 = Complex(0.0,0.0);
                  CsumKr = Complex(0.0,0.0);
                  CsumKt = Complex(0.0,0.0);
                  CsumKp = Complex(0.0,0.0);

     	          for (k=-L0; k<=L0; k++){

                    Ctmp1 = Conjg(Comp2Real[L0][L0+M0][L0+k]);

                    /* S */

                    Ctmp0 = TmpOLP[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumS0 = Cadd(CsumS0,Ctmp2);
 
                    /* dS/dr */

                    Ctmp0 = TmpOLPr[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumSr = Cadd(CsumSr,Ctmp2);

                    /* dS/dt */

                    Ctmp0 = TmpOLPt[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumSt = Cadd(CsumSt,Ctmp2);

                    /* dS/dp */

                    Ctmp0 = TmpOLPp[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumSp = Cadd(CsumSp,Ctmp2);

                    /* K */

                    Ctmp0 = TmpKin[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumK0 = Cadd(CsumK0,Ctmp2);

                    /* dK/dr */

                    Ctmp0 = TmpKinr[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumKr = Cadd(CsumKr,Ctmp2);

                    /* dK/dt */

                    Ctmp0 = TmpKint[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumKt = Cadd(CsumKt,Ctmp2);

                    /* dK/dp */

                    Ctmp0 = TmpKinp[L0][Mul0][L0+k][L1][Mul1][L1+M1];
                    Ctmp2 = Cmul(Ctmp1,Ctmp0);
                    CsumKp = Cadd(CsumKp,Ctmp2);

		  }

                  CmatS0[L0+M0][L1+M1] = CsumS0;
                  CmatSr[L0+M0][L1+M1] = CsumSr;
                  CmatSt[L0+M0][L1+M1] = CsumSt;
                  CmatSp[L0+M0][L1+M1] = CsumSp;

                  CmatK0[L0+M0][L1+M1] = CsumK0;
                  CmatKr[L0+M0][L1+M1] = CsumKr;
                  CmatKt[L0+M0][L1+M1] = CsumKt;
                  CmatKp[L0+M0][L1+M1] = CsumKp;
		}
	      }

    	      for (M0=-L0; M0<=L0; M0++){
		for (M1=-L1; M1<=L1; M1++){

                  CsumS_Lx = Complex(0.0,0.0);
                  CsumS_Ly = Complex(0.0,0.0);
                  CsumS_Lz = Complex(0.0,0.0);

                  CsumS0 = Complex(0.0,0.0);
                  CsumSr = Complex(0.0,0.0);
                  CsumSt = Complex(0.0,0.0);
                  CsumSp = Complex(0.0,0.0);
                  CsumK0 = Complex(0.0,0.0);
                  CsumKr = Complex(0.0,0.0);
                  CsumKt = Complex(0.0,0.0);
                  CsumKp = Complex(0.0,0.0);

     	          for (k=-L1; k<=L1; k++){

                    /*** S_Lx ***/ 

                    /*  Y k+1 */
                    if (k<L1){
                      coe0 = sqrt((double)((L1-k)*(L1+k+1)));
                      Ctmp1 = Cmul(CmatS0[L0+M0][L1+k+1],Comp2Real[L1][L1+M1][L1+k]);
                      Ctmp1.r = 0.5*coe0*Ctmp1.r;
                      Ctmp1.i = 0.5*coe0*Ctmp1.i;
                      CsumS_Lx = Cadd(CsumS_Lx,Ctmp1);
		    }

                    /*  Y k-1 */
                    if (-L1<k){
                      coe1 = sqrt((double)((L1+k)*(L1-k+1)));
                      Ctmp1 = Cmul(CmatS0[L0+M0][L1+k-1],Comp2Real[L1][L1+M1][L1+k]);
                      Ctmp1.r = 0.5*coe1*Ctmp1.r;
                      Ctmp1.i = 0.5*coe1*Ctmp1.i;
                      CsumS_Lx = Cadd(CsumS_Lx,Ctmp1);
		    }

                    /*** S_Ly ***/ 

                    /*  Y k+1 */

                    if (k<L1){
                      Ctmp1 = Cmul(CmatS0[L0+M0][L1+k+1],Comp2Real[L1][L1+M1][L1+k]);
                      Ctmp2.r = 0.5*coe0*Ctmp1.i;
                      Ctmp2.i =-0.5*coe0*Ctmp1.r;
                      CsumS_Ly = Cadd(CsumS_Ly,Ctmp2);
		    }

                    /*  Y k-1 */

                    if (-L1<k){
                      Ctmp1 = Cmul(CmatS0[L0+M0][L1+k-1],Comp2Real[L1][L1+M1][L1+k]);
                      Ctmp2.r =-0.5*coe1*Ctmp1.i;
                      Ctmp2.i = 0.5*coe1*Ctmp1.r;
                      CsumS_Ly = Cadd(CsumS_Ly,Ctmp2);
		    }

                    /*** S_Lz ***/ 

                    Ctmp1    = Cmul(CmatS0[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    Ctmp1.r = (double)k*Ctmp1.r;;
                    Ctmp1.i = (double)k*Ctmp1.i;
                    CsumS_Lz = Cadd(CsumS_Lz,Ctmp1);

                    /* S */ 

                    Ctmp1 = Cmul(CmatS0[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumS0 = Cadd(CsumS0,Ctmp1);

                    /* dS/dr */ 

                    Ctmp1 = Cmul(CmatSr[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumSr = Cadd(CsumSr,Ctmp1);

                    /* dS/dt */ 

                    Ctmp1 = Cmul(CmatSt[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumSt = Cadd(CsumSt,Ctmp1);

                    /* dS/dp */ 

                    Ctmp1 = Cmul(CmatSp[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumSp = Cadd(CsumSp,Ctmp1);

                    /* K */ 

                    Ctmp1 = Cmul(CmatK0[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumK0 = Cadd(CsumK0,Ctmp1);

                    /* dK/dr */ 

                    Ctmp1 = Cmul(CmatKr[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumKr = Cadd(CsumKr,Ctmp1);

                    /* dK/dt */ 

                    Ctmp1 = Cmul(CmatKt[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumKt = Cadd(CsumKt,Ctmp1);

                    /* dK/dp */ 

                    Ctmp1 = Cmul(CmatKp[L0+M0][L1+k],Comp2Real[L1][L1+M1][L1+k]);
                    CsumKp = Cadd(CsumKp,Ctmp1);

		  }
                  
                  OLP_L[0][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 8.0*CsumS_Lx.i; 
                  OLP_L[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 8.0*CsumS_Ly.i; 
                  OLP_L[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 8.0*CsumS_Lz.i;
                  
                  /* add a small value for stabilization of eigenvalue routine */

                  OLP[0][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 8.0*CsumS0.r + 1.0*rnd(1.0e-13);
		  H0[0][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 4.0*CsumK0.r;

                  if (h_AN!=0){

                    if (fabs(siT)<10e-14){

		      OLP[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(siT*coP*CsumSr.r + coT*coP/r*CsumSt.r);

		      OLP[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(siT*siP*CsumSr.r + coT*siP/r*CsumSt.r);

		      OLP[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(coT*CsumSr.r - siT/r*CsumSt.r);

		      H0[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(siT*coP*CsumKr.r + coT*coP/r*CsumKt.r);

		      H0[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(siT*siP*CsumKr.r + coT*siP/r*CsumKt.r);

		      H0[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(coT*CsumKr.r - siT/r*CsumKt.r);
                    }

                    else{

		      OLP[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(siT*coP*CsumSr.r + coT*coP/r*CsumSt.r
			     - siP/siT/r*CsumSp.r);

		      OLP[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(siT*siP*CsumSr.r + coT*siP/r*CsumSt.r
			     + coP/siT/r*CsumSp.r);

		      OLP[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-8.0*(coT*CsumSr.r - siT/r*CsumSt.r);

		      H0[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(siT*coP*CsumKr.r + coT*coP/r*CsumKt.r
			     - siP/siT/r*CsumKp.r);

		      H0[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(siT*siP*CsumKr.r + coT*siP/r*CsumKt.r
			     + coP/siT/r*CsumKp.r);

		      H0[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] =
			-4.0*(coT*CsumKr.r - siT/r*CsumKt.r);
		    }

		  }
		  else{
		    OLP[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
	            OLP[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
		    OLP[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
 	            H0[1][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
	            H0[2][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
	            H0[3][Mc_AN][h_AN][num0+L0+M0][num1+L1+M1] = 0.0;
                  }

		}
	      }

              num1 = num1 + 2*L1 + 1; 
	    }
	  }

          num0 = num0 + 2*L0 + 1; 
	}
      }

      dtime(&Etime_atom);
      time_per_atom[Gc_AN] += Etime_atom - Stime_atom;
    }

    /* freeing of arrays */

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpOLP[i][j][k][l][m]);
	    }
	    free(TmpOLP[i][j][k][l]);
	  }
	  free(TmpOLP[i][j][k]);
	}
	free(TmpOLP[i][j]);
      }
      free(TmpOLP[i]);
    }
    free(TmpOLP);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpOLPr[i][j][k][l][m]);
	    }
	    free(TmpOLPr[i][j][k][l]);
	  }
	  free(TmpOLPr[i][j][k]);
	}
	free(TmpOLPr[i][j]);
      }
      free(TmpOLPr[i]);
    }
    free(TmpOLPr);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpOLPt[i][j][k][l][m]);
	    }
	    free(TmpOLPt[i][j][k][l]);
	  }
	  free(TmpOLPt[i][j][k]);
	}
	free(TmpOLPt[i][j]);
      }
      free(TmpOLPt[i]);
    }
    free(TmpOLPt);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpOLPp[i][j][k][l][m]);
	    }
	    free(TmpOLPp[i][j][k][l]);
	  }
	  free(TmpOLPp[i][j][k]);
	}
	free(TmpOLPp[i][j]);
      }
      free(TmpOLPp[i]);
    }
    free(TmpOLPp);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpKin[i][j][k][l][m]);
	    }
	    free(TmpKin[i][j][k][l]);
	  }
	  free(TmpKin[i][j][k]);
	}
	free(TmpKin[i][j]);
      }
      free(TmpKin[i]);
    }
    free(TmpKin);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpKinr[i][j][k][l][m]);
	    }
	    free(TmpKinr[i][j][k][l]);
	  }
	  free(TmpKinr[i][j][k]);
	}
	free(TmpKinr[i][j]);
      }
      free(TmpKinr[i]);
    }
    free(TmpKinr);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpKint[i][j][k][l][m]);
	    }
	    free(TmpKint[i][j][k][l]);
	  }
	  free(TmpKint[i][j][k]);
	}
	free(TmpKint[i][j]);
      }
      free(TmpKint[i]);
    }
    free(TmpKint);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(2*(List_YOUSO[25]+1)+1); k++){
	  for (l=0; l<(List_YOUSO[25]+1); l++){
	    for (m=0; m<List_YOUSO[24]; m++){
	      free(TmpKinp[i][j][k][l][m]);
	    }
	    free(TmpKinp[i][j][k][l]);
	  }
	  free(TmpKinp[i][j][k]);
	}
	free(TmpKinp[i][j]);
      }
      free(TmpKinp[i]);
    }
    free(TmpKinp);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  free(SumS0[i][j][k]);
	}
	free(SumS0[i][j]);
      }
      free(SumS0[i]);
    }
    free(SumS0);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  free(SumK0[i][j][k]);
	}
	free(SumK0[i][j]);
      }
      free(SumK0[i]);
    }
    free(SumK0);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  free(SumSr0[i][j][k]);
	}
	free(SumSr0[i][j]);
      }
      free(SumSr0[i]);
    }
    free(SumSr0);

    for (i=0; i<(List_YOUSO[25]+1); i++){
      for (j=0; j<List_YOUSO[24]; j++){
	for (k=0; k<(List_YOUSO[25]+1); k++){
	  free(SumKr0[i][j][k]);
	}
	free(SumKr0[i][j]);
      }
      free(SumKr0[i]);
    }
    free(SumKr0);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatS0[i]);
    }
    free(CmatS0);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatSr[i]);
    }
    free(CmatSr);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatSt[i]);
    }
    free(CmatSt);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatSp[i]);
    }
    free(CmatSp);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatK0[i]);
    }
    free(CmatK0);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatKr[i]);
    }
    free(CmatKr);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatKt[i]);
    }
    free(CmatKt);

    for (i=0; i<(2*(List_YOUSO[25]+1)+1); i++){
      free(CmatKp[i]);
    }
    free(CmatKp);

  } /* #pragma omp parallel */


  /*
  printf("OLP_Lx\n");
  for (Mc_AN=1; Mc_AN<=Matomnum; Mc_AN++){
    Gc_AN = M2G[Mc_AN];    
    Cwan = WhatSpecies[Gc_AN];

    for (i=0; i<Spe_Total_NO[Cwan]; i++){
      for (j=0; j<Spe_Total_NO[Cwan]; j++){
        printf("%7.4f ",OLP_L[0][Mc_AN][0][i][j]);
      }
      printf("\n");
    }
  }

  printf("OLP_Ly\n");
  for (Mc_AN=1; Mc_AN<=Matomnum; Mc_AN++){
    Gc_AN = M2G[Mc_AN];    
    Cwan = WhatSpecies[Gc_AN];

    for (i=0; i<Spe_Total_NO[Cwan]; i++){
      for (j=0; j<Spe_Total_NO[Cwan]; j++){
        printf("%7.4f ",OLP_L[1][Mc_AN][0][i][j]);
      }
      printf("\n");
    }
  }

  printf("OLP_Lz\n");
  for (Mc_AN=1; Mc_AN<=Matomnum; Mc_AN++){
    Gc_AN = M2G[Mc_AN];    
    Cwan = WhatSpecies[Gc_AN];

    for (i=0; i<Spe_Total_NO[Cwan]; i++){
      for (j=0; j<Spe_Total_NO[Cwan]; j++){
        printf("%7.4f ",OLP_L[2][Mc_AN][0][i][j]);
      }
      printf("\n");
    }
  }
  */

  /****************************************************
                   freeing of arrays:
  ****************************************************/

  free(OneD2h_AN);
  free(OneD2Mc_AN);

  /* for time */
  dtime(&TEtime);
  time0 = TEtime - TStime;

  return time0;
}


