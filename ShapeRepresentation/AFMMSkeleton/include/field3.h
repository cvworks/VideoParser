#ifndef FIELD3_H
#define FIELD3_H


#include <stdio.h>
#include <string.h>
#include <io.h>
#include <iostream>
#include "byteswap.h"
#include "field.h"

class FLAGS3;

template <class T> class FIELD3
	{
	public:

			FIELD3(int=0,int=0,int=0);		//Ctor
			FIELD3(const FIELD3&);			//Copy ctor
			FIELD3(const FIELD3&,int rx,int ry,int rz);	//Ctor by subsampling arg every rx,ry,rz samples
	       	       ~FIELD3();				//Dtor
	  void		size(const FIELD3&);
	  void		size(int,int,int);
	  T&            value(int,int,int=0);			//value at (i,j,k)
	  const T&      value(int,int,int=0) const;		//const version of above
	  T		gradnorm(int,int,int=0) const;		//norm of grad at (i,j,k)
	  void		extract(int axis,int pos,FIELD<T>&);	//extract 2D slice from this
	  void		insert(int axis,int pos,FIELD<T>&);	//insert 2D slice into this
	  T*	        data()					{ return v;  }
	  const T*      data() const				{ return v;  }
	  int		dimX()			{ return nx; }	//number of columns
	  int		dimY()			{ return ny; }	//number of rows
	  int		dimZ()			{ return nz; }  //number of planes
	  int		dimX() const		{ return nx; }	//number of columns
	  int		dimY() const		{ return ny; }	//number of rows
	  int		dimZ() const		{ return nz; }  //number of planes
	  FIELD3&	operator=(FIELD3&);			//assignment op
	  FIELD3&	operator=(T);				//assignment op
	  FIELD3&        operator+=(const FIELD3&);		//addition op
	  void		gradnorm(FIELD3&) const;			//norm of grad as field
	  void		minmax(T&,T&,T&) const;			//min, max, avg for field
	  FIELD3&	operator*=(T);				//multiply field by scalar
	  static FIELD3*	read(char*);				//read field from VTK file	
	  void		write(char*) const;			//write field to VTK data file
	  void		writePPM(char*,int dir=0,int plane=-1) const;
								//write 2D-slice across axis 'dir' at position 'plane' as PPM file
								//If plane not given, middle plane across 'dir' is assumed.
	  static void	setVTKAscii(int);			//writes all VTK files as ASCII or BINARY
	  static int	getVTKAscii();				//returns if VTK files are written as ASCII or BINARY
	  static void   setVTKCellData(int);			//writes all VTK files as cell or point data
	  static int	getVTKCellData();
	
	private:

	  static FIELD3* readVTK(char*);                         //read field from VTK scalar data file
	  static FIELD3* readPGM(char*);				//read field from PGM grayscale voxel file
	  static FIELD3* readASCII(char*);                       //read field from plain ASCII data file
	  
	  int		nx,ny,nz;				//nx = ncols, ny = nrows, nz = nplanes
	  T*		v;
	};


extern int FIELD3_VTK_ascii;					//static-like object used to determine how (ascii/binary) vtk files are written
extern int FIELD3_VTK_cells;					//static-like object used to determine if points or cells are written in vtk files

template <class T> class VFIELD3
	{
	public:
			VFIELD3(int x=0,int y=0,int z=0):v0(x,y,z),v1(x,y,z),v2(x,y,z){}
	void		setValue(int,int,int,T*);	
	void		write(const char*) const;
	void		write(const char*,FLAGS3&) const;	
	void		size(int i,int j,int k)		{ v0.size(i,j,k); v1.size(i,j,k); v2.size(i,j,k); } 
	void		size(const FIELD3<T>& f)         { v0.size(f); v1.size(f); v2.size(f); }	
	
	FIELD3<T>	v0;
	FIELD3<T>	v1;
	FIELD3<T>	v2;
	int		dimX() const			{ return v0.dimX(); }
	int		dimY() const			{ return v0.dimY(); }
	int		dimZ() const			{ return v0.dimZ(); }
	
	static VFIELD3*	read(char*);
	};
		

