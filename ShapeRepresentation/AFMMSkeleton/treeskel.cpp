#include <vector>
#include "treeskel.h"
#include "dqueue.h"
#include "genrl.h"
#include "Tools/BasicUtils.h"
#include <Tools/MathUtils.h>

using namespace std;

unsigned char VISITED = 1;
unsigned char BLOCKED = 10;
unsigned char DEFAULT = 0;

const int g_nXCoord = 129;
const int g_nYCoord = 299;
const bool g_bDbgPrint = false;

#ifdef _DEBUG
#define IF_DBG_MSG2(C, M, A, B) if (C) DBG_MSG3(M,A,B)
#else
#define IF_DBG_MSG2(C, M, A, B) C
#endif

bool TreeSkel::AreTherePhantomBranches()
{
	for(int i = 0; i < branches.Count(); i++)
	{
		if(branches[i]->geomLength() == 0)
			return true;
	}
	return false;
}


bool TreeSkel::IsThereABranchWithTwoTheSameEndpts()
{
	for(int i = 0; i < branches.Count(); i++)
	{
		if(branches[i]->end1->x == branches[i]->end2->x
		  && branches[i]->end1->y == branches[i]->end2->y)
			return true;
	}
	return false;
}

TreeSkel::~TreeSkel()
{

  for(map<Coord,TreePoint*>::iterator i=skel.begin();i!=skel.end();i++)
     delete (*i).second;					//Delete all skel-points we allocated
     
  for(int k = 0, nCount = MemoryLeakRemoval.Count(); k < nCount; k++)
  	delete MemoryLeakRemoval[k];
     
     for(int j = 0; j < branches.Count(); j++)
     	delete branches[j];
}


void TreeSkel::getNbs(FIELD<int>* s,int i,int j,Coord* nbs,int& n,Coord* unbs,int& un)	//get skel-nbs of a skel-pt
{
   n=0; un=0;
   for(int ii=i-1;ii<=i+1;ii++)
      for(int jj=j-1;jj<=j+1;jj++)
	if ((i!=ii || j!=jj) && s->value(ii,jj))
	{   
	    nbs[n] = Coord(ii,jj); n++; 
	    if (unknown(ii,jj))
	    { unbs[un] = Coord(ii,jj); un++; }
	}
}


void TreeSkel::getNbs(FIELD<int>* s,TreePoint* p,Coord* nbs,int& n)	//get skel-nbs of a skel-pt
{
   n=0; int i = p->i, j = p->j;

   for(int ii=i-1;ii<=i+1;ii++)
      for(int jj=j-1;jj<=j+1;jj++)
	if ((i!=ii || j!=jj) && s->value(ii,jj))
	{   nbs[n] = Coord(ii,jj); n++;  }
}




TreeSkel::TreePoint* TreeSkel::getPoint(int i,int j)
{
   map<Coord,TreePoint*>::iterator it = skel.find(Coord(i,j));
   return (it!=skel.end())? (*it).second : 0;
}



TreeSkel::TreePoint* TreeSkel::setPoint(int i,int j,POINT_TYPE t)
{						//Set type of point at (i,j). Create point if not there.
   Coord c(i,j); TreePoint *p;//,*q;
   map<Coord,TreePoint*>::iterator it = skel.find(c);
   if (it==skel.end()) 
   {
      p = new TreePoint(c,t);
      skel.insert(map<Coord,TreePoint*>::value_type(c,p));
/*!!!      for(int ii=i-1;ii<=i+1;ii++)
        for(int jj=j-1;jj<=j+1;jj++)
	   if ((i!=ii || j!=jj) && point_types.value(ii,jj)!=BACKGROUND)
	     if ((q = getPoint(ii,jj)))
	     {
			   
	      nbs[n] = Coord(ii,jj); n++; 
	      if (unknown(ii,jj))
	      { unbs[un] = Coord(ii,jj); un++; }
	     }
*/
   }
   else 
      (p = (*it).second)->type = t;

   point_types.value(i,j) = t;
   return p;
}


/*
	This function tells how many neighbors the point i,j has
*/
DARRAY<Coord> hasNbrNbs(FIELD<int>* s, int i, int j)
{
	DARRAY<Coord> nbs;
	Coord pair;
	if(s->value(i,j) == 0)
		return nbs;
	
	for(int ii=i-1; ii <= i+1; ii++)
		for(int jj = j-1; jj <= j+1; jj++)
			//for points that are not the point itself, and are skeleton points
			if(!(i==ii && j==jj) && s->value(ii,jj)>0) 
			{
				pair.i = ii;
				pair.j = jj;
				nbs.Add(pair);
			}
	
	return nbs;
}


bool TreeSkel::FixBranchOf1Point(FIELD<int>* s, int i, int j)
{
	/*
	For example:
	
	| | | |O| | |
	 - - - - - -
	| | | |O| | |
	 - - - - - -
	| |O|X| | | |
	 - - - - - -
	| | | |O|O|O|
	
	The branch consisting out of 1
	point causes trouble in the creating
	the graph structure and since a branch of
	one point isn't significant anyway,
	we will remove it.
	*/
	DARRAY<Coord> nbs = hasNbrNbs(s,i,j);
	double d1,d2,d3;
	if(nbs.Count() == 3)
	{
		d1 = fabs((double)nbs[0].i-nbs[1].i)+fabs((double)nbs[0].j-nbs[1].j);
		d2 = fabs((double)nbs[1].i-nbs[2].i)+fabs((double)nbs[1].j-nbs[2].j);
		d3 = fabs((double)nbs[2].i-nbs[0].i)+fabs((double)nbs[2].j-nbs[0].j);
		//if the 3 neighbors are in the right configuration
		if (d1>1 && d2>1 && d3>1)
		{
			DARRAY<Coord> nbs0 = hasNbrNbs(s,nbs[0].i,nbs[0].j);
			DARRAY<Coord> nbs1 = hasNbrNbs(s,nbs[1].i,nbs[1].j);
			DARRAY<Coord> nbs2 = hasNbrNbs(s,nbs[2].i,nbs[2].j);
			if(nbs0.Count() == 1 && nbs1.Count() >= 1 && nbs2.Count() >= 1)
			{
				s->value(nbs[0].i,nbs[0].j) = 0;
				return true;
			}
			if(nbs0.Count() >= 1 && nbs1.Count() == 1 && nbs2.Count() >= 1)
			{
				s->value(nbs[1].i,nbs[1].j) = 0;
				return true;
			}
			if(nbs0.Count() >= 1 && nbs1.Count() >= 1 && nbs2.Count() == 1)
			{
				s->value(nbs[2].i,nbs[2].j) = 0;
				return true;
			}
		}
	}
	
	return false;
}

bool SpecialCaseHorse15(FIELD<int>* s, int i, int j)
{
	/*
	| | |O| |O| |	| | |O| |O| |
	 - - - - - -	 - - - - - -
	| | | |O| | |	| | | |O| | |
	 - - - - - -	 - - - - - -
	| |O|F| | | |	| |O|F| |O| |
	 - - - - - -	 - - - - - -
	| | | |O| | |	| | | | |O| |
	 - - - - - -	 - - - - - -
	| | | |?|?| |	| | | | |O| |

	F = origin (i,j)
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i,j+2) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i+2,j+2) > 0 &&
	   //s->value(i+2,j-2) > 0 &&
	   
	   s->value(i,j+1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i,j-2) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j+2) == 0 &&
	   //s->value(i+1,j-2) == 0 &&
	   s->value(i+2,j) == 0 &&
	   s->value(i+2,j-1) == 0 &&
	   s->value(i+2,j+1) == 0 &&
	   s->value(i+3,j) == 0 &&
	   s->value(i+3,j+1) == 0 &&
	   s->value(i+3,j-1) == 0)
	{
		s->value(i+2,j) = s->value(i,j);
		s->value(i+2,j-1) = s->value(i,j);
		s->value(i+1,j-1) = 0;
		return true;
	}
	return false;
}

bool SpecialLadyBug63(FIELD<int>* s, int i, int j)
{
	/*
	| | | |O| | | |
	 - - - - - - -
	| | | |O| | |O|
	 - - - - - - -
	| | |O| |O|O| |
	 - - - - - - -
	| |O| |F| | | |
	 - - - - - - -
	| |O| |O| | | |
	
	These are supposed to be 2
	separate branches
	
	F = origin (i,j)
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i,j+2) > 0 &&
	   s->value(i,j+3) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i-2,j) > 0 &&
	   s->value(i-2,j-1) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+2,j+1) > 0 &&
	   s->value(i+3,j+2) > 0 &&
	   
	   s->value(i,j+1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j+2) == 0 &&
	   s->value(i-1,j+3) == 0 &&
	   s->value(i-2,j+1) == 0 &&
	   s->value(i-3,j) == 0 &&
	   s->value(i-3,j-1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i+1,j+2) == 0 &&
	   s->value(i+1,j+3) == 0 &&
	   s->value(i+2,j) == 0 &&
	   s->value(i+2,j+2) == 0)
	{
		s->value(i+1,j) = s->value(i,j);
		s->value(i,j) = 0;
		s->value(i+1,j+1) = 0;
		return true;
	}
	return false;
}



bool NearSquare(FIELD<int>* s, int i, int j)
{
	/*
	D = don't care
	|O|O|D|
	 - - - 
	|O| |O|
	 - - -
	|D|O|O|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		/*
		| | |?|?|
		 - - - -  
		|O|M|M|?|
		 - - - -
		|O| |M| |
		 - - - -
		|D|O|O| |
		*/
		if(s->value(i+1,j+2) == 0 &&
		   s->value(i+2,j+2) == 0 &&
		   s->value(i+2,j+1) == 0)
		{
			s->value(i,j+1) = 0;
			s->value(i+1,j+1) = 0;
			s->value(i+1,j) = 0;
			return true;
		}
		
		/*
		| |O|O|D|
		 - - - -  
		| |M| |O|
		 - - - -
		|?|M|M|O|
		 - - - -
		|?|?| | |
		*/
		if(s->value(i-1,j-2) == 0 &&
		   s->value(i-2,j-2) == 0 &&
		   s->value(i-2,j-1) == 0)
		{
			s->value(i,j-1) = 0;
			s->value(i-1,j-1) = 0;
			s->value(i-1,j) = 0;
			return true;
		}
	}

	/*
	D = don't care
	|D|O|O|
	 - - - 
	|O| |O|
	 - - -
	|O|O|D|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		/*
		|?|?| | |
		 - - - -  
		|?|M|M|O|
		 - - - -
		| |M| |O|
		 - - - -
		| |O|O|D|
		*/
		if(s->value(i-1,j+2) == 0 &&
		   s->value(i-2,j+2) == 0 &&
		   s->value(i-2,j+1) == 0)
		{
			s->value(i,j+1) = 0;
			s->value(i-1,j+1) = 0;
			s->value(i-1,j) = 0;
			return true;
		}
		
		/*
		|D|O|O| |
		 - - - -  
		|O| |M| |
		 - - - -
		|O|M|M|?|
		 - - - -
		| | |?|?|
		*/
		if(s->value(i+1,j-2) == 0 &&
		   s->value(i+2,j-2) == 0 &&
		   s->value(i+2,j-1) == 0)
		{
			s->value(i,j-1) = 0;
			s->value(i+1,j-1) = 0;
			s->value(i+1,j) = 0;
			return true;
		}
	}
	return false;
}


bool has4NbsInType5Config(FIELD<int>* s, int i, int j)
{
//################# MASK ONE, 4 ROTATIONS ##############################
	/*
	| |O| |		|?| |O| |
	 - - -		 - - - -
	|O|X|O|		|M| |X|O|
	 - - - 	  -->    - - - -
	|O| | |		| |O| | |
	*/
	
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j) > 0 
	   && s->value(i,j+1) > 0 && s->value(i+1,j) > 0)
	{
		if(s->value(i-2,j+1) > 0)
			s->value(i-2,j) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		return true;
	}

	/*
			| |M|?|
			 - - - 
	|O|O| |		|O| | |
	 - - -		 - - - 
	| |X|O|		| |X|O|
	 - - - 	  -->    - - - 
	| |O| |		| |O| |
	*/
	if(s->value(i-1,j+1) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i+1,j+2) > 0)
			s->value(i,j+2) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		return true;
	}

	/*
	| | |O|		| | |O| |
	 - - -		 - - - -
	|O|X|O|		|O|X| |M|
	 - - - 	  -->    - - - -
	| |O| |		| |O| |?|
	*/
	
	if(s->value(i-1,j) > 0 && s->value(i+1,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i+2,j-1) > 0)
			s->value(i+2,j) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		return true;
	}
	
	/*
	| |O| |		| |O| |
	 - - -		 - - - 
	|O|X| |		|O|X| |
	 - - - 	  -->    - - - 
	| |O|O|		| | |O|
			 - - - 
			|?|M| |

	*/
	if(s->value(i-1,j) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j-1) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i-1,j-2) > 0)
			s->value(i,j-2) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		return true;
	}
//################# MASK TWO, 4 ROTATIONS ##############################
	/*
	| |O| |		| |O| |?|
	 - - -		 - - - -
	|O|X|O|		|O|X| |M|
	 - - - 	  -->    - - - - 
	| | |O|		| | |O| |
	*/
	
	if(s->value(i+1,j-1) > 0 && s->value(i-1,j) > 0 
	   && s->value(i,j+1) > 0 && s->value(i+1,j) > 0)
	{
		if(s->value(i+2,j+1) > 0)
			s->value(i+2,j) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		return true;
	}

	/*
	| |O| |		| |O| |
	 - - -		 - - - 
	| |X|O|		| |X|O|
	 - - - 	  -->    - - - 
	|O|O| |		|O| | |
			 - - - 
			| |M|?|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i+1,j-2) > 0)
			s->value(i,j-2) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		return true;
	}

	/*
	|O| | |		| |O| | |
	 - - -		 - - - -
	|O|X|O|		|M| |X|O|
	 - - - 	  -->    - - - -
	| |O| |		|?| |O| |
	*/
	
	if(s->value(i-1,j) > 0 && s->value(i-1,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i-2,j-1) > 0)
			s->value(i-2,j) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		return true;
	}
	
	/*
			|?|M| |
			 - - - 
	| |O|O|		| | |O|
	 - - -		 - - - 
	|O|X| |		|O|X| |
	 - - - 	  -->    - - - 
	| |O| |		| | |O|
	*/
	if(s->value(i-1,j) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i-1,j+2) > 0)
			s->value(i,j+2) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		return true;
	}
	
	return false;
}

/*
	M = maybe, depending on the ? being > 0 or not
*/
bool has4NbsInType4Config(FIELD<int>* s, int i, int j)
{
//################# MASK ONE, 4 ROTATIONS ##############################
	/*
	|O| |O|		|?|M| |O|
	 - - -		 - - - -
	| |X|O|		| |O|X|O|
	 - - - 	  -->    - - - -
	| |O| |		| |O| | |
			 - - - -
			| | |M|?|
	*/
	
	if(s->value(i-1,j+1) > 0 && s->value(i,j-1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i+1,j+1) > 0)
	{
		s->value(i-1,j) = s->value(i,j);
		s->value(i-1,j-1) = s->value(i,j-1);
		if(s->value(i+1,j-2) > 0)
			s->value(i,j-2) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		if(s->value(i-2,j+1) > 0)
			s->value(i-1,j+1) = 0;
		return true;
	}

	/*
	|O|O| |		|O|O| |?|
	 - - -		 - - - - 
	| |X|O|		| |X| |M|
	 - - - 	  -->    - - - - 
	|O| | |		|M|O|O| |
			 - - - -
			|?| | | |
	*/
	if(s->value(i-1,j+1) > 0 && s->value(i-1,j-1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i,j+1) > 0)
	{
		s->value(i,j-1) = s->value(i,j);
		s->value(i+1,j-1) = s->value(i+1,j);
		if(s->value(i+2,j+1) > 0)
			s->value(i+2,j) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		if(s->value(i-1,j-2) > 0)
			s->value(i-1,j-1) = 0;
		return true;
	}
	
	/*
			|?|M| | |
			 - - - - 
	| |O| |		| | |O| | 
	 - - -		 - - - - 
	|O|X| |		|O|X|O| | 
	 - - - 	  -->    - - - - 
	|O| |O|		|O| |M|?|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j) > 0 
	   && s->value(i,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		s->value(i+1,j) = s->value(i,j);
		s->value(i+1,j+1) = s->value(i,j+1);
		if(s->value(i-1,j+2) > 0)
			s->value(i,j+2) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		if(s->value(i+2,j-1) > 0)
			s->value(i+1,j-1) = 0;
		return true;
	}
	
	/*
			| | | |?|
			 - - - - 
	| | |O|		| |O|O|M|
	 - - -		 - - - - 
	|O|X| |		|M| |X| |
	 - - - 	  -->    - - - - 
	| |O|O|		|?| |O|O|
	*/
	if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		s->value(i,j+1) = s->value(i,j);
		s->value(i-1,j+1) = s->value(i-1,j);
		if(s->value(i-2,j-1) > 0)
			s->value(i-2,j) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		if(s->value(i+1,j+2) > 0)
			s->value(i+1,j+1) = 0;
		return true;
	}
//################# MASK TWO, 4 ROTATIONS ##############################
	/*
	|O| |O|		|O| |M|?|
	 - - -		 - - - - 
	|O|X| |		|O|X|O| |
	 - - - 	  -->    - - - - 
	| |O| |		| | |O| |
			 - - - - 
			|?|M| | |
	*/
	if(s->value(i-1,j+1) > 0 && s->value(i-1,j) > 0 
	   && s->value(i,j-1) > 0 && s->value(i+1,j+1) > 0)
	{
		s->value(i+1,j) = s->value(i,j);
		s->value(i+1,j-1) = s->value(i,j-1);
		if(s->value(i-1,j-2) > 0)
			s->value(i,j-2) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		if(s->value(i+2,j+1) > 0)
			s->value(i+1,j+1) = 0;
		return true;
	}

	/*
			|?| | | |
			 - - - - 
	|O| | |		|M|O|O| |
	 - - -		 - - - - 
	| |X|O|		| |X| |M|
	 - - - 	  -->    - - - - 
	|O|O| |		|O|O| |?|
	*/
	if(s->value(i-1,j+1) > 0 && s->value(i-1,j-1) > 0 
	   && s->value(i,j-1) > 0 && s->value(i+1,j) > 0)
	{
		s->value(i,j+1) = s->value(i,j);
		s->value(i+1,j+1) = s->value(i+1,j);
		if(s->value(i+2,j-1) > 0)
			s->value(i+2,j) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		if(s->value(i-1,j+2) > 0)
			s->value(i-1,j+1) = 0;
		return true;
	}
	
	/*
			| | |M|?|
			 - - - -
	| |O| |		| |O| | |
	 - - -		 - - - -
	| |X|O|		| |O|X|O|
	 - - - 	  -->    - - - -
	|O| |O|		|?|M| |O|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i+1,j-1) > 0)
	{
		s->value(i-1,j) = s->value(i,j);
		s->value(i-1,j+1) = s->value(i,j+1);
		if(s->value(i+1,j+2) > 0)
			s->value(i,j+2) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		if(s->value(i-2,j-1) > 0)
			s->value(i-1,j-1) = 0;
		return true;
	}
	
	/*
	| |O|O|		|?| |O|O|
	 - - -		 - - - - 
	|O|X| |		|M| |X| |
	 - - - 	  -->    - - - - 
	| | |O|		| |O|O|M|
			 - - - -
			| | | |?|
	*/
	if(s->value(i-1,j) > 0 && s->value(i,j+1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		s->value(i,j-1) = s->value(i,j);
		s->value(i-1,j-1) = s->value(i-1,j);
		if(s->value(i-2,j+1) > 0)
			s->value(i-2,j) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		if(s->value(i+1,j-2) > 0)
			s->value(i+1,j-1) = 0;
		return true;
	}
	return false;
}



