#ifndef TREE_SKEL_H
#define TREE_SKEL_H

#include "darray.h"
#include "field.h"
#include "genrl.h"
#include <map>
#include <vector>
#include <memory>
#include "Tools/PiecewiseApprox/PolyLineApprox.h"

typedef std::shared_ptr< FIELD<float> > FieldPtr;


class TreeSkel
{
public:

	TreeSkel(FIELD<int>*);		//Builds this out of bilevel,thinned skel-img
	~TreeSkel();	

	enum 	POINT_TYPE			//Skel-pts are classified as follows
	{
		BACKGROUND=0, END=1, REGULAR=2, BRANCH=3, PROBLEM=4, UNKNOWN=5
	};   


	struct  TreeBranch;

	struct  TreePoint			//A point in the tree-skel
	{
		TreePoint(const Coord& c, //Ctor
			POINT_TYPE t):
		i(c.i),j(c.j),dRadius(0),type(t),flag(0) {}    

		int		     hasNb(TreePoint*) const;
		//Checks if 2 pts are nbs  	 
		int		     getNb(TreePoint*) const;
		float		     energy() const;

		static float	     distance(const TreePoint*,const TreePoint*);


		int		     i,j;	//Coords
		float		     x,y;	//Floating-point coords
		float		     dRadius;	//Radius of a TreePoint
		POINT_TYPE	     type;	//Type (classification)	
		DARRAY<TreePoint*,2> nbs;	//Nb-points in the skel
		DARRAY<TreeBranch*,2> branches; //Branches this is part of
		DARRAY<float,5>      len0;	//!!!
		DARRAY<float,5>	     K;		//!!!
		unsigned char	     flag;	//!!!

		friend std::ostream& operator<<(std::ostream& os, const TreePoint& pt)
		{ 
			return os << "(x:" << pt.x << ", y:" << pt.y << ", r:" << pt.dRadius 
				<< ", num br: " << pt.branches.Count() 
				<< ", num nbs:" << pt.nbs.Count() 
				<< ", type:" << pt.type << ")";
		}
	};



	struct  TreeBranch			//A branch in the tree-skel	
	{
		TreeBranch(TreeSkel*);
		void		addEnds(TreePoint*,TreePoint*);
		void		addPoint(TreePoint*);      		
		int		length() const;	//Return #total = regular+endpoints on this branch
		TreePoint*      getPoint(int);	
		float		geomLength() 	//Return geometric length of the branch
			const;
		int		internal() const;  //Tells if this is an internal skel-branch or not
		int 		external() const;  //Tells if this is an external skel-branch or not
		int     	computePoint(float); //Get idx of point closest to given arc-length
		void		computeLength();     //Compute branch's geom-length				   	
		void		render(FIELD<float>&,int type);
		//Render simplified branch in image

		TreePoint       *end1,*end2;	//The two endpoints (either BRANCH or END) of this branch	
		DARRAY<TreePoint*>	points; //The REGULAR points of this branch (i.e. w/o endpoints)

		bool operator==(const TreeBranch& rhs) const
		{
			return (end1 == rhs.end1 && end2 == rhs.end2);
		}

		bool operator!=(const TreeBranch& rhs) const
		{
			return !operator==(rhs);
		}

		friend std::ostream& operator<<(std::ostream& os, const TreeBranch& br)
		{ 
			TreePoint* pts[] = {br.end1, br.end2};

			os << "[";

			for (int i = 0; i < 2; i++)
			{
				os << "endPt" << i + 1 << ":";

				if (pts[i])
					os << *(pts[i]);
				else
					os << "null";

				if (i == 0)
					os << ", ";
			}

			return os << ", num pts: " << br.points.Count() << "]";
		}

	private:

		void		renderLine(FIELD<float>&,TreePoint*,TreePoint*);

		float		geom_length;		
	}; 

	void	simplify(float len,float smooth_factor,int maxiter,int internal_only=1);	
	//Simplify starting with shortest internal branch

	void	simplifyLaplacian(double);

	void	GetCorrectRadii(FIELD<float>*);

	void	GetShockGraphParams(double, double, double);
	void	AssignRadius(const FIELD<float>&);
	void    RemoveAllZeroBranches();
	void    RadiiAllBranches();
	void	RemoveAPhantomEdge(TreeBranch& b);

	void	render(FIELD<float>&,int type=0);
	//Render simplified skeleton as image
	//type=0: render as points; 
	//type=1: render as points connected with lines
	//type=2: render as singe line connecting branch's ends