template <class T> inline FIELD3<T>::FIELD3(int nx_,int ny_,int nz_): nx(nx_),ny(ny_),nz(nz_),v((nx_*ny_*nz_)? new T[nx_*ny_*nz_] : 0)
{  }

template <class T> inline FIELD3<T>::FIELD3(const FIELD3<T>& f): nx(f.nx),ny(f.ny),nz(f.nz),v((f.nx*f.ny*f.nz)? new T[f.nx*f.ny*f.nz] : 0)
{  if (nx*ny*nz) memcpy(v,f.v,nx*ny*nz*sizeof(T));  }

template <class T> FIELD3<T>::FIELD3(const FIELD3<T>& f,int rx,int ry,int rz): nx(f.nx/rx),ny(f.ny/ry),nz(f.nz/rz)
{  
  v = new T[nx*ny*nz];

  for(int k=0;k<nz;k++)
     for(int j=0;j<ny;j++)
        for(int i=0;i<nx;i++)
	   value(i,j,k) = f.value(i*rx,j*ry,k*rz);
}

template <class T> void FIELD3<T>::setVTKAscii(int a)
{  FIELD3_VTK_ascii = a;  }

template <class T> int FIELD3<T>::getVTKAscii()
{  return FIELD3_VTK_ascii;  }

template <class T> void FIELD3<T>::setVTKCellData(int a)
{  FIELD3_VTK_cells = a;  }

template <class T> int FIELD3<T>::getVTKCellData()
{  return FIELD3_VTK_cells;  }


template <class T> inline T& FIELD3<T>::value(int i,int j,int k)
{
  i = (i<=0) ? i : (i>=nx) ? 2*nx-i-1 : i;
  j = (j<=0) ? j : (j>=ny) ? 2*ny-j-1 : j; 
  k = (k<=0) ? k : (k>=nz) ? 2*nz-k-1 : k; 
  return *(v+nx*(k*ny+j)+i);
}

template <class T> inline const T& FIELD3<T>::value(int i,int j,int k) const
{
  i = (i<=0) ? i : (i>=nx) ? 2*nx-i-1 : i;
  j = (j<=0) ? j : (j>=ny) ? 2*ny-j-1 : j; 
  k = (k<=0) ? k : (k>=nz) ? 2*nz-k-1 : k; 
  return *(v+nx*(k*ny+j)+i);
}

template <class T> inline T FIELD3<T>::gradnorm(int i,int j,int k) const
{
   T ux = value(i+1,j,k)-value(i-1,j,k);
   T uy = value(i,j+1,k)-value(i,j-1,k);
   T uz = value(i,j,k+1)-value(i,j,k-1);
   return (ux*ux+uy*uy+uz*uz)/4;
}

template<class T> inline void FIELD3<T>::extract(int ax,int pos,FIELD<T>& sl)
{
  int i,j;
  switch(ax)
  {
     case 0:   sl.size(ny,nz);
     	       for(i=0;i<ny;i++)
     	          for(j=0;j<nz;j++) sl.value(i,j) = value(pos,i,j);
     	       break;      	
     case 1:   sl.size(nx,nz);
     	       for(i=0;i<nx;i++)
     	          for(j=0;j<nz;j++) sl.value(i,j) = value(i,pos,j);
     	       break;      	
     case 2:   sl.size(nx,ny);
     	       for(i=0;i<nx;i++)
     	          for(j=0;j<ny;j++) sl.value(i,j) = value(i,j,pos);
     	       break;      	
  }
}

template<class T> inline void FIELD3<T>::insert(int ax,int pos,FIELD<T>& sl)
{
  int i,j;
  switch(ax)
  {
     case 0:   for(i=0;i<ny;i++)
     	          for(j=0;j<nz;j++) value(pos,i,j) = sl.value(i,j);
     	       break;      	
     case 1:   for(i=0;i<nx;i++)
     	          for(j=0;j<nz;j++) value(i,pos,j) = sl.value(i,j);
     	       break;      	
     case 2:   
     	       for(i=0;i<nx;i++)
     	          for(j=0;j<ny;j++) value(i,j,pos) = sl.value(i,j);
     	       break;      	
  }
}