/*
	Checks wheter we have something like :
			|X| |X|		|X| | |
			 - - -   OR   	 - - -
			| |X| |		| |X|X|
			 - - - 		 - - -
			| |X|X|		|X| |X|
		
	and ifso, fixes the problem
*/
bool has4NbsInType3Config(FIELD<int>* s, int i, int j)
{
//################# MASK ONE, 4 ROTATIONS ##############################
	/*
			|?| | |
			 - - -
	| |O|O|		|M| |O|
	 - - -		 - - -
	| |X| |		| |X| |
	 - - - 	  -->    - - -
	|O| |O|		|O| |O|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i+1,j-1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i,j+1) > 0)
	{
		if(s->value(i-1,j+2) > 0)
			s->value(i-1,j+1) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		return true;
	}

	/*
	|O| |O|		| |O| |O|
	 - - -		 - - - -
	|O|X| |		| | |X| |
	 - - - 	  -->    - - - -
	| | |O|		|?|M| |O|
	*/
	if(s->value(i-1,j) > 0 && s->value(i-1,j+1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		if(s->value(i-2,j-1) > 0)
			s->value(i-1,j-1) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		return true;
	}

	/*
	|O| |O|		|O| |O|
	 - - -		 - - -
	| |X| |		| |X| |
	 - - - 	  -->    - - -
	|O|O| |		|O| |M|
			 - - -
			| | |?|
	*/
	if(s->value(i,j-1) > 0 && s->value(i-1,j-1) > 0 
	   && s->value(i-1,j+1) > 0 && s->value(i+1,j+1) > 0)
	{
		if(s->value(i+1,j-2) > 0)
			s->value(i+1,j-1) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		return true;
	}
	
	/*
	|O| | |		|O| |M|?|
	 - - -		 - - - -
	| |X|O|		| |X| | |
	 - - - 	  -->    - - - -
	|O| |O|		|O| |O| |
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0 
	   && s->value(i+1,j) > 0 && s->value(i+1,j-1) > 0)
	{
		if(s->value(i+2,j+1) > 0)
			s->value(i+1,j+1) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		return true;
	}
//################# MASK TWO, 4 ROTATIONS ##############################
	/*
	| | |O|		|?|M| |O|
	 - - -		 - - - -
	|O|X| |		| | |X| |
	 - - - 	  -->    - - - -
	|O| |O|		| |O| |O|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		if(s->value(i-2,j+1) > 0)
			s->value(i-1,j+1) = s->value(i-1,j);
		s->value(i-1,j) = 0;
		return true;
	}

	/*
	|O| |O|		|O| |O|
	 - - -		 - - -
	| |X| |		| |X| |
	 - - - 	  -->    - - -
	| |O|O|		|M| |O|
			 - - -
			|?| | |
	*/
	if(s->value(i-1,j+1) > 0 && s->value(i+1,j+1) > 0 
	   && s->value(i+1,j-1) > 0 && s->value(i,j-1) > 0)
	{
		if(s->value(i-1,j-2) > 0)
			s->value(i-1,j-1) = s->value(i,j-1);
		s->value(i,j-1) = 0;
		return true;
	}

	/*
	|O| |O|		|O| |O| |
	 - - -		 - - - -
	| |X|O|		| |X| | |
	 - - - 	  -->    - - - -
	|O| | |		|O| |M|?|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0 
	   && s->value(i+1,j+1) > 0 && s->value(i+1,j) > 0)
	{
		if(s->value(i+2,j-1) > 0)
			s->value(i+1,j-1) = s->value(i+1,j);
		s->value(i+1,j) = 0;
		return true;
	}
	
	/*
			| | |?|
			 - - -
	|O|O| |		|O| |M|
	 - - -		 - - -
	| |X| |		| |X| |
	 - - - 	  -->    - - -
	|O| |O|		|O| |O|
	*/
	if(s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0 
	   && s->value(i,j+1) > 0 && s->value(i+1,j-1) > 0)
	{
		if(s->value(i+1,j+2) > 0)
			s->value(i+1,j+1) = s->value(i,j+1);
		s->value(i,j+1) = 0;
		return true;
	}
	
	return false;
}


/*
	Check whether point (i,j) has exactly 3 neighbors with the added
	condition that the 3 neighbors form a triangle for which all sides
	have lengths greater then 1
*/
bool TreeSkel::hasExact3NbsInRightTriangle(FIELD<int>* s, int i, int j)
{
	bool bSpecialCase = false;
	DARRAY<Coord> nbs = hasNbrNbs(s,i,j);
	double d1,d2,d3;
	
	if(nbs.Count() == 3)
	{
		d1 = fabs((double)nbs[0].i-nbs[1].i)+fabs((double)nbs[0].j-nbs[1].j);
		d2 = fabs((double)nbs[1].i-nbs[2].i)+fabs((double)nbs[1].j-nbs[2].j);
		d3 = fabs((double)nbs[2].i-nbs[0].i)+fabs((double)nbs[2].j-nbs[0].j);
		if (d1>1 && d2>1 && d3>1)
			bSpecialCase = true;
	}
	
	return bSpecialCase;
}



/*
	Check whether we have a 2x2 grid with with a diagonal
	of type 1
*/
bool TreeSkel::diagonalType1(FIELD<int>* s, int i, int j)
{
	bool bDiagonal = false;
	if(s->value(i,j)>0 && s->value(i+1,j+1)>0 &&
		s->value(i+1,j)==0 && s->value(i,j+1)==0)
		bDiagonal = true;
	return bDiagonal;
}

/*
	Check whether we have a 2x2 grid with with a diagonal
	of type 2
*/
bool TreeSkel::diagonalType2(FIELD<int>* s, int i, int j)
{
	bool bDiagonal = false;
	if(s->value(i+1,j)>0 && s->value(i,j+1)>0 &&
		s->value(i+1,j+1)==0 && s->value(i,j)==0)
		bDiagonal = true;
	return bDiagonal;
}


bool FixCleanDiamond(FIELD<int>* s, int i, int j)
{
	/*
	|?|O|?|
	 - - - 
	|O| |O|
	 - - -
	|?|O|?|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i,j+1) > 0)
	{
		/*
		|?|?|?|
		 - - -
		|?|M|?|
		 - - - 
		|O| |O|
		 - - -
		| |O| |
		*/
		if(s->value(i-1,j+2) == 0 &&
		   s->value(i,j+2) == 0 &&
		   s->value(i-1,j+1) == 0 &&
		   s->value(i+1,j+1) == 0 &&
		   s->value(i+1,j+2) == 0)
		{
			s->value(i,j+1) = 0;
			return true;
		}
		
		/*
		| |O|?|?|
		 - - - -
		|O| |M|?|
		 - - - -
		| |O|?|?|
		*/
		if(s->value(i+2,j+1) == 0 &&
		   s->value(i+2,j) == 0 &&
		   s->value(i+1,j+1) == 0 &&
		   s->value(i+1,j-1) == 0 &&
		   s->value(i+2,j-1) == 0)
		{
			s->value(i+1,j) = 0;
			return true;
		}
		
		/*
		| |O| |
		 - - - 
		|O| |O|
		 - - -
		|?|M|?|
		 - - -
		|?|?|?|
		*/
		if(s->value(i-1,j-2) == 0 &&
		   s->value(i,j-2) == 0 &&
		   s->value(i+1,j-1) == 0 &&
		   s->value(i-1,j-1) == 0 &&
		   s->value(i+1,j-2) == 0)
		{
			s->value(i,j-1) = 0;
			return true;
		}
		
		/*
		|?|?|O| |
		 - - - - 
		|?|M| |O|
		 - - - - 
		|?|?|O| |
		*/
		if(s->value(i-2,j+1) == 0 &&
		   s->value(i-2,j) == 0 &&
		   s->value(i-1,j-1) == 0 &&
		   s->value(i-1,j+1) == 0 &&
		   s->value(i-2,j-1) == 0)
		{
			s->value(i-1,j) = 0;
			return true;
		}
	}
	
	return false;
}

bool FixAlmostH(FIELD<int>* s, int i, int j)
{
	/*
	M = put point there if R or ?
	R = remove point if it exists
	
			| | | |M| |
			 - - - - -
	|O| |O|		|O| |O|R|M|
	 - - -		 - - - - - 
	|O|X| |		|O|X|O| |?|
	 - - - 	  -->    - - - - -
	|O| |O|		|O| |O|R|M| 
			 - - - - - 
			| | | |M| |
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i+1,j) = s->value(i,j);
		if(s->value(i+2,j+1) > 0)
		{
			s->value(i+2,j+2) = s->value(i+2,j+1);
			s->value(i+2,j+1) = 0;
			if(s->value(i+3,j) > 0)
				s->value(i+3,j+1) = s->value(i+2,j+2);
		}
		if(s->value(i+2,j-1) > 0)
		{
			s->value(i+2,j-2) = s->value(i+2,j-1);
			s->value(i+2,j-1) = 0;
			if(s->value(i+3,j) > 0)
				s->value(i+3,j-1) = s->value(i+2,j-2);
		}
		return true;
	}

	/*
	|O|O|O|
	 - - -
	| |X| |
	 - - - 
	|O| |O|
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j-1) = s->value(i,j);
		if(s->value(i-1,j-2) > 0)
		{
			s->value(i-2,j-2) = s->value(i-1,j-2);
			s->value(i-1,j-2) = 0;
			if(s->value(i,j-3) > 0)
				s->value(i-1,j-3) = s->value(i-2,j-2);
		}
		if(s->value(i+1,j-2) > 0)
		{
			s->value(i+2,j-2) = s->value(i+1,j-2);
			s->value(i+1,j-2) = 0;
			if(s->value(i,j-3) > 0)
				s->value(i+1,j-3) = s->value(i+2,j-2);
		}
		return true;
	}

	/*
	|O| |O|
	 - - -
	| |X|O|
	 - - - 
	|O| |O|
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i-1,j) = s->value(i,j);
		if(s->value(i-2,j+1) > 0)
		{
			s->value(i-2,j+2) = s->value(i-2,j+1);
			s->value(i-2,j+1) = 0;
			if(s->value(i-3,j) > 0)
				s->value(i-3,j+1) = s->value(i-2,j+2);
		}
		if(s->value(i-2,j-1) > 0)
		{
			s->value(i-2,j-2) = s->value(i-2,j-1);
			s->value(i-2,j-1) = 0;
			if(s->value(i-3,j) > 0)
				s->value(i-3,j-1) = s->value(i-2,j-2);
		}
		return true;
	}
	
	/*
	|O| |O|
	 - - -
	| |X| |
	 - - - 
	|O|O|O|
	*/
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j+1) = s->value(i,j);
		if(s->value(i+1,j+2) > 0)
		{
			s->value(i+2,j+2) = s->value(i+1,j+2);
			s->value(i+1,j+2) = 0;
			if(s->value(i,j+3) > 0)
				s->value(i+1,j+3) = s->value(i+2,j+2);
		}
		if(s->value(i-1,j+2) > 0)
		{
			s->value(i-2,j+2) = s->value(i-1,j+2);
			s->value(i-1,j+2) = 0;
			if(s->value(i,j+3) > 0)
				s->value(i-1,j+3) = s->value(i-2,j+2);
		}
		return true;
	}
	
	return false;
}

bool TreeSkel::FixDiamond(FIELD<int>* s, int i, int j)
{
	/*
	| |O|O|		| |O|O|
	 - - -		 - - - 
	|O| |O|		|O| | |
	 - - - 	  -->    - - - 
	| |O|O|		| |O|O|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i+1,j) = 0;
		return true;
	}
	
	/*
	| |O| |		| |O| |
	 - - -		 - - - 
	|O| |O|		|O| |O|
	 - - - 	  -->    - - - 
	|O|O|O|		|O| |O|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j-1) = 0;
		return true;
	}
	
	/*
	|O|O| |		|O|O| |
	 - - -		 - - - 
	|O| |O|		| | |O|
	 - - - 	  -->    - - - 
	|O|O| |		|O|O| |
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i-1,j) = 0;
		return true;
	}
	
	/*
	|O|O|O|		|O| |O|
	 - - -		 - - - 
	|O| |O|		|O| |O|
	 - - - 	  -->    - - - 
	| |O| |		| |O| |
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j+1) = 0;
		return true;
	}
	
	/*
	| |O| |		| |O| |
	 - - -		 - - - 
	|O| |O|		|O|O|O|
	 - - - 	  -->    - - - 
	| |O| |		| |O| |
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j) = s->value(i,j-1);
		DBG_MSG3("\nCLEAN DIAMOND FIXED TO CROSS at",i,j);
		return true;
	}
	
	
	/*
	| |O| |		|?| |O| |
	 - - -		 - - - - 
	|O| |O|		| |M| |O|
	 - - - 	  -->    - - - - 
	|O|O| |		| |O|M| |
			 - - - -
			| | | |?|
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		if(s->value(i-2,j+1) == 0)
		{
			s->value(i-1,j) = 0;
			return true;
		}
		else if(s->value(i+1,j-2) == 0)
		{
			s->value(i,j-1) = 0;
			return true;
		}
// 		else
// 		{
// 			printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
// 			char getchar;
//     			cin >> getchar;
// 		}
	}
	
	/*
	|O|O| |
	 - - -
	|O| |O|
	 - - - 
	| |O| |
	
	FIX AS ABOVE
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		if(s->value(i-2,j-1) == 0)
		{
			s->value(i-1,j) = 0;
			return true;
		}
		else if(s->value(i+1,j+2) == 0)
		{
			s->value(i,j+1) = 0;
			return true;
		}
// 		else
// 		{
// 			printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
// 			char getchar;
//     			cin >> getchar;
// 		}
	}
	
	/*
	| |O|O|
	 - - -
	|O| |O|
	 - - - 
	| |O| |
	
	FIX AS ABOVE
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		if(s->value(i-1,j+2) == 0)
		{
			s->value(i,j+1) = 0;
			return true;
		}
		else if(s->value(i+2,j-1) == 0)
		{
			s->value(i+1,j) = 0;
			return true;
		}
// 		else
// 		{
// 			printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
// 			char getchar;
//     			cin >> getchar;
// 		}
	}
	
	/*
	| |O| |
	 - - -
	|O| |O|
	 - - - 
	| |O|O|
	
	FIX AS ABOVE
	*/
	if(s->value(i,j) == 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		if(s->value(i+2,j+1) == 0)
		{
			s->value(i+1,j) = 0;
			return true;
		}
		else if(s->value(i-1,j-2) == 0)
		{
			s->value(i,j-1) = 0;
			return true;
		}
// 		else
// 		{
// 			printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
// 			char getchar;
//     			cin >> getchar;
// 		}
	}
	
	return false;
}



bool TreeSkel::FixCube(FIELD<int>* s, int i, int j)
{
	bool bFirst = false;
	bool bSecond = false;
	//if we have a cube
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		printNbh(s,i,j,"Before",5);
		
		if(s->value(i-1,j-1) == 0 || s->value(i+2,j+2) == 0)
		{
			if (s->value(i-1,j-1) == 0) 
			{
				bFirst = true;
				s->value(i,j) = 0;
			}
			if (s->value(i+2,j+2) == 0)
			{
				bSecond = true;
				s->value(i+1,j+1) = 0;
			}
			printNbh(s,i,j,"After",5);
			if(!bFirst && bSecond)
				FixPseudoCross(s, i, j);
			if(bFirst && !bSecond)
				FixPseudoCross(s, i+1, j+1);
			return true;
		}
		
		if(s->value(i-1,j+2) == 0 || s->value(i+2,j-1) == 0)
		{
			if (s->value(i-1,j+2) == 0) 
			{
				bFirst = true;
				s->value(i,j+1) = 0;
			}
			if (s->value(i+2,j-1) == 0) 
			{
				bSecond = true;
				s->value(i+1,j) = 0;
			}
			printNbh(s,i,j,"After",5);
			if(!bFirst && bSecond)
				FixPseudoCross(s, i, j+1);
			if(bFirst && !bSecond)
				FixPseudoCross(s, i+1, j);
			return true;
		}
	}
	
	return false;
}


void TreeSkel::printNbh(FIELD<int>* s, int i,int j, const char* msg, int nbhRad /*=3*/)
{
	int ii, jj, k;
	map<Coord,TreePoint*>::iterator it;
	
	if (!g_bDbgPrint)
		return;
	
	cerr << "\n" << msg << endl;
	
	for(jj = j + nbhRad; jj >= j - nbhRad; jj--) 
	{
		for(ii = i - nbhRad; ii <= i + nbhRad; ii++) 
		{	
			if (s != NULL && s->value(ii,jj) > 0)
				cerr << "| X ";
			else if (s == NULL && ((it = skel.find(Coord(ii, jj))) != skel.end( )))
				cerr << "|" << it->second->type << "," <<  it->second->nbs.Count();
			else
				cerr << "|   ";
		}
		cerr << "|\n ";
		for (k = 0; k <= 2 * nbhRad; k++) cerr << "--- ";
		cerr << "\n";
	}
	cerr.flush();
}


bool TreeSkel::FixTConfig(FIELD<int>* s, int i, int j)
{
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	   
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}

	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j) = 0;
		return true;
	}

//###############################################################
//#################### L CONFIGS ################################
//###############################################################
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}

	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) > 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}

	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) > 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) > 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) > 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) > 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) == 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) == 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) > 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	if(s->value(i,j) > 0 &&
	   s->value(i+1,j+1) > 0 &&
	   s->value(i+1,j) == 0 &&
	   s->value(i+1,j-1) == 0 &&
	   s->value(i,j-1) == 0 &&
	   s->value(i-1,j-1) == 0 &&
	   s->value(i-1,j) == 0 &&
	   s->value(i-1,j+1) == 0 &&
	   s->value(i,j+1) > 0)
	{
		s->value(i,j) = 0;
		return true;
	}
	
	return false;
}



