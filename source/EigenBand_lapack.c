#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "openmx_common.h"
#include "lapack_prototypes.h"

#define  measure_time   0

static void Eigen_zheev(dcomplex **A, double *W, int N0);
static void Eigen_zheevx(dcomplex **A, double *W, int N0, int MaxN, int ev_flag);
static void Eigen_HH(dcomplex **ac, double *ko, int n, int EVmax, int ev_flag);


void EigenBand_lapack(dcomplex **A, double *W, int N0, int MaxN, int ev_flag)
{

  Eigen_HH(A,W,N0,MaxN,ev_flag);

  /*
  Eigen_zheevx(A,W,N0,MaxN,ev_flag);
  */

  /*
  Eigen_zheev(A,W,N0);
  */
}


#pragma optimization_level 1
void Eigen_HH(dcomplex **ac, double *ko, int n, int EVmax, int ev_flag)
{
  /**********************************************************************
    Eigen_HH:

    Eigen_HH.c is a subroutine to solve a standard eigenvalue problem
    with a Hermite complex matrix using Householder method and lapack's
    dstevx, dstegr, or dstedc.

    Log of Eigen_HH.c:

       Dec/07/2004  Released by T.Ozaki

  ***********************************************************************/

  double ABSTOL=3.0e-15;

  dcomplex **ad,*u,*b1,*p,*q,tmp0,tmp1,tmp2,tmp3,u1,u2,p1;
  dcomplex ss0,ss1,ss2,ss3,ss,p10,p11,p12,p13;
  double *D,*E,*uu,*alphar,*alphai,
         s1,s2,s3,r,
         sum,ar,ai,br,bi,e,
         a1,a2,a3,a4,a5,a6,b7,r0,r1,r2,
         r3,x1,x2,xap,
         bb,bb1,ui,uj,uij;

  int jj,jj1,jj2,k,ii,ll,i3,i2,j2,
    i,j,i1,j1,n1,n2,ik,ks,i1s,
      jk,po1,nn,count;

  double Stime, Etime;
  double Stime1, Etime1;
  double Stime2, Etime2;
  double time1,time2;

  /****************************************************
    allocation of arrays:
  ****************************************************/

  n2 = n + 5;

  ad = (dcomplex**)malloc(sizeof(dcomplex*)*n2);
  for (i=0; i<n2; i++){
    ad[i] = (dcomplex*)malloc(sizeof(dcomplex)*n2);
  }

  b1 = (dcomplex*)malloc(sizeof(dcomplex)*n2);
  u = (dcomplex*)malloc(sizeof(dcomplex)*n2);
  uu = (double*)malloc(sizeof(double)*n2);
  p = (dcomplex*)malloc(sizeof(dcomplex)*n2);
  q = (dcomplex*)malloc(sizeof(dcomplex)*n2);

  D = (double*)malloc(sizeof(double)*n2);
  E = (double*)malloc(sizeof(double)*n2);

  alphar = (double*)malloc(sizeof(double)*n2);
  alphai = (double*)malloc(sizeof(double)*n2);

  for (i=1; i<=(n+2); i++){
    uu[i] = 0.0;
  }

  if (measure_time==1) printf("size n=%3d EVmax=%2d\n",n,EVmax);
  if (measure_time==1) dtime(&Stime);

  /****************************************************
                Householder transformation
  ****************************************************/

  for (i=1; i<=(n-1); i++){

    /* original version */

    /*
    s1 = ac[i+1][i].r * ac[i+1][i].r + ac[i+1][i].i * ac[i+1][i].i;
    s2 = 0.0;

    u[i+1].r = ac[i+1][i].r;
    u[i+1].i = ac[i+1][i].i;
 
    for (i1=i+2; i1<=n; i1++){

      tmp1.r = ac[i1][i].r; 
      tmp1.i = ac[i1][i].i; 

      s2 += tmp1.r*tmp1.r + tmp1.i*tmp1.i;

      u[i1].r = tmp1.r;
      u[i1].i = tmp1.i;
    }
    s3 = fabs(s1 + s2);
    */

    /* unrolling version */

    s1 = ac[i+1][i].r * ac[i+1][i].r + ac[i+1][i].i * ac[i+1][i].i;
    s2 = 0.0;

    u[i+1].r = ac[i+1][i].r;
    u[i+1].i = ac[i+1][i].i;
 
    for (i1=i+2; i1<=(n-1); i1+=2){

      s2 += ac[i1+0][i].r*ac[i1+0][i].r + ac[i1+0][i].i*ac[i1+0][i].i
  	  + ac[i1+1][i].r*ac[i1+1][i].r + ac[i1+1][i].i*ac[i1+1][i].i;

      u[i1+0] = ac[i1+0][i];
      u[i1+1] = ac[i1+1][i];
    }

    i1s = n + 1 - (n+1-(i+2))%2;

    for (i1=i1s; ((i+2)<=i1 && i1<=n); i1++){
      s2 += ac[i1][i].r*ac[i1][i].r + ac[i1][i].i*ac[i1][i].i;
      u[i1] = ac[i1][i];
    }
    s3 = fabs(s1 + s2);

    if ( ABSTOL<(fabs(ac[i+1][i].r)+fabs(ac[i+1][i].i)) ){
      if (ac[i+1][i].r<0.0)  s3 =  sqrt(s3);
      else                   s3 = -sqrt(s3);
    }
    else{
      s3 = sqrt(s3);
    }

    if ( ABSTOL<fabs(s2) || 1.0e-10<fabs(u[i+1].i) || i==(n-1) ){

      ss.r = ac[i+1][i].r;
      ss.i = ac[i+1][i].i;

      ac[i+1][i].r = s3;
      ac[i+1][i].i = 0.0;
      ac[i][i+1].r = s3;
      ac[i][i+1].i = 0.0;

      u[i+1].r = u[i+1].r - s3;
      u[i+1].i = u[i+1].i;
      
      u1.r = s3 * s3 - ss.r * s3;
      u1.i =         - ss.i * s3;
      u2.r = 2.0 * u1.r;
      u2.i = 2.0 * u1.i;
      
      e = u2.r/(u1.r*u1.r + u1.i*u1.i);
      ar = e*u1.r;
      ai = e*u1.i;

      /* store alpha */
      alphar[i] = ar;
      alphai[i] = ai;

      /* store u2 */
      uu[i] = u2.r;

      /* store the first component of u */
      b1[i].r = ss.r - s3;
      b1[i].i = ss.i;

      r = 0.0;
      for (i1=i+1; i1<=n; i1++){

	p1.r = 0.0;
	p1.i = 0.0;
	for (j=i+1; j<=n; j++){
	  p1.r += ac[i1][j].r * u[j].r - ac[i1][j].i * u[j].i;
	  p1.i += ac[i1][j].r * u[j].i + ac[i1][j].i * u[j].r;
	}
	p[i1].r = p1.r / u1.r;
	p[i1].i = p1.i / u1.r;

	r += u[i1].r * p[i1].r + u[i1].i * p[i1].i;
      }
      r = 0.5*r / u2.r;

      br =  ar*r;
      bi = -ai*r;

      for (i1=i+1; i1<=n; i1++){
	tmp1.r = 0.5*(p[i1].r - (br * u[i1].r - bi*u[i1].i));
	tmp1.i = 0.5*(p[i1].i - (br * u[i1].i + bi*u[i1].r));
	q[i1].r = ar * tmp1.r - ai * tmp1.i; 
	q[i1].i = ar * tmp1.i + ai * tmp1.r; 
      }

      for (i1=i+1; i1<=n; i1++){
        tmp1.r = u[i1].r;
        tmp1.i = u[i1].i;
        tmp2.r = q[i1].r; 
        tmp2.i = q[i1].i; 
	for (j1=i+1; j1<=n; j1++){
	  ac[i1][j1].r -= ( tmp1.r * q[j1].r + tmp1.i * q[j1].i
                           +tmp2.r * u[j1].r + tmp2.i * u[j1].i );
	  ac[i1][j1].i -= (-tmp1.r * q[j1].i + tmp1.i * q[j1].r
                           -tmp2.r * u[j1].i + tmp2.i * u[j1].r );
	}
      }
    }
  }

  for (i=1; i<=n; i++){
    for (j=1; j<=n; j++){
      ad[i][j].r = ac[i][j].r;
      ad[i][j].i = ac[i][j].i;
    }
  }

  if (measure_time==1){
    dtime(&Etime);
    printf("T1   %15.12f\n",Etime-Stime);
  }

  /****************************************************
                     call a lapack routine
  ****************************************************/

  if (measure_time==1) dtime(&Stime);

  for (i=1; i<=n; i++){
    D[i-1] = ad[i][i  ].r;
    E[i-1] = ad[i][i+1].r;
  }

  if      (dste_flag==0) lapack_dstegr2(n,EVmax,D,E,ko,ac);
  else if (dste_flag==1) lapack_dstedc2(n,D,E,ko,ac);
  else if (dste_flag==2) lapack_dstevx2(n,EVmax,D,E,ko,ac,ev_flag);

  if (measure_time==1){
    dtime(&Etime);
    printf("T2   %15.12f\n",Etime-Stime);
  }

  /****************************************************
    transformation of eigenvectors to original space
  ****************************************************/

  if (measure_time==1) dtime(&Stime);

  if (ev_flag){

    /* ad stores u */
    for (i=2; i<=n; i++){
      ad[i-1][i].r = b1[i-1].r;
      ad[i-1][i].i =-b1[i-1].i;
      ad[i][i-1].r = b1[i-1].r;
      ad[i][i-1].i = b1[i-1].i;
    }

    /* original version */

    /*
      for (k=1; k<=EVmax; k++){
  
      for (nn=1; nn<=n-1; nn++){

      if ( (1.0e-3*ABSTOL)<fabs(uu[n-nn])){

      tmp1.r = 0.0;
      tmp1.i = 0.0;

      for (i=n-nn+1; i<=n; i++){
      tmp1.r += ad[n-nn][i].r * ac[k][i].r - ad[n-nn][i].i * ac[k][i].i;
      tmp1.i += ad[n-nn][i].i * ac[k][i].r + ad[n-nn][i].r * ac[k][i].i;
      }

      ss.r = (alphar[n-nn]*tmp1.r - alphai[n-nn]*tmp1.i) / uu[n-nn];
      ss.i = (alphar[n-nn]*tmp1.i + alphai[n-nn]*tmp1.r) / uu[n-nn];

      for (i=n-nn+1; i<=n; i++){
      ac[k][i].r -= ss.r * ad[n-nn][i].r + ss.i * ad[n-nn][i].i;
      ac[k][i].i -=-ss.r * ad[n-nn][i].i + ss.i * ad[n-nn][i].r;
      }
      }
      }
      }
    */

    /* unrolling version */

    for (k=1; k<=(EVmax-3); k+=4){
  
      for (nn=1; nn<=n-1; nn++){

	if ( (1.0e-3*ABSTOL)<fabs(uu[n-nn])){

	  tmp0.r = 0.0;	tmp0.i = 0.0;
	  tmp1.r = 0.0;	tmp1.i = 0.0;
	  tmp2.r = 0.0;	tmp2.i = 0.0;
	  tmp3.r = 0.0;	tmp3.i = 0.0;

	  for (i=n-nn+1; i<=n; i++){

	    tmp0.r += ad[n-nn][i].r * ac[k+0][i].r - ad[n-nn][i].i * ac[k+0][i].i;
	    tmp0.i += ad[n-nn][i].i * ac[k+0][i].r + ad[n-nn][i].r * ac[k+0][i].i;

	    tmp1.r += ad[n-nn][i].r * ac[k+1][i].r - ad[n-nn][i].i * ac[k+1][i].i;
	    tmp1.i += ad[n-nn][i].i * ac[k+1][i].r + ad[n-nn][i].r * ac[k+1][i].i;

	    tmp2.r += ad[n-nn][i].r * ac[k+2][i].r - ad[n-nn][i].i * ac[k+2][i].i;
	    tmp2.i += ad[n-nn][i].i * ac[k+2][i].r + ad[n-nn][i].r * ac[k+2][i].i;

	    tmp3.r += ad[n-nn][i].r * ac[k+3][i].r - ad[n-nn][i].i * ac[k+3][i].i;
	    tmp3.i += ad[n-nn][i].i * ac[k+3][i].r + ad[n-nn][i].r * ac[k+3][i].i;
	  }

	  ss0.r = (alphar[n-nn]*tmp0.r - alphai[n-nn]*tmp0.i) / uu[n-nn];
	  ss0.i = (alphar[n-nn]*tmp0.i + alphai[n-nn]*tmp0.r) / uu[n-nn];

	  ss1.r = (alphar[n-nn]*tmp1.r - alphai[n-nn]*tmp1.i) / uu[n-nn];
	  ss1.i = (alphar[n-nn]*tmp1.i + alphai[n-nn]*tmp1.r) / uu[n-nn];

	  ss2.r = (alphar[n-nn]*tmp2.r - alphai[n-nn]*tmp2.i) / uu[n-nn];
	  ss2.i = (alphar[n-nn]*tmp2.i + alphai[n-nn]*tmp2.r) / uu[n-nn];

	  ss3.r = (alphar[n-nn]*tmp3.r - alphai[n-nn]*tmp3.i) / uu[n-nn];
	  ss3.i = (alphar[n-nn]*tmp3.i + alphai[n-nn]*tmp3.r) / uu[n-nn];

	  for (i=n-nn+1; i<=n; i++){
	    ac[k+0][i].r -= ss0.r * ad[n-nn][i].r + ss0.i * ad[n-nn][i].i;
	    ac[k+0][i].i -=-ss0.r * ad[n-nn][i].i + ss0.i * ad[n-nn][i].r;

	    ac[k+1][i].r -= ss1.r * ad[n-nn][i].r + ss1.i * ad[n-nn][i].i;
	    ac[k+1][i].i -=-ss1.r * ad[n-nn][i].i + ss1.i * ad[n-nn][i].r;

	    ac[k+2][i].r -= ss2.r * ad[n-nn][i].r + ss2.i * ad[n-nn][i].i;
	    ac[k+2][i].i -=-ss2.r * ad[n-nn][i].i + ss2.i * ad[n-nn][i].r;

	    ac[k+3][i].r -= ss3.r * ad[n-nn][i].r + ss3.i * ad[n-nn][i].i;
	    ac[k+3][i].i -=-ss3.r * ad[n-nn][i].i + ss3.i * ad[n-nn][i].r;
	  }
	}
      }
    }

    ks = EVmax - EVmax%4 + 1;

    for (k=ks; k<=EVmax; k++){
  
      for (nn=1; nn<=n-1; nn++){

	if ( (1.0e-3*ABSTOL)<fabs(uu[n-nn])){

	  tmp1.r = 0.0;
	  tmp1.i = 0.0;

	  for (i=n-nn+1; i<=n; i++){
	    tmp1.r += ad[n-nn][i].r * ac[k][i].r - ad[n-nn][i].i * ac[k][i].i;
	    tmp1.i += ad[n-nn][i].i * ac[k][i].r + ad[n-nn][i].r * ac[k][i].i;
	  }

	  ss.r = (alphar[n-nn]*tmp1.r - alphai[n-nn]*tmp1.i) / uu[n-nn];
	  ss.i = (alphar[n-nn]*tmp1.i + alphai[n-nn]*tmp1.r) / uu[n-nn];

	  for (i=n-nn+1; i<=n; i++){
	    ac[k][i].r -= ss.r * ad[n-nn][i].r + ss.i * ad[n-nn][i].i;
	    ac[k][i].i -=-ss.r * ad[n-nn][i].i + ss.i * ad[n-nn][i].r;
	  }
	}
      }
    }

    if (measure_time==1){
      dtime(&Etime);
      printf("T4   %15.12f\n",Etime-Stime);
    }

    /****************************************************
                     normalization
    ****************************************************/

    if (measure_time==1) dtime(&Stime);
  
    for (j=1; j<=EVmax; j++){
      sum = 0.0;
      for (i=1; i<=n; i++){
	sum += ac[j][i].r * ac[j][i].r + ac[j][i].i * ac[j][i].i;
      }
      sum = 1.0/sqrt(sum);
      for (i=1; i<=n; i++){
	ac[j][i].r = ac[j][i].r * sum;
	ac[j][i].i = ac[j][i].i * sum;
      }
    }

    if (measure_time==1){
      dtime(&Etime);
      printf("T5   %15.12f\n",Etime-Stime);
    }

    /****************************************************
                     transpose ac
    ****************************************************/

    for (i=1; i<=n; i++){
      for (j=(i+1); j<=n; j++){
	tmp1 = ac[i][j];
	tmp2 = ac[j][i];
	ac[i][j] = tmp2;
	ac[j][i] = tmp1;
      }
    }

  }

  /*
  printf("check normalization\n");
  for (i=1; i<=n; i++){
    for (j=1; j<=n; j++){
      sum = 0.0;
      for (k=1; k<=n; k++){
        sum += ac[k][i].r*ac[k][j].r + ac[k][i].i*ac[k][j].i; 
      }
      printf("%15.12f ",sum);
    }
    printf("\n");
  }
  */

  /****************************************************
                  freeing of arrays:
  ****************************************************/

  for (i=0; i<n2; i++){
    free(ad[i]);
  }
  free(ad);

  free(b1);
  free(u);
  free(uu);
  free(p);
  free(q);
  free(D);
  free(E);
  free(alphar);
  free(alphai);
}



