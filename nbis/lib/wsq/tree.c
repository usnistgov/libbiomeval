/*******************************************************************************

License: 
This software and/or related materials was developed at the National Institute
of Standards and Technology (NIST) by employees of the Federal Government
in the course of their official duties. Pursuant to title 17 Section 105
of the United States Code, this software is not subject to copyright
protection and is in the public domain. 

This software and/or related materials have been determined to be not subject
to the EAR (see Part 734.3 of the EAR for exact details) because it is
a publicly available technology and software, and is freely distributed
to any interested party with no licensing requirements.  Therefore, it is 
permissible to distribute this software as a free download from the internet.

Disclaimer: 
This software and/or related materials was developed to promote biometric
standards and biometric technology testing for the Federal Government
in accordance with the USA PATRIOT Act and the Enhanced Border Security
and Visa Entry Reform Act. Specific hardware and software products identified
in this software were used in order to perform the software development.
In no case does such identification imply recommendation or endorsement
by the National Institute of Standards and Technology, nor does it imply that
the products and equipment identified are necessarily the best available
for the purpose.

This software and/or related materials are provided "AS-IS" without warranty
of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
licensed product, however used. In no event shall NIST be liable for any
damages and/or costs, including but not limited to incidental or consequential
damages of any kind, including economic damage or injury to property and lost
profits, regardless of whether NIST shall be advised, have reason to know,
or in fact shall know of the possibility.

By using this software, you agree to bear all risk relating to quality,
use and performance of the software and/or related materials.  You agree
to hold the Government harmless from any claim arising from your use
of the software.

*******************************************************************************/


/***********************************************************************
      LIBRARY: WSQ - Grayscale Image Compression

      FILE:    TREE.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    11/24/1999

      Contains routines responsible for manipulating tree structures
      used in WSQ image compression.

      ROUTINES:
#cat: biomeval_nbis_build_wsbiomeval_nbis_q_trees - Builds WSQ decomposition trees.
#cat:
#cat: biomeval_nbis_build_w_tree - Build subband x-y locations for creating wavelets.
#cat:
#cat: biomeval_nbis_w_tree4 - Derives location and size of subband splits.
#cat:
#cat: biomeval_nbis_build_q_tree - Build WSQ quantization tree of all 64 wavelet
#cat:                subband x-y locations and sizes.
#cat: biomeval_nbis_q_tree16 - Derive location and size for a 4x4 window of subbands.
#cat:
#cat: biomeval_nbis_q_tree4 - Derive location and size for 2x2 window of subbands.
#cat:

***********************************************************************/

#include <stdio.h>
#include <wsq.h>
#include <ihead.h>

/************************************************************************/
/*              Routines used to generate the "trees" used              */
/*              in creating the wavelet subbands (biomeval_nbis_w_tree)               */
/*              and when quantizing the subbands (biomeval_nbis_q_tree) in            */
/*              the WSQ compression/decompression algorithms.           */
/************************************************************************/
/* Build WSQ decomposition trees.                                       */
/************************************************************************/
void biomeval_nbis_build_wsbiomeval_nbis_q_trees(W_TREE biomeval_nbis_w_tree[], const int biomeval_nbis_w_treelen,
                     Q_TREE biomeval_nbis_q_tree[], const int biomeval_nbis_q_treelen,
                     const int width, const int height)
{
   /* Build a W-TREE structure for the image. */
   biomeval_nbis_build_w_tree(biomeval_nbis_w_tree, width, height);
   /* Build a Q-TREE structure for the image. */
   biomeval_nbis_build_q_tree(biomeval_nbis_w_tree, biomeval_nbis_q_tree);
}