	std::map<Coord,TreePoint*> skel;		//All points in the skel
	DARRAY<TreePoint*> MemoryLeakRemoval;
	DARRAY<TreeBranch*>   branches;		//All branches in the skel
	FIELD<POINT_TYPE>     point_types;	//Types of points (regular,branch,endpoint) !!!!might vanish

	double dMinSlope;
	double dMinError;
	double dMaxAccelChg;


private:
	bool IsThereABranchWithTwoTheSameEndpts();
	bool AreTherePhantomBranches();
	void WriteAllBranchesToMATLABFile();
	int findEndPntToAdjust(const TreeBranch&, const TreeBranch&) const;
	void FixNeighbors(const TreeBranch*, TreePoint*, const TreePoint*);
	bool PhantomIsNeighbor(const TreeBranch&, const TreeBranch&) const;
	void DisconnectPhantomFromNeighbors(const TreeBranch&, TreePoint*);
	void UpdateNeighborBranchInfo(const TreeBranch&, TreePoint*);
	void UpdateNeighborTreePointInfo(const TreeBranch&, TreePoint*);
	void UpdateTheNeighborTreePoints(const TreeBranch&, TreePoint*);
	void CheckThatPhantomIsDisconnected(TreeBranch&);
	void WriteAllBranchesToScreen();
	void RemovePhantomFromListOfBranches(TreeBranch&);

	std::pair< std::vector< std::pair<int,int> >, int> getInsideBranchesToBeRemoved(double);
	std::vector<int> getNeighborhood(std::vector< std::pair<int,int> >, TreeBranch*);
	bool isNeighbor(const TreeBranch&, const TreeBranch&) const;
	std::vector<int> GetListOfBrInNbh(std::vector< std::pair<int,int> >,int);
	DARRAY<TreeBranch*> GetListOfBrTBUpdated(std::vector<int> );
	std::pair<int,int> GetPntWithLargestRadius(std::vector<int> );
	DARRAY<int> GetListOfNumberingNbhs(std::vector< std::pair<int,int> > );
	DARRAY<TreeBranch*> GetListOfBrTBUpdatedWithEndPnt(std::vector<int>, std::pair<int,int>);
	void fixBranch(TreeBranch*, std::vector<int>, std::pair<int,int>);
	int GetEndPntAttachedToPhantom(TreeBranch*, std::vector<int>);
	int PhantomIsNb(const TreeBranch&, const TreeBranch&) const;
	vpl::POINTS ConvertBranchIntoPoints(TreeBranch*, int);
	int GetPntIndexBeginLigature(TreeBranch*,double,int);
	void DisplaceBranch(TreeBranch*,int,int,TreePoint*);
	void DeletePartOfBranch(TreeBranch*,int,int);
	void AssignNewEndPntToBranch(TreeBranch*,int,TreePoint*);
	void ConnectBranchToNewEndPnt(TreeBranch*,int,TreePoint*);
	DARRAY<std::pair<int,int> > Bresenham(const int,const int,const int,const int);
	void ReAssignRadiusValues(TreeBranch*,int,TreePoint*,double);
	int GetIndexWhereBranchWasCut(TreeBranch*, int);
	int GetIndexOfEndNewRadius(TreeBranch*,int,TreePoint*);
	void PopulateRadiiWithSlope(TreeBranch*,double dSlope,int,int);
	void PopulateRadiiWithoutSlope(TreeBranch*,int,TreePoint*,int);
	bool GetPntClosestToBranch(double, double,DARRAY<std::pair<int,int> >,double);
public:
	void RemoveOutsideBranch(DARRAY< int > nIndices);
	void glueBranches(TreeBranch* pBr1, TreeBranch* pBr2);
	double getSizeSmallestInsideBranch();
	int getSmallestInsideBranch();
	std::pair<bool,double> performNextSimplification();
	std::pair<bool,int> determineMergePnt(int);
	int getIndexMidPoint(int);
	DARRAY<TreeBranch*> getBrTBUpdatedWithEndPnt(TreeBranch*,int);
	void fixBrnchToExistEnd(TreeBranch*,TreeBranch*,int);
	DARRAY<TreeBranch*> getBrTBUpdatedWithoutEndPnt(TreeBranch*);
	void fixBrnchToNonExistEnd(TreeBranch*,TreeBranch*,TreePoint*);
	void FixPseudoCross(FIELD<int>*,int,int);
	bool diagonalType1(FIELD<int>*,int,int);
	bool diagonalType2(FIELD<int>*,int,int);
	bool hasExact3NbsInRightTriangle(FIELD<int>*,int,int);
	//DARRAY<Coord> hasNbrNbs(FIELD<int>*,int,int);
	bool branchIsRemovable(TreeBranch*);
	//bool has4NbsInRightConfig(FIELD<int>*,int,int);
	bool PartIsRealLigaturePart(int, int, TreeBranch*);
	bool FixCube(FIELD<int>* s, int i, int j);
	bool FixTConfig(FIELD<int>* s, int i, int j);
	bool FixDiamond(FIELD<int>* s, int i, int j);
	void printNbh(FIELD<int>*,int,int, const char*, int nbhRad = 3);
	bool FixBranchOf1Point(FIELD<int>* s, int i, int j);

