#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libemd.h"

/***************************************************************/

void readin(char *string);

/***************************************************************/

float dist(emd_feature_t *F1, emd_feature_t *F2)
{
  int dX = F1->X - F2->X, dY = F1->Y - F2->Y, dZ = F1->Z - F2->Z;
  return sqrt(dX*dX + dY*dY + dZ*dZ);
}

/***************************************************************/

int main(int argc, char* argv[])
{
 char* usage = "./main [InputFile]";
 char *FileName;
 int argn = 2;

 if ( argn == argc )
        FileName = argv[1];
  else {
        fprintf(stderr, "\n\tUsage : %s\n\n", usage);
        exit(-1);
       }
 readin(FileName);
 
 /* anstatt readin(FileName) 2 pbm-Bilder 
  * (z.B. aus ~/Data/IKEA/DataSet01/Segmentation/Pbm02/*_pbm.png)
  * einlesen und schwarze Pixel (x,y) in Feature[i].X=x, Feature[i].Y=y, 
  * Feature[i].Z=0, Weights[i]=1.0 eintragen. 
  * Schwerpunkt der Pixelmenge von feature-Punkten abziehen.
  * Weights normieren auf gleiche Masse.
  * float e = emd(&signature1, &signature2, dist, 0, 0);
  * aufrufen und Ergebnis ausgeben
  */

 return 0;
}

/***************************************************************/

void readin(char *FileName)
{
 FILE *fp; 
 int number1, number2; //number of points in distributions.
 int i, x, y, z;
 float mass;

 if ((fp = fopen(FileName, "r")) == NULL)
   {
     fprintf(stderr,"can not open the file !!\n");
     exit(-1);
   }

 fscanf(fp,"%d",&number1);
 {
     float w1[number1];		 // mass of points.
     emd_feature_t f1[number1];
     
     for (i=0; i<number1; i++){
	 fscanf(fp,"%d",&x);	f1[i].X = x;
	 fscanf(fp,"%d",&y);	f1[i].Y = y;
	 fscanf(fp,"%d",&z);	f1[i].Z = z;
     }
     for (i=0; i<number1; i++){
	 fscanf(fp,"%f",&mass);
	 w1[i] = mass;
     }
     
     fscanf(fp,"%d",&number2);
     {
	 float w2[number2];           // mass of points.
	 emd_feature_t f2[number2]; 
	 
	 for (i=0; i<number2; i++){
	     fscanf(fp,"%d",&x);     f2[i].X = x;
	     fscanf(fp,"%d",&y);     f2[i].Y = y;
	     fscanf(fp,"%d",&z);     f2[i].Z = z;
	 }
	 for (i=0; i<number2; i++){
	     fscanf(fp,"%f",&mass);
	     w2[i] = mass;
	 }
	 {
	     emd_signature_t  s1 = { number1, f1, w1},
		 s2 = { number2, f2, w2}; /* construct 2 signatures. */
		 
		 float       e;
		 e = emd(&s1, &s2, dist, 0, 0);       /* computing earth mover's distance by calling emd */
		 printf("emd=%f\n", e);
	 }
     }
 }
/*
 for (i=0; i<number1; i++) 
	cout << Coord1[i][0] << " " << Coord1[i][1] << " " <<Coord1[i][2] << 
		" Mass :" << Mass1[i] << endl;

 for (i=0; i<number2; i++)
        cout << Coord2[i][0] << " " << Coord2[i][1] << " " <<Coord2[i][2] <<
                " Mass :" << Mass2[i] << endl;
*/

}