template <class T> inline FIELD3<T>::~FIELD3()
{  delete[] v;  }

template <class T> inline void FIELD3<T>::size(const FIELD3& f)
{
   delete[] v;
   nx = f.nx; ny = f.ny; nz = f.nz;
   v = (nx*ny*nz) ? new T[nx*ny*nz] : 0;
}
template <class T> inline void FIELD3<T>::size(int i,int j,int k)
{
   delete[] v;
   nx = i; ny = j; nz = k;
   v = (nx*ny*nz) ? new T[nx*ny*nz] : 0;
}


template <class T> FIELD3<T>& FIELD3<T>::operator=(FIELD3& f)
{
   if (nx!=f.nx || ny!=f.ny || nz!=f.nz)
	size(f);
   if (nx*ny*nz) memcpy(v,f.v,nx*ny*nz*sizeof(T));
   return *this;
}   


template <class T> FIELD3<T>& FIELD3<T>::operator+=(const FIELD3& f)
{
   if (f.dimX()==dimX() && f.dimY()==dimY() && f.dimZ()==dimZ())
   { 
      const T* fptr = f.data();
      for(T *vptr=v,*vend=v+nx*ny*nz;vptr<vend;vptr++,fptr++)
         (*vptr) += (*fptr);
   }
   return *this;
}


template <class T> FIELD3<T>& FIELD3<T>::operator=(T val)
{
   for(T* vptr=v,*vend=v+nx*ny*nz;vptr<vend;vptr++)
      (*vptr) = val;
   return *this;
}   

template <class T> void FIELD3<T>::gradnorm(FIELD3& f) const
{
   f.size(*this);
  
   for(int k=0;k<nz;k++)
      for(int j=0;j<ny;j++)
         for(int i=0;i<nx;i++)
            f.value(i,j,k) = gradnorm(i,j,k);
}

template <class T> void FIELD3<T>::minmax(T& m,T& M,T& a) const
{
   const float INFINITY_2 = INFINITY/2;   
   
   if (nx*ny*nz<2) { m = M = a = 0; return; }
   m = v[0];
   M = -T(INFINITY);
   a = 0;


   for(T* vptr = v,*vend = v+nx*ny*nz;vptr<vend;vptr++)
   {
	if (m > *vptr) m = *vptr;
    	if (M < *vptr && *vptr < INFINITY_2) M = *vptr;
	a += *vptr;
   }
   a /= nx*ny*nz;
}      

template <class T> FIELD3<T>& FIELD3<T>::operator*=(T f)
{
   for(T* vptr = v,*vend = v+nx*ny*nz;vptr<vend;vptr++)
      *vptr *= f;
   return *this;
}



template <class T> FIELD3<T>* FIELD3<T>::read(char* fname)
{
   FILE* fp = fopen(fname,"r");
   if (!fp) return 0;
   
   char buf[10];
   if (fscanf(fp,"%s",buf)!=1) return 0;
   fclose(fp);
   
   if (!strcmp(buf,"#")) return readVTK(fname);
   if (!strcmp(buf,"P5")) return readPGM(fname);
   return readASCII(fname);
}   
   


template <class T> FIELD3<T>* FIELD3<T>::readVTK(char* fname)
{
   FILE* fp = fopen(fname,"r");
   if (!fp) return 0;

   char buf[2000]; 
   

   FIELD3<T>* f = 0;
   
   fgets(buf,2000,fp);				//skip 1st line #vtk...
   fgets(buf,2000,fp); 				//skip 2nd comment line
   fscanf(fp,"%s",buf);				//read 'ASCII' or 'BINARY'
   int is_ascii = !strcmp(buf,"ASCII");
   int dimX,dimY,dimZ;

   for(;fscanf(fp,"%s",buf)==1;)
   {
      if (!strcmp(buf,"DIMENSIONS"))
      {
	 fscanf(fp,"%d%d%d",&dimX,&dimY,&dimZ);
	 f = new FIELD3<T>(dimX,dimY,dimZ);
      }	
	
      if (!strcmp(buf,"LOOKUP_TABLE"))		//skip name of lookup-table
      { fgets(buf,2000,fp); break;  }		//and ALL to the end-of-line (important since
      						//binary data may follow)
   }

   if (is_ascii)				
      for(T* d = f->data();fscanf(fp,"%f",d)==1;d++);
   else						//binary-read data
   {
      fread(f->data(),sizeof(T)*dimX*dimY*dimZ,1,fp); //binary-read all data as it comes
      if (sizeof(T)==4)				//swap dwords if needed
         swap4BERange((char*)f->data(),dimX*dimY*dimZ);
      else			
      if (sizeof(T)==2)				//swap words if needed
         swap2BERange((char*)f->data(),dimX*dimY*dimZ);
   }   
   
   fclose(fp);   
   return f;
}


