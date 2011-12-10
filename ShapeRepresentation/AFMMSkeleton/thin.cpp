#include <thin.h>


int ThinningMethod::test_template(int dir,int i,int j)	//Tests if (i,j) can be safely deleted in deletion-pass for direction 'dir'
{
   int up = f->value(i,j+1), dn = f->value(i,j-1), rt = f->value(i+1,j), lt = f->value(i-1,j);
   int N2 = f->value(i+1,j+1), N4 = f->value(i-1,j+1), N6 = f->value(i-1,j-1), N8 = f->value(i+1,j-1);

   //Ensure we're free in given direction
   if ((dir==0 && up) || (dir==1 && dn) || (dir==2 && lt) || (dir==3 && rt)) return 0;

   //Ensure we've got a nb on every axis
   if (up+dn==0 || lt+rt==0) return 0;

   //Still see we're not in the 'fish' situation:)
   if (up+dn==2 || rt+lt==2) return 1;
   if (up && rt && N6) return 0;
   if (up && lt && N8) return 0;
   if (dn && lt && N2) return 0;
   if (dn && rt && N4) return 0;
   return 1;
}



int ThinningMethod::deletion(int dir)			//Applies one deletion-pass 
{							//
   del.Flush();						//Cleanup del, since we'll insert points in it
   if (del.Size()<int(boundary->size())) 			//See if we've got to resize del - probably happens just the 1st time...
      del.Init(boundary->size());

   int ndel = 0;					//Count #deleted pixels

   PriList new_boundary;				//Stores newly discovered boundary pixels
   PriList::iterator p;

   for(p=boundary->begin();p!=boundary->end();p++) 
   {							//Iterate over all boundary-pixels (the only ones we can thin)
       Coord& c = (*p).second;				//Get current boundary pixel
       if (test_template(dir,c.i,c.j)) del.Add(c);
   }

   for(int q=0;q<del.Count();q++)
   {
	     Coord& c = del[q];
	     int ii = c.i, jj = c.j;		     	
             f->value(ii,jj) = 0; ndel++;
             b->value(ii,jj) = 0; 
             for(int i=ii-1;i<=ii+1;i++)		//Find new boundary-pixels created due to deletion of ii,jj,kk
               for(int j=jj-1;j<=jj+1;j++)
	           if (i==ii || j==jj)	
                   if (f->value(i,j) && !b->value(i,j))	//Discovered nonzero neighbor of deleted pixel which is not yet known in b[]
	           {
		     float v = dt->value(i,j);		//Add this neighbor to new_boundary[] in order of DT
		     new_boundary.insert(PriList::value_type(v,Coord(i,j))); 	
		     b->value(i,j) = 1;			//...and mark it as boundary pixel in b[]
                   }
   }  	

   boundary2->erase(boundary2->begin(),boundary2->end()); //Update boundary[] by removing from it all pixels which were deleted 
   for(p=boundary->begin();p!=boundary->end();p++)
   {
      Coord& c = (*p).second;
      if (b->value(c.i,c.j)) boundary2->insert(*p);
   }
   for(p=new_boundary.begin();p!=new_boundary.end();p++) //Update boundary[] by adding to it all pixels which became boundary due to the deletions.
      boundary2->insert(*p);

   PriList* tmp = boundary; 				//Swap boundary and boundary2
   boundary = boundary2; boundary2 = tmp; 
   return ndel;						//Return #deleted pixels
}



FIELD<int>* ThinningMethod::threshold(FIELD<float>* f,float low)
{
   FIELD<int>* g = new FIELD<int>(f->dimX(),f->dimY()); //Init bilevel-obj field
   b = new FIELD<int>(f->dimX(),f->dimY());		//Init boundary-marker field
   *b = 0;

   float* vptr = f->data(); int* fptr = g->data();
   for(float *vend =vptr+f->dimX()*f->dimY();vptr<vend;vptr++,fptr++)
      *fptr = ((low>0 && *vptr<low) || (low<0 && *vptr>-low))? 1 : 0;

     for(int j=0;j<f->dimY();j++)
       for(int i=0;i<f->dimX();i++)
       if (g->value(i,j))
	  if (g->value(i-1,j)==0 || g->value(i+1,j)==0 || g->value(i,j-1)==0 || g->value(i,j+1)==0)
	  { 
	     b->value(i,j) = 1;				//Insert pts in order of DT 		
	     float v = dt->value(i,j);
	     boundary->insert(PriList::value_type(v,Coord(i,j))); 
	  }

   return g;
}



ThinningMethod::ThinningMethod(FIELD<float>* d,FIELD<float>* dt_,float K)
	       :dt(dt_),total_del(0)
{
    //boundary_arr.Init(10000);				//Alloc some space for boundary-pixels
    //boundary_arr2.Init(10000);			//Alloc some space for boundary-pixels
    boundary  = &boundary_arr;
    boundary2 = &boundary_arr2;

    f = threshold(d,K);					//Threshold input file to obtain bin-volume. 
							//This also initializes boundary and b[]
}

ThinningMethod::~ThinningMethod()
{  }


FIELD<int>* ThinningMethod::thin()
{
    total_del=0;
    for(int nd=1;nd;)					//Directionally delete as long as it's possible: 
    {
      nd=0;						//Do the 4 directional-iterations, accumulate #deleted pts	
      for(int dir=0;dir<4;dir++)
         nd += deletion(dir);
      total_del += nd;					//Update total # deleted pts
    } 

    del.Init(0);			
    delete b;						//Release all memory we allocated
    boundary_arr.erase(boundary_arr.begin(),boundary_arr.end());				 
    boundary_arr2.erase(boundary_arr2.begin(),boundary_arr2.end());
    return f;						//Return thinned object
}