/********************************************************************/
/* Routine to obtain subband "x-y locations" for creating wavelets. */
/********************************************************************/
void biomeval_nbis_build_w_tree(
   W_TREE biomeval_nbis_w_tree[],   /* wavelet tree structure */
   const int width,   /* image width            */
   const int height)  /* image height           */
{
   int lenx, lenx2, leny, leny2;  /* starting lengths of sections of
                                     the image being split into subbands */
   int node;

   for(node = 0; node < 20; node++) {
      biomeval_nbis_w_tree[node].inv_rw = 0;
      biomeval_nbis_w_tree[node].inv_cl = 0;
   }
   biomeval_nbis_w_tree[2].inv_rw = 1;
   biomeval_nbis_w_tree[4].inv_rw = 1;
   biomeval_nbis_w_tree[7].inv_rw = 1;
   biomeval_nbis_w_tree[9].inv_rw = 1;
   biomeval_nbis_w_tree[11].inv_rw = 1;
   biomeval_nbis_w_tree[13].inv_rw = 1;
   biomeval_nbis_w_tree[16].inv_rw = 1;
   biomeval_nbis_w_tree[18].inv_rw = 1;
   biomeval_nbis_w_tree[3].inv_cl = 1;
   biomeval_nbis_w_tree[5].inv_cl = 1;
   biomeval_nbis_w_tree[8].inv_cl = 1;
   biomeval_nbis_w_tree[9].inv_cl = 1;
   biomeval_nbis_w_tree[12].inv_cl = 1;
   biomeval_nbis_w_tree[13].inv_cl = 1;
   biomeval_nbis_w_tree[17].inv_cl = 1;
   biomeval_nbis_w_tree[18].inv_cl = 1;

   biomeval_nbis_w_tree4(biomeval_nbis_w_tree, 0, 1, width, height, 0, 0, 1);

   if((biomeval_nbis_w_tree[1].lenx % 2) == 0) {
      lenx = biomeval_nbis_w_tree[1].lenx / 2;
      lenx2 = lenx;
   }
   else {
      lenx = (biomeval_nbis_w_tree[1].lenx + 1) / 2;
      lenx2 = lenx - 1;
   }

   if((biomeval_nbis_w_tree[1].leny % 2) == 0) {
      leny = biomeval_nbis_w_tree[1].leny / 2;
      leny2 = leny;
   }
   else {
      leny = (biomeval_nbis_w_tree[1].leny + 1) / 2;
      leny2 = leny - 1;
   }

   biomeval_nbis_w_tree4(biomeval_nbis_w_tree, 4, 6, lenx2, leny, lenx, 0, 0);
   biomeval_nbis_w_tree4(biomeval_nbis_w_tree, 5, 10, lenx, leny2, 0, leny, 0);
   biomeval_nbis_w_tree4(biomeval_nbis_w_tree, 14, 15, lenx, leny, 0, 0, 0);

   biomeval_nbis_w_tree[19].x = 0;
   biomeval_nbis_w_tree[19].y = 0;
   if((biomeval_nbis_w_tree[15].lenx % 2) == 0)
      biomeval_nbis_w_tree[19].lenx = biomeval_nbis_w_tree[15].lenx / 2;
   else
      biomeval_nbis_w_tree[19].lenx = (biomeval_nbis_w_tree[15].lenx + 1) / 2;

   if((biomeval_nbis_w_tree[15].leny % 2) == 0)
      biomeval_nbis_w_tree[19].leny = biomeval_nbis_w_tree[15].leny / 2;
   else
      biomeval_nbis_w_tree[19].leny = (biomeval_nbis_w_tree[15].leny + 1) / 2;

   if(debug > 1) {
      for(node = 0; node < 20; node++)
         fprintf(stderr,
         "t%d -> x = %d  y = %d : dx = %d  dy = %d : ir = %d  ic = %d\n",
         node, biomeval_nbis_w_tree[node].x, biomeval_nbis_w_tree[node].y,
         biomeval_nbis_w_tree[node].lenx, biomeval_nbis_w_tree[node].leny,
         biomeval_nbis_w_tree[node].inv_rw, biomeval_nbis_w_tree[node].inv_cl);
      fprintf(stderr, "\n\n");
   }

   return;
}