/*!
	This is a function that will fix 2 configurations:
	
		TYPE 1: | | | | |	TYPE 2:	| | | | |
			 - - - -		 - - - -
			| | |X| |		| |X| | |
			 - - - -   AND  	 - - - -
			| |X| | |		| | |X| |
			 - - - - 		 - - - -
			| | | | |		| | | | |
	
	Where X are skeleton points having exactly 3 neighbors and
	those 3 neighbors form a triangle with sides having lengths greater
	then 1
	For instance:
			
			| |O| | |	| |O| | |
			 - - - -	 - - - -
			|O|X| |O|	|O|X|O|O|
			 - - - -   -->   - - - -
			| | |X| |	| |O| | |
			 - - - - 	 - - - -
			| |O| | |	| |O| | |
*/
void TreeSkel::FixPseudoCross(FIELD<int>* s, int i, int j)
{
	DARRAY<Coord> nbs = hasNbrNbs(s,i,j); 
	
	//first check whether we have the diagonal of type 1
	if (diagonalType1(s,i,j) == true)
	{
		//and we have the exact right properties for the 2 points in the diagonal
		if(hasExact3NbsInRightTriangle(s,i,j) == true)
		{
			if(hasExact3NbsInRightTriangle(s,i+1,j+1) == true)
			{
				DBG_MSG3("\nSPECIAL CASE TYPE 1 FIXED at",i,j);
				//try to see whether we can make a + configuration
				if((s->value(i-1,j) > 0 && s->value(i,j-1) > 0)&&
					(s->value(i+1,j+2) > 0 && s->value(i+2,j+1) > 0))
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i,j+2) = s->value(i+1,j+2);
					s->value(i+2,j) = s->value(i+2,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					if(s->value(i,j+3) == 0)
						s->value(i+1,j+3) = s->value(i+1,j+2);
					if(s->value(i+3,j) == 0)
						s->value(i+3,j+1) = s->value(i+2,j+1);
					s->value(i+1,j+1) = 0;
					s->value(i+1,j+2) = 0;
					s->value(i+2,j+1) = 0;
				}
				
				/*
					|?|?|?|?|	|?|?|?|?|
					 - - - -	 - - - -
					|?| |X| |	|?| |X| |
					 - - - -   -->   - - - -
					|?|X| |O|	|?|X| |R|
					 - - - - 	 - - - -
					|?| |O|O|	|?| |R|O|
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j-1) > 0 &&
					s->value(i+2,j) > 0)
				{
					if(s->value(i,j-2) == 0)
						s->value(i+1,j-1) = 0;
					else if(s->value(i+3,j+1) == 0)
						s->value(i+2,j) = 0;
					else
					{
						printNbh(s,i,j,"  2  NOT FIXED  2 !!!!",5);
						char getchar;
    						cin >> getchar;
					}
				}
				
				/*
					|O|O| |?|	|O|R| |?|
					 - - - -	 - - - -
					|O| |X|?|	|R| |X|?|
					 - - - -   -->   - - - -
					| |X| |?|	| |X| |?|
					 - - - - 	 - - - -
					|?|?|?|?|	|?|?|?|?|
				
				*/
				else if(s->value(i-1,j+1) > 0 && s->value(i-1,j+2) > 0 &&
					s->value(i,j+2) > 0)
				{
					if(s->value(i+1,j+3) == 0)
						s->value(i,j+2) = 0;
					else if(s->value(i-2,j) == 0)
						s->value(i-1,j+1) = 0;
					else
					{
						printNbh(s,i,j,"  3  NOT FIXED  3 !!!!",5);
						char getchar;
    						cin >> getchar;
					}
				}
				
				
				/*
					| | | |O|	| | |O|O|
					 - - - -	 - - - -
					| | |X| |	| |O| | |
					 - - - -   -->   - - - -
					|O|X| |O|	|O|X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 &&
					s->value(i+2,j) > 0 && s->value(i+2,j+2) > 0)
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+1,j+2) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				/*
					| |O| | |	| |O| | |
					 - - - -	 - - - -
					| | |X|O|	| |O| |O|
					 - - - -   -->   - - - -
					|O|X| | |	|O|X|O| |
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 &&
					s->value(i+2,j+1) > 0 && s->value(i,j+2) > 0)
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					| | |X| |	| |O| | |
					 - - - -   -->   - - - -
					|O|X| |O|	|O|X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 &&
					s->value(i+1,j+2) > 0 && s->value(i+2,j) > 0)
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				/*
					| |O| |O|	| |O| |O|
					 - - - -	 - - - -
					| | |X| |	| |O| |O|
					 - - - -   -->   - - - -
					|O|X| | |	|O|X|O| |
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 &&
					s->value(i,j+2) > 0 && s->value(i+2,j+2) > 0)
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+2,j+1) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				/*
					| |O| | |	| |O| | |
					 - - - -	 - - - -
					| | |X| |	| |O| | |
					 - - - -   -->   - - - -
					|O|X| |O|	|O|X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if(s->value(i-1,j) > 0 && s->value(i,j-1) > 0 &&
					s->value(i,j+2) > 0 && s->value(i+2,j) > 0)
				{
					s->value(i,j+1) = s->value(i+1,j+1);
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
//####################################################################################################
				/*
					| |O| |O|	| |O| |O|
					 - - - -	 - - - -
					| | |X| |	| | |X| |
					 - - - -   -->   - - - -
					| |X| | |	| | |O| |
					 - - - - 	 - - - -
					|O| |O| |	|O|O|O| |
				
				*/
				else if((s->value(i-1,j-1) > 0 && s->value(i+1,j-1) > 0)&&
					(s->value(i,j+2) > 0 && s->value(i+2,j+2) > 0))
				{
					s->value(i,j-1) = s->value(i,j);
					s->value(i+1,j) = s->value(i,j);
					s->value(i,j) = 0;
				}
				
				/*
					| | | |O|	| | | |O|
					 - - - -	 - - - -
					|O| |X| |	|O|O|X| |
					 - - - -   -->   - - - -
					| |X| |O|	|O| | |O|
					 - - - - 	 - - - -
					|O| | | |	|O| | | |
				
				*/
				else if((s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0)&&
					(s->value(i+2,j) > 0 && s->value(i+2,j+2) > 0))
				{
					s->value(i-1,j) = s->value(i,j);
					s->value(i,j+1) = s->value(i,j);
					s->value(i,j) = 0;
				}
//##########################################################################################################
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					|O| |X| |	|O|O|X| |
					 - - - -   -->   - - - -
					| |X| |O|	|O| | |O|
					 - - - - 	 - - - -
					|O| | | |	|O| | | |
				
				*/
				else if((s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0)&&
					(s->value(i+2,j) > 0 && s->value(i+1,j+2) > 0))
				{
					s->value(i-1,j) = s->value(i,j);
					s->value(i,j+1) = s->value(i,j);
					s->value(i,j) = 0;
				}
				
				/*
					| | | |O|	| | | |O|
					 - - - -	 - - - -
					|O| |X| |	|O| | |O|
					 - - - -   -->   - - - -
					| |X| |O|	| |X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				
				else if((s->value(i-1,j+1) > 0 && s->value(i,j-1) > 0)&&
					(s->value(i+2,j) > 0 && s->value(i+2,j+2) > 0))
				{
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+2,j+1) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				
				/*
					| | | |O|	| | | |O|
					 - - - -	 - - - -
					|O| |X| |	|O| | |O|
					 - - - -   -->   - - - -
					| |X| |O|	| |X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if((s->value(i,j-1) > 0 && s->value(i-1,j+1) > 0)&&
					(s->value(i+2,j) > 0 && s->value(i+2,j+2) > 0))
				{
					s->value(i+1,j) = s->value(i+1,j+1);
					s->value(i+2,j+1) = s->value(i+1,j+1);
					s->value(i+1,j+1) = 0;
				}
				
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					|O| |X| |	|O|O|X| |
					 - - - -   -->   - - - -
					| |X| |O|	|O| | |O|
					 - - - - 	 - - - -
					|O| | | |	|O| | | |
				
				*/
				else if((s->value(i-1,j-1) > 0 && s->value(i-1,j+1) > 0)&&
					(s->value(i+2,j) > 0 && s->value(i+1,j+2) > 0))
				{
					s->value(i-1,j) = s->value(i,j);
					s->value(i,j+1) = s->value(i,j);
					s->value(i,j) = 0;
				}
//#####################################################################################################
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					|O| |X| |	|O| |X| |
					 - - - -   -->   - - - -
					| |X| |O|	| |X|O|O|
					 - - - - 	 - - - -
					| |O| | |	| | |O| |
							 - - - - 
							| |M|?| |
				
				*/
				else if((s->value(i,j-1) > 0 && s->value(i-1,j+1) > 0)&&
					(s->value(i+1,j+2) > 0 && s->value(i+2,j) > 0))
				{
					s->value(i+1,j-1) = s->value(i,j);
					s->value(i+1,j) = s->value(i,j);
					s->value(i,j-1) = 0;
					if(s->value(i+1,j-2) == 0)
						s->value(i,j-2) = s->value(i+1,j-1);
				}
				/*
					| |O| | |	| | |O| | |
					 - - - -	 - - - - -
					| | |X|O|	|?|O|O|X|O|
					 - - - -   -->   - - - - -
					|O|X| | |	|M| |X| | |
					 - - - - 	 - - - - -
					| | |O| |	| | | |O| |
				*/
				else if((s->value(i-1,j) > 0 && s->value(i+1,j-1) > 0)&&
					(s->value(i,j+2) > 0 && s->value(i+2,j+1) > 0))
				{
					s->value(i,j+1) = s->value(i,j);
					s->value(i-1,j+1) = s->value(i,j);
					s->value(i-1,j) = 0;
					if(s->value(i-2,j+1) == 0)
						s->value(i-2,j) = s->value(i-1,j+1);
				}
				
				
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					| | |X|O|	| |O|X|O|
					 - - - -   -->   - - - -
					| |X| | |	| | |O| |
					 - - - - 	 - - - -
					|?| | | |	|?| | | |
				*/
				else if(s->value(i+1,j+2) > 0 && s->value(i+2,j+1) > 0)
				{
					if(s->value(i-1,j-1) == 0)
					{
						s->value(i,j+1) = s->value(i,j);
						s->value(i+1,j) = s->value(i,j);
						s->value(i,j) = 0;
					}
					/*
						| | |O| |	| | |O| |
						 - - - -	 - - - -
						|O| |X|O|	|O|O|X|O|
						 - - - -   -->   - - - -
						| |X| | |	| | |O| |
						 - - - - 	 - - - -
						|O| | | |	|O|O| | |
					*/
					else if(s->value(i-1,j-1) > 0 &&
						s->value(i-1,j+1) > 0)
					{
						s->value(i,j+1) = s->value(i,j);
						s->value(i+1,j) = s->value(i,j);
						s->value(i,j-1) = s->value(i,j);
						s->value(i,j) = 0;
					}
					/*
						| | |O| |	| | |O| |
						 - - - -	 - - - -
						| | |X|O|	| |O|X|O|
						 - - - -   -->   - - - -
						| |X| | |	|O| |O| |
						 - - - - 	 - - - -
						|O| |O| |	|O| |O| |
					*/
					else if(s->value(i-1,j-1) > 0 &&
						s->value(i+1,j-1) > 0)
					{
						s->value(i,j+1) = s->value(i,j);
						s->value(i+1,j) = s->value(i,j);
						s->value(i-1,j) = s->value(i,j);
						s->value(i,j) = 0;
					}
				}
				
				/*
					| | | | |	|?|M| | |
					 - - - -	 - - - -
					| |O| | |	|O| | | |
					 - - - -	 - - - -
					| | |X|O|	|O| |X|O|
					 - - - -   -->   - - - -
					| |X| | |	| |X| | |
					 - - - - 	 - - - -
					|O| |O| |	|O| |O| |
				
				*/
				else if((s->value(i-1,j-1) > 0 && s->value(i+1,j-1) > 0)&&
					(s->value(i,j+2) > 0 && s->value(i+2,j+1) > 0))
				{
					s->value(i-1,j+1) = s->value(i,j+2);
					s->value(i-1,j+2) = s->value(i,j+2);
					if(s->value(i-1,j+3) == 0)
						s->value(i,j+3) = s->value(i,j+2);
					s->value(i,j+2) = 0;
				}
				
				else //NOTHING FIXED!!!!
				{
					if(!SpecialCaseHorse15(s,i,j))
					{
						printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
						char getchar;
    						cin >> getchar;
					}
				}
				printNbh(s,i,j,"After",5);
			}
		}
	}
	else if (diagonalType2(s,i,j) == true)
	{
		//and we have the exact right properties for the 2 points in the diagonal
		if(hasExact3NbsInRightTriangle(s,i+1,j) == true)
		{
			 if(hasExact3NbsInRightTriangle(s,i,j+1) == true)
			 {
			 	DBG_MSG3("\nSPECIAL CASE TYPE 2 FIXED at",i,j);
				printNbh(s,i,j,"Before",5);
				//try to see whether we can make a + configuration
				if((s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0)&&
					(s->value(i,j+2) > 0 && s->value(i-1,j+1) > 0))
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i-1,j) = s->value(i-1,j+1);
					s->value(i+1,j+2) = s->value(i,j+2);
					if(s->value(i-2,j) == 0)
						s->value(i-2,j+1) = s->value(i-1,j+1);
					if(s->value(i+1,j+3) == 0)
						s->value(i,j+3) = s->value(i,j+2);
					s->value(i-1,j+1) = 0;
					s->value(i,j+2) = 0;
					s->value(i,j+1) = 0;
				}
				/*
					|O| | | |	|O|O| | |
					 - - - -	 - - - -
					| |X| | |	| | |O| |
					 - - - -   -->   - - - -
					|O| |X|O|	|O|O|X|O|
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0 &&
					s->value(i-1,j) > 0 && s->value(i-1,j+2) > 0)
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i,j+2) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
				/*
					|O| |O| |	|O| |O| |
					 - - - -	 - - - -
					| |X| | |	|O| |O| |
					 - - - -   -->   - - - -
					| | |X|O|	| |O|X|O|
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0 &&
					s->value(i-1,j+2) > 0 && s->value(i+1,j+2) > 0)
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i-1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
				/*
					| |O| | |	| |O| | |
					 - - - -	 - - - -
					| |X| | |	| | |O| |
					 - - - -   -->   - - - -
					|O| |X|O|	|O|O|X|O|
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0 &&
					s->value(i-1,j) > 0 && s->value(i,j+2) > 0)
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					|O|X| | |	|O| |O| |
					 - - - -   -->   - - - -
					| | |X|O|	| |O|X|O|
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0 &&
					s->value(i-1,j+1) > 0 && s->value(i+1,j+2) > 0)
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					| |X| | |	| | |O| |
					 - - - -   -->   - - - -
					|O| |X|O|	|O|O|X|O|
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if(s->value(i+1,j-1) > 0 && s->value(i+2,j) > 0 &&
					s->value(i-1,j) > 0 && s->value(i+1,j+2) > 0)
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
//###############################################################################################
				/*
					|O| |O| |	|O| |O| |
					 - - - -	 - - - -
					| |X| | |	| |X| | |
					 - - - -   -->   - - - -
					| | |X| |	| |O| | |
					 - - - - 	 - - - -
					| |O| |O|	| |O|O|O|
				
				*/
				else if((s->value(i,j-1) > 0 && s->value(i+2,j-1) > 0)&&
					(s->value(i-1,j+2) > 0 && s->value(i+1,j+2) > 0))
				{
					s->value(i,j) = s->value(i+1,j);
					s->value(i+1,j-1) = s->value(i+1,j);
					s->value(i+1,j) = 0;
				}
				
				/*
					|O| | | |	|O| | | |
					 - - - -	 - - - -
					| |X| |O|	| |X|O|O|
					 - - - -   -->   - - - -
					|O| |X| |	|O| | |O|
					 - - - - 	 - - - -
					| | | |O|	| | | |O|
				
				*/
				else if((s->value(i-1,j) > 0 && s->value(i-1,j+2) > 0)&&
					(s->value(i+2,j+1) > 0 && s->value(i+2,j-1) > 0))
				{
					s->value(i+1,j+1) = s->value(i+1,j);
					s->value(i+2,j) = s->value(i+1,j);
					s->value(i+1,j) = 0;
				}
//####################################################################################################
				/*
					| |O| | |	| |O| | |
					 - - - -	 - - - -
					| |X| |O|	| |X|O|O|
					 - - - -   -->   - - - -
					|O| |X| |	|O| | |O|
					 - - - - 	 - - - -
					| | | |O|	| | | |O|
				
				*/
				else if((s->value(i-1,j) > 0 && s->value(i,j+2) > 0)&&
					(s->value(i+2,j+1) > 0 && s->value(i+2,j-1) > 0))
				{
					s->value(i+1,j+1) = s->value(i+1,j);
					s->value(i+2,j) = s->value(i+1,j);
					s->value(i+1,j) = 0;
				}
				
				/*
					|O| | | |	|O| | | |
					 - - - -	 - - - -
					| |X| |O|	|O| | |O|
					 - - - -   -->   - - - -
					|O| |X| |	|O|O|X| |
					 - - - - 	 - - - -
					| | |O| |	| | |O| |
				
				*/
				else if((s->value(i-1,j) > 0 && s->value(i-1,j+2) > 0)&&
					(s->value(i+2,j+1) > 0 && s->value(i+1,j-1) > 0))
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i-1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
				
				/*
					| | |O| |	| | |O| |
					 - - - -	 - - - -
					|O|X| | |	|O|X| | |
					 - - - -   -->   - - - -
					| | |X| |	| |O| | |
					 - - - - 	 - - - -
					| |O| |O|	| |O|O|O|
				
				*/
				else if((s->value(i,j-1) > 0 && s->value(i+2,j-1) > 0)&&
					(s->value(i-1,j+1) > 0 && s->value(i+1,j+2) > 0))
				{
					s->value(i,j) = s->value(i+1,j);
					s->value(i+1,j-1) = s->value(i+1,j);
					s->value(i+1,j) = 0;
				}
				
				/*
					|O| |O| |	|O|O|O| |
					 - - - -	 - - - -
					| |X| | |	| | |O| |
					 - - - -   -->   - - - -
					| | |X|O|	| | |X|O|
					 - - - - 	 - - - -
					| |O| | |	| |O| | |
				
				*/
				else if((s->value(i,j-1) > 0 && s->value(i+2,j) > 0)&&
					(s->value(i-1,j+2) > 0 && s->value(i+1,j+2) > 0))
				{
					s->value(i,j+2) = s->value(i,j+1);
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i,j+1) = 0;
				}
//#####################################################################################################
				else if((s->value(i-1,j) > 0 && s->value(i,j+2) > 0)&&
					(s->value(i+1,j-1) > 0 && s->value(i+2,j+1) > 0))
				{
					s->value(i,j) = s->value(i,j+1);
					s->value(i,j-1) = s->value(i+1,j);
					s->value(i+1,j-1) = 0;
					if(s->value(i,j-2) == 0)
						s->value(i+1,j-2) = s->value(i,j-1);
				}
				else if((s->value(i-1,j+1) > 0 && s->value(i+1,j+2) > 0)&&
					(s->value(i,j-1) > 0 && s->value(i+2,j) > 0))
				{
					s->value(i+1,j+1) = s->value(i,j+1);
					s->value(i+2,j+1) = s->value(i+1,j);
					s->value(i+2,j) = 0;
					if(s->value(i+3,j+1) == 0)
						s->value(i+3,j) = s->value(i+2,j+1);
				}
				
				/*
					| |O| | |	| |O| | |
					 - - - -	 - - - -
					|O|X| | |	|O|X|O| |
					 - - - -   -->   - - - -
					| | |X| |	| |O| | |
					 - - - - 	 - - - -
					| | | |?|	| | | |?|
				*/
				else if(s->value(i-1,j+1) > 0 &&
					s->value(i,j+2) > 0)
				{
					if(s->value(i+2,j-1) == 0)
					{
						s->value(i,j) = s->value(i+1,j);
						s->value(i+1,j+1) = s->value(i+1,j);
						s->value(i+1,j) = 0;
					}
					/*
						| |O| | |	| |O| | |
						 - - - -	 - - - -
						|O|X| |O|	|O|X|O|O|
						 - - - -   -->   - - - -
						| | |X| |	| |O| | |
						 - - - - 	 - - - -
						| | | |O|	| | |O|O|
					*/
					else if(s->value(i+2,j-1) > 0 &&
						s->value(i+2,j+1) > 0)
					{
						s->value(i,j) = s->value(i+1,j);
						s->value(i+1,j+1) = s->value(i+1,j);
						s->value(i+1,j-1) = s->value(i+1,j);
						s->value(i+1,j) = 0;
					}
					/*
						| |O| | |	| |O| | |
						 - - - -	 - - - -
						|O|X| | |	|O|X|O| |
						 - - - -   -->   - - - -
						| | |X| |	| |O| |O|
						 - - - - 	 - - - -
						| |O| |O|	| |O| |O|
					*/
					else if(s->value(i+2,j-1) > 0 &&
						s->value(i,j-1) > 0)
					{
						s->value(i,j) = s->value(i+1,j);
						s->value(i+1,j+1) = s->value(i+1,j);
						s->value(i+2,j) = s->value(i+1,j);
						s->value(i+1,j) = 0;
					}
				}
				
// // 				TODO TODO TODO TODO TODO TODO
// 				/*
// 					| | | | |	|?|M| | |
// 					 - - - -	 - - - -
// 					| |O| | |	|O| | | |
// 					 - - - -	 - - - -
// 					| | |X|O|	|O| |X|O|
// 					 - - - -   -->   - - - -
// 					| |X| | |	| |X| | |
// 					 - - - - 	 - - - -
// 					|O| |O| |	|O| |O| |
// 				
// 				*/
// 				else if((s->value(i-1,j-1) > 0 && s->value(i+1,j-1) > 0)&&
// 					(s->value(i,j+2) > 0 && s->value(i+2,j+1) > 0))
// 				{
// 					s->value(i-1,j+1) = s->value(i,j+2);
// 					s->value(i-1,j+2) = s->value(i,j+2);
// 					if(s->value(i-1,j+3) == 0)
// 						s->value(i,j+3) = s->value(i,j+2);
// 					s->value(i,j+2) = 0;
// 				}
				
				else //NOTHING FIXED!!!!
				{
					if(!SpecialLadyBug63(s,i,j))
					{
						printNbh(s,i,j,"NOT FIXED!!!!!!!!!!!",5);
						char getchar;
    						cin >> getchar;
					}
				}
 				printNbh(s,i,j,"After",5);
			 }
		}
	}
	
	if(nbs.Count() == 4) //the point has exactly 4 neighbors. Maybe we are in yet another special case
	{
		if(has4NbsInType3Config(s,i,j) == true)
		{
			DBG_MSG3("\nSPECIAL CASE TYPE 3 FIXED at",i,j);
		}
		if(has4NbsInType4Config(s,i,j) == true)
		{
			DBG_MSG3("\nSPECIAL CASE TYPE 4 FIXED at",i,j);
		}
		if(has4NbsInType5Config(s,i,j) == true)
		{
			DBG_MSG3("\nSPECIAL CASE TYPE 5 FIXED at",i,j);
		}
	}
	cerr.flush();
}

TreeSkel::TreeSkel(FIELD<int>* s): point_types(s->dimX(),s->dimY())
{
   int i=0,j=0,k=0,n=0,un=0; Coord nbs[10],unbs[10]; DQUEUE<Coord> q; float d1,d2,d3;
   
   point_types = BACKGROUND;					//initialize point_types
   
   memset(nbs,-1,10*sizeof(Coord));
   memset(unbs,-1,10*sizeof(Coord));
//##############################################################################################

	printNbh(s, g_nXCoord, g_nYCoord, "Original", 5);

	for(i = 1; i < s->dimX() - 3; i++)
		for(j = 1; j < s->dimY() - 3; j++)
		{
			IF_DBG_MSG2(FixBranchOf1Point(s,i,j), "\nBRANCH CONSISTING OUT OF 1 POINT FIXED at",i,j);
			IF_DBG_MSG2(SpecialLadyBug63(s,i,j),  "\nSPECIAL LADYBUG 63 FIXED at",i,j);
			IF_DBG_MSG2(FixDiamond(s,i,j),        "\nDIAMOND FIXED at",i,j);
			IF_DBG_MSG2(FixCleanDiamond(s,i,j),   "\nCLEAN DIAMOND FIXED at",i,j);
			IF_DBG_MSG2(FixAlmostH(s,i,j),       "\nALMOST H FIXED at",i,j);
		}
	

	for(i=1;i<s->dimX()-3;i++)			//Preprosses .... for a special case we found.
		for(j=1;j<s->dimY()-3;j++)		//A situation where 2 possible endpoints of inside branches
			FixPseudoCross(s, i, j);	//are in  diagonal in a 2x2 grid
			
	//FixField(s);
	for(i = 1; i < s->dimX() - 3; i++)
		for(j = 1; j < s->dimY() - 3; j++)
		{
			IF_DBG_MSG2(FixTConfig(s,i,j),      "\nT or L FIXED at",i,j);
			IF_DBG_MSG2(FixCube(s,i,j),        "\nCUBE FIXED at",i,j);
			IF_DBG_MSG2(FixDiamond(s,i,j),      "\nDIAMOND FIXED at",i,j);
			IF_DBG_MSG2(FixCleanDiamond(s,i,j), "\nCLEAN DIAMOND FIXED at",i,j);
		}

	for(i=1;i<s->dimX()-3;i++)			//Preprosses .... for a special case we found.
		for(j=1;j<s->dimY()-3;j++)		//A situation where 2 possible endpoints of inside branches
			FixPseudoCross(s, i, j);	//are in  diagonal in a 2x2 grid
			
	//FixField(s);
	
	for(i = 1; i < s->dimX() - 3; i++)
	{
		for(j = 1; j < s->dimY() - 3; j++)
		{
			IF_DBG_MSG2(FixTConfig(s,i,j),      "\nT or L FIXED at",i,j);
			IF_DBG_MSG2(FixCube(s,i,j),         "\nCUBE FIXED at",i,j);
			IF_DBG_MSG2(FixDiamond(s,i,j),      "\nDIAMOND FIXED at",i,j);
			IF_DBG_MSG2(FixCleanDiamond(s,i,j), "\nCLEAN DIAMOND FIXED at",i,j);
			IF_DBG_MSG2(NearSquare(s,i,j),      "\nNEAR SQUARE FIXED at",i,j);
		}
	}
		

// 	for(i=1;i<s->dimX()-3;i++)
// 		for(j=1;j<s->dimY()-3;j++)
// 			if(OneSpecialLadyBug(s,i,j))
// 				DBG_MSG3("\nONE SPECIAL LADYBUG FIXED at",i,j);
	
	printNbh(s, g_nXCoord, g_nYCoord, "New case", 5);
//##############################################################################################

   for(i=0;i<s->dimX();i++)					//1. Find endpoints. Put them in q.
      for(j=0;j<s->dimY();j++)
      if (s->value(i,j))					//Skel point detected, test if endpoint
      {
	setPoint(i,j,UNKNOWN);
	getNbs(s,i,j,nbs,n,unbs,un);
        if (n<2) setPoint(i,j,END);				//less than 2 nbs: endpoint
        if (n==1) q.Put(nbs[0]);   				//one nb: put it in q for evolution
	if (n==2 && (nbs[0].i==nbs[1].i || nbs[0].j==nbs[1].j)) //two nbs: may be endpoint
	{
	   d1 = fabs((float)nbs[0].i-i)+fabs((float)nbs[0].j-j);
	   d2 = fabs((float)nbs[1].i-i)+fabs((float)nbs[1].j-j);
	   k = (d1<d2)? 0 : (d2<d1)? 1 : -1;
	   if (k!=-1)
	   {  setPoint(i,j,END); q.Put(nbs[k]); }
	}
      }

   cout<<"Pass 1: detected endpoints: "<<q.Count()<<endl; 

   while (q.Count())						//2. Label rest of skel-points
   {
      Coord p = q.Get(); i=p.i; j=p.j; 

      if (!unknown(p) && !problem(p)) continue;			//Got an already processed point, skip it

      getNbs(s,i,j,nbs,n,unbs,un); int test_branch=0; 
      
      //if (i == 175 && j == 75)
      //	cerr << "\nNum nbs = " << n << "Unk = " << un << endl;

      if (problem(p))						//Don't treat problem-points unless all their nbs
      {								//are treated. Then make them regular.
        if (un==0) setPoint(i,j,REGULAR); else q.Put(p); 
        continue; 
      }

      if (n==2)							//Two nbs: regular point on a branch
      {  setPoint(i,j,REGULAR); q.Put(unbs[0]); }
      else if (n==3)						//Three nbs: regular OR branching point,
      {								//must analyze further
	 d1 = fabs((float)nbs[0].i-nbs[1].i)+fabs((float)nbs[0].j-nbs[1].j);
	 d2 = fabs((float)nbs[1].i-nbs[2].i)+fabs((float)nbs[1].j-nbs[2].j);
	 d3 = fabs((float)nbs[2].i-nbs[0].i)+fabs((float)nbs[2].j-nbs[0].j);

	 if (d1>1 && d2>1 && d3>1)				//Min-dist between two nbs > 1, so it
	    test_branch = 1; 					//could be a branching-point. 
	 else							//Regular point: continue from 
	 {							//its closest unknown neighbor
	   if (un==1) k=0; 					//Just one unknown-nb
	   else							//Two unknown-nbs, get closest
	   {
	      d1 = fabs((float)unbs[0].i-i)+fabs((float)unbs[0].j-j);		//
	      d2 = fabs((float)unbs[1].i-i)+fabs((float)unbs[1].j-j);
  	      k  = (d1<d2)? 0 : (d2<d1)? 1 : 			//Determine the closest unknown neighbor
		   (unknown(unbs[0]))? 0 : 1;			//and continue from it
	   }
	   setPoint(i,j,REGULAR); q.Put(unbs[k]); 
	 }
      } 
      else if (n>=4) 						//4 or more nbs: regular or branching-point
      {								//must analyze further
         if ((s->value(i-1,j) && s->value(i+1,j) && s->value(i,j-1) && s->value(i,j+1)) ||
	     (s->value(i-1,j-1) && s->value(i-1,j+1) && s->value(i+1,j+1) && s->value(i+1,j-1)))
	    test_branch = 1;					//4 skel-nbs in '+' or 'X' pattern, maybe branching-point
	 else							
	 {							//not a branching point
	    setPoint(i,j,REGULAR);
	    for(k=0;k<un;k++) q.Put(unbs[k]);
	 }
      }

      if (test_branch)						//We detected previously that (i,j) MAY
      {								//be a branching-point. Here we check if
	 if (diamond(i,j))					//the 'diamond' patterns occurs. If so,
	 {							//we can't yet detect if this is a branching-pt.
            setPoint(i,j,(un==2)? PROBLEM:REGULAR);             //We thus postpone processing this point and
	    if (un==2) q.Put(p);				//put it back in the queue.	
	    else q.Put(unbs[0]); 
	 }
	 else							//No 'diamond' pattern. The last thing to check is
	 {							//if we don't have a problem-point as our neighbor.
	   for(k=0;k<n;k++)					//If so, (i,j) is part of its 'diamond' so it is
	      if (problem(nbs[k])) break;			//a regular point, just as the problem-point as well.
	   if (k<n)
	      setPoint(i,j,REGULAR); 
	   else								//We checked everything - this is a 
	      setPoint(i,j,BRANCH); 				//branching-point...
	   q.Put(unbs[0]); q.Put(unbs[1]);
	 }
      }
   }

   // Build data structure that represents the connectivity between skel points. ie, the skeleton
   for(map<Coord,TreePoint*>::iterator it(skel.begin());it!=skel.end();it++)
   {										//Create the correct nbs[] structures of all points
     TreePoint* p = (*it).second;				//WARNING: not fully correct for all configs.
     getNbs(s,p,nbs,n);    					//Get, in nbs[], all candidate-nbs of p
     for(int nn=0;nn<n;nn++)			
     {
	map<Coord,TreePoint*>::iterator jt = skel.find(nbs[nn]);
	if (jt!=skel.end())
        { 
           TreePoint* q = (*jt).second; 			//Get q = candidate-nb
	   int skip = 0;		
										//p and q are nbs indeed ONLY if they have no
	   getNbs(s,q,unbs,un);					//common-nb r, s.th. d(p,r)==d(q,r)==1, i.e. they
	   for(int ii=0;ii<n;ii++)					//are not diagonal, having common-nb r at the corner...
              for(int jj=0;jj<un;jj++)
                 if (nbs[ii]==unbs[jj] && Coord(p->i,p->j).distance_m(nbs[ii])==1 &&
		    Coord(q->i,q->j).distance_m(unbs[jj])==1)
                    {  skip = 1; goto A; }
           A: 
           if (!skip) p->nbs.Append(q);			//p,q indeed nbs
        }
	else
		ASSERT(false);
     }
   }  

   buildBranches();							//Finally, build the branches[] structure

}



