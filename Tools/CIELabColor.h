#ifndef cie_lab_color_h_
#define cie_lab_color_h_

#ifdef VCL_NEEDS_PRAGMA_INTERFACE
#pragma interface
#endif

#include <vcl_iostream.h>

//: This is the appropriate pixel type for CIE Lab color with 32-bit depth per channel.
//
//    Currently also includes the following `utilities':
//    -  conversion to grey (luminance of CIELabColor).
//    -  arithmetic operations
/*template <class T>
struct CIELabColor
{
  typedef T value_type;

  inline CIELabColor() { }

  //:Create grey (v,v,v) CIELabColor cell from value v.
  // This provides a conversion from T to CIELabColor<T>

  inline CIELabColor(T v):
    L(v), a(v), b(v) { }

  //:Construct an CIELabColor value.
  inline CIELabColor(T luminance , T opp_color_a, T opp_color_b):
    L(luminance), a(opp_color_a), b(opp_color_b) { }

  // The Lab values
  T L, a, b;

  inline T Lum() const { return L; }
  inline T A() const { return a; }
  inline T B() const { return b; }

  //:Convert CIELabColor to gray
  inline T grey() const { return L; }

  //: equality
  inline bool operator== (CIELabColor<T> const& o) const { return L==o.L && a==o.a && b==o.b; }

  // operators
  inline CIELabColor<T>  operator+  (CIELabColor<T> const& A) const { return CIELabColor<T>(L+A.L,a+A.a,b+A.b); }
  inline CIELabColor<T>  operator-  (CIELabColor<T> const& A) const { return CIELabColor<T>(L-A.L,a-A.a,b-A.b); }
  inline CIELabColor<T>  operator/  (CIELabColor<T> const& A) const { return CIELabColor<T>(L/A.L,a/A.a,b/A.b);}
  inline CIELabColor<T>& operator+= (CIELabColor<T> const& A) { L+=A.L,a+=A.a,b+=A.b; return *this; }
  inline CIELabColor<T>& operator-= (CIELabColor<T> const& A) { L-=A.L,a-=A.a,b-=A.b; return *this; }
  inline CIELabColor<T>  operator*  (T A) const { return CIELabColor<T>(L*A,a*A,b*A); }
  inline CIELabColor<T>  operator/  (T A) const { return CIELabColor<T>(L/A,a/A,b/A); }
  inline CIELabColor<T>& operator*= (T A) { L*=A,a*=A,b*=A; return *this; }
  inline CIELabColor<T>& operator/= (T A) { L/=A,a/=A,b/=A; return *this; }

  template <class S> inline
  CIELabColor(CIELabColor<S> const& that):
    L(T(that.L)),
    a(T(that.a)),
    b(T(that.b)) { }

  template <class S> inline
  CIELabColor<T>& operator=(CIELabColor<S> const& that) 
  {
    L=T(that.L);
    a=T(that.a);
    b=T(that.b);
    return *this;
  }
};

template <class T>
inline
vcl_ostream& operator<<(vcl_ostream& s, CIELabColor<T> const& rgb)
{
  return s << '[' << rgb.L << ' ' << rgb.a << ' ' << rgb.b << ']';
}

//VCL_DEFINE_SPECIALIZATION
//vcl_ostream& operator<<(vcl_ostream& s, CIELabColor<unsigned char> const& rgb);


// ** Arithmetic operators

template <class T>
inline
bool operator!= (CIELabColor<T> const& a, CIELabColor<T> const& b) 
{
  return !(a==b);
}

template <class T>
inline
CIELabColor<T> average(CIELabColor<T> const& a, CIELabColor<T> const& b)
{
  return CIELabColor<T>((a.L + b.L)/2, (a.a + b.a)/2, (a.b + b.b)/2);
}

template <class T>
inline
CIELabColor<T> operator+(CIELabColor<T> const& a, CIELabColor<T> const& b)
{
  return CIELabColor<T>(a.L + b.L, a.a + b.a, a.b + b.b);
}

template <class T>
inline
CIELabColor<T> operator*(CIELabColor<T> const& a, CIELabColor<T> const& b)
{
  return CIELabColor<T>(a.L * b.L, a.a * b.a, a.b * b.b);
}

template <class T>
inline
CIELabColor<double> operator*(double b, CIELabColor<T> const& a)
{
  return CIELabColor<double>(a.L * b, a.a * b, a.b * b);
}

template <class T>
inline
CIELabColor<double> operator*(CIELabColor<T> const& a, double b)
{
  return CIELabColor<double>(a.L * b, a.a * b, a.b * b);
}

template <class T>
inline
CIELabColor<double> operator/(CIELabColor<T> const& a, double b)
{
  return CIELabColor<double>(a.L / b, a.a / b, a.b / b);
}*/

#endif // cie_lab_color_h_