template <class T> FIELD3<T>* FIELD3<T>::readPGM(char* fname)		//read PGM-like voxel file into this
{									//
   FILE* fp = fopen(fname,"r"); if (!fp) return 0;			//
									//
   const int SIZE = 2048;
   char buf[SIZE]; int dimX,dimY,dimZ,range;
   fscanf(fp,"%*s");				//skip "P5" header

   for(;;)
   {
     fscanf(fp,"%s",buf);			//get dimX or #comment
     if (buf[0]=='#') fgets(buf,SIZE,fp); 
        else { dimX = atoi(buf); break; }
   }
   for(;;)
   {
     fscanf(fp,"%s",buf);			//get dimY or #comment
     if (buf[0]=='#') fgets(buf,SIZE,fp); 
        else { dimY = atoi(buf); break; }
   }
   for(;;)
   {
     fscanf(fp,"%s",buf);			//get dimZ or #comment
     if (buf[0]=='#') fgets(buf,SIZE,fp); 
        else { dimZ = atoi(buf); break; }
   }
   for(;;)
   {
     fscanf(fp,"%s",buf);			//get range or #comment
     if (buf[0]=='#') fgets(buf,SIZE,fp); 
        else { range = atoi(buf); break; }
   }
   
   fgets(buf,SIZE,fp);						//skip last newline before binary data
  
   long crt = ftell(fp);					//Measure how many bytes we still have 
   fseek(fp,0,SEEK_END);					//from current position till end-of-file.
   long bsz = ftell(fp)-crt;					//Then divide this by #voxels to get	
   fseek(fp,crt,SEEK_SET);					//voxel-size, in bytes. This is needed to know
   int vox_sz = int(bsz/(dimX*dimY*dimZ));			//how to read the voxels (bytes,floats,whatever)

   FIELD3<T>* f = new FIELD3<T>(dimX,dimY,dimZ);

   int buf_sz = int(SIZE/vox_sz);				//buf must hold buf_sz*vox_sz items
   int bb = buf_sz;						//force buffer to be filled right away
  
   if (vox_sz==1)						//read UNSIGNED CHAR file:
     for(T *d = f->data(),*end=d+dimX*dimY*dimZ;d<end;d++)	//read the binary data into the field
     {								//be careful: buf is a char, we first need
	if (bb==buf_sz) { fread(buf,buf_sz,1,fp); bb=0; }	//to convert the read bytes to unsigned char and then assign
	*d = (unsigned char)buf[bb++];				//to the field!
     }
   else if (vox_sz==4)						//read 4-byte FLOAT file:
     for(T *d = f->data(),*end=d+dimX*dimY*dimZ;d<end;d++)	//read the binary data into the field
     {								//be careful: buf is a char*, we first need
	if (bb==buf_sz) { fread(buf,buf_sz,vox_sz,fp); bb=0; }	//to cast it to a float* and then assign

	int p = bb*4;
	#define SWAP(a,b) { char c=a; a=b; b=c; }
	SWAP(buf[p],buf[p+3]);					//
	SWAP(buf[p+1],buf[p+2]);				//

	*d = ((float*)buf)[bb++];				//to the field!

	if (*d > 5) *d = 5;
     }


   fclose(fp);  				
   return f;
}