void TreeSkel::buildBranches()					//Builds the TreeBranch data structure out of the skel[]
{								//WARNING: not fully correct yet for all configurations
    for(map<Coord,TreePoint*>::iterator it(skel.begin());it!=skel.end();it++)
    {
       TreePoint* p = (*it).second;

       if ((p->type==END && p->nbs.Count()!=1)	||
	   (p->type==REGULAR && p->nbs.Count()!=2)  ||
           (p->type==BRANCH && p->nbs.Count()<3))
	   cout<<"Point "<<p->i<<" "<<p->j<<" type "<<p->type<<" nbs "<<p->nbs.Count()<<endl;

	   //1. Initialize float-coords from int-coords
	   //   From now on, we'll work only with floats
       p->x = (float)p->i;
	   p->y = (float)p->j;								

       if (p->type!=REGULAR || p->branches.Count()) continue;	//2. Locate a regular point without branch:

       TreeBranch* b = new TreeBranch(this);			//   Make a new branch

       DQUEUE<TreePoint*> q; q.Put(p); DARRAY<TreePoint*> endp;
       while(q.Count())						//3. Add all connected regular points to this branch
       {
	  TreePoint* p = q.Get(); 
          b->addPoint(p); 
	  int i;
	/*
          int has_branch = 0;
          for(i=0;i<p->nbs.Count();i++)
	     if (p->nbs[i]->type==BRANCH) 
	     { has_branch = 1; break; }
	*/
          for(i=0;i<p->nbs.Count();i++)
          {
             TreePoint* n = p->nbs[i];
             if (n->type==REGULAR && n->branches.Count()==0)
                q.Put(n);
             if (n->type==BRANCH || n->type==END)		//Branch or endpoint found? This will be end of the
                endp.Append(n);					//current TreeBranch...
          }
       }   
       
       if (endp.Count()!=2)
       {
		printNbh(NULL, g_nXCoord, g_nYCoord, "New case", 10);  
		
		cout<<"Error: buildBranches: branch has "<<endp.Count()<<" endpoints instead of 2"<<endl; 
		cout<<"Point "<<p->i<<" "<<p->j<<" type "<<p->type<<" nbs "<<p->nbs.Count()<<endl;
		
		
		for(int i=0;i<endp.Count();i++)
			cout<<"point "<<endp[i]->i<<" "<<endp[i]->j<<" type "<<endp[i]->type<<endl; return;
	}

      
       b->addEnds(endp[0],endp[1]); 				//4. Finalize the TreeBranch by adding its ends
     
       TreePoint *prev2=0, *prev=endp[0], *pt=0; int i=0,j;	//5. Sort points on this branch in order, starting from endp[0]
       for(;;)							//   (needed since step 3 doesn't guarantee any order)		
       {
	  for(j=0;j<prev->nbs.Count();j++)			//   Locate next REGULAR point on branch after 'prev'
	  {
	    pt = prev->nbs[j];
	    if (pt->type==REGULAR && pt!=prev2 && pt->branches[0]==b) break;
	  }
	  if (j==prev->nbs.Count()) break;			//   Not found: we're at end2, done
	  
 	  b->points[i] = pt; i++;
	  prev2 = prev; prev = pt;
       }
    }

    for(int i=0;i<branches.Count();i++)				//6. Compute branches' geom-lengths
       branches[i]->computeLength();

    cout<<"Pass 2: "<<branches.Count()<<" branches built"<<endl;

}

/*
	Given two branches, this funtion will tell us whether
	they are neighbors or not
*/
bool TreeSkel::isNeighbor(const TreeBranch& branch1, const TreeBranch& branch2) const
{
	int i;
	bool bIsNeighbor = false;
	for(i = 0; i < branch1.end1->branches.Count(); i++)
		if(*branch1.end1->branches[i] == branch2)
			bIsNeighbor = true;
	for(i = 0; i < branch1.end2->branches.Count(); i++)
		if(*branch1.end2->branches[i] == branch2)
			bIsNeighbor = true;
	return bIsNeighbor;
}

/*
	Given a vector with inside branches that need to be removed
	and a new found branch that needs to be removed from the
	skeleton this function will return the neighborhood number
	the new branch belongs to, or -1 if it has no existing
	neighborhood
*/
std::vector<int> TreeSkel::getNeighborhood(std::vector< std::pair<int,int> > markedBranches, TreeBranch* newInsideBranch)
{
	unsigned int i;
	std::vector<int> neighborhood;
	for(i = 0; i < markedBranches.size(); i++)
	{
		int nPosition = markedBranches[i].first;
		if(isNeighbor(*branches[nPosition],*newInsideBranch) == true)
			neighborhood.push_back(markedBranches[i].second);
	}
	return neighborhood;
}

/*
	Given dLength this function will return a vector
	containing the positions in DARRAY<TreeBranch*> branches
	of inside branches with a length of at most dLength
	In addition to the index in the dynamic array, 
	each entry will get a "neighborhood" number assigned
	specifying to which neighborhood of connected inside
	branches to be removed a branch belongs
*/
std::pair< std::vector< std::pair<int,int> >, int> TreeSkel::getInsideBranchesToBeRemoved(double dLength)
{
	int i, NumberOfNbhMerges = 0, nNeighborhoodNumber = 0;
	unsigned int j;
	std::vector< std::pair<int,int> > markedBranches;
	std::pair<int,int> branchInfo;
	for(i = 0; i < branches.Count(); i++)
	{
		if(branches[i]->geomLength() <= dLength && branches[i]->internal())
		{	
			branchInfo.first = i;
			std::vector<int> neighborhood = getNeighborhood(markedBranches, branches[i]);
			if(neighborhood.size() == 0)
				branchInfo.second = nNeighborhoodNumber++;
			else // there was an actual neighborhood
			{
				if(neighborhood.size() == 1)
					branchInfo.second = neighborhood[0];
				else if(neighborhood.size() == 2)
				{
					branchInfo.second = neighborhood[0];
					NumberOfNbhMerges++;
					//merge 2 neighborhoods. There were 2 separate
					//neighborhoods, but the branch we are dealing with
					//right now connects the two
					for(j = 0; j < markedBranches.size(); j++)
					{
						if(markedBranches[j].second == neighborhood[1])
							markedBranches[j].second = neighborhood[0];
					}
				}	
				else //undefined
					ASSERT(false);
			}
			markedBranches.push_back(branchInfo);	
		}
	}
	std::pair< std::vector< std::pair<int,int> >, int> allinfo;
	allinfo.first = markedBranches;
	allinfo.second = nNeighborhoodNumber - NumberOfNbhMerges; //the actual number of neighborhoods but they start
					      //counting on 0!!!
	return allinfo;
}

/* 
	Given the list of all the branches that need to be
	removed and a certain neighborhood denoted by i
	this funtion will return a list of the branches
	in that neighborhood
*/
std::vector<int> TreeSkel::GetListOfBrInNbh(std::vector< std::pair<int,int> > branchesTBRemoved,int nbh)
{
	unsigned int i;
	std::vector<int> branchesInNbh;
	for(i = 0; i < branchesTBRemoved.size(); i++)
		if(branchesTBRemoved[i].second == nbh)
			branchesInNbh.push_back(branchesTBRemoved[i].first);

	//there has to be at least one branch in this neighborhood
	//otherwise this neighborhood would not exist
	ASSERT(branchesInNbh.size() != 0);
	
	return branchesInNbh;
}



/*
	Given a list of branches in a neighborhood that are going to 
	be removed from the skeleton, return the index of the branch and
	the endpoint (1 or 2, or -1 if the point was not an endpoint) 
	of that branch that has the largest radius in that neighborhood. 
	That point will be the new endpoint of all the branches that 
	were attached to this neighborhood. 
*/
std::pair<int,int> TreeSkel::GetPntWithLargestRadius(std::vector<int> listOfBranchesInNbh)
{
	//find branch with largest radius value
	unsigned int i;
	int j;
	double dLargestRadiusEndPoint = 0, dLargestRadiusTreePoint = 0;
	int nPosBrWithLargestRadius = -1;
	std::pair<int,int> branchWithPnt, TreePointAndBranch;
	TreeBranch* phantomToBe = NULL;
	DARRAY<TreePoint*> allEndpoints; 
	for(i = 0; i < listOfBranchesInNbh.size(); i++)
	{
		phantomToBe = branches[listOfBranchesInNbh[i]];
		allEndpoints.Append(phantomToBe->end1);
		allEndpoints.Append(phantomToBe->end2);
		if(phantomToBe->end1->dRadius > dLargestRadiusEndPoint || phantomToBe->end2->dRadius > dLargestRadiusEndPoint)
		{
			dLargestRadiusEndPoint = (phantomToBe->end1->dRadius >= phantomToBe->end2->dRadius)? phantomToBe->end1->dRadius : phantomToBe->end2->dRadius;
			nPosBrWithLargestRadius = listOfBranchesInNbh[i];
		}
	}
	for(i = 0; i < listOfBranchesInNbh.size(); i++)
	{
		phantomToBe = branches[listOfBranchesInNbh[i]];
		for(j = 0; j < phantomToBe->points.Count(); j++)
		{
			if( phantomToBe->points[j]->dRadius > dLargestRadiusTreePoint)
			{
				dLargestRadiusTreePoint = phantomToBe->points[j]->dRadius;
				TreePointAndBranch.first = i;
				TreePointAndBranch.second = j;
			}
		}
	}
	//cerr << "\nRadius of largest endpoint: " << dLargestRadiusEndPoint << "\nRadius of largest TreePoint: " << dLargestRadiusTreePoint << endl;
	//find out whether it is really an endpoint or we need to take a point in the middle
	branchWithPnt.first = nPosBrWithLargestRadius;
	TreeBranch* pBrnchWithTheRadius = branches[nPosBrWithLargestRadius];
	if(pBrnchWithTheRadius->end1->dRadius > pBrnchWithTheRadius->end2->dRadius)
		branchWithPnt.second = 1;
	else if (pBrnchWithTheRadius->end2->dRadius > pBrnchWithTheRadius->end1->dRadius)
		branchWithPnt.second = 2;
	else if (pBrnchWithTheRadius->end2->dRadius == pBrnchWithTheRadius->end1->dRadius)
		branchWithPnt.second = -1;
	return branchWithPnt;
}



/*
	Given a list of branches in a neighborhood that are going to be
	removed from the skeleton, and an existing endpoint that we will
	be using, return a list of branches that are
	attached to these to-be-phantoms but not that that endpoint 
*/
DARRAY<TreeSkel::TreeBranch*> TreeSkel::GetListOfBrTBUpdatedWithEndPnt(std::vector<int> listOfBranchesInNbh, std::pair<int,int> branchEndpointLargestRadius)
{
	DARRAY<TreeBranch*> listOfBrTBUpdated, listOfPhantomsToBe;
	unsigned int i;
	int j;
	for(i = 0; i < listOfBranchesInNbh.size(); i++)
	{
		TreeBranch* phantomToBe = branches[listOfBranchesInNbh[i]];
		listOfPhantomsToBe.Add(phantomToBe);
		if(!(listOfBranchesInNbh[i] == branchEndpointLargestRadius.first && branchEndpointLargestRadius.second == 1))
		{
			for(j = 0; j < phantomToBe->end1->branches.Count(); j++)
				listOfBrTBUpdated.Append(phantomToBe->end1->branches[j]);
		}
		if(!(listOfBranchesInNbh[i] == branchEndpointLargestRadius.first && branchEndpointLargestRadius.second == 2))
		{
			for(j = 0; j < phantomToBe->end2->branches.Count(); j++)
				listOfBrTBUpdated.Append(phantomToBe->end2->branches[j]);
		}
	}

	int nPhantomRemovedFromList = 0;
	
	for(j = 0; j < listOfPhantomsToBe.Count(); j++)
	{
		nPhantomRemovedFromList = listOfBrTBUpdated.Remove(listOfPhantomsToBe[j]);
		//since I am trying to add all neighbors of a phantom to be, the phantom
		//itself is also in the list. It has to be removed
		ASSERT(nPhantomRemovedFromList != -1);
	}
	return listOfBrTBUpdated;
}


/*
	Given a list of branches in a neighborhood that are going to be
	removed from the skeleton, return a list of branches that are
	attached to these to-be-phantoms and will need to be updated
	because of the to-be-phantoms
*/
DARRAY<TreeSkel::TreeBranch*> TreeSkel::GetListOfBrTBUpdated(std::vector<int> listOfBranchesInNbh)
{
	DARRAY<TreeBranch*> listOfBrTBUpdated, listOfPhantomsToBe;
	unsigned int i;
	int j;
	for(i = 0; i < listOfBranchesInNbh.size(); i++)
	{
		TreeBranch* phantomToBe = branches[listOfBranchesInNbh[i]];
		listOfPhantomsToBe.Add(phantomToBe);
		for(j = 0; j < phantomToBe->end1->branches.Count(); j++)
			listOfBrTBUpdated.Append(phantomToBe->end1->branches[j]);
		for(j = 0; j < phantomToBe->end2->branches.Count(); j++)
			listOfBrTBUpdated.Append(phantomToBe->end2->branches[j]);
	}

	int nPhantomRemovedFromList = 0;
	
	for(j = 0; j < listOfPhantomsToBe.Count(); j++)
	{
		nPhantomRemovedFromList = listOfBrTBUpdated.Remove(listOfPhantomsToBe[j]);
		//since I am trying to add all neighbors of a phantom to be, the phantom
		//itself is also in the list. It has to be removed
		ASSERT(nPhantomRemovedFromList != -1);
	}
	return listOfBrTBUpdated;
}

/*
	Returns the numbering of the Neighborhoods
*/
DARRAY<int> TreeSkel::GetListOfNumberingNbhs(std::vector< std::pair<int,int> > branchesTBRemoved)
{
	unsigned int i;
	DARRAY<int> nbhNumbers;
	for(i = 0; i < branchesTBRemoved.size(); i++)
	{
		nbhNumbers.Append(branchesTBRemoved[i].second);
	}
	return nbhNumbers;
}


/*
	Return the endpoint that is connected to a phantom branch to be. Return either 1 or 2
*/
int TreeSkel::GetEndPntAttachedToPhantom(TreeBranch* pBranchToFix, std::vector<int> listOfBranchesInNbh)
{
	unsigned int i;
	int nNeededEndPoint = 0;
	bool bFound = false;
	for(i = 0; !bFound && i < listOfBranchesInNbh.size(); i++)
	{
		nNeededEndPoint = PhantomIsNb(*pBranchToFix, *branches[listOfBranchesInNbh[i]]);
		if(nNeededEndPoint != 0)
		{
			bFound = true;
		}
	}
	ASSERT(nNeededEndPoint != 0);
	return nNeededEndPoint;
}


/*
	This function converts the branch that needs to be fixed into a smartarray
	of POINTs. This smartarray is necessary because it is what ApproxPoly
	uses to fit lines to the data
*/
vpl::POINTS TreeSkel::ConvertBranchIntoPoints(TreeBranch* pBranchToFix, int nEndPoint)
{
	vpl::POINTS data;
	vpl::Point* aPoint = new vpl::Point(0,0);
	int i;
	double dCumDist = 0;
	std::pair<double, double> prevCoords;
	if(nEndPoint == 1) //add TreePoints in normal order
	{
		aPoint->Set(dCumDist,pBranchToFix->end1->dRadius);
		prevCoords.first = pBranchToFix->end1->x;
		prevCoords.second = pBranchToFix->end1->y;
		data.AddTail(*aPoint);
		for(i = 0; i < pBranchToFix->points.Count(); i++)
		{
			dCumDist += sqrt(fabs(prevCoords.first - pBranchToFix->points[i]->x) + fabs(prevCoords.second - pBranchToFix->points[i]->y));
			aPoint->Set(dCumDist,pBranchToFix->points[i]->dRadius);
			data.AddTail(*aPoint);
			prevCoords.first = pBranchToFix->points[i]->x;
			prevCoords.second = pBranchToFix->points[i]->y;
		}
		dCumDist += sqrt(fabs(prevCoords.first - pBranchToFix->end2->x) + fabs(prevCoords.second - pBranchToFix->end2->y));
		aPoint->Set(dCumDist,pBranchToFix->end2->dRadius);
		data.AddTail(*aPoint);
	}
	if(nEndPoint == 2) //add TreePoints in reverse order
	{
		aPoint->Set(dCumDist,pBranchToFix->end2->dRadius);
		prevCoords.first = pBranchToFix->end2->x;
		prevCoords.second = pBranchToFix->end2->y;
		data.AddTail(*aPoint);
		for(i = pBranchToFix->points.Count()-1; i > -1; i--)
		{
			dCumDist += sqrt(fabs(prevCoords.first - pBranchToFix->points[i]->x) + fabs(prevCoords.second - pBranchToFix->points[i]->y));
			aPoint->Set(dCumDist,pBranchToFix->points[i]->dRadius);
			data.AddTail(*aPoint);
			prevCoords.first = pBranchToFix->points[i]->x;
			prevCoords.second = pBranchToFix->points[i]->y;
		}
		dCumDist += sqrt(fabs(prevCoords.first - pBranchToFix->end1->x) + fabs(prevCoords.second - pBranchToFix->end1->y));
		aPoint->Set(dCumDist,pBranchToFix->end1->dRadius);
		data.AddTail(*aPoint);
	}
	delete aPoint;
	return data;
}

/*
	Given a branch, a geometric distance specifying the length
	of the ligature and the endpoint of the branch where
	the ligature starts, this function will return the index
	of the TreePoint that lies on or just beyond this distance
*/
int TreeSkel::GetPntIndexBeginLigature(TreeBranch* pBranchToFix,double dDistance,int nEndPoint)
{
	double dCumDist = 0;
	int nIndex = 0,i;
	std::pair<double,double> prevCoor;
	TreePoint* pCurPnt;
	bool bFound = false;
	if(nEndPoint == 1)
	{
		nIndex = pBranchToFix->points.Count() - 1;
		prevCoor.first = pBranchToFix->end1->x;
		prevCoor.second = pBranchToFix->end1->y;
		for(i = 0; !bFound && i < pBranchToFix->points.Count(); i++)
		{
			pCurPnt = pBranchToFix->points[i];
			dCumDist += sqrt(fabs(prevCoor.first - pCurPnt->x) + fabs(prevCoor.second - pCurPnt->y));
			if(dCumDist >= dDistance)
			{
				nIndex = i;
				bFound = true;
			}
			prevCoor.first = pCurPnt->x;
			prevCoor.second = pCurPnt->y;
		}
	}
	if(nEndPoint == 2)
	{
		nIndex = 0;
		prevCoor.first = pBranchToFix->end2->x;
		prevCoor.second = pBranchToFix->end2->y;
		for(i = pBranchToFix->points.Count()-1; !bFound && i > -1; i--)
		{
			pCurPnt = pBranchToFix->points[i];
			dCumDist += sqrt(fabs(prevCoor.first - pCurPnt->x) + fabs(prevCoor.second - pCurPnt->y));
			if(dCumDist >= dDistance)
			{
				nIndex = i;
				bFound = true;
			}
			prevCoor.first = pCurPnt->x;
			prevCoor.second = pCurPnt->y;
		}
	}
	return nIndex;
}

/*
	Deletes part of the branch. Depending on whether the branch is attached
	to the phantom with end1 or end2 it deletes from the front or from
	the back of the DARRAY with treepoints
*/
void TreeSkel::DeletePartOfBranch(TreeBranch* pBranchToFix,int nPntIndex,int nEndPointAttachedToPhantom)
{
	int i;
	//delete part of the branch (old ligature part)
	if(nEndPointAttachedToPhantom == 1) //delete from beginning of DARRAY<TreePoint*> points
		for(i = 0; i < nPntIndex; i++)
			pBranchToFix->points.Delete(0);
	else if(nEndPointAttachedToPhantom == 2)
		for(i = pBranchToFix->points.Count()-1; i > nPntIndex; i--)
			pBranchToFix->points.Delete(i);
}


/*
	Assigns a new endpoint to the endpoint that was attached to the phantom.
*/
void TreeSkel::AssignNewEndPntToBranch(TreeBranch* pBranchToFix,int nEndPointAttachedToPhantom,TreePoint* pNewEndPointForBranch)
{
	ASSERT(pNewEndPointForBranch != NULL);
	
	if(nEndPointAttachedToPhantom == 1)
	{
		pBranchToFix->end1 = pNewEndPointForBranch;
		pNewEndPointForBranch->branches.Add(pBranchToFix);
	}
	else if(nEndPointAttachedToPhantom == 2)
	{
		pBranchToFix->end2 = pNewEndPointForBranch;
		pNewEndPointForBranch->branches.Add(pBranchToFix);
	}
	else
		ASSERT(false);
}


/*
	MidPoint algorithm. Returns a DARRAY with the coordinates
	of the points on the line
*/
DARRAY<std::pair<int,int> > TreeSkel::Bresenham(const int x1,const int y1,const int x2,const int y2)
{
	std::pair<int,int> coordinates;
	DARRAY<std::pair<int,int> > linePiece;
	int slope;
	int dx, dy, incE, incNE, d, x, y;
	//reverse lines where x1 > x2
	if(x1 > x2)
		return Bresenham(x2,y2,x1,y1);
	dx = x2 - x1;
	dy = y2 - y1;
	//adjust y-increment for negatively sloped lines
	if(dy < 0)
	{
		slope = -1;
		dy = -dy;
	}
	else
		slope = 1;
	//Bresenham constants
	incE = 2 * dy;
	incNE = 2 * dy - 2 * dx;
	d = 2 * dy - dx;
	y = y1;
	for(x = x1+1; x < x2; x++)
	{
		coordinates.first = x;
		coordinates.second = y;
		linePiece.Add(coordinates);
		if(d <= 0)
			d += incE;
		else
		{
			d += incNE;
			y += slope;
		}
	}
	return linePiece;
}

