#ifndef IMAGEITERATOR_H
#define IMAGEITERATOR_H
template <class PEL>
class IplImageIterator {

  int i, i0,j;
  PEL* data;
  int step;
  int nl, nc;
  int nch;

 public:
         void goBackToFirst(){i=0; j=0;}

  /* constructor */
  IplImageIterator(IplImage* image,
     int x=0, int y=0, int dx= 0, int dy=0) :
       i(x), j(y), i0(0) {

    data= reinterpret_cast<PEL*>(image->imageData);
    step= image->widthStep / sizeof(PEL);

    nl= image->height;
    if ((y+dy)>0 && (y+dy)<nl) nl= y+dy;
    if (y<0) j=0;
    data+= step*j;

    nc= image->width ;
    if ((x+dx)>0 && (x+dx)<nc) nc= x+dx;
    nc*= image->nChannels;
    if (x>0) i0= x*image->nChannels;
    i= i0;

    nch= image->nChannels;
  }

  /* has next ? */
  bool operator!() const { return j < nl; }

  /* next pixel or next color component */


  IplImageIterator& operator++() {
        i++;
    if (i >= nc) { i=i0; j++; data+= step; }
    return *this;}

  IplImageIterator& operator++(int) {return operator++();}

  IplImageIterator& operator+=(int s) {
        i+= s;
    if (i >= nc) { i=i0; j++; data+= step; }
    return *this;}

  /* pixel access */
  PEL& operator*() { return data[i]; }
  const PEL operator*() const { return data[i]; }
  const PEL neighbor(int dx, int dy) const
    { return *(data+dy*step+i+dx*nch); }
  PEL* operator&() const { return data+i; }

  /* current pixel coordinates */
  int column() const { return i/nch; }
  int line() const { return j; }
  bool isFirstLine() {return (j==0); }
};

#endif // IMAGEITERATOR_H