void Eigen_zheev(dcomplex **A, double *W, int N0)
{
  char *JOBZ="V";
  char *UPLO="L";
  INTEGER N=N0;
  INTEGER LWORK;
  dcomplex *A0;
  dcomplex *WORK;
  double *RWORK;
  INTEGER INFO;
  int i,j;

  A0 = (dcomplex*)malloc(sizeof(dcomplex)*N*N);

  for (i=1; i<=N; i++) {
    for (j=1; j<=N; j++) {
         /* A[i,j] */
        A0[(j-1)*N+i-1] = A[i][j];
    }
  }


  LWORK=3*N; 
  WORK=(dcomplex*)malloc(sizeof(dcomplex)*LWORK);
  RWORK=(double*)malloc(sizeof(double)*(3*N-2));
  F77_NAME(zheev,ZHEEV)(JOBZ,UPLO, &N, A0, &N, W, WORK, &LWORK, RWORK, &INFO  );


  if (INFO!=0) {
     printf("************************************************************\n");
     printf("  EigenBand_lapack: cheev_()=%d\n",INFO);
     printf("************************************************************\n");
     exit(10);
  }

/*
  printf("EigenBand_lapack:");
  for (i=0;i<N;i++) printf("%f ",W[i]); printf("\n");
  for (i=0;i<N*N;i++) {
    printf("(%f %f) ",A0[i].r,A0[i].i);
  }
  printf("\n");
*/

  for (i=1;i<=N;i++) {
    for (j=1;j<=N;j++) {
         /* A[i,j] */
        A[i][j].r = A0[(j-1)*N+i-1].r;
        A[i][j].i = A0[(j-1)*N+i-1].i;
    }
  }


  /* shift by 1 */
  for (i=N;i>=1;i--) {
    W[i] =W[i-1];
  }


  free(A0); free(RWORK); free(WORK); 

}