template <class T> FIELD3<T>* FIELD3<T>::readASCII(char* fname)	//read plain ASCII file into this
{
   FILE* fp = fopen(fname,"r");
   if (!fp) return 0;

   FIELD3<T>* f = 0;

   int dimX,dimY,dimZ;
   fscanf(fp,"%d%d%d",&dimX,&dimY,&dimZ);
   f = new FIELD3<T>(dimX,dimY,dimZ);

   for(T* d = f->data();fscanf(fp,"%f",d)==1;d++);

   fclose(fp);  				
   return f;
}


inline void FIELD3<int>::write(char* fname) const
{
   FILE* fp = fopen(fname,"w");
   if (!fp) return;
   int corr = getVTKCellData();			//correct nx,ny,nz depending of point or cell data written

   fprintf(fp,"# vtk DataFile Version 2.0\n"
	      "vtk output\n"
	      "%s\n"
	      "DATASET STRUCTURED_POINTS\n"
	      "DIMENSIONS %d %d %d\n"
	      "SPACING 1 1 1\n"
	      "ORIGIN 0 0 0\n"
	      "%s %d\n"
	      "SCALARS scalars int\n"
	      "LOOKUP_TABLE default\n",
	      (getVTKAscii())? "ASCII":"BINARY",
	      nx+corr,ny+corr,nz+corr,
	      (getVTKCellData())? "CELL_DATA":"POINT_DATA",
	      nx*ny*nz);

   if (getVTKAscii())
     for(int* vend=v+nx*ny*nz,*vptr=v;vptr<vend;vptr++)
        fprintf(fp,"%d\n",*vptr);
   else						//write binary file
   {
      if (sizeof(int)==4) swap4BERange((char*)v,nx*ny*nz);	//swap to big endian format (VTK file format)
      else		  swap2BERange((char*)v,nx*ny*nz);	//swap 2 or 4 bytes, depending on int size
      fwrite(v,nx*ny*nz*sizeof(int),1,fp);			//write data
      if (sizeof(int)==4) swap4BERange((char*)v,nx*ny*nz);	//ok, now swap data back to out in-core rep
      else		  swap2BERange((char*)v,nx*ny*nz);
   }
     
   fclose(fp);
}	

inline void FIELD3<float>::write(char* fname) const
{
   FILE* fp = fopen(fname,"w");
   if (!fp) return;
   int corr = getVTKCellData();                 //correct nx,ny,nz depending of point or cell data written

   fprintf(fp,"# vtk DataFile Version 2.0\n"
	      "vtk output\n"
	      "%s\n"
	      "DATASET STRUCTURED_POINTS\n"
	      "DIMENSIONS %d %d %d\n"
	      "SPACING 1 1 1\n"
	      "ORIGIN 0 0 0\n"
	      "%s %d\n"
	      "SCALARS scalars float\n"
	      "LOOKUP_TABLE default\n",
	      (getVTKAscii())? "ASCII":"BINARY",
	      nx+corr,ny+corr,nz+corr,
	      (getVTKCellData())? "CELL_DATA":"POINT_DATA",
	      nx*ny*nz);

   if (getVTKAscii())
     for(float* vend=v+nx*ny*nz,*vptr=v;vptr<vend;vptr++)
        fprintf(fp,"%f\n",*vptr);
   else						//write binary file
   {
      if (sizeof(float)==4) swap4BERange((char*)v,nx*ny*nz);	//swap to big endian format (VTK file format)
      else		    swap2BERange((char*)v,nx*ny*nz);	//swap 2 or 4 bytes, depending on int size
      fwrite(v,nx*ny*nz*sizeof(float),1,fp);			//write data
      if (sizeof(float)==4) swap4BERange((char*)v,nx*ny*nz);	//ok, now swap data back to out in-core rep
      else	  	    swap2BERange((char*)v,nx*ny*nz);
   }
      
   fclose(fp);
}	

template <class T> void FIELD3<T>::write(char* fname) const
{
   int asc = getVTKAscii();
   setVTKAscii(1);
   FIELD3<float>::write(fname);
   setVTKAscii(asc);
}	