	int getBestPntIndexForSlope(TreeBranch* pBrnchTofix, int nPntIndex, int nEndPoint, TreePoint* pNewEndPointForBranch, double dLigatureSlope);

	void	shorten(float,TreeBranch&,float,int);       
	TreePoint* getHighestE(float&);
	void	smooth(TreeBranch*,TreePoint*,TreePoint*,float,int);
	void	buildBranches();
	int	unknown(int,int) const;
	int	unknown(const Coord&) const;
	int     diamond(int,int) const;
	int	problem(const Coord&) const;	
	int     branch(const Coord&) const;
	void 	getNbs(FIELD<int>*,int,int,Coord*,int&,Coord*,int&);	
	void    getNbs(FIELD<int>*,TreePoint*,Coord*,int&);
	TreePoint*
		setPoint(int,int,POINT_TYPE);
	TreePoint*
		getPoint(int,int);


};


inline int TreeSkel::unknown(int i,int j) const
{  return point_types.value(i,j)==UNKNOWN;  }

inline int TreeSkel::unknown(const Coord& c) const
{  return point_types.value(c.i,c.j)==UNKNOWN;  }

inline int TreeSkel::diamond(int i,int j) const
{
	if (unknown(i-1,j+1) && unknown(i+1,j+1) && unknown(i,j+2)) return 1;
	if (unknown(i+1,j+1) && unknown(i+1,j-1) && unknown(i+2,j)) return 2;
	if (unknown(i-1,j+1) && unknown(i-1,j-1) && unknown(i-2,j)) return 3;
	if (unknown(i+1,j-1) && unknown(i-1,j-1) && unknown(i,j-2)) return 4;

	return 0;
}	

inline int TreeSkel::problem(const Coord& c) const
{  return point_types.value(c.i,c.j)==PROBLEM;  }

inline int TreeSkel::branch(const Coord& c) const
{  return point_types.value(c.i,c.j)==BRANCH;  }


inline int TreeSkel::TreePoint::hasNb(TreePoint* p) const
{  return (nbs.Contains(p)!=-1);  }

inline int TreeSkel::TreePoint::getNb(TreePoint* p) const
{  return nbs.Contains(p);  }


inline TreeSkel::TreeBranch::TreeBranch(TreeSkel* s): end1(0),end2(0)
{  s->branches.Add(this);  }

inline void TreeSkel::TreeBranch::addEnds(TreePoint* e1,TreePoint* e2)
{  end1 = e1; end2 = e2;  e1->branches.Add(this); e2->branches.Add(this);  }

inline void TreeSkel::TreeBranch::addPoint(TreePoint* p)
{  points.Add(p);  p->branches.Add(this);  }


inline int TreeSkel::TreeBranch::length() const
{  return points.Count() + 2;  }

inline float TreeSkel::TreeBranch::geomLength() const
{  return geom_length;  }

inline int TreeSkel::TreeBranch::internal() const
{  return end1->type!=END && end2->type!=END;  }

inline int TreeSkel::TreeBranch::external() const
{  return (end1->type == END && end2->type != END) || (end1->type != END && end2->type == END);  }

inline TreeSkel::TreePoint* TreeSkel::TreeBranch::getPoint(int i) 
{  return (i<1)? end1 : (i>points.Count())? end2 : points[i-1];  }

inline float TreeSkel::TreePoint::distance(const TreePoint* p,const TreePoint* q)
{  return sqrt((p->x-q->x)*(p->x-q->x) + (p->y-q->y)*(p->y-q->y));  }

#endif

