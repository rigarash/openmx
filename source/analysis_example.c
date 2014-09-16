/**********************************************************************
  analysis_example.c:

     analysis_example.c provides examples for analyzing and 
     unitilizing Kohn-Sham Hamiltonian, overlap, and density
     matrices which are stored in filename.scfout.

  Log of analysis_example.c:

     2/July/2003  Released by T.Ozaki 

  ******************************************************************
    You can utilize a filename.scfout which is generated by the SCF
    calculation of OpenMX by the following procedure:

     1. Define your main routine as follows:

           int main(int argc, char *argv[]) 

     2. Include a header file, "read_scfout.h", in your main routine
        (if you want, also in other routines) as follows:

           #include "read_scfout.h"
  
     3. Call a function, read_scfout(), in the main routine as follows:

           read_scfout(argv);
  ******************************************************************

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "read_scfout.h"

int main(int argc, char *argv[]) 
{

  static int ct_AN,h_AN,Gh_AN,i,j,TNO1,TNO2;  
  static int spin,Rn;

  static double *a;
  static FILE *fp;

  read_scfout(argv);

  /**********************************************************************
   Example 1:

   Print the numbers of atoms

     atomnum:   the number of atoms in the total system
     Catomnum:  the number of atoms in the central region
     Latomnum:  the number of atoms in the left lead
     Ratomnum:  the number of atoms in the right lead

     grobal index of atom runs
     Catomnum -> Catomnum + Latomnum -> Catomnum + Latomnum + Ratomnum
  ***********************************************************************/

  printf("atomnum=%i\n", atomnum);
  printf("Catomnum=%i\n",Catomnum);
  printf("Latomnum=%i\n",Latomnum);
  printf("Ratomnum=%i\n",Ratomnum);

  /**********************************************************************
   Example 2:

   Print Kohn-Sham Hamiltonian

      Hks[spin][ct_AN][h_AN][i][j]

      spin:  spin=0, up
             spin=1, down

      ct_AN: global index of atoms
      h_AN   local index of neighbouring atoms for the atom ct_AN
      i:     orbital index in the atom ct_AN
      j:     orbital index in the atom h_AN

   NOTE: 

      For instance, if the basis specification of the atom ct_AN is s2p2,
      then the obital index runs in order of
                    s, s', px, py, pz, px', py', pz'

      Transformation of the local index h_AN to the grobal index Gh_AN
      is made as

                       Gh_AN = natn[ct_AN][h_AN];

      Also, the cell index is given by

                       Rn = ncn[ct_AN][h_AN];
      
      Each component l, m, or n (Rn = l*a + m*b + n*c) are given by
   
                       l = atv_ijk[Rn][1];
                       m = atv_ijk[Rn][2];
                       n = atv_ijk[Rn][3];
  ***********************************************************************/

  for (spin=0; spin<=SpinP_switch; spin++){
    printf("\n\nKohn-Sham Hamiltonian spin=%i\n",spin);
    for (ct_AN=1; ct_AN<=atomnum; ct_AN++){
      TNO1 = Total_NumOrbs[ct_AN];
      for (h_AN=0; h_AN<=FNAN[ct_AN]; h_AN++){
        Gh_AN = natn[ct_AN][h_AN];
        Rn = ncn[ct_AN][h_AN];
        TNO2 = Total_NumOrbs[Gh_AN];
        printf("glbal index=%i  local index=%i (grobal=%i, Rn=%i)\n",
                ct_AN,h_AN,Gh_AN,Rn);
        for (i=0; i<TNO1; i++){
          for (j=0; j<TNO2; j++){
            printf("%10.7f ",Hks[spin][ct_AN][h_AN][i][j]); 
	  }
          printf("\n");
	}
      }
    }
  }

  /**********************************************************************
   Example 3:

   Print overlap matrix

      OLP[ct_AN][h_AN][i][j]

      ct_AN: global index of atoms
      h_AN   local index of neighbouring atoms for the atom ct_AN
      i:     orbital index in the atom ct_AN
      j:     orbital index in the atom h_AN

   NOTE: 

      For instance, if the basis specification of the atom ct_AN is s2p2,
      then the obital index runs in order of
                    s, s', px, py, pz, px', py', pz'

      Transformation of the local index h_AN to the grobal index Gh_AN
      is made as

                       Gh_AN = natn[ct_AN][h_AN];

      Also, the cell index is given by

                       Rn = ncn[ct_AN][h_AN];
      
      Each component l, m, or n (Rn = l*a + m*b + n*c) are given by
   
                       l = atv_ijk[Rn][1];
                       m = atv_ijk[Rn][2];
                       n = atv_ijk[Rn][3];
  ***********************************************************************/

  printf("\n\nOverlap matrix\n");
  for (ct_AN=1; ct_AN<=atomnum; ct_AN++){
    TNO1 = Total_NumOrbs[ct_AN];
    for (h_AN=0; h_AN<=FNAN[ct_AN]; h_AN++){
      Gh_AN = natn[ct_AN][h_AN];
      Rn = ncn[ct_AN][h_AN];
      TNO2 = Total_NumOrbs[Gh_AN];
      printf("glbal index=%i  local index=%i (grobal=%i, Rn=%i)\n",
              ct_AN,h_AN,Gh_AN,Rn);
      for (i=0; i<TNO1; i++){
        for (j=0; j<TNO2; j++){
          printf("%10.7f ",OLP[ct_AN][h_AN][i][j]); 
        }
        printf("\n");
      }
    }
  }

  /**********************************************************************
   Example 4:

   Print overlap matrix with position operator x

      OLPpox[ct_AN][h_AN][i][j]

      ct_AN: global index of atoms
      h_AN   local index of neighbouring atoms for the atom ct_AN
      i:     orbital index in the atom ct_AN
      j:     orbital index in the atom h_AN

   NOTE: 

      For instance, if the basis specification of the atom ct_AN is s2p2,
      then the obital index runs in order of
                    s, s', px, py, pz, px', py', pz'

      Transformation of the local index h_AN to the grobal index Gh_AN
      is made as

                       Gh_AN = natn[ct_AN][h_AN];

      Also, the cell index is given by

                       Rn = ncn[ct_AN][h_AN];
      
      Each component l, m, or n (Rn = l*a + m*b + n*c) are given by
   
                       l = atv_ijk[Rn][1];
                       m = atv_ijk[Rn][2];
                       n = atv_ijk[Rn][3];
  ***********************************************************************/

  printf("\n\nOverlap matrix with position operator x\n");
  for (ct_AN=1; ct_AN<=atomnum; ct_AN++){
    TNO1 = Total_NumOrbs[ct_AN];
    for (h_AN=0; h_AN<=FNAN[ct_AN]; h_AN++){
      Gh_AN = natn[ct_AN][h_AN];
      Rn = ncn[ct_AN][h_AN];
      TNO2 = Total_NumOrbs[Gh_AN];
      printf("glbal index=%i  local index=%i (grobal=%i, Rn=%i)\n",
              ct_AN,h_AN,Gh_AN,Rn);
      for (i=0; i<TNO1; i++){
        for (j=0; j<TNO2; j++){
          printf("%10.7f ",OLPpox[ct_AN][h_AN][i][j]); 
        }
        printf("\n");
      }
    }
  }

  /**********************************************************************
   Example 5:

   Print density matrix

      DM[spin][ct_AN][h_AN][i][j]

      spin:  spin=0, up
             spin=1, down

      ct_AN: global index of atoms
      h_AN   local index of neighbouring atoms for the atom ct_AN
      i:     orbital index in the atom ct_AN
      j:     orbital index in the atom h_AN

   NOTE: 

      For instance, if the basis specification of the atom ct_AN is s2p2,
      then the obital index runs in order of
                    s, s', px, py, pz, px', py', pz'

      Transformation of the local index h_AN to the grobal index Gh_AN
      is made as

                       Gh_AN = natn[ct_AN][h_AN];

      Also, the cell index is given by

                       Rn = ncn[ct_AN][h_AN];
      
      Each component l, m, or n (Rn = l*a + m*b + n*c) are given by
   
                       l = atv_ijk[Rn][1];
                       m = atv_ijk[Rn][2];
                       n = atv_ijk[Rn][3];
  ***********************************************************************/

  for (spin=0; spin<=SpinP_switch; spin++){
    printf("\n\nDensity matrix spin=%i\n",spin);
    for (ct_AN=1; ct_AN<=atomnum; ct_AN++){
      TNO1 = Total_NumOrbs[ct_AN];
      for (h_AN=0; h_AN<=FNAN[ct_AN]; h_AN++){
        Gh_AN = natn[ct_AN][h_AN];
        Rn = ncn[ct_AN][h_AN];
        TNO2 = Total_NumOrbs[Gh_AN];

        printf("glbal index=%i  local index=%i (grobal=%i, Rn=%i)\n",
                ct_AN,h_AN,Gh_AN,Rn);

        for (i=0; i<TNO1; i++){
          for (j=0; j<TNO2; j++){
            printf("%10.7f ",DM[spin][ct_AN][h_AN][i][j]); 
	  }
          printf("\n");
	}
      }
    }
  }

}