bool TreeSkel::GetPntClosestToBranch(double x11, double y11,DARRAY<std::pair<int,int> > linePieceCoordinates,double dSlope)
{
	bool bBeginIsClosest = false;
	double dBeginX = linePieceCoordinates[0].first;
	double dBeginY = linePieceCoordinates[0].second;
	double dEuclDisBegin = 0;
	if(dSlope <=1)
		dEuclDisBegin = sqrt((x11 - dBeginX)*(x11 - dBeginX) + (y11 - dBeginY)*(y11 - dBeginY));
	else //dSlop > 1
		dEuclDisBegin = sqrt((x11 - dBeginY)*(x11 - dBeginY) + (y11 - dBeginX)*(y11 - dBeginX));
	double dEndX = linePieceCoordinates[linePieceCoordinates.Count()-1].first;
	double dEndY = linePieceCoordinates[linePieceCoordinates.Count()-1].second;
	double dEuclDisEnd = 0;
	if(dSlope <=1)
		dEuclDisEnd = sqrt((x11 - dEndX)*(x11 - dEndX) + (y11 - dEndY)*(y11 - dEndY));
	else //dSlop > 1
		dEuclDisEnd = sqrt((x11 - dEndY)*(x11 - dEndY) + (y11 - dEndX)*(y11 - dEndX));
	if(dEuclDisBegin < dEuclDisEnd)
		bBeginIsClosest = true;
	return bBeginIsClosest;
}


/*
	This function will draw a straight line from where the branch
	now stops towards the newendpoint
*/
void TreeSkel::ConnectBranchToNewEndPnt(TreeBranch* pBranchToFix,int nEndPointAttachedToPhantom,TreePoint* pNewEndPointForBranch)
{
	TreePoint* pNewTreePnt = NULL;
	int i;
	DARRAY<std::pair<int,int> > linePieceCoordinates;
	int x1 = 0, y1 = 0;
	if(nEndPointAttachedToPhantom == 1)
	{
		x1 = pBranchToFix->points[0]->i; 
		y1 = pBranchToFix->points[0]->j;
	}
	else if(nEndPointAttachedToPhantom == 2)
	{
		int nLastIndex = pBranchToFix->points.Count()-1;
		x1 = pBranchToFix->points[nLastIndex]->i; 
		y1 = pBranchToFix->points[nLastIndex]->j;
	}
	int x2 = pNewEndPointForBranch->i, y2 = pNewEndPointForBranch->j;
	double distanceX = x2 - x1, distanceY = y2 - y1;
	double dSlope = fabs(distanceY/distanceX);
	(dSlope <= 1)?linePieceCoordinates = Bresenham(x1,y1,x2,y2):linePieceCoordinates = Bresenham(y1,x1,y2,x2);
	//check whether begin point of linepiece is closest to the existing branch or not
	double x11 = x1, y11 = y1;
	bool bBeginPntIsClosest = GetPntClosestToBranch(x11,y11,linePieceCoordinates,dSlope);
	DARRAY<TreePoint*> newPartOfTheBranch, newEntireBranch;
	
	for(i = 0; i < linePieceCoordinates.Count(); i++)
	{
		if(dSlope <= 1)
		{
			Coord c(linePieceCoordinates[i].first,linePieceCoordinates[i].second);
			pNewTreePnt = new TreePoint(c,REGULAR);
			MemoryLeakRemoval.Add(pNewTreePnt);
			pNewTreePnt->x = (float)linePieceCoordinates[i].first;
			pNewTreePnt->y = (float)linePieceCoordinates[i].second;
		}
		else
		{
			Coord c(linePieceCoordinates[i].second,linePieceCoordinates[i].first);
			pNewTreePnt = new TreePoint(c,REGULAR);
			MemoryLeakRemoval.Add(pNewTreePnt);
			pNewTreePnt->x = (float)linePieceCoordinates[i].second;
			pNewTreePnt->y = (float)linePieceCoordinates[i].first;
		}
		pNewTreePnt->dRadius = 0;
		newPartOfTheBranch.Add(pNewTreePnt);
	}
	if(nEndPointAttachedToPhantom == 1)
	{
		if(bBeginPntIsClosest)
			for(i = newPartOfTheBranch.Count()-1; i > -1; i--)
				newEntireBranch.Add(newPartOfTheBranch[i]);
		else //!bBeginPntIsClosest
			for(i = 0; i < newPartOfTheBranch.Count(); i++)
				newEntireBranch.Add(newPartOfTheBranch[i]);
		for(i = 0; i < pBranchToFix->points.Count(); i++)
			newEntireBranch.Add(pBranchToFix->points[i]);
	}
	else //nEndPointAttachedToPhantom == 2
	{
		for(i = 0; i < pBranchToFix->points.Count(); i++)
			newEntireBranch.Add(pBranchToFix->points[i]);
		if(bBeginPntIsClosest)
			for(i = 0; i < newPartOfTheBranch.Count(); i++)
				newEntireBranch.Add(newPartOfTheBranch[i]);
		else //!bBeginPntIsClosest
			for(i = newPartOfTheBranch.Count()-1; i > -1; i--)
				newEntireBranch.Add(newPartOfTheBranch[i]);
	}
	pBranchToFix->points = newEntireBranch;
}


/*
	This function will displace pBranchToFix. It will delete a number of points from inside the 
	branch, namely upto nPntIndex and attach the remaining part of the branch to
	pNewEndPointForBranch with a straight line.
*/
void TreeSkel::DisplaceBranch(TreeBranch* pBranchToFix,int nPntIndex,int nEndPointAttachedToPhantom,TreePoint* pNewEndPointForBranch)
{
	//delete part of the branch (old ligature part)
	DeletePartOfBranch(pBranchToFix,nPntIndex,nEndPointAttachedToPhantom);
	//draw a straight line from where we deleted it to the new endpoint
	ConnectBranchToNewEndPnt(pBranchToFix,nEndPointAttachedToPhantom,pNewEndPointForBranch);
	//assign new endpoint to the branch
	AssignNewEndPntToBranch(pBranchToFix,nEndPointAttachedToPhantom,pNewEndPointForBranch);
}


/*
	This function will return the position where the 
	last true radius value is in the DARRAY points
*/
int TreeSkel::GetIndexWhereBranchWasCut(TreeBranch* pBranchToFix, int nEndPoint)
{
	int i, nIndex = -1;
	bool bFound = false;
	if(nEndPoint == 1) //branch was cut of from the beginning of the points DARRAY
		for(i = 0; i < pBranchToFix->points.Count() && !bFound; i++) 
		{
			if(pBranchToFix->points[i]->dRadius != 0)
			{
				bFound = true;
				nIndex = i;
			}
		}
	else if(nEndPoint == 2) //branch was cut of from the end of the points DARRAY
		for(i = pBranchToFix->points.Count()-1;!bFound && i > -1; i--)
		{
			if(0 != pBranchToFix->points[i]->dRadius)
			{
				nIndex = i;
				bFound = true;
			}
		}
	return nIndex;
}


/*
	This function will calculate where the *new* ligature stops. So where
	the radius of the new endpoint intersects the branch
*/
int TreeSkel::GetIndexOfEndNewRadius(TreeBranch* pBranchToFix,int nEndPoint,TreePoint* pNewEndPointForBranch)
{
	int i, nIndex = -1;
	bool bFound = false;
	double dRadiusToBeFound = pNewEndPointForBranch->dRadius;
	double dDistance = 0;
	int nOriginalCoorX = pNewEndPointForBranch->i, nOriginalCoorY = pNewEndPointForBranch->j; 
	if(nEndPoint == 1) //branch was cut of from the beginning of the points DARRAY
		for(i = 0; i < pBranchToFix->points.Count() && !bFound; i++)
		{
			dDistance = sqrt((double)(nOriginalCoorX - pBranchToFix->points[i]->i)*(nOriginalCoorX - pBranchToFix->points[i]->i)
					 + (nOriginalCoorY - pBranchToFix->points[i]->j)*(nOriginalCoorY - pBranchToFix->points[i]->j));
			if(dDistance >= dRadiusToBeFound)
			{
				bFound = true;
				nIndex = i;
			}
		}
	else if(nEndPoint == 2) //branch was cut of from the end of the points DARRAY
	for(i = pBranchToFix->points.Count()-1 ; i > -1 && !bFound; i--)
		{
			dDistance = sqrt((double)(nOriginalCoorX - pBranchToFix->points[i]->i)*(nOriginalCoorX - pBranchToFix->points[i]->i)
					+ (nOriginalCoorY - pBranchToFix->points[i]->j)*(nOriginalCoorY - pBranchToFix->points[i]->j));
			if(dDistance >= dRadiusToBeFound)
			{
				bFound = true;
				nIndex = i;
			}
		}
	return nIndex;
}


/*
	Given a known slope on a piece of a branch and a begin en endpoint to where this
	slope should be carried along the radii values, this function will populate the
	radii
*/
void TreeSkel::PopulateRadiiWithSlope(TreeBranch* pBranchToFix,double dSlope,int nIndexWhereCut,int nEndofNewRadius)
{
	int i;
	float dPrevRadius = pBranchToFix->points[nIndexWhereCut]->dRadius;
	
	ASSERT(dPrevRadius != 0);
	
	if(nIndexWhereCut < nEndofNewRadius)
		for(i = nIndexWhereCut+1; i <= nEndofNewRadius; i++)
		{
			pBranchToFix->points[i]->dRadius = dPrevRadius + (float)dSlope;
			dPrevRadius = pBranchToFix->points[i]->dRadius;
		}
	else if(nIndexWhereCut > nEndofNewRadius)
		for(i = nIndexWhereCut-1; i >= nEndofNewRadius; i--)
		{
			pBranchToFix->points[i]->dRadius = dPrevRadius + (float)dSlope;
			dPrevRadius = pBranchToFix->points[i]->dRadius;;
		}
}


/*
	This function will populate the radii along the new ligature part.
	The part of the branch that is attached to the new endpoint towards
	the end of that radius at that endpoint
*/
void TreeSkel::PopulateRadiiWithoutSlope(TreeBranch* pBranchToFix,int nEndPoint,TreePoint* pNewEndPointForBranch,int nEndofNewRadius)
{
	int i;
	double dBigRadius = pNewEndPointForBranch->dRadius;
	double dSmallRadius = 0;
	if(nEndofNewRadius != -1)
		dSmallRadius = pBranchToFix->points[nEndofNewRadius]->dRadius;
	else
	{
		dSmallRadius = (nEndPoint == 1)?pBranchToFix->end2->dRadius:pBranchToFix->end1->dRadius;
		nEndofNewRadius = (nEndPoint == 1)? pBranchToFix->points.Count(): -1;
	}
	double dSlope = 0, dRadiusToAssign = 0, divisor = 0;
	
	ASSERT(dSmallRadius != 0 && dBigRadius != 0);
	
	if(nEndPoint == 1)//big radius starts at positoin 0 in the DARRAY
	{
		divisor = nEndofNewRadius;
		dSlope = (dBigRadius - dSmallRadius)/divisor;
		dRadiusToAssign = dBigRadius - dSlope;
		for(i = 0; i < nEndofNewRadius; i++)
		{
			pBranchToFix->points[i]->dRadius = (float)dRadiusToAssign;
			dRadiusToAssign = pBranchToFix->points[i]->dRadius - dSlope;
		}
	}
	else if(nEndPoint == 2)
	{
		divisor = pBranchToFix->points.Count() - nEndofNewRadius;
		dSlope = (dBigRadius - dSmallRadius)/divisor;
		dRadiusToAssign = dBigRadius - dSlope;
		for(i = pBranchToFix->points.Count()-1; i > nEndofNewRadius; i--)
		{
			pBranchToFix->points[i]->dRadius = (float)dRadiusToAssign;
			dRadiusToAssign = pBranchToFix->points[i]->dRadius - dSlope;
		}
	}
}


/*
	Given the TreeBranch that needs to be fixed, which endpoint was connected to the 
	phantombranch (nEndPoint) the newpoint (for the radius)
	and a slope we are going to use for the new non-ligature part, this function will assign
	the radii
*/
void TreeSkel::ReAssignRadiusValues(TreeBranch* pBranchToFix,int nEndPoint,TreePoint* pNewEndPointForBranch,double dSlope)
{
	int nIndexWhereCut = GetIndexWhereBranchWasCut(pBranchToFix,nEndPoint);
	int nEndofNewRadius = 0;
	if(nIndexWhereCut != -1)
		nEndofNewRadius = GetIndexOfEndNewRadius(pBranchToFix,nEndPoint,pNewEndPointForBranch);
	if(nEndofNewRadius != -1 && nIndexWhereCut != -1) //we have a piece of the branch where we can assign "old" radius values too
	{
		if(pBranchToFix->points[nEndofNewRadius]->dRadius == 0)
		{
			PopulateRadiiWithSlope(pBranchToFix,-dSlope,nIndexWhereCut,nEndofNewRadius);
		}
		else //pBranchToFix->points[nEndofNewRadius]->dRadius != 0 which means that the end of the new radius cuts a piece of the branch that we will keep
		{
			nEndofNewRadius = nIndexWhereCut;
		}
	}
	PopulateRadiiWithoutSlope(pBranchToFix,nEndPoint,pNewEndPointForBranch,nEndofNewRadius);
}


/*
	Given a TreeBranch that needs to be fixed, a list of branches in the 
	neighborhood (phantoms to be) and a std::pair<int,int> telling us
	which endpoint of which phantom branch is going to be the new
	endpoint for this TreeBranch, this function will connect this TreeBranch
	to that endpoint and update x,y coordinates as well as radius values on
	the branch
*/
void TreeSkel::fixBranch(TreeBranch* pBranchToFix, std::vector<int> listOfBranchesInNbh, 
						 std::pair<int,int> branchEndpointLargestRadius)
{
	int i;
	int nEndLigature = 0;
	bool bFound = false;
	TreePoint* pNewEndPointForBranch = NULL;
	//find the endpoint of the pBranchToFix that that is attached to one of the phantoms
	int nEndPoint = GetEndPntAttachedToPhantom(pBranchToFix, listOfBranchesInNbh);
	//convert the TreeSkel Points in the branch into a smartarray of POINTs starting from the connection
	//with the phantom branch
	vpl::POINTS convertedBranch = ConvertBranchIntoPoints( pBranchToFix, nEndPoint);
	//time to fit the convertedBranch with a few lines
	vpl::PolyLineApprox poly(convertedBranch.GetSize()/dMinError, dMinSlope, 10, 0.3);
	//poly.SetDbgMode(true);
	poly.Fit(convertedBranch);
	int nSize = poly.m_knots.GetSize();
	ASSERT(nSize >= 1);
	//get the slope of the "extra" part non-ligature
	//the first slope is of the ligature part, the second of the non ligature part
	//which we want to put on the "extra" non ligature part too
	double dSlope = 0;
	
	if(nSize == 1)	//slope of only segment
		dSlope = poly.m_knots[0].seg.m;
	else	//slope of segment next to the ligature segment
	{
		for(i = 1; i < nSize && !bFound; i++)
		{
			dSlope = poly.m_knots[i].seg.m;
			nEndLigature = i-1;
			double m0 = poly.m_knots[i - 1].seg.m;
			double m1 = poly.m_knots[i].seg.m;
			double dMax = (fabs(m0) >= fabs(m1))? m0 : m1;
			double mr = fabs(m1 - m0) / fabs(dMax);
			//cout << "disp('Acc change: " << mr << ", Max:" << dMaxAccelChg << "');\n";
			if (mr >= dMaxAccelChg )
			{
				bFound = true;
			}
		}
	}	
	int nPntIndex = GetPntIndexBeginLigature(pBranchToFix, poly.m_knots[nEndLigature].seg.p1.x, nEndPoint);
	
	if(branchEndpointLargestRadius.second == 1)
		pNewEndPointForBranch = branches[branchEndpointLargestRadius.first]->end1;
	else if(branchEndpointLargestRadius.second == 2)
		pNewEndPointForBranch = branches[branchEndpointLargestRadius.first]->end2;
	//transfer/shift/displace the branch
	DisplaceBranch(pBranchToFix,nPntIndex,nEndPoint,pNewEndPointForBranch);
	//assign radius values
	ReAssignRadiusValues(pBranchToFix,nEndPoint,pNewEndPointForBranch,dSlope);
}


/*
	Returns the size of the smallest inside branch or, if there
	are no more inside branches, -1
*/
double TreeSkel::getSizeSmallestInsideBranch()
{
	double dSmallestInsideBranch = 1000000;
	int i;
	
	for(i = 0; i < branches.Count(); i++)
		if((branches[i]->internal() == 1) && (branches[i]->geomLength() < dSmallestInsideBranch))
			dSmallestInsideBranch = branches[i]->geomLength();
			
	if(dSmallestInsideBranch == 1000000)
		dSmallestInsideBranch = -1;
	
	return dSmallestInsideBranch;
}


/*
	This function looks at the circles that are being drawn
	by the branch with respect to the endpoint with the largest
	radius. If the sum of the overlap of the circles
	is not enough, it means that this branch is a 
	branch that accounts for a substantial part of the object
	and thus we don't want to remove it from the graph.
	The the overlap is less then 40% we return false
*/
bool TreeSkel::PartIsRealLigaturePart(int nEndPoint, int nPntIndex, TreeBranch* pBranch)
{
	bool bRemovable = false;
	int i;
	double dSumNormOverlap = 0;
	double dNormalizedOverlap = 0;
	
	//c = distance between middle circle end1 and middle circle end2
	double x0,x1,y0,y1,r0,r1,c,CBA,CBD,CAB,CAD,overlapArea,areaSmallCircle;
	
	if(nEndPoint == 1) //get the endpoint to compare the overlap with
	{
		x0 = pBranch->end1->x;
		y0 = pBranch->end1->y;
		r0 = pBranch->end1->dRadius;
	}
	else 
	{
		x0 = pBranch->end2->x;
		y0 = pBranch->end2->y;
		r0 = pBranch->end2->dRadius;
	}

// 	cerr << "Radius to compare with:           " << r0 << " (" << x0 << "," << y0 << ")\n\n";
// 	cerr << "Endpoint = " << nEndPoint << endl;
	
	if(nEndPoint == 1) //travers the branch points in normal order upto nPntIndex
	{
//		cerr << "1" << endl;
		for(i = 0; i < nPntIndex; i++)
		{
			ASSERT(i < pBranch->points.Count() && i > -1);

			x1 = pBranch->points[i]->x;
			y1 = pBranch->points[i]->y;
			r1 = pBranch->points[i]->dRadius;
			c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
			if(c <= (r0 + r1)) //in case they overlap
			{
				CBA = acos((r1*r1 + c*c - r0*r0)/(2*r1*c));
				CBD = 2 * CBA;
				CAB = acos((r0*r0 + c*c - r1*r1)/(2*r0*c));
				CAD = 2 * CAB;
				overlapArea = 0.5 * CBD * r1*r1 - 0.5 * r1*r1 * sin(CBD) +
								0.5 * CAD * r0*r0 - 0.5 * r0*r0 * sin(CAD);
				areaSmallCircle = 3.14159*r1*r1;
// 				cerr << "Amount of overlap in circles at the end points: " << overlapArea/areaSmallCircle << endl;
				//sum normalized overlap
				dSumNormOverlap += overlapArea/areaSmallCircle;
// 				cerr << "OVERLAP = " << overlapArea/areaSmallCircle << " for point (" << 
// 					 pBranch->points[i]->i << "," << pBranch->points[i]->j << ")" << endl;
			}
		}
		dNormalizedOverlap = dSumNormOverlap / nPntIndex;

// 		cerr << "THE NORMALIZED OVERLAP = " << dNormalizedOverlap << " for the branch between (" << 
// 		pBranch->end1->i << "," << pBranch->end1->j << ") and (" << pBranch->points[nPntIndex]->i << ","
// 		<< pBranch->points[nPntIndex]->j << ")" << endl;
	}
	else //nEndPoint == 2
	{
// 		DBG_MSG3("2 ",nPntIndex,pBranch->points.Count())
		for(i = nPntIndex + 1; i < pBranch->points.Count() ; i++)
		{
			ASSERT(i < pBranch->points.Count() && i > -1);

			x1 = pBranch->points[i]->x;
			y1 = pBranch->points[i]->y;
			r1 = pBranch->points[i]->dRadius;
			c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
			if(c <= (r0 + r1)) //in case they overlap
			{
				CBA = acos((r1*r1 + c*c - r0*r0)/(2*r1*c));
				CBD = 2 * CBA;
				CAB = acos((r0*r0 + c*c - r1*r1)/(2*r0*c));
				CAD = 2 * CAB;
				overlapArea = 0.5 * CBD * r1*r1 - 0.5 * r1*r1 * sin(CBD) +
								0.5 * CAD * r0*r0 - 0.5 * r0*r0 * sin(CAD);
				areaSmallCircle = 3.14159*r1*r1;
// 				cerr << "Amount of overlap in circles at the end points: " << overlapArea/areaSmallCircle << endl;
				//sum normalized overlap
				dSumNormOverlap += overlapArea/areaSmallCircle;
// 				cerr << "OVERLAP = " << overlapArea/areaSmallCircle << " for point (" << 
// 					 pBranch->points[i]->i << "," << pBranch->points[i]->j << ")" << endl;
			}
		}
		dNormalizedOverlap = dSumNormOverlap / (pBranch->points.Count() - nPntIndex + 1);
		
// 		cerr << "THE NORMALIZED OVERLAP = " << dNormalizedOverlap << " for the branch between (" << 
// 		pBranch->end2->i << "," << pBranch->end2->j << ") and (" << pBranch->points[nPntIndex]->i << ","
// 		<< pBranch->points[nPntIndex]->j << ")" << endl;
	}

	//THRESHOLD for deciding whether the overlap is enough or not
	
	cerr << "Normalized overlap between the two endpoints: " << dNormalizedOverlap << endl;
	
	if(dNormalizedOverlap > 0.7)
		bRemovable = true;

	return bRemovable;
}