template <class T> void FIELD3<T>::writePPM(char* fname,int dir,int plane) const
{
   FILE* fp = fopen(fname,"w");
   if (!fp) return;

   float m,M; T m_,M_,avg_; minmax(m_,M_,avg_);
   m = m_; M = M_; 

   const int SIZE = 3000;
   unsigned char buf[SIZE];
   int bb=0;

   int DX=0,DY=0;					//get DX,DY == dimensions of slice orthogonal to axis 'dir'
   if (dir==0) { DX = dimY(); DY = dimZ(); if (plane==-1) plane = dimX()/2; }
   if (dir==1) { DX = dimX(); DY = dimZ(); if (plane==-1) plane = dimY()/2; }
   if (dir==2) { DX = dimX(); DY = dimY(); if (plane==-1) plane = dimZ()/2; }

   fprintf(fp,"P6 %d %d 255\n",DX,DY);

   for(int j=0;j<DY;j++)				//iterate over all points of the slice plane
      for(int i=0;i<DX;i++)
      {
	 int i0=0,i1=0,i2=0;				//translate the iteration-idxs i,j and slice-position plane into field-idxs i0,i1,i2
         if (dir==0) { i0 = plane; i1 = i;     i2 = j; }
         if (dir==1) { i0 = i;     i1 = plane; i2 = j; }
         if (dir==2) { i0 = i;     i1 = j;     i2 = plane; }

         T val = value(i0,i1,i2);			//get value on slice plane

         float r,g,b,v = (val-m)/(M-m); 
         v = max(v,0); 
         if (v>M) { r=g=b=1; } else v = min(v,1);
         float2rgb(v,r,g,b);
      
         buf[bb++] = (unsigned char)(int)(r*255);
         buf[bb++] = (unsigned char)(int)(g*255);
         buf[bb++] = (unsigned char)(int)(b*255);
         if (bb==SIZE)
         {  fwrite(buf,1,SIZE,fp); bb = 0;  }
      }
   if (bb) fwrite(buf,1,bb,fp);

   fclose(fp);
}	





//----------------------  VFIELD3  ----------------------------


template <class T> inline void VFIELD3<T>::setValue(int i,int j,int k,T* val)
{
   v0.value(i,j,k) = val[0];
   v1.value(i,j,k) = val[1];
   v2.value(i,j,k) = val[2];
}


template <class T> VFIELD3<T>* VFIELD3<T>::read(char* fname)
{
   FILE* fp = fopen(fname,"r");
   if (!fp) return 0;

   VFIELD3<T>* f = 0;

   char buf[100]; 
   for(;fscanf(fp,"%s",buf)==1;)
   {
      if (!strcmp(buf,"DIMENSIONS"))
      {
	int dimX,dimY,dimZ;
	fscanf(fp,"%d%d%d",&dimX,&dimY,&dimZ);
	f = new VFIELD3<T>(dimX,dimY,dimZ);
      }	
	
      if (!strcmp(buf,"VECTORS"))
      {
	 fscanf(fp,"%*s%*s");
	 break;
      }
   }

   for(T* d0=f->v0.data(),*d1=f->v1.data(),*d2=f->v2.data();fscanf(fp,"%f%f%f",d0,d1,d2)>1;d0++,d1++,d2++);

   fclose(fp);  				
   return f;
}

template <class T> void VFIELD3<T>::write(const char* fname,FLAGS3& f) const
{
   FILE* fp = fopen(fname,"w");
   if (!fp) return;

   fprintf(fp,"# vtk DataFile Version 2.0\n"
	      "vtk output\n"
	      "ASCII\n"
	      "DATASET STRUCTURED_POINTS\n"
	      "DIMENSIONS %d %d 1\n"
	      "SPACING 1 1 1\n"
	      "ORIGIN 0 0 0\n"
	      "POINT_DATA %d\n"
	      "VECTORS vectors float\n",
	      dimX(),dimY(),dimX()*dimY());

   for(int k=0;k<dimZ();k++)
     for(int j=0;j<dimY();j++)
        for(int i=0;i<dimX();i++)
           if (f.alive(i,j,k))
              fprintf(fp,"0 0 0\n");
           else
              fprintf(fp,"%f %f %f\n",v0.value(i,j,k),v1.value(i,j,k),v2.value(i,j,k));

   fclose(fp);
}	




#endif