/***************************************************************/
/* Gives location and size of subband splits for biomeval_nbis_build_w_tree. */
/***************************************************************/
void biomeval_nbis_w_tree4(
   W_TREE biomeval_nbis_w_tree[],    /* wavelet tree structure                      */
   const int start1,   /* biomeval_nbis_w_tree locations to start calculating       */
   const int start2,   /*    subband split locations and sizes        */
   const int lenx,     /* (temp) subband split location and sizes     */
   const int leny,
   const int x,
   const int y,
   const int stop1)    /* 0 normal operation, 1 used to avoid marking */
                       /*    size and location of subbands 60-63      */
{
   int evenx, eveny;   /* Check length of subband for even or odd */
   int p1, p2;         /* biomeval_nbis_w_tree locations for storing subband sizes and
                          locations */
   
   p1 = start1;
   p2 = start2;

   evenx = lenx % 2;
   eveny = leny % 2;

   biomeval_nbis_w_tree[p1].x = x;
   biomeval_nbis_w_tree[p1].y = y;
   biomeval_nbis_w_tree[p1].lenx = lenx;
   biomeval_nbis_w_tree[p1].leny = leny;
   
   biomeval_nbis_w_tree[p2].x = x;
   biomeval_nbis_w_tree[p2+2].x = x;
   biomeval_nbis_w_tree[p2].y = y;
   biomeval_nbis_w_tree[p2+1].y = y;

   if(evenx == 0) {
      biomeval_nbis_w_tree[p2].lenx = lenx / 2;
      biomeval_nbis_w_tree[p2+1].lenx = biomeval_nbis_w_tree[p2].lenx;
   }
   else {
      if(p1 == 4) {
         biomeval_nbis_w_tree[p2].lenx = (lenx - 1) / 2;
         biomeval_nbis_w_tree[p2+1].lenx = biomeval_nbis_w_tree[p2].lenx + 1;
      }
      else {
         biomeval_nbis_w_tree[p2].lenx = (lenx + 1) / 2;
         biomeval_nbis_w_tree[p2+1].lenx = biomeval_nbis_w_tree[p2].lenx - 1;
      }
   }
   biomeval_nbis_w_tree[p2+1].x = biomeval_nbis_w_tree[p2].lenx + x;
   if(stop1 == 0) {
      biomeval_nbis_w_tree[p2+3].lenx = biomeval_nbis_w_tree[p2+1].lenx;
      biomeval_nbis_w_tree[p2+3].x = biomeval_nbis_w_tree[p2+1].x;
   }
   biomeval_nbis_w_tree[p2+2].lenx = biomeval_nbis_w_tree[p2].lenx;


   if(eveny == 0) {
      biomeval_nbis_w_tree[p2].leny = leny / 2;
      biomeval_nbis_w_tree[p2+2].leny = biomeval_nbis_w_tree[p2].leny;
   }
   else {
      if(p1 == 5) {
         biomeval_nbis_w_tree[p2].leny = (leny - 1) / 2;
         biomeval_nbis_w_tree[p2+2].leny = biomeval_nbis_w_tree[p2].leny + 1;
      }
      else {
         biomeval_nbis_w_tree[p2].leny = (leny + 1) / 2;
         biomeval_nbis_w_tree[p2+2].leny = biomeval_nbis_w_tree[p2].leny - 1;
      }
   }
   biomeval_nbis_w_tree[p2+2].y = biomeval_nbis_w_tree[p2].leny + y;
   if(stop1 == 0) {
      biomeval_nbis_w_tree[p2+3].leny = biomeval_nbis_w_tree[p2+2].leny;
      biomeval_nbis_w_tree[p2+3].y = biomeval_nbis_w_tree[p2+2].y;
   }
   biomeval_nbis_w_tree[p2+1].leny = biomeval_nbis_w_tree[p2].leny;
}

/****************************************************************/
void biomeval_nbis_build_q_tree(
   W_TREE *biomeval_nbis_w_tree,  /* wavelet tree structure */
   Q_TREE *biomeval_nbis_q_tree)   /* quantization tree structure */
{
   int node;

   biomeval_nbis_q_tree16(biomeval_nbis_q_tree,3,biomeval_nbis_w_tree[14].lenx,biomeval_nbis_w_tree[14].leny,
              biomeval_nbis_w_tree[14].x,biomeval_nbis_w_tree[14].y, 0, 0);
   biomeval_nbis_q_tree16(biomeval_nbis_q_tree,19,biomeval_nbis_w_tree[4].lenx,biomeval_nbis_w_tree[4].leny,
              biomeval_nbis_w_tree[4].x,biomeval_nbis_w_tree[4].y, 0, 1);
   biomeval_nbis_q_tree16(biomeval_nbis_q_tree,48,biomeval_nbis_w_tree[0].lenx,biomeval_nbis_w_tree[0].leny,
              biomeval_nbis_w_tree[0].x,biomeval_nbis_w_tree[0].y, 0, 0);
   biomeval_nbis_q_tree16(biomeval_nbis_q_tree,35,biomeval_nbis_w_tree[5].lenx,biomeval_nbis_w_tree[5].leny,
              biomeval_nbis_w_tree[5].x,biomeval_nbis_w_tree[5].y, 1, 0);
   biomeval_nbis_q_tree4(biomeval_nbis_q_tree,0,biomeval_nbis_w_tree[19].lenx,biomeval_nbis_w_tree[19].leny,
             biomeval_nbis_w_tree[19].x,biomeval_nbis_w_tree[19].y);

   if(debug > 1) {
      for(node = 0; node < 60; node++)
         fprintf(stderr, "t%d -> x = %d  y = %d : lx = %d  ly = %d\n",
         node, biomeval_nbis_q_tree[node].x, biomeval_nbis_q_tree[node].y,
         biomeval_nbis_q_tree[node].lenx, biomeval_nbis_q_tree[node].leny);
      fprintf(stderr, "\n\n");
   }
   return;
}