void Eigen_zheevx(dcomplex **A, double *W, int N0, int MaxN, int ev_flag)
{
  char *JOBZ0="N";
  char *JOBZ1="V";
  char *UPLO="L";
  char *RANGE="I";
  INTEGER N=N0;
  INTEGER LDA=N;
  double VL,VU;
  INTEGER  IL,IU;
  double ABSTOL=1.0e-13; /* the default value = 1.0e-12 */
  INTEGER M;
  dcomplex *Z;
  INTEGER LDZ=N;
  dcomplex *WORK;
  INTEGER LWORK;
  double *RWORK;
  INTEGER  *IWORK;
  INTEGER *IFAIL,INFO;
  dcomplex *A0;
  int i,j;

  IL = 1;
  IU = MaxN; 

  A0=(dcomplex*)malloc(sizeof(dcomplex)*N*N);
  for (i=0; i<N*N; i++){
    A0[i].r = 0.0;
    A0[i].i = 0.0;
  }

  for (i=1;i<=N;i++) {
    for (j=1;j<=N;j++) {
       /* A[i,j] */
       A0[(j-1)*N+i-1].r = A[i][j].r;
       A0[(j-1)*N+i-1].i = A[i][j].i;
    }
  }

  Z=(dcomplex*)malloc(sizeof(dcomplex)*N*N);
  for (i=0; i<N*N; i++){
    Z[i].r = 0.0;
    Z[i].i = 0.0;
  }

  LWORK= 3*N;
  WORK=(dcomplex*)malloc(sizeof(dcomplex)*LWORK);
  for (i=0; i<LWORK; i++){
    WORK[i].r =0.0;
    WORK[i].i =0.0;
  }

  RWORK=(double*)malloc(sizeof(double)*7*N);
  for (i=0; i<7*N; i++) RWORK[i] = 0.0;

  IWORK=(INTEGER*)malloc(sizeof(INTEGER)*5*N);
  for (i=0; i<5*N; i++) IWORK[i] = 0;

  IFAIL=(INTEGER*)malloc(sizeof(INTEGER)*N);
  for (i=0; i<N; i++) IFAIL[i] = 0;

  if (ev_flag==1){
    F77_NAME(zheevx,ZHEEVX)(JOBZ1, RANGE, UPLO, &N, A0, &LDA, &VL, &VU, &IL, &IU,
                            &ABSTOL, &M, W, Z, &LDZ, WORK, &LWORK, RWORK,
                            IWORK, IFAIL, &INFO );

    for (i=1;i<=N;i++) {
      for (j=1;j<=N;j++) {
	/* A[i,j] */
	A[i][j].r = Z[(j-1)*N+i-1].r;
	A[i][j].i = Z[(j-1)*N+i-1].i;
      }
    }
  }
  else {
    F77_NAME(zheevx,ZHEEVX)(JOBZ0, RANGE, UPLO, &N, A0, &LDA, &VL, &VU, &IL, &IU,
                            &ABSTOL, &M, W, Z, &LDZ, WORK, &LWORK, RWORK,
                            IWORK, IFAIL, &INFO );
  }

  /*
  printf("ret=%d\n",INFO);
  for (i=0;i<N;i++) printf("%lf ",W[i]);
  */

  /* shift by 1 */
  for (i=N;i>=1;i--) {
    W[i] =W[i-1];
  }

  free(IFAIL); free(IWORK); free(RWORK); free(WORK); free(A0); free(Z);

}