/*
	This function looks at the circles that are being drawn
	by the two endpoints of the branch. If the two circles
	don't overlap enough, it means that this branch is a 
	branch that accounts for a substantial part of the object
	and thus we don't want to remove it from the graph.
	If the overlap is less then 40% we return false
*/
bool TreeSkel::branchIsRemovable(TreeBranch* pBranch)
{
	//cerr << "\nIN FUNCTION BRANCH IS REMOVABLE\n\n";
	bool bRemovable = false;
//	int i;
	double dSumNormOverlap = 0;
	double dNormalizedOverlap = 0;
	
	//c = distance between middle circle end1 and middle circle end2
	double x0,x1,y0,y1,r0,r1,c,CBA,CBD,CAB,CAD,overlapArea,areaSmallCircle;
	
	r0 = pBranch->end1->dRadius;
	r1 = pBranch->end2->dRadius;
	if(r0 > r1) //endpoint 1 has the largest radius
	{
		x0 = pBranch->end1->x;
		y0 = pBranch->end1->y;
	}
	else //endpoint 2 has the larget radius
	{
		x0 = pBranch->end2->x;
		y0 = pBranch->end2->y;
		r0 = r1;
	}

	//cerr << "Radius to compare with:           " << r0 << " (" << x0 << "," << y0 << ")\n\n";
	
	//check all the circles that are being drawn by the points
	//on the skeleton branch against the largest circle
// 	for(i = 0; i < pBranch->points.Count(); i++)
// 	{
// 		x1 = pBranch->points[i]->x;
// 		y1 = pBranch->points[i]->y;
// 		r1 = pBranch->points[i]->dRadius;
// 		
// 		c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
// 		
// 		if(c <= (r0 + r1)) //in case they overlap
// 		{
// 			CBA = acos((r1*r1 + c*c - r0*r0)/(2*r1*c));
// 			CBD = 2 * CBA;
// 			CAB = acos((r0*r0 + c*c - r1*r1)/(2*r0*c));
// 			CAD = 2 * CAB;
// 			overlapArea = 0.5 * CBD * r1*r1 - 0.5 * r1*r1 * sin(CBD) +
// 						0.5 * CAD * r0*r0 - 0.5 * r0*r0 * sin(CAD);
// 			areaSmallCircle = 3.14159*r1*r1;
// 			dSumNormOverlap += overlapArea/areaSmallCircle;
// 			//cerr << "OVERLAP = " << overlapArea/areaSmallCircle << " for point (" << 
// 			//	pBranch->points[i]->i << "," << pBranch->points[i]->j << ") " << r1 << endl;
// 		}
// 	}

	//finally add the amount of overlap caused by the two
	//endpoints
	if(pBranch->end1->dRadius > pBranch->end2->dRadius) //check against end2
	{
		x1 = pBranch->end2->x;
		y1 = pBranch->end2->y;
		r1 = pBranch->end2->dRadius;
	}
	else //check against end1
	{
		x1 = pBranch->end1->x;
		y1 = pBranch->end1->y;
		r1 = pBranch->end1->dRadius;
	}

	c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
	
	if(c <= (r0 + r1)) //in case they overlap
	{
		CBA = acos((r1*r1 + c*c - r0*r0)/(2*r1*c));
		CBD = 2 * CBA;
		CAB = acos((r0*r0 + c*c - r1*r1)/(2*r0*c));
		CAD = 2 * CAB;
		overlapArea = 0.5 * CBD * r1*r1 - 0.5 * r1*r1 * sin(CBD) +
					0.5 * CAD * r0*r0 - 0.5 * r0*r0 * sin(CAD);
		areaSmallCircle = 3.14159*r1*r1;
		dSumNormOverlap += overlapArea/areaSmallCircle;
	}

	//get the normalization of the summed up overlap
// 	dNormalizedOverlap = dSumNormOverlap / (pBranch->points.Count() + 1);

	dNormalizedOverlap = dSumNormOverlap; 
	//THRESHOLD for deciding whether the overlap is enough or not
	cerr << "THE NORMALIZED OVERLAP = " << dNormalizedOverlap << " for the branch between (" << 
		pBranch->end1->i << "," << pBranch->end1->j << ") and (" << pBranch->end2->i << ","
		<< pBranch->end2->j << ")" << endl;
	if(dNormalizedOverlap > 0.75)
		bRemovable = true;

//################################################################################ ADDED ON JANUARI 04

// 	bool bOneShock = false, bFound = false;
// 	int nEndLigature = 0;
// 	
// 	//convert the TreeSkel Points in the branch into a smartarray of POINTs 
// 	//starting from the connection with the phantom branch
// 	vpl::POINTS convertedBranch = ConvertBranchIntoPoints(pBranch, 1);
// 	
// 	//time to fit the convertedBranch with a few lines
// 	vpl::ApproxPoly poly(convertedBranch.GetSize()/dMinError,dMinSlope,10.0);
// 	
// 	//poly.SetDbgMode(true);
// 	poly.Fit(convertedBranch);
// 	int nSize = poly.m_knots.GetSize();
// 	ASSERT(nSize >= 1);
// 	
// 	if(nSize == 1)
// 		bOneShock = true;
// 	
// 	if(nSize > 1)
// 	{
// 		for(i = 1; i < nSize && !bFound; i++)
// 		{
// 			nEndLigature = i-1;
// 			double m0 = poly.m_knots[i - 1].seg.m;
// 			double m1 = poly.m_knots[i].seg.m;
// 			double dMax = (fabs(m0) >= fabs(m1))? m0 : m1;
// 			double mr = fabs(m1 - m0) / fabs(dMax);
// 			//cout << "disp('Acc change: " << mr << ", Max:" << dMaxAccelChg << "');\n";
// 			if (mr >= dMaxAccelChg )
// 			{
// 				bFound = true;
// 			}
// 		}
// 	}
// 	
// 	if(nSize > 1 && nEndLigature + 1 == nSize)
// 		bOneShock = true;
// 	
// 	if(!bOneShock)
// 		bRemovable = false;

//###############################################################################################
		
	return bRemovable;
}


/*
	Returns the index where the smallest inside branch is
	located, or -1 if there is no inside branch anymore
*/
int TreeSkel::getSmallestInsideBranch()
{
	double dSmallestInsideBranch = 1000000;
	int i;
	int nIndex = -1;
	
	for(i = 0; i < branches.Count(); i++)
		if(branches[i]->internal() == 1)
		{
			if((branchIsRemovable(branches[i]) == true) && (branches[i]->geomLength() < dSmallestInsideBranch))
			{
				dSmallestInsideBranch = branches[i]->geomLength();
				nIndex = i;
			}
		}
	
	return nIndex;
}


/*
	returns the index of the treepoint
	in the geometric middle of the the branch
*/
int TreeSkel::getIndexMidPoint(int nIndex)
{
	int nGeomIndex = -1,i;
	double nlastx = branches[nIndex]->end1->i, nlasty = branches[nIndex]->end1->j;
	double dDistance = 0;
	
	for(i = 0; i < branches[nIndex]->points.Count() && dDistance < branches[nIndex]->geomLength()/2; i++)
	{
		dDistance += sqrt((nlastx - branches[nIndex]->points[i]->i)*(nlastx - branches[nIndex]->points[i]->i)+
				(nlasty - branches[nIndex]->points[i]->j)*(nlasty - branches[nIndex]->points[i]->j));
		nlastx = branches[nIndex]->points[i]->i;
		nlasty = branches[nIndex]->points[i]->j;
		nGeomIndex = i;
	}
	
	ASSERT(nGeomIndex >= 0 && nGeomIndex < branches[nIndex]->points.Count());
	
	return nGeomIndex;
}


/*
	Given an index to branch (that will be removed)
	determine what point we will use to merge all neighbor
	branches to.
	
	The return pair <bool,int> will hold:
	true if it is an endpoint and the int indicating which endpoint
	false if it is a treepoint and the int indicating its position in DARRAY points
*/
std::pair<bool,int> TreeSkel::determineMergePnt(int nIndex)
{
	int i;
	std::pair<bool,int> MergePoint;
	MergePoint.first = false;
	MergePoint.second = -1;
	double dSizeLrgTreePnt = 0;
	int nPosOfTreePnt = 0;
	
	double dSizeLrgEndpnt = (branches[nIndex]->end1->dRadius > branches[nIndex]->end1->dRadius)? branches[nIndex]->end1->dRadius:branches[nIndex]->end2->dRadius;
	double dSizeSmallestEndpnt = (branches[nIndex]->end1->dRadius > branches[nIndex]->end1->dRadius)? branches[nIndex]->end2->dRadius:branches[nIndex]->end1->dRadius;
	
	for(i = 0; i < branches[nIndex]->points.Count(); i++)
		if(branches[nIndex]->points[i]->dRadius > dSizeLrgTreePnt)
		{
			dSizeLrgTreePnt = branches[nIndex]->points[i]->dRadius;
			nPosOfTreePnt = i;
		}
	
	double dPercentage = fabs(dSizeLrgTreePnt - dSizeLrgEndpnt) / dSizeLrgTreePnt;
	//cerr << "Percentage in DetermineMergePoint: " << dPercentage << endl;
	
	//in case we are going to merge in a treepoint with the largest radius
	if(dSizeLrgTreePnt > dSizeLrgEndpnt && dPercentage >= 0.25) 
		MergePoint.second = nPosOfTreePnt;
	else //check whether we are going to take an endpoint or the middle of the branch
	{
		dPercentage = fabs(dSizeLrgEndpnt - dSizeSmallestEndpnt) / dSizeLrgEndpnt;
		//cerr << "Percentage in DetermineMergePoint 2 : " << dPercentage << endl;
		if(dPercentage > 0.05) //we take the endpoint with the largest radius
		{
			MergePoint.first = true;
			MergePoint.second = (branches[nIndex]->end1->dRadius > branches[nIndex]->end2->dRadius)?1:2;
		}
		else // dPercentage <= 0.05 so the endpoints are similar in their radius
		{
			if(branches[nIndex]->points.Count() < 5) //no use in searching for the middle
			{
				MergePoint.first = true;
				MergePoint.second = (branches[nIndex]->end1->dRadius > branches[nIndex]->end2->dRadius)?1:2;
			}
			else
			{
				MergePoint.second = getIndexMidPoint(nIndex);
				ASSERT(MergePoint.second < branches[nIndex]->points.Count());
			}
		}
	}
	
	ASSERT(MergePoint.second != -1);

	return MergePoint;
}



/*
	Given a branch that will be removed from the graph and an endpoint that will be the new mergepoint
	for the branhces attached, this function will return a list of branches that need to be
	updated. The branches that are attached to the endpoint which will be the new mergepoint don't
	need to be updated. Only the branches attached to the other side
*/
DARRAY<TreeSkel::TreeBranch*> TreeSkel::getBrTBUpdatedWithEndPnt(TreeBranch* pPhantom,int mergePoint)
{
	int i;
	TreePoint* pNonMergePoint = (mergePoint == 1)?pPhantom->end2:pPhantom->end1;
	DARRAY<TreeBranch*> list;
	
	for(i = 0; i < pNonMergePoint->branches.Count(); i++)
	{
		if(pNonMergePoint->branches[i] != pPhantom)
			list.Add(pNonMergePoint->branches[i]);
	}
	
	ASSERT(list.Count() == pNonMergePoint->branches.Count()-1);
	
	return list;
}




int TreeSkel::PhantomIsNb(const TreeBranch& branch, const TreeBranch& phantom) const
{
	int i;
	int nIsNeighbor = 0;
	for(i = 0; i < branch.end1->branches.Count(); i++)
		if(*branch.end1->branches[i] == phantom)
			nIsNeighbor = 1;
	for(i = 0; i < branch.end2->branches.Count(); i++)
		if(*branch.end2->branches[i] == phantom)
			nIsNeighbor = 2;
	return nIsNeighbor;
}

/*
	We find the point in a given part of a branch (upto nPntIndex) that will
	approximate dLigatureSlope when we attach that point to pNewEndPointForBranch
*/
int TreeSkel::getBestPntIndexForSlope(TreeBranch* pBrnchTofix, int nPntIndex, int nEndPoint, TreePoint* pNewEndPointForBranch, double dLigatureSlope)
{
	//info about the new endpoint
	double x0 = pNewEndPointForBranch->x,
		y0 = pNewEndPointForBranch->y,
		r0 = pNewEndPointForBranch->dRadius,
		bestdist = 1000000, dist =  0, slope = 0,
		x1 = 0, y1 = 0, r1 = 0, c = 0;
	int i,result = nPntIndex;

// 	cerr << "\nBRANCH TO FIX: " << *pBrnchTofix << endl;
// 	cerr << "GET BEST INDEX FOR SLOPE INITIAL: " << nPntIndex << endl;
// 	cerr << "AND THE INITIAL SLOPE:            " << dLigatureSlope << endl;
	
	if(nEndPoint == 1)
	{
		for(i = 0; i < nPntIndex; i++)
		{
			x1 = pBrnchTofix->points[i]->x;
			y1 = pBrnchTofix->points[i]->y;
			r1 = pBrnchTofix->points[i]->dRadius;

			c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));

			slope = (r1 - r0) / c;
// 			cerr << "found slope:       " << slope << endl;
			dist = fabs(dLigatureSlope - slope);
			if(dist < bestdist)
			{
				result = i;
				bestdist = dist;
			}
		}
	}
	else
	{
		for(i = pBrnchTofix->points.Count() - 1; i > nPntIndex; i--)
		{
			x1 = pBrnchTofix->points[i]->x;
			y1 = pBrnchTofix->points[i]->y;
			r1 = pBrnchTofix->points[i]->dRadius;

			c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));

			slope = (r1 - r0) / c;
// 			cerr << "found slope:       " << slope << endl;
			dist = fabs(dLigatureSlope - slope);
			if(dist < bestdist)
			{
				result = i;
				bestdist = dist;
			}
		}
	}
	
// 	cerr << "GET BEST INDEX FOR SLOPE AFTER CALCULATION: " << result << endl;
	
	return result;
}



/*
	This function will "fix" a given branch. That means that it will
	be disconnected from the phantom branch, a part of the branch will
	be cut off (the ligature part) and it will be attached to the new
	(existing) endpoint.
*/
void TreeSkel::fixBrnchToExistEnd(TreeBranch* pBrnchToFix, TreeBranch* pPhantom, int mergePoint)
{
	int i;
	int nEndLigature = 0;
	bool bFound = false;
	
	//the new endpoint for the branch to be fixed:
	TreePoint* pNewEndPointForBranch = (mergePoint==1)?pPhantom->end1:pPhantom->end2;
	
	//find the endpoint of the pBrnchToFix that is attached to the phantom
	int nEndPoint = PhantomIsNb(*pBrnchToFix, *pPhantom);
	ASSERT((nEndPoint == 1) || (nEndPoint == 2));
	
	//convert the TreeSkel Points in the branch into a smartarray of POINTs 
	//starting from the connection with the phantom branch
	vpl::POINTS convertedBranch = ConvertBranchIntoPoints(pBrnchToFix, nEndPoint);
	
	//time to fit the convertedBranch with a few lines
	vpl::PolyLineApprox poly(convertedBranch.GetSize()/dMinError,dMinSlope,10, 0.3);
	
	//poly.SetDbgMode(true);
	poly.Fit(convertedBranch);
	int nSize = poly.m_knots.GetSize();
	ASSERT(nSize >= 1);
	
	//get the slope of the "extra" part non-ligature
	//the first slope is of the ligature part, the second of the non ligature part
	//which we want to put on the "extra" non ligature part too
	double dSlope = 0,dLigatureSlope = 0;
	
// 	for(i = 0; i < nSize; i++)
// 		cerr << "Slope of line segment: " << poly.m_knots[i].seg.m << endl;
	
	if(nSize == 1)	//slope of only segment
	{
		dSlope = poly.m_knots[0].seg.m;
		dLigatureSlope = poly.m_knots[0].seg.m;
	}
	else	//slope of segment next to the ligature segment
	{
		for(i = 1; i < nSize && !bFound; i++)
		{
			dSlope = poly.m_knots[i].seg.m;
			dLigatureSlope = poly.m_knots[i-1].seg.m;
			nEndLigature = i-1;
			double m0 = poly.m_knots[i - 1].seg.m;
			double m1 = poly.m_knots[i].seg.m;
			double dMax = (fabs(m0) >= fabs(m1))? m0 : m1;
			double mr = fabs(m1 - m0) / fabs(dMax);
// 			cerr << "Slope 1: " << m0 << " slope 2: " << m1 << " disp('Acc change: " << mr << ", Max:" << dMaxAccelChg << "');\n";
			if (mr >= dMaxAccelChg )
			{
				bFound = true;
			}
		}
	}	
	
	int nPntIndex = GetPntIndexBeginLigature(pBrnchToFix, poly.m_knots[nEndLigature].seg.p1.x, nEndPoint);

	//check whether the part we want to cut of is really a ligature part... The slope of the line
	//should be at least negative. Secondly the two circles at the begin and the end of the ligature
	//part should definitely overlap. If now we are not going to cut it off.
	cerr << "Slope of linesegment we will cut off: " << dLigatureSlope << endl;
DBG_SHOW(dLigatureSlope);
	if(dLigatureSlope < -0.1 && PartIsRealLigaturePart(nEndPoint, nPntIndex, pBrnchToFix) && (nSize != 1))
	{
		//transfer/shift/displace the branch
		DisplaceBranch(pBrnchToFix,nPntIndex,nEndPoint,pNewEndPointForBranch);
		//assign radius values
		ReAssignRadiusValues(pBrnchToFix,nEndPoint,pNewEndPointForBranch,dSlope);
	}
	else //the part we want to cut of is not really a ligature part
	{
		//cerr << "We decided not to cut of the part of the branch... " << endl;
		//we are now going to look for the point which if we use that one to
		//cut of the branch the slope of the new part is as close to the
		//slope it already has as possible, so we won't create new shockparts
		nPntIndex = getBestPntIndexForSlope(pBrnchToFix, nPntIndex, nEndPoint, pNewEndPointForBranch, dLigatureSlope);
		//nPntIndex = (nEndPoint == 1)? 0 : pBrnchToFix->points.Count()-1;
		//transfer/shift/displace the branch
		DisplaceBranch(pBrnchToFix,nPntIndex,nEndPoint,pNewEndPointForBranch);
		//assign radius values
		ReAssignRadiusValues(pBrnchToFix,nEndPoint,pNewEndPointForBranch,dSlope);
	}
}

/*
	Given a pPhantom branch that will be removed from the skeleton
	this function will return a list with ALL branches that are
	attached to the phantom branch. This, because we are going to
	merge its neighbors onto a new endpoint (in case this function
	is called that is)
*/
DARRAY<TreeSkel::TreeBranch*> TreeSkel::getBrTBUpdatedWithoutEndPnt(TreeBranch* pPhantom)
{
	int i;
	DARRAY<TreeBranch*> list;
	
	//add all the neighbors of endpoint 1 except the phantom itself
	for(i = 0; i < pPhantom->end1->branches.Count(); i++)
	{
		if(pPhantom->end1->branches[i] != pPhantom)
			list.Add(pPhantom->end1->branches[i]);
	}
	
	//add all the neighbors of endpoint 2 except the phantom itself
	for(i = 0; i < pPhantom->end2->branches.Count(); i++)
	{
		if(pPhantom->end2->branches[i] != pPhantom)
			list.Add(pPhantom->end2->branches[i]);
	}
		
/*	
DBG_MSG1("Count1",pPhantom->end1->branches.Count());
DBG_MSG1("Count2",pPhantom->end2->branches.Count());
DBG_MSG1("List",list.Count());

cerr << "Phantom:    " << *pPhantom << endl << endl;
for(i = 0; i < pPhantom->end1->branches.Count(); i++)
	cerr << "\t" << *pPhantom->end1->branches[i] << endl;
cerr << endl;
for(i = 0; i < pPhantom->end2->branches.Count(); i++)
	cerr << "\t" << *pPhantom->end2->branches[i] << endl;

 
for(i = 0; i < branches.Count(); i++)
{
	cerr << *branches[i] << endl;
}*/

	
	ASSERT(list.Count() == (pPhantom->end1->branches.Count() + pPhantom->end2->branches.Count() - 2));

	//ASSERT(list.Count() <= (pPhantom->end1->branches.Count() + pPhantom->end2->branches.Count() - 2))	

	return list;
}


/*
	This function will "fix" a given branch. That means that it will
	be disconnected from the phantom branch, a part of the branch will
	be cut off (the ligature part) and it will be attached to a new 
	endpoint. This endpoint is passed through as a parameter.
*/
void TreeSkel::fixBrnchToNonExistEnd(TreeBranch* pBrnchToFix, TreeBranch* pPhantom, TreePoint* pNewEndPnt)
{
	int i;
	int nEndLigature = 0;
	bool bFound = false;
	
	//the new endpoint for the branch to be fixed:
	TreePoint* pNewEndPointForBranch = pNewEndPnt;
	
	//find the endpoint of the pBrnchToFix that is attached to the phantom
	int nEndPoint = PhantomIsNb(*pBrnchToFix, *pPhantom);
	ASSERT((nEndPoint == 1) || (nEndPoint == 2));
	
	//convert the TreeSkel Points in the branch into a smartarray of POINTs 
	//starting from the connection with the phantom branch
	vpl::POINTS convertedBranch = ConvertBranchIntoPoints(pBrnchToFix, nEndPoint);
	
	//time to fit the convertedBranch with a few lines
	vpl::PolyLineApprox poly(convertedBranch.GetSize()/dMinError,dMinSlope,10, 0.3);
	
	//poly.SetDbgMode(true);
	poly.Fit(convertedBranch);
	int nSize = poly.m_knots.GetSize();
	ASSERT(nSize >= 1);
	
	//get the slope of the "extra" part non-ligature
	//the first slope is of the ligature part, the second of the non ligature part
	//which we want to put on the "extra" non ligature part too
	double dSlope = 0,dLigatureSlope = 0;
	
	if(nSize == 1)	//slope of only segment
	{
		dSlope = poly.m_knots[0].seg.m;
		dLigatureSlope = poly.m_knots[0].seg.m;
	}
	else	//slope of segment next to the ligature segment
	{
		for(i = 1; i < nSize && !bFound; i++)
		{
			dSlope = poly.m_knots[i].seg.m;
			dLigatureSlope = poly.m_knots[i-1].seg.m;
			nEndLigature = i-1;
			double m0 = poly.m_knots[i - 1].seg.m;
			double m1 = poly.m_knots[i].seg.m;
			double dMax = (fabs(m0) >= fabs(m1))? m0 : m1;
			double mr = fabs(m1 - m0) / fabs(dMax);
// 			cerr << "disp('Acc change: " << mr << ", Max:" << dMaxAccelChg << "');\n";
			if (mr >= dMaxAccelChg )
			{
				bFound = true;
			}
		}
	}	
	
	int nPntIndex = GetPntIndexBeginLigature(pBrnchToFix, poly.m_knots[nEndLigature].seg.p1.x, nEndPoint);

	//check whether the part we want to cut of is really a ligature part... The slope of the line
	//should be at least negative. Secondly the two circles at the begin and the end of the ligature
	//part should definitely overlap. If not we are not going to cut it off.
	//cerr << "Ligature Slope " << dLigatureSlope << endl;
	
	cerr << "Slope of linesegment we will cut off: " << dLigatureSlope << endl;
DBG_SHOW(dLigatureSlope);
	
	if(dLigatureSlope < -0.1 && PartIsRealLigaturePart(nEndPoint,nPntIndex,pBrnchToFix))
	{
		//transfer/shift/displace the branch
		DisplaceBranch(pBrnchToFix,nPntIndex,nEndPoint,pNewEndPointForBranch);
		//assign radius values
		ReAssignRadiusValues(pBrnchToFix,nEndPoint,pNewEndPointForBranch,dSlope);
	}
	else //the part we want to cut of is not really a ligature part
	{
		//cerr << "We decided not to cut of the part of the branch... " << endl;
		//we are now going to look for the point which if we use that one to
		//cut of the branch the slope of the new part is as close to the
		//slope it already has as possible, so we won't create new shockparts
		nPntIndex = getBestPntIndexForSlope(pBrnchToFix, nPntIndex, nEndPoint, pNewEndPointForBranch, dLigatureSlope);
		//nPntIndex = (nEndPoint == 1)? 0 : pBrnchToFix->points.Count()-1;
		//transfer/shift/displace the branch
		DisplaceBranch(pBrnchToFix,nPntIndex,nEndPoint,pNewEndPointForBranch);
		//assign radius values
		ReAssignRadiusValues(pBrnchToFix,nEndPoint,pNewEndPointForBranch,dSlope);
	}
}