/*****************************************************************/
void biomeval_nbis_q_tree16(
   Q_TREE *biomeval_nbis_q_tree,   /* quantization tree structure */
   const int start,  /* biomeval_nbis_q_tree location of first subband        */
                     /*   in the subband group being calculated */
   const int lenx,   /* (temp) subband location and sizes */
   const int leny,
   const int x,
   const int y,
   const int rw,  /* NEW */   /* spectral invert 1st row/col splits */
   const int cl)  /* NEW */
{
   int tempx, temp2x;   /* temporary x values */
   int tempy, temp2y;   /* temporary y values */
   int evenx, eveny;    /* Check length of subband for even or odd */
   int p;               /* indicates subband information being stored */

   p = start;
   evenx = lenx % 2;
   eveny = leny % 2;

   if(evenx == 0) {
      tempx = lenx / 2;
      temp2x = tempx;
   }
   else {
      if(cl) {
         temp2x = (lenx + 1) / 2;
         tempx = temp2x - 1;
      }
      else  {
        tempx = (lenx + 1) / 2;
        temp2x = tempx - 1;
      }
   }

   if(eveny == 0) {
      tempy = leny / 2;
      temp2y = tempy;
   }
   else {
      if(rw) {
         temp2y = (leny + 1) / 2;
         tempy = temp2y - 1;
      }
      else {
        tempy = (leny + 1) / 2;
        temp2y = tempy - 1;
      }
   }

   evenx = tempx % 2;
   eveny = tempy % 2;

   biomeval_nbis_q_tree[p].x = x;
   biomeval_nbis_q_tree[p+2].x = x;
   biomeval_nbis_q_tree[p].y = y;
   biomeval_nbis_q_tree[p+1].y = y;
   if(evenx == 0) {
      biomeval_nbis_q_tree[p].lenx = tempx / 2;
      biomeval_nbis_q_tree[p+1].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+2].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+3].lenx = biomeval_nbis_q_tree[p].lenx;
   }
   else {
      biomeval_nbis_q_tree[p].lenx = (tempx + 1) / 2;
      biomeval_nbis_q_tree[p+1].lenx = biomeval_nbis_q_tree[p].lenx - 1;
      biomeval_nbis_q_tree[p+2].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+3].lenx = biomeval_nbis_q_tree[p+1].lenx;
   }
   biomeval_nbis_q_tree[p+1].x = x + biomeval_nbis_q_tree[p].lenx;
   biomeval_nbis_q_tree[p+3].x = biomeval_nbis_q_tree[p+1].x;
   if(eveny == 0) {
      biomeval_nbis_q_tree[p].leny = tempy / 2;
      biomeval_nbis_q_tree[p+1].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+2].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+3].leny = biomeval_nbis_q_tree[p].leny;
   }
   else {
      biomeval_nbis_q_tree[p].leny = (tempy + 1) / 2;
      biomeval_nbis_q_tree[p+1].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+2].leny = biomeval_nbis_q_tree[p].leny - 1;
      biomeval_nbis_q_tree[p+3].leny = biomeval_nbis_q_tree[p+2].leny;
   }
   biomeval_nbis_q_tree[p+2].y = y + biomeval_nbis_q_tree[p].leny;
   biomeval_nbis_q_tree[p+3].y = biomeval_nbis_q_tree[p+2].y;


   evenx = temp2x % 2;

   biomeval_nbis_q_tree[p+4].x = x + tempx;
   biomeval_nbis_q_tree[p+6].x = biomeval_nbis_q_tree[p+4].x;
   biomeval_nbis_q_tree[p+4].y = y;
   biomeval_nbis_q_tree[p+5].y = y;
   biomeval_nbis_q_tree[p+6].y = biomeval_nbis_q_tree[p+2].y;
   biomeval_nbis_q_tree[p+7].y = biomeval_nbis_q_tree[p+2].y;
   biomeval_nbis_q_tree[p+4].leny = biomeval_nbis_q_tree[p].leny;
   biomeval_nbis_q_tree[p+5].leny = biomeval_nbis_q_tree[p].leny;
   biomeval_nbis_q_tree[p+6].leny = biomeval_nbis_q_tree[p+2].leny;
   biomeval_nbis_q_tree[p+7].leny = biomeval_nbis_q_tree[p+2].leny;
   if(evenx == 0) {
      biomeval_nbis_q_tree[p+4].lenx = temp2x / 2;
      biomeval_nbis_q_tree[p+5].lenx = biomeval_nbis_q_tree[p+4].lenx;
      biomeval_nbis_q_tree[p+6].lenx = biomeval_nbis_q_tree[p+4].lenx;
      biomeval_nbis_q_tree[p+7].lenx = biomeval_nbis_q_tree[p+4].lenx;
   }
   else {
      biomeval_nbis_q_tree[p+5].lenx = (temp2x + 1) / 2;
      biomeval_nbis_q_tree[p+4].lenx = biomeval_nbis_q_tree[p+5].lenx - 1;
      biomeval_nbis_q_tree[p+6].lenx = biomeval_nbis_q_tree[p+4].lenx;
      biomeval_nbis_q_tree[p+7].lenx = biomeval_nbis_q_tree[p+5].lenx;
   }
   biomeval_nbis_q_tree[p+5].x = biomeval_nbis_q_tree[p+4].x + biomeval_nbis_q_tree[p+4].lenx;
   biomeval_nbis_q_tree[p+7].x = biomeval_nbis_q_tree[p+5].x;


   eveny = temp2y % 2;

   biomeval_nbis_q_tree[p+8].x = x;
   biomeval_nbis_q_tree[p+9].x = biomeval_nbis_q_tree[p+1].x;
   biomeval_nbis_q_tree[p+10].x = x;
   biomeval_nbis_q_tree[p+11].x = biomeval_nbis_q_tree[p+1].x;
   biomeval_nbis_q_tree[p+8].y = y + tempy;
   biomeval_nbis_q_tree[p+9].y = biomeval_nbis_q_tree[p+8].y;
   biomeval_nbis_q_tree[p+8].lenx = biomeval_nbis_q_tree[p].lenx;
   biomeval_nbis_q_tree[p+9].lenx = biomeval_nbis_q_tree[p+1].lenx;
   biomeval_nbis_q_tree[p+10].lenx = biomeval_nbis_q_tree[p].lenx;
   biomeval_nbis_q_tree[p+11].lenx = biomeval_nbis_q_tree[p+1].lenx;
   if(eveny == 0) {
      biomeval_nbis_q_tree[p+8].leny = temp2y / 2;
      biomeval_nbis_q_tree[p+9].leny = biomeval_nbis_q_tree[p+8].leny;
      biomeval_nbis_q_tree[p+10].leny = biomeval_nbis_q_tree[p+8].leny;
      biomeval_nbis_q_tree[p+11].leny = biomeval_nbis_q_tree[p+8].leny;
   }
   else {
      biomeval_nbis_q_tree[p+10].leny = (temp2y + 1) / 2;
      biomeval_nbis_q_tree[p+11].leny = biomeval_nbis_q_tree[p+10].leny;
      biomeval_nbis_q_tree[p+8].leny = biomeval_nbis_q_tree[p+10].leny - 1;
      biomeval_nbis_q_tree[p+9].leny = biomeval_nbis_q_tree[p+8].leny;
   }
   biomeval_nbis_q_tree[p+10].y = biomeval_nbis_q_tree[p+8].y + biomeval_nbis_q_tree[p+8].leny;
   biomeval_nbis_q_tree[p+11].y = biomeval_nbis_q_tree[p+10].y;


   biomeval_nbis_q_tree[p+12].x = biomeval_nbis_q_tree[p+4].x;
   biomeval_nbis_q_tree[p+13].x = biomeval_nbis_q_tree[p+5].x;
   biomeval_nbis_q_tree[p+14].x = biomeval_nbis_q_tree[p+4].x;
   biomeval_nbis_q_tree[p+15].x = biomeval_nbis_q_tree[p+5].x;
   biomeval_nbis_q_tree[p+12].y = biomeval_nbis_q_tree[p+8].y;
   biomeval_nbis_q_tree[p+13].y = biomeval_nbis_q_tree[p+8].y;
   biomeval_nbis_q_tree[p+14].y = biomeval_nbis_q_tree[p+10].y;
   biomeval_nbis_q_tree[p+15].y = biomeval_nbis_q_tree[p+10].y;
   biomeval_nbis_q_tree[p+12].lenx = biomeval_nbis_q_tree[p+4].lenx;
   biomeval_nbis_q_tree[p+13].lenx = biomeval_nbis_q_tree[p+5].lenx;
   biomeval_nbis_q_tree[p+14].lenx = biomeval_nbis_q_tree[p+4].lenx;
   biomeval_nbis_q_tree[p+15].lenx = biomeval_nbis_q_tree[p+5].lenx;
   biomeval_nbis_q_tree[p+12].leny = biomeval_nbis_q_tree[p+8].leny;
   biomeval_nbis_q_tree[p+13].leny = biomeval_nbis_q_tree[p+8].leny;
   biomeval_nbis_q_tree[p+14].leny = biomeval_nbis_q_tree[p+10].leny;
   biomeval_nbis_q_tree[p+15].leny = biomeval_nbis_q_tree[p+10].leny;
}

