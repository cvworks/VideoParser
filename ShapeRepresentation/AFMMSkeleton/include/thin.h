#ifndef THIN_2D_H
#define THIN_2D_H

#include "field.h"
#include "darray.h"
#include "genrl.h"
#include <map>

class  	ThinningMethod
	{
	public:

		     ThinningMethod(FIELD<float>* dt,FIELD<float>* d,float K);
		    ~ThinningMethod();
		
	FIELD<int>*  thin();
	int	     getNumDeletedPts() const;		//Get how many pts were deleted
	
	private:
	
	typedef std::multimap<float,Coord> PriList;

	int	      deletion(int);			//Executes one directional-pass in given direction	
	int	      test_template(int,int,int);	//Tests if deletion-template is satisfied for given (dir,i,j)
	FIELD<int>*   threshold(FIELD<float>* f,float); //Thresholds input-data to a bilevel img


	FIELD<int>*   b;				//Marks which points are in current 'boundary' with 1, else 0
	FIELD<int>*   f;				//The thinned object
	FIELD<float>* dt;				//The skeleton's DT, controls deletion order
	PriList       boundary_arr,boundary_arr2;	//Store the thinning-boundary
	DARRAY<Coord> del; 				//Stores pixels to be deleted (made non-automatic for speed)
	PriList       *boundary,*boundary2;		//Dynamically refer to boundary_arr,boundary_arr2 buffers
	int	      total_del;			//# points deleted
	};


inline int ThinningMethod::getNumDeletedPts() const
{  return total_del;  }


#endif