/*
	When this funtion is called it tries to remove the smallest inside branch
	in the skeleton. Tries, because there might not be a smallest inside branch.
	It will return a boolean value saying whether it has or has not removed
	an inside branch and additionally an integer indicating the lenght of
	the inside branch that was removed.
	
*/
std::pair<bool,double> TreeSkel::performNextSimplification()
{
	int i;
	std::pair<bool,double> SucceededAndLength;
	SucceededAndLength.first = false;
	SucceededAndLength.second = -1;
	//get smallest inside branch
	int nIndex = getSmallestInsideBranch();
	DARRAY<TreeBranch*> branchesTBUpdated;

	int nPhantomRemoved = 0;
	
	if(nIndex != -1) //means we have found the shortest inside branch that will be removed
	{
		SucceededAndLength.first = true;
		SucceededAndLength.second = branches[nIndex]->geomLength();
		std::pair<bool,int> mergePoint = determineMergePnt(nIndex);
		if(mergePoint.first == true) //we are merging in an endpoint so we do not change all attached branches
		{
			branchesTBUpdated = getBrTBUpdatedWithEndPnt(branches[nIndex],mergePoint.second);
			for(i = 0; i < branchesTBUpdated.Count(); i++)
				fixBrnchToExistEnd(branchesTBUpdated[i],branches[nIndex],mergePoint.second);
			
			//remove the reference to the phantom branch from the "new" endpoint
			TreePoint* pNewEndPoint = (mergePoint.second==1)?branches[nIndex]->end1:branches[nIndex]->end2;
			nPhantomRemoved = pNewEndPoint->branches.Remove(branches[nIndex]);
			
			//check if the phantom was succesfully removed
			ASSERT(nPhantomRemoved != -1);
			
			//remove the phantom branch from the DARRAY branches
			delete branches[nIndex];
			branches.Delete(nIndex);
			
			cerr << "\nBranches have been updated onto an existing endpoint. Structural simplification is done\n";
		}
		else //we are not merging in on of the endpoints
		{
			branchesTBUpdated = getBrTBUpdatedWithoutEndPnt(branches[nIndex]);
			
			//create a new endpoint for the branches to be fixed
			const TreePoint* pInfo = branches[nIndex]->points[mergePoint.second];
			Coord c(pInfo->i,pInfo->j);
			TreePoint* pNewEndPnt = new TreePoint(c,BRANCH);
			MemoryLeakRemoval.Add(pNewEndPnt);
			pNewEndPnt->x = pInfo->x;
			pNewEndPnt->y = pInfo->y;	
			pNewEndPnt->dRadius = pInfo->dRadius;
			pNewEndPnt->flag = pInfo->flag;
			pNewEndPnt->K = pInfo->K;
			pNewEndPnt->len0 = pInfo->len0; 
			
			for(i = 0; i < branchesTBUpdated.Count(); i++)
				fixBrnchToNonExistEnd(branchesTBUpdated[i],branches[nIndex],pNewEndPnt);
			
			//remove the phantom branch from the DARRAY branches
			delete branches[nIndex];
			branches.Delete(nIndex);
			
			cerr << "\nBranches have been updated onto a non-existing endpoint. Structural simplificatoin is done\n";
		}
	}
	
	return SucceededAndLength;
}


/*
	This function will make 1 continuous branch out of the
	two given branches. (Glue them together)
*/
void TreeSkel::glueBranches(TreeBranch* pBr1, TreeBranch* pBr2)
{
	int i;
	//branch 1 is going to be the branch that will substitute
	//branch 1 and 2
	//STEP 1: change the common endpoint of branch 1 to the non-common
	//endpoint of branch2

	int nEnd1 = pBr2->end1->branches.Contains(pBr1);
	int nEnd2 = pBr2->end2->branches.Contains(pBr1);
	
	// Check if we have a circular branch
	bool bIsCycle = !((nEnd1 == -1 && nEnd2 != -1) || (nEnd1 != -1 && nEnd2 == -1));
	if(bIsCycle)
	{
		WARNING(bIsCycle, "We are removing a circular branch");
		//int succes = branches.Remove(pBr2);
		//ASSERT(succes != -1)
		//delete pBr2;
		return;
	}
	
	ASSERT((nEnd1 == -1 && nEnd2 != -1) || (nEnd1 != -1 && nEnd2 == -1));
	
	int nCommonForBr2 = (nEnd1 != -1)? 1 : 2;
	int nCommonForBr1 = (pBr1->end1->branches.Contains(pBr2) != -1)? 1: 2;
	
	DARRAY< TreePoint* > intermediate;
	int succes = -1;
	
	if(nCommonForBr1 == 1) //common endpoint for branch 1 was end1
	{
		//Assign "extended" endpoint
		pBr1->end1 = (nCommonForBr2 == 1)? pBr2->end2 : pBr2->end1;
		//add ref to pBr1
		pBr1->end1->branches.Append(pBr1);
		//remove ref to pBr2
		ASSERT( pBr1->end1->branches.Remove(pBr2) != -1);
		if(nCommonForBr2 == 1)
		{
			for(i = pBr2->points.Count() - 1; i > -1; i--)
				intermediate.Add(pBr2->points[i]);
			//add the common endpoint which now is just a branch point
			intermediate.Add(pBr2->end1);
		}
		else //nCommonForBr2 == 2
		{
			for(i = 0; i < pBr2->points.Count(); i++)
				intermediate.Add(pBr2->points[i]);
			//add the common endpoint 
			intermediate.Add(pBr2->end2);
		}
		for(i = 0; i < pBr1->points.Count(); i++)
			intermediate.Add(pBr1->points[i]);
			
		//intermediate now stores the entire new branch
		pBr1->points = intermediate;
		
		//delete the branch 2 that now is part of branch 1
		succes = branches.Remove(pBr2);
		ASSERT(succes != -1);
		delete pBr2;
	}
	else //common endpoint for branch 1 was end2
	{
		pBr1->end2 = (nCommonForBr2 == 1)? pBr2->end2 : pBr2->end1;
		//add ref to pBr1
		pBr1->end2->branches.Append(pBr1);
		//remove ref to pBr2
		ASSERT( pBr1->end2->branches.Remove(pBr2) != -1);
		for(i = 0; i < pBr1->points.Count(); i++)
			intermediate.Add(pBr1->points[i]);
		//Assign "extended" endpoint
		if(nCommonForBr2 == 1)
		{
			//add the common endpoint which now is just a branch point
			intermediate.Add(pBr2->end1);
			for(i = 0; i < pBr2->points.Count(); i++)
				intermediate.Add(pBr2->points[i]);
		}
		else //nCommonForBr2 == 2
		{
			//add the common endpoint
			intermediate.Add(pBr2->end2);
			for(i = pBr2->points.Count() - 1; i > -1; i--)
				intermediate.Add(pBr2->points[i]);
		}	
		//intermediate now stores the entire new branch
		pBr1->points = intermediate;
		
		//delete the branch 2 that now is part of branch 1
		succes = branches.Remove(pBr2);
		ASSERT(succes != -1);
		delete pBr2;
	}
}


/*
	Given a branch index in the DARRAY branches which
	indicates an outside branch we want to remove, this
	function will make sure it is removed and the resulting
	skeleton is sound
*/
void TreeSkel::RemoveOutsideBranch(DARRAY< int > nIndices)
{
	int nBrRefRem1 = -1, nBrRefRem2 = -1;
	TreeBranch *TBRem, *pBr1, *pBr2;
	TreePoint* pEndPoint;
	int i;
	
// 	cerr << "Number of branches initial: " << branches.Count() << endl;
// 	cerr << "Number of outside branches we want to remove: " 
// 		<< nIndices.Count() << endl;

	cerr << "NUMBER OF BRANCHES WE WANT TO REMOVE: " << nIndices.Count() << endl;
	
	if(nIndices.Count() == branches.Count())
		return;
// 	cerr << "\nbefore\n";
// 	for(i = 0; i < branches.Count(); i++)
// 	{
// 		
// 		cerr << *branches[i] << endl;
// 	}
		
	//remove all the outside branches first
	for(i = nIndices.Count() - 1; i > -1; i--)
	{
//  		cerr << "Deleting branch with index: " << nIndices[i] << endl;
		//delete references to the branch from it's endpoints
		TBRem = branches[nIndices[i]];
// 		cerr << "End1 has nbs: " << TBRem->end1->branches.Count();
		nBrRefRem1 = TBRem->end1->branches.Remove(TBRem);
// 		cerr << " and now: " << TBRem->end1->branches.Count() << endl;
// 		cerr << "End2 has nbs: " << TBRem->end2->branches.Count();
		nBrRefRem2 = TBRem->end2->branches.Remove(TBRem);
// 		cerr << " and now: " << TBRem->end2->branches.Count() << endl;
		ASSERT(nBrRefRem1 != -1 && nBrRefRem2 != -1);
		
		//delete actual branch
		delete branches[nIndices[i]];
		branches.Delete(nIndices[i]);
	}
// 	cerr << "\nafter deleting outside branches\n";
// 	for(i = 0; i < branches.Count(); i++)
// 	{
// 		
// 		cerr << *branches[i] << endl;
// 	}
	
// 	cerr << "Number of branches when the outside branches are deleted: " 
// 		<< branches.Count() << endl;
// 	
	//fix graph structure; i.e. branch outs of 2
	
	DARRAY< TreePoint* > EndsToFix;
	for(i = 0; i < branches.Count(); i++)
	{
		if(branches[i]->end1->branches.Count() == 2)
			EndsToFix.Append(branches[i]->end1);
		if(branches[i]->end2->branches.Count() == 2)
			EndsToFix.Append(branches[i]->end2);
	}
	
// 	cerr << "Number of endpoints found: " << EndsToFix.Count() << endl;
	
	for(i = 0; i < EndsToFix.Count(); i++)
	{
		pEndPoint = EndsToFix[i];
// 		cerr << "\ninside loop" << i << endl;
		if(pEndPoint->branches.Count() == 2)
		{
			pBr1 = pEndPoint->branches[0];
			pBr2 = pEndPoint->branches[1];
			glueBranches(pBr1, pBr2);
// 			cerr << "\nafter a glue\n";
// 			for(j = 0; j < branches.Count(); j++)
// 			{
// 				
// 				cerr << *branches[j] << endl;
// 			}
		}
	}
	

	
// 	cerr << "Number of branches after glueing: " << branches.Count() << endl;
// 	
	for(i = 0; i < branches.Count(); i++)
	{
		//cerr << *branches[i] << endl;
		if(branches[i]->end1->branches.Count() == 1)
			branches[i]->end1->type = END;
		if(branches[i]->end2->branches.Count() == 1)
			branches[i]->end2->type = END;
		if(branches[i]->end1->branches.Count() > 2)
			branches[i]->end1->type = BRANCH;
		if(branches[i]->end2->branches.Count() > 2)
			branches[i]->end2->type = BRANCH;
		branches[i]->computeLength();
	}

	
// 	if(branches[nIndex]->end1->branches.Count() == 3 || branches[nIndex]->end2->branches.Count() == 3)
// 	{
// 		if(branches[nIndex]->end1->branches.Count() == 3)
// 		{
// 			//remove the pointer to this branch from the endpoint
// 			nBranchRefRemoved = branches[nIndex]->end1->branches.Remove(branches[nIndex]);
// 			ASSERT(nBranchRefRemoved != -1)
// 			
// 			TreeBranch* pBr1 = branches[nIndex]->end1->branches[0];
// 			TreeBranch* pBr2 = branches[nIndex]->end1->branches[1];
// 			glueBranches(pBr1, pBr2);
// 		}
// 		
// 		if(branches[nIndex]->end2->branches.Count() == 3)
// 		{
// 			//remove the pointer to this branch from the endpoint
// 			nBranchRefRemoved = branches[nIndex]->end2->branches.Remove(branches[nIndex]);
// 			ASSERT(nBranchRefRemoved != -1)
// 			
// 			TreeBranch* pBr1 = branches[nIndex]->end2->branches[0];
// 			TreeBranch* pBr2 = branches[nIndex]->end2->branches[1];
// 			glueBranches(pBr1, pBr2);
// 		}
// 	}
// 	else //the endpoint that is a branch point has more then 2 other branches attached to it, so we only need
// 		//to remove the reference to this branch from that endpoint (no glueing)
// 	{
// 		if(branches[nIndex]->end1->branches.Count() > 3)
// 		{
// 			//remove the pointer to this branch from the endpoint
// 			nBranchRefRemoved = branches[nIndex]->end1->branches.Remove(branches[nIndex]);
// 			ASSERT(nBranchRefRemoved != -1)
// 		}
// 		
// 		if(branches[nIndex]->end2->branches.Count() > 3)
// 		{
// 			//remove the pointer to this branch from the endpoint
// 			nBranchRefRemoved = branches[nIndex]->end2->branches.Remove(branches[nIndex]);
// 			ASSERT(nBranchRefRemoved != -1)
// 		}
// 	}
// 	//in the last step the branch was disconnected from the graph
// 	//its points will be deleted when this object is being destroyed
// 	//so we don't have to do that. We just delete the reference to this
// 	//branch from the DARRAY branches
// 	delete branches[nIndex];
// 	branches.Delete(nIndex);
}

/*
	This function will simplify the skeleton, by removing
	all branches that have a length that is shorter or equal
	to dLength.
	When a branch is being removed, the remaining branches are
	stitched together while minimizing the change in curvature
	of those branches. This is done by representing the points
	of the branches as Laplacian coordinates.
*/
void TreeSkel::simplifyLaplacian(double dLength)
{
	//find all inside branches that should be removed with appropriate neighborhoods
	std::pair< std::vector< std::pair<int,int> >, int> TBRemovedInfo = getInsideBranchesToBeRemoved(dLength);
	std::vector< std::pair<int,int> > branchesTBRemoved = TBRemovedInfo.first;
	int nNumberOfNeighborhoods = TBRemovedInfo.second;
	DARRAY<int> NeighborhoodNumbering = GetListOfNumberingNbhs(branchesTBRemoved);
	
	ASSERT(NeighborhoodNumbering.Count() == nNumberOfNeighborhoods);
	
	DARRAY<int> IndicesOfPhantoms;
	int i,j;
	for(i = 0; i < nNumberOfNeighborhoods; i++)
	{
		int nNbhNumber = NeighborhoodNumbering[i];
		//get a list of the branches in neighborhood i
		std::vector<int> listOfBranchesInNbh = GetListOfBrInNbh(branchesTBRemoved,nNbhNumber);
		//find the endpoint with the largest radius
		std::pair<int,int> branchEndpointLargestRadius = GetPntWithLargestRadius(listOfBranchesInNbh);
		DARRAY<TreeBranch*> branchesTBUpdated;
		if(branchEndpointLargestRadius.second != -1) //"easiest" case, we can use an existing endpoint
		{
			//we have an endpoint that will remain, find all branches that are not
			//attached to the remaining endpoint, because these need to be changed
			branchesTBUpdated = GetListOfBrTBUpdatedWithEndPnt(listOfBranchesInNbh, branchEndpointLargestRadius);
			for(j = 0; j < branchesTBUpdated.Count(); j++)
				fixBranch(branchesTBUpdated[j],listOfBranchesInNbh,branchEndpointLargestRadius);
		}
		else //TODO, update new endpoint
		{
			//given the list of branches in the neighborhood (which are the ones that
			//need to be removed) find all branches that are attached to them 
			cerr << "\nTODO TODO TODO TODO TODO TODO\n";
			branchesTBUpdated = GetListOfBrTBUpdated(listOfBranchesInNbh);
			//UPDATE NEW ENDPOINT
		}
		//Remove the reference to the phantom from the "new endpoint"
		//TODO
		TreePoint* pNewEndPoint = NULL;
		if(branchEndpointLargestRadius.second == 1)
			pNewEndPoint = branches[branchEndpointLargestRadius.first]->end1;
		else
			pNewEndPoint = branches[branchEndpointLargestRadius.first]->end2;
		unsigned int k;
		
		for(k = 0; k < listOfBranchesInNbh.size(); k++)
		{
			IndicesOfPhantoms.AddOrd(listOfBranchesInNbh[k]);
			pNewEndPoint->branches.Remove(branches[listOfBranchesInNbh[k]]);
		}
	}
	for(i = IndicesOfPhantoms.Count()-1; i > -1; i--)
		branches.Delete(IndicesOfPhantoms[i]);

		
	for(i = 0; i < branches.Count(); i++)
	{
		cerr << *branches[i] << endl;
		cerr << "\t" << *branches[i]->points[0] << endl;
		cerr << "\t" << *branches[i]->points[1] << endl;
		cerr << "\t" << *branches[i]->points[2] << endl;
		cerr << "\t" << *branches[i]->points[branches[i]->points.Count()-1] << endl;
		cerr << "\t" << *branches[i]->points[branches[i]->points.Count()-2] << endl;
		cerr << "\t" << *branches[i]->points[branches[i]->points.Count()-3] << endl;
	}
	
	cerr << "\nStructural simplification is ready\n";
}



void TreeSkel::simplify(float len,float smooth_fact,int maxiter,int internal_only)
								//Simplify skeleton by shortening its shortest branch
{								//One can choose to simplify internal branches only, or all branches
   int i;

//    for(i = 0; i < branches.Count(); i++)
//    {
//    	cerr << "Number of points inside the branch: " << branches[i]->points.Count() 
// 	<< " and length of the branch: " << branches[i]->geomLength() << endl;
//    }
//    
   float fStoredLen = len;
   int nNumberOfBranchesToShorten = 0;
   //err << "\n\nWe Start in simplify. The length given to shorten branches with is: " << len << endl;
   for(i = 0; i < branches.Count(); i++)
   {
   	if(branches[i]->geomLength() <= fStoredLen && branches[i]->internal())
		nNumberOfBranchesToShorten++;
   	//cerr << "Number of points inside the branch: " << branches[i]->points.Count() 
	//<< " and length of the branch: " << branches[i]->geomLength() << endl;
   }
   
   if(nNumberOfBranchesToShorten == 0)
   	len = 0;
   
   while (len>0)						//Shorten all branches until we've cut off 'len' units:
   {
     float lenm = 1.0e8; int brm = -1;
     for(i=0;i<branches.Count();i++)				//Get which is the shortest branch in the whole skeleton
     {
        TreeBranch* b = branches[i];
        float       l = b->geomLength();
  	int	    L = b->length();
        if ((internal_only && b->internal()) && (l < lenm) && (L>2))
        {  lenm = b->geomLength(); brm = i;  }			//!!! Don't shorten branches with no internal points
     }

     float act_len = (lenm<len)? lenm : len;			//Actual length to shorten branch with

     
     if (brm!=-1)						//Branch found, shorten it...
     {
     
        shorten(act_len,*branches[brm],smooth_fact,maxiter);
	nNumberOfBranchesToShorten--;
	
     }
	
     if(nNumberOfBranchesToShorten == 0)
     	len = 0;
     else
     	len = fStoredLen;


     //len -= act_len;						//Find out how much we still have to shorten

     int nBranchesCount = branches.Count();
     for(i=0;i<nBranchesCount;i++)				//Compute branches' midpoints and geom-lengths
     {
        branches[i]->computeLength();
// 	if( branches[i]->geomLength() == 0 )
// 	{
// 		//Remove this phantom branch
// 		//RemoveAPhantomEdge(*branches[i]);
// 	}
     }

   }
  cout<<"Structural simplification ready"<<endl;
  WriteAllBranchesToMATLABFile();
  
//    cerr << "\n\nAt the end of simplify:\n";
//    for(i = 0; i < branches.Count(); i++)
//    {
//    	cerr << "Number of points inside the branch: " << branches[i]->points.Count() 
// 	<< " and length of the branch: " << branches[i]->geomLength() << endl;
//    }
  
}




void TreeSkel::shorten(float len,TreeBranch& b,float smooth_fact,int maxiter)	
								//Shorten branch b by 'len'
{
    
   float l = b.geomLength();
   int   L = b.length()-2, i;
   int  p1 = b.computePoint((l-len)/2)-1;			//Compute points between which the branch must be shortened
   int  p2 = b.computePoint((l+len)/2)-1;			//

   if (p1<0) p1=0; if (p1>=L) p1=L-1;
   if (p2<0) p2=0; if (p2>=L) p2=L-1;
   TreePoint* m1 = b.points[p1];
   TreePoint* m2 = b.points[p2];

   cout<<"Pass 3: remove points "<<p1<<".."<<p2<<" out of total of "<<L<<endl;

   for(i=p1;i<=p2;i++)						//Delete the range [p1..p2] of regular branch points
      b.points.Delete(p1);

      
//skel is a map which contains ALL points in the skeleton
   for(map<Coord,TreePoint*>::iterator it=skel.begin();it!=skel.end();it++)
   {								//Impose K=Kmin on all springs
      TreePoint* p = (*it).second;
      p->flag = DEFAULT;	
      for(i=0;i<p->nbs.Count();i++) 
      { 
         TreePoint* q = p->nbs[i];
         p->K[i] = 1.0; p->len0[i] = TreePoint::distance(p,q); 
      }
   }
 
   
   for(i=0;i<b.length();i++)					//Impose K=Kmax on all springs on current branch
   {
      TreePoint* p = b.getPoint(i);
      for(int j=0;j<p->nbs.Count();j++)
      {
	TreePoint* q = p->nbs[j];
	if (p->type==REGULAR || (q->type==REGULAR && q->branches[0]==&b))
	   p->K[j] = 1.0;
      }
  }


   TreePoint* M1 = b.getPoint(p1);				//These are the remaining points to be stitched
   TreePoint* M2 = b.getPoint(p1+1);				//on the branch
   int ni1 = M1->getNb(m1);					//Stitch back branch, i.e. connect M1 directly to M2
   int ni2 = M2->getNb(m2);					//In principle, this stitching is enough, since both
   M1->nbs[ni1] = M2;						//M1 and M2 are regular points, i.e. have exactly 2 nbs
   M2->nbs[ni2] = M1;

   
   smooth(&b,M1,M2,smooth_fact,maxiter);			//Finally, smooth simplified branch
   
   if((b.end1->x == b.end2->x) && (b.end1->y == b.end2->y))
   {
   	cerr << "\nREMOVING A PHANTOM\n";
	RemoveAPhantomEdge(b);
   }
}



float TreeSkel::TreePoint::energy() const			//Computes the elastic energy of a given point
{
   float E = 0;
   for(int i=0;i<nbs.Count();i++)
   {  
      float ddist = TreePoint::distance(this,nbs[i]) - len0[i];
      E += K[i]*pow(ddist,2);
   }
   return E;
}



