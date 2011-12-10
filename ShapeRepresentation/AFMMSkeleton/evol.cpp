#include <Tools/MathUtils.h>

#include "flags.h"											
#include "evol.h"
#include "darray.h"

static FIELD<OrigSet>* fo    = 0;
static FLAGS*          flags = 0;
static std::multimap<float,Coord> q;

int get_known_nbs(Coord& a,Coord* nbs)	        		//Finds nbs[] = all known neighbors of a
{                                                               //For the DT, we can just look at the NSEW nbs and it's sufficient. 
   int i=a.i,j=a.j,N=0;                                         //For the CDT, we must look at all nbs (diagonal too) for it to be correct
   for(int ii=i-1;ii<=i+1;ii++)
      for(int jj=j-1;jj<=j+1;jj++)
      {
	 //if (i!=ii && j!=jj) continue;			//Look only at NSEW nbs
	 if (i==ii && j==jj) continue;				//Look at all nbs (NSEW + diagonal)
	 if (flags->narrowband(ii,jj))
	    nbs[N++] = Coord(ii,jj);
      }
   return N;
}



void OrigSet::initialize(int i,int j)
	        {
	           erase(begin(),end());
                   std::multimap<float,Coord>::value_type v(0.0f,Coord(i,j));
	           insert(v);

                   Coord c(i,j); Coord nbs[20]; 
                   int N = get_known_nbs(c,nbs);                //Finds nbs[] = all known neighbors of a
                   for(int n=0;n<N;n++)
                   {
                     float d = c.dist(nbs[n]);
                     std::multimap<float,Coord>::value_type v(d,nbs[n]);	
                     insert(v);						
                   }
	        }




void init(FLAGS* f)			                        //Initializes the data-structs of the class.
{								//Sets fo[] on the boundary.
   fo    = new FIELD<OrigSet>(f->dimX(),f->dimY());
   flags = f;

   for(int i=0;i<f->dimX();i++)									
     for(int j=0;j<f->dimY();j++)
     if (f->narrowband(i,j))					//For the boundary points:
     {								
	fo->value(i,j).initialize(i,j);				//Initialize fo[] to the point itself
        std::multimap<float,Coord>::value_type v(0.0f,Coord(i,j));	//Add the point to the priority-queue q
	q.insert(v);						//with distance zero
     }
}


int find_free_nb(Coord& p,Coord& a)				//Finds a = one free (not yet discovered)
{								//neighbor of point p
   if (flags->faraway(p.i-1,p.j)) { a = Coord(p.i-1,p.j); return 1; }
   if (flags->faraway(p.i+1,p.j)) { a = Coord(p.i+1,p.j); return 1; }
   if (flags->faraway(p.i,p.j-1)) { a = Coord(p.i,p.j-1); return 1; }
   if (flags->faraway(p.i,p.j+1)) { a = Coord(p.i,p.j+1); return 1; }
   return 0;
}


FIELD<OrigSet>* evolve(FLAGS* f,float tol)
{
   init(f); Coord nbs[20]; OrigSet o;

   if (tol == -1) // Default value of tol is sqrt(2)
	   tol = (float) M_SQRT2; //Diego: sqrt(2.0); 

   while(q.size())					//evolve over all points:
   {
      std::multimap<float,Coord>::iterator it=q.begin();
      Coord p = (*it).second, a;			//1. get p = the known-pt closest to boundary

      if (!find_free_nb(p,a)) 				//get a = an unknown-nb of p
      { q.erase(it); continue; }			//if p has only known-nbs, we're done with it
	
      int N = get_known_nbs(a,nbs);			//get nbs = all known-nbs of a

      o.erase(o.begin(),o.end());			//2. o gathers the possible OrigSet for the new point a
      for(int i=0;i<N;i++)	        		//build o from all known-nbs:
      {							//gather all distinct Origins of all nbs, 
         Coord&    n = nbs[i];				//in ascending distance order:
         OrigSet& on = fo->value(n.i,n.j);

         for(std::multimap<float,Coord>::iterator it=on.begin();it!=on.end();it++)
         {						//for all Origins oi of neighbor n:
            Coord& oi = (*it).second;
            Coord   ci = Coord(oi);
            std::multimap<float,Coord>::iterator jt=o.begin();
            for(;jt!=o.end();jt++)			//see if we've already gathered this Coord
               if (Coord((*jt).second) == ci) break;
            if (jt==o.end())				//new Coord oi found: insert it in o in order of
	    {						//its distance to point a
	       float d = sqrt(float((oi.i-a.i)*(oi.i-a.i)+(oi.j-a.j)*(oi.j-a.j)));
               std::multimap<float,Coord>::value_type v(d,oi);
	       o.insert(v);
            }
         }
      }
	   
      //Now o contains all distinct (possible) origins of a. 
      //Keep those having the minimal distance, within given tolerance, and assign them to a

     std::multimap<float,Coord>::iterator jt = o.begin();
     float  dmin = (*jt).first;			//dmin = distance to closest origin of a

     OrigSet& oa = fo->value(a.i,a.j);
     for(;jt!=o.end();jt++)			//keep those origins of dmin-distance:
     {	
       float  d = (*jt).first;
       if (d-dmin>=tol) continue;
       std::multimap<float,Coord>::value_type v(d,(*jt).second);	
       oa.insert(v);	
     }

     flags->value(a.i,a.j) = FLAGS::NARROW_BAND;
     std::multimap<float,Coord>::value_type v(dmin,a);
     q.insert(v);
   }

   return fo;					//return origins+dists for all points
}




void computeOrigsAndDT(FIELD<int>*& n, FIELD<float>*& d)
{						//postprocessing: computes #origins and DT for all points
   int i,j;					//(DT = for all points, dist-to-closest-origin)
   n = new FIELD<int>(fo->dimX(),fo->dimY());
   d = new FIELD<float>(fo->dimX(),fo->dimY());

   for(i=0;i<n->dimX();i++)
     for(j=0;j<n->dimY();j++)
     {	
	n->value(i,j) = fo->value(i,j).size();
        d->value(i,j) = (*fo->value(i,j).begin()).first;
     }
}



 		    
	
   																