/********************************************************************/
void biomeval_nbis_q_tree4(
   Q_TREE *biomeval_nbis_q_tree,   /* quantization tree structure */
   const int start,  /* biomeval_nbis_q_tree location of first subband         */
                     /*    in the subband group being calculated */
   const int lenx,   /* (temp) subband location and sizes */
   const int leny,
   const int x,
   const int  y)        
{
   int evenx, eveny;    /* Check length of subband for even or odd */
   int p;               /* indicates subband information being stored */


   p = start;
   evenx = lenx % 2;
   eveny = leny % 2;


   biomeval_nbis_q_tree[p].x = x;
   biomeval_nbis_q_tree[p+2].x = x;
   biomeval_nbis_q_tree[p].y = y;
   biomeval_nbis_q_tree[p+1].y = y;
   if(evenx == 0) {
      biomeval_nbis_q_tree[p].lenx = lenx / 2;
      biomeval_nbis_q_tree[p+1].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+2].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+3].lenx = biomeval_nbis_q_tree[p].lenx;
   }
   else {
      biomeval_nbis_q_tree[p].lenx = (lenx + 1) / 2;
      biomeval_nbis_q_tree[p+1].lenx = biomeval_nbis_q_tree[p].lenx - 1;
      biomeval_nbis_q_tree[p+2].lenx = biomeval_nbis_q_tree[p].lenx;
      biomeval_nbis_q_tree[p+3].lenx = biomeval_nbis_q_tree[p+1].lenx;
   }
   biomeval_nbis_q_tree[p+1].x = x + biomeval_nbis_q_tree[p].lenx;
   biomeval_nbis_q_tree[p+3].x = biomeval_nbis_q_tree[p+1].x;
   if(eveny == 0) {
      biomeval_nbis_q_tree[p].leny = leny / 2;
      biomeval_nbis_q_tree[p+1].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+2].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+3].leny = biomeval_nbis_q_tree[p].leny;
   }
   else {
      biomeval_nbis_q_tree[p].leny = (leny + 1) / 2;
      biomeval_nbis_q_tree[p+1].leny = biomeval_nbis_q_tree[p].leny;
      biomeval_nbis_q_tree[p+2].leny = biomeval_nbis_q_tree[p].leny - 1;
      biomeval_nbis_q_tree[p+3].leny = biomeval_nbis_q_tree[p+2].leny;
   }
   biomeval_nbis_q_tree[p+2].y = y + biomeval_nbis_q_tree[p].leny;
   biomeval_nbis_q_tree[p+3].y = biomeval_nbis_q_tree[p+2].y;
}