void TreeSkel::smooth(TreeBranch* b,TreePoint* m1,TreePoint* m2,float smooth_fact,int maxiter)
{
   int i,j,n; 

   for(i=0;i<branches.Count();i++)				//Initialize len0[] for this branch
   {
      TreeBranch* b = branches[i];
      for(j=0;j<b->length();j++)
      {
         TreePoint* p = b->getPoint(j);
         for(n=0;n<p->nbs.Count();n++)
         {
            TreePoint* q = p->nbs[n];
	    if ((p==m1 && q==m2) || (p==m2 && q==m1))
	       p->len0[n] = 1.0; 
         }
      }
   }
   
	
   float dx = (m2->x-m1->x)/2.0f, dy = (m2->y-m1->y)/2.0f;	//(dx,dy) are the amounts to move the points m1,m2 with
   for(i=0;;i++)						//Move points of 1st half of shortened branch to its middle
   {	 							//(this accelerates the smoothing quite a lot)
     TreePoint* p = b->getPoint(i);				//Also, block the points, so the smoothing won't displace
     p->x += dx; p->y += dy;					//them back...
     p->flag = BLOCKED;
     if (p==m1) break;
   }
   for(i++;i<b->length();i++)					//Move points of the 2nd half of the shortened branch
   {	 							//to its middle too
     TreePoint* p = b->getPoint(i);
     p->x -= dx; p->y -= dy;
     p->flag = BLOCKED;
   }

   for(int iter=0;iter<maxiter;iter++)				//Perform the smoothing iterations
   {
      float E; TreePoint* p = getHighestE(E);			//Get highest energy point to move:
      if (!p) { VISITED = 1-VISITED; continue; }		//No one found, unblock visited pts, and try again
     
      float Fx = 0, Fy = 0; int N = p->nbs.Count();
      
      //float Ksum = 0;
      for(int i=0;i<N;i++)					//Move point p w.r.t. forces exerted by all its nbs:
      {
	TreePoint* q = p->nbs[i]; 
	float K      = p->K[i];
        float fx     = q->x-p->x;
        float fy     = q->y-p->y;
        float d      = sqrt(fx*fx+fy*fy);
        if (d<0.0001) d=1;
	float f      = K*fabs(TreePoint::distance(p,q)-p->len0[i]);
	fx *= f/d; fy *= f/d;
 	Fx += fx; Fy += fy;
	//Fx += K*(q->x-p->x); Fy += K*(q->y-p->y);
	//Ksum += K;
      }  
      //!!!Fx /= Ksum; Fy /= Ksum; 
      
      float dx = smooth_fact*Fx, dy = smooth_fact*Fy;
      p->x    += dx; p->y += dy;				//Ok, move point p, mark it as visited to give a chance
      p->flag  = VISITED;					//to other points to move too
   }
   
   
} 
   



TreeSkel::TreePoint* TreeSkel::getHighestE(float& E) 		//Gets point with highest energy, to be moved
{								//during smoothing. Returns 0 if none found
   E = 0; TreePoint* pmax = 0;
   for(int i=0;i<branches.Count();i++)
   {
      TreeBranch* b = branches[i];
      for(int j=0;j<b->length();j++)				//We don't consider the blocked, visited, or end-points
      {								//(i.e. they don't move)
         TreePoint* p = b->getPoint(j);
	 if (p->flag==VISITED || p->type==END || p->flag==BLOCKED) continue;
         float      e = p->energy();
	 if (e>E)
	 {  E = e; pmax = p;  }
      }
   }
   return pmax;
}






void TreeSkel::TreeBranch::computeLength()	//Compute branch's geom-length
{						
   int N = points.Count(); 			//get # internal points
   float dst = 0; TreePoint* prev = end1;
   for(int i=0;i<N;i++)				//compute dst = geom-length of the branch
   {
      TreePoint* p = points[i];
      dst += sqrt((prev->x-p->x)*(prev->x-p->x) + (prev->y-p->y)*(prev->y-p->y));
      prev = p;
   }
   dst += sqrt((prev->x-end2->x)*(prev->x-end2->x) + (prev->y-end2->y)*(prev->y-end2->y));   
   geom_length = dst;				//store geom-length in branch
}




int TreeSkel::TreeBranch::computePoint(float l)	//Return idx of point closest to given arc-length
{			
   float dmin = 1.0e8; int imin = 0;
   float dst = 0; TreePoint* prev = end1; 	//find out the point closest to the given length
   for(int i=0;i<points.Count();i++)
   {
      TreePoint* p = points[i];
      dst += sqrt((prev->x-p->x)*(prev->x-p->x) + (prev->y-p->y)*(prev->y-p->y));
      if (fabs(dst-l) < dmin)
      {  imin = i; dmin = fabs(dst-l);  }
      prev = p;
   }

   return imin+1; 				//return its index in the _global_ point-set 
						//(i.e. 0=1st endpoint,1=1st internal point (points[0]),etc...
}



void TreeSkel::TreeBranch::render(FIELD<float>& f,int ren_type)
{
   TreePoint* prev = end1;

   if (ren_type==2) renderLine(f,end1,end2);

   for(int i=0;i<points.Count();i++)
   {
      TreePoint* p = points[i];
      if (ren_type == 0)
         f.value(int(prev->x),int(prev->y)) = 1;
      else
         renderLine(f,prev,p);
      prev = p;
   }

   if (ren_type == 0)
     f.value(int(prev->x),int(prev->y)) = 1;
   else
     renderLine(f,prev,end2);    
}



void TreeSkel::TreeBranch::renderLine(FIELD<float>& f,TreePoint* p1,TreePoint* p2)
{
   float l  = sqrt((p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y));
   float dt = 1 / l;
   for(float t=0;t<=1;t+=dt)
   {
      float x = (1-t)*p1->x + t*p2->x;
      float y = (1-t)*p1->y + t*p2->y;
      f.value(int(x),int(y)) = 1;
   }
}


void TreeSkel::render(FIELD<float>& f,int ren_type)		//Renders all simplified branches as image
{
   f = 0;							//First, clear image to be rendered
   for(int i=0;i<branches.Count();i++)				//Render all branches
      branches[i]->render(f,ren_type);
}



/*
	change the radii on the skeleton points
	to the correct values
*/
void TreeSkel::GetCorrectRadii(FIELD<float>* DTField)
{
	int nXCoor, nYCoor;
	for(int k = 0; k < branches.Count(); k++)
	{
		TreeBranch* pBr = branches[k];
		nXCoor = pBr->end1->i;
		nYCoor = pBr->end1->j;
		DTField->value(nXCoor, nYCoor) = pBr->end1->dRadius;
		DTField->value(nXCoor+1, nYCoor) = pBr->end1->dRadius;
		DTField->value(nXCoor, nYCoor+1) = pBr->end1->dRadius;
		DTField->value(nXCoor-1, nYCoor) = pBr->end1->dRadius;
		DTField->value(nXCoor, nYCoor+1) = pBr->end1->dRadius;
		nXCoor = pBr->end2->i;
		nYCoor = pBr->end2->j;
		DTField->value(nXCoor, nYCoor) = pBr->end2->dRadius;
		DTField->value(nXCoor+1, nYCoor) = pBr->end2->dRadius;
		DTField->value(nXCoor, nYCoor+1) = pBr->end2->dRadius;
		DTField->value(nXCoor-1, nYCoor) = pBr->end2->dRadius;
		DTField->value(nXCoor, nYCoor+1) = pBr->end2->dRadius;
		for(int m = 0; m < pBr->points.Count(); m++)
		{
			const TreePoint* pPoint = pBr->points[m];
			nXCoor = pPoint->i;
			nYCoor = pPoint->j;
			DTField->value(nXCoor, nYCoor) = pPoint->dRadius;
			DTField->value(nXCoor+1, nYCoor) = pPoint->dRadius;
			DTField->value(nXCoor, nYCoor+1) = pPoint->dRadius;
			DTField->value(nXCoor-1, nYCoor) = pPoint->dRadius;
			DTField->value(nXCoor, nYCoor+1) = pPoint->dRadius;
		}
	}
}


/*
	This function will use the values in the DT Field
	and assign them as the radius to each skeleton point
	in the skeleton
*/
void TreeSkel::AssignRadius(const FIELD<float>& DTField)
{
	float fXCoor, fYCoor;
	for(int k = 0; k < branches.Count(); k++)
	{
		TreeBranch* pBr = branches[k];
		//cerr << endl << "% number of inner points " << pBr->points.Count() << " plus 2 endpoints" << endl;
		fXCoor = pBr->end1->x;
		fYCoor = pBr->end1->y;
		pBr->end1->dRadius = DTField.value(fXCoor, fYCoor);
		
		//cerr << endl << endl << "1 " << pBr->end1->dRadius << endl;
		
		fXCoor = pBr->end2->x;
		fYCoor = pBr->end2->y;
		pBr->end2->dRadius = DTField.value(fXCoor, fYCoor);
		for(int m = 0; m < pBr->points.Count(); m++)
		{
			TreePoint* pPoint = pBr->points[m];
			fXCoor = pPoint->x;
			fYCoor = pPoint->y;
			pPoint->dRadius = DTField.value(fXCoor, fYCoor);
			
			//cerr << m + 2 << " " << pPoint->dRadius << endl;
		}
		
		//cerr << pBr->points.Count() + 2 << " " << pBr->end2->dRadius << endl << endl << endl;
	}
}

// void TreeSkel::FixNeighbors(const TreeBranch* pBr, TreePoint* pEndPointBrToFix, const TreePoint* pOtherEndPt)
// //Given a phantom branch and an endpoint to fix as well as the other endpoint, this function
// //will go through the branches that are neighbors of pEndPointBrToFix and add 
// //as neighbors the neighbors from the pOtherEndPt
// {
// 	int nCount = pEndPointBrToFix->branches.Count();
// 	
// 	for(int i = 0; i < nCount; i++)
// 	{
// 		TreeBranch& neighborBr = *pEndPointBrToFix->branches[i]; //one of the neighbors of pEndPointBrToFix
// 		
// 		if(neighborBr == *pBr) // if neighbor of itself
// 			continue;
// 			
// 		cerr << "Handling one neighbor branch\n";
// 		
// 		TreePoint& commonEndPt = *findEndPntToAdjust(neighborBr, *pBr); //the endpoint that has the 
// 										//phantom branch as a neighbor
// 		//we now found the endpoint of the neighbor branch that will get new neighbors
// 		for(int j = 0; j < pOtherEndPt->branches.Count(); j++)
// 		{
// 			commonEndPt.branches.Add(pOtherEndPt->branches[j]);
// 		}
// 		//we delete the reference to the phantom branch
// 		//pNeighborEndPt->branches.Remove(pBr);
// 	}
// 
// }

// void TreeSkel::RemoveAllZeroBranches()
// {
// 	for(int k = 0; k < branches.Count(); k++)
// 	{
// 		TreeBranch* pBr = branches[k];
// 		
// 		if( pBr->geomLength() == 0)
// 		//this branch is a phantom branch.
// 		//it has neighbors connected to both
// 		//its endpoints, and we should merge this neighbor information
// 		{
// 			cerr << "\nPhantom branch: " << *pBr << endl;
// 			for(int i = 0; i < pBr->end1->branches.Count(); i++)
// 			{
// 				cerr << "\tendPt 1 nbs: " << *pBr->end1->branches[i] << endl;
// 				TreeBranch* pNbBr = pBr->end1->branches[i];
// 				int j;
// 				for(j = 0; j < pNbBr->end1->branches.Count(); j++)
// 				{
// 					cerr << "\t\tnbs of nbs end1: " << *pNbBr->end1->branches[j] << endl;
// 				}
// 				for(j = 0; j < pNbBr->end2->branches.Count(); j++)
// 				{
// 					cerr << "\t\tnbs of nbs end2: " << *pNbBr->end2->branches[j] << endl;
// 				}
// 			}
// 			/*cerr << "\nFirst endpoint\n";
// 			FixNeighbors(pBr, pBr->end1, pBr->end2);
// 			cerr << "Second endpoint\n";
// 			FixNeighbors(pBr, pBr->end2, pBr->end1);*/
// 		}
// 	}
// }


int TreeSkel::findEndPntToAdjust(const TreeBranch& branchToFix, const TreeBranch& zeroBranch) const
{
// 	TreePoint* pEndPntOne = branchToFix.end1;
// 	TreePoint* pEndPntTwo = &branchToFix.end2;
// 	TreePoint* neededEndPnt = NULL;
	int i;
	int nPosition = 2;
	for(i = 0; i < branchToFix.end1->branches.Count(); i++)
	{
		const TreeBranch& someNeighbor = *branchToFix.end1->branches[i];
		if(someNeighbor == zeroBranch)
			nPosition = 0;
	}
	for(i = 0; i < branchToFix.end2->branches.Count(); i++)
	{
		const TreeBranch& someNeighbor = *branchToFix.end2->branches[i];
		if(someNeighbor == zeroBranch)
			nPosition = 1;
	}
	
	ASSERT(!( nPosition== 2));
	return nPosition;
}


/*
	In the simplification process (after shorten() is called) a 
	branch is collapsed and we are left a branch that has zero
	length. This branch is redundant and should just be removed
	This function removes the phantom and attaches appropriate 
	branches to each other. (Delete edge, reconnect graph)
*/
void	TreeSkel::RemoveAPhantomEdge(TreeBranch& b)
{	
	//first we create a new node. All branches that were connected to the 
	//phantom edge will be to this replacement node
	Coord c(b.end1->i,b.end1->j);
	TreePoint* pTheNewNode = new TreePoint(c,BRANCH);
	pTheNewNode->x = b.end1->x;	//same for float value coordinates
	pTheNewNode->y = b.end1->y;	
	pTheNewNode->dRadius = b.end1->dRadius;
	pTheNewNode->flag = b.end1->flag;
	pTheNewNode->K = b.end1->K;
	pTheNewNode->len0 = b.end1->len0; 
	
	//now we will disconnect the edge, by replacing endpoints that
	//are also endpoints of the phantom edge with the newnode
	DisconnectPhantomFromNeighbors(b, pTheNewNode);
	
	//update neighbor branch information of the new node
	UpdateNeighborBranchInfo(b, pTheNewNode);
	
	//update neighbor TreePoint information of the new node
	UpdateNeighborTreePointInfo(b, pTheNewNode);
	
	//update neighbor TreePoint information of the neighbor TreePoints
	UpdateTheNeighborTreePoints(b, pTheNewNode);
	
	//check that no branch points to the phantom anymore
	CheckThatPhantomIsDisconnected(b);
	
	//STILL TODO!!! REMOVE THE TWO ENDPOINTS OF THE PHANTOM
	//BRANCH FROM THE STRUCTURE AS WELL AS THE BRANCH ITSELF
	
	RemovePhantomFromListOfBranches(b);
	
	
}

void TreeSkel::RemovePhantomFromListOfBranches(TreeBranch& phantom)
{
	int i;
	int nNumberOfBranches = branches.Count();
	for(i = 0; i < nNumberOfBranches; i++)
	{
		if(*branches[i] == phantom)
			branches.Delete(i);
	}
}


void TreeSkel::WriteAllBranchesToScreen()
{
	int i;
	for(i = 0; i < branches.Count(); i++)
		cerr << "\n" << *branches[i] ;
	cerr << endl;
}


/*
	This function will check whether there are still references
	in the graph structure that point to the phantom edge
*/
void TreeSkel::CheckThatPhantomIsDisconnected(TreeBranch& phantom)
{
	int i;
	for(i = 0; i < branches.Count(); i++)
	{
		if(*branches[i] != phantom)
		{
			ASSERT(!PhantomIsNeighbor(*branches[i],phantom));
		}
	}
}

/*
	This function will update the neighbor TreePoint information
	of the neighbor TreePoints of the new node
*/
void TreeSkel::UpdateTheNeighborTreePoints(const TreeBranch& phantom, TreePoint* pTheNewNode)
{
	int i, nPosEnd1 = -1, nPosEnd2 = -1;
	TreePoint* pTPToAdd = pTheNewNode;
	
	for(i = 0; i < pTheNewNode->nbs.Count(); i++)
	{
		nPosEnd1 = pTheNewNode->nbs[i]->nbs.Remove(phantom.end1);
		nPosEnd2 = pTheNewNode->nbs[i]->nbs.Remove(phantom.end2);
		
		//one of the endpoints should have been found
		ASSERT(!((nPosEnd1 == -1)&&(nPosEnd2 == -1)));
		
		pTheNewNode->nbs[i]->nbs.Add(pTPToAdd);
		
	}
}


/*
	This function will set the appropriate neighbor TreePoint
	information for the new node
	Appropriate meaning, all neighbor TreePoints of the phantom
	edge except the neighbor TreePoints that are endpoints
	of the phantom edge
*/
void TreeSkel::UpdateNeighborTreePointInfo(const TreeBranch& phantom, TreePoint* pNewNode)
{
	int i;

	DBG_ONLY(int nTotalNbsTreePointsFirst = 
		phantom.end1->nbs.Count() + phantom.end2->nbs.Count())

	int nAddedNbsTreePoints = 0;
	
	for(i = 0; i < phantom.end1->nbs.Count(); i++)
		if(phantom.end1->nbs[i] != phantom.end2)
		{
			pNewNode->nbs.Add(phantom.end1->nbs[i]);
			nAddedNbsTreePoints++;
		}
	for(i = 0; i < phantom.end2->nbs.Count(); i++)
		if(phantom.end2->nbs[i] != phantom.end1)
		{
			pNewNode->nbs.Add(phantom.end2->nbs[i]);
			nAddedNbsTreePoints++;
		}
	
	ASSERT(nTotalNbsTreePointsFirst - nAddedNbsTreePoints == 2);
}



/*
	This function will set the appropriate neighbor branch
	information for the new node
	Appropriate meaning, all neighbors of the phantom edge
	expect the neighbors that are the phantom edge itself
*/
void TreeSkel::UpdateNeighborBranchInfo(const TreeBranch& phantom, TreePoint* pNewNode)
{
	int i;

	DBG_ONLY(int nTotalNeighborsFirst = 
		phantom.end1->branches.Count() + phantom.end2->branches.Count())

	int nAddedNumberNeighbors = 0;
	
	for(i = 0; i < phantom.end1->branches.Count(); i++)
		if(*phantom.end1->branches[i] != phantom)
		{
			pNewNode->branches.Add(phantom.end1->branches[i]);
			nAddedNumberNeighbors++;
		}
	for(i = 0; i < phantom.end2->branches.Count(); i++)
		if(*phantom.end2->branches[i] != phantom)
		{
			pNewNode->branches.Add(phantom.end2->branches[i]);
			nAddedNumberNeighbors++;
		}
	
	ASSERT(nTotalNeighborsFirst - nAddedNumberNeighbors == 2);
}


/*
	This function will disconnect the phantom from the graph.
	The way this is done, is that for all neighbors of the phantom
	other then the phantom itself, the endpoint of that neighbor which
	is a common endpoint with the phantom (so the shared endpoint between
	a neighbor and the phantom) will be replaced by pNewNode
*/
void TreeSkel::DisconnectPhantomFromNeighbors(const TreeBranch& phantom, TreePoint* pNewNode)
{
	int i;
	for(i = 0; i < phantom.end1->branches.Count(); i++)
		if(*phantom.end1->branches[i] != phantom) //only for neighbors other then the phantom
		{
			if(findEndPntToAdjust(*phantom.end1->branches[i], phantom) == 0)
				phantom.end1->branches[i]->end1 = pNewNode;
			else
				phantom.end1->branches[i]->end2 = pNewNode;
		}
	for(i = 0; i < phantom.end2->branches.Count(); i++)
		if(*phantom.end2->branches[i] != phantom) 
		{
			if(findEndPntToAdjust(*phantom.end2->branches[i], phantom) == 0)
				phantom.end2->branches[i]->end1 = pNewNode;
			else
				phantom.end2->branches[i]->end2 = pNewNode;
		}
}


bool TreeSkel::PhantomIsNeighbor(const TreeBranch& branch, const TreeBranch& phantom) const
{
	int i;
	bool bIsNeighbor = false;
	for(i = 0; i < branch.end1->branches.Count(); i++)
		if(*branch.end1->branches[i] == phantom)
			bIsNeighbor = true;
	for(i = 0; i < branch.end2->branches.Count(); i++)
		if(*branch.end2->branches[i] == phantom)
			bIsNeighbor = true;
	return bIsNeighbor;
}




void TreeSkel::RadiiAllBranches()
{
	FILE* fp;
	fp = fopen("RadiiAllBranches.m", "w");
	int i,j;
	fprintf(fp,"m = [\n");
	for(i = 0; i < branches.Count(); i++)
	{
		fprintf(fp,"%d,%d,%f\n",branches[i]->end1->i,branches[i]->end1->j,branches[i]->end1->dRadius);
		for(j = 0; j < branches[i]->points.Count(); j++)
			fprintf(fp,"%d,%d,%f\n",branches[i]->points[j]->i,branches[i]->points[j]->j,branches[i]->points[j]->dRadius);
		fprintf(fp,"%d,%d,%f\n",branches[i]->end2->i,branches[i]->end2->j,branches[i]->end2->dRadius);
	}
	fprintf(fp,"];\n");
	fclose(fp);
}



void TreeSkel::GetShockGraphParams(double dMinSlopei, double dMinErrori, double dMaxAccelChgi)
{
	dMinSlope = dMinSlopei;
	dMinError = dMinErrori;
	dMaxAccelChg = dMaxAccelChgi;
}

void TreeSkel::WriteAllBranchesToMATLABFile()
{
	FILE* fp;
	fp = fopen("branchCoordinates.m", "w");
	int i,j;
	for(i = 0; i < branches.Count(); i++)
	{
		if(branches[i]->geomLength() == 0)
			continue;
		else
		{
			fprintf(fp,"m%d = [\n",i);
			fprintf(fp,"%f,%f,%f\n",branches[i]->end1->x,branches[i]->end1->y,branches[i]->end1->dRadius);
			for(j = 0; j < branches[i]->points.Count(); j++)
				fprintf(fp,"%f,%f,%f\n",branches[i]->points[j]->x,branches[i]->points[j]->y,branches[i]->points[j]->dRadius);
			fprintf(fp,"%f,%f,%f\n]\n\n",branches[i]->end2->x,branches[i]->end2->y,branches[i]->end2->dRadius);
		}
	}
	fclose(fp);
}

/*
void TreeSkel::writeDot()
{
   FILE* fp = fopen("skel.dot","w");

   fprintf(fp,"graph G {\n");
   fprintf(fp,"   margin = \"0,0\"; \n");
   fprintf(fp,"   node [width=0, height=0, label=\"\", shape=box];\n");

   for(int i=0;i<branches.Count();i++)
   {
      TreeBranch* b = branches[i];

      for(int j=0;j<b->length();j++)
      {
         TreePoint* p = b->getPoint(j);
         fprintf(fp,"  \"%x\"   [pos = \"%f,%f", p, p->x, p->y);
         if (p->type==END) fprintf(fp,"!");
         fprintf(fp,"\"];\n");

         for(int n=0;n<p->nbs.Count();n++)
         {
            TreePoint* q = p->nbs[n];
            fprintf(fp,"\"%x\" -- \"%x\";\n", p,q);
         }
      }
   }

   fprintf(fp,"}\n\n");
   fclose(fp);
}
*/



