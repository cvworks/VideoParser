#ifndef VASSILIS_DRAWING_H
#define VASSILIS_DRAWING_H


#include "matrix.h"
#include "precomputed.h"
#include "basics/angles.h"


class vPolygon
{
public:
  vint8 num_vertices;
  vector<vint8> x_coordinates;
  vector<vint8> y_coordinates;

  vPolygon();
  ~vPolygon();
  void AddVertex(vint8 row, vint8 col);
  void DeleteVertex();
  void Print();
  // Writes the four bounds of the polygon into
  // the arguments. Returns 1 of the set is not empty, 
  // 0 if the set is empty (in which case the bounds are not
  // defined).
  ushort GetBounds(vint8 * top, vint8 * bottom, vint8 * left, vint8 * right);
};


class vPoint2
{
public:
  double row, col;
  vPoint2();
  vPoint2(double in_row, double in_col)
  {
    row = in_row;
    col = in_col;
  }
};


class vPoint3
{
public:
  double x, y, z;
  vPoint3() {};
  vPoint3(double in_x, double in_y, double in_z)
  {
    x = in_x;
    y = in_y;
    z = in_z;
  }

  vint8 Print(const char * text);
};


class vPoint3f
{
public:
  float x, y, z;
  vPoint3f() {};
  vPoint3f(float in_x, float in_y, float in_z)
  {
    x = in_x;
    y = in_y;
    z = in_z;
  }

  vint8 Print(const char * text);
};


class vCircle
{
public:
  double center_row, center_col;
  double radius;

  vCircle()
  {
    center_row = center_col = radius = 0;
  }

  vCircle(double in_row, double in_col, double in_radius)
  {
    center_row = in_row;
    center_col = in_col;
    radius = in_radius;
  }
};


class vCircleVint8
{
public:
  vint8 center_row, center_col;
  vint8 radius;

  vCircleVint8()
  {
    center_row = center_col = radius = 0;
  }

  vCircleVint8(vint8 in_row, vint8 in_col, vint8 in_radius)
  {
    center_row = in_row;
    center_col = in_col;
    radius = in_radius;
  }
};


vint8 vLeftEnd(vPolygon * polygon);
vint8 vRightEnd(vPolygon * polygon);
vint8 vTopEnd(vPolygon * polygon);
vint8 vBottomEnd(vPolygon * polygon);


// This class was created to represent sets of line segments. Each
// line segment is defined by its two endpoints, which are 2D pixel
// points (integers). Internally, the set of segments is represented
// as the union of smaller sets of segments, each of which is defined
// by a polygon. A polygon with vertices A, B, C, D, for example,
// is used to represent the set of line segments {AB, BC, CD} (note,
// AD is not included).
//
// To create a line set, first call the constructor (which automatically
// initializes an empty polygon). Then, add vertices by calling AddVertex()
// and new polygons by calling StartPolygon(). Calling DeleteVertex() deletes
// the most recently added vertex. If that results in the current polygon 
// having no vertices, then the current polygon gets deleted, and the preceding
// polygon becomes current (unless the current polygon is the only polygon in
// the set of polygons, in which case we keep it there).
class vLineSet
{
private:
  vector<voidp> polygons;
  
  // The color we should use to draw the lines
  uchar r, g, b;
  ushort DrawColor(color_image * the_image);
  ushort DrawGray(grayscale_image * gray_image);

public:
  vLineSet();

  ~vLineSet();

  vPolygon * CurrentPolygon();

  // Adds a new polygon to the list of polygons
  ushort StartPolygon();


  // Adds a vertex to the most recent polygon
  void AddVertex(vint8 row, vint8 col);

  // Deletes the last vertex of the most recent polygon.
  void DeleteVertex();

  // Writes the four bounds of the line segment set into
  // the arguments. Returns 1 of the set is not empty, 
  // 0 if the set is empty (in which case the bounds are not
  // defined).
  ushort GetBounds(vint8 * top, vint8 * bottom, vint8 * left, vint8 * right);

  // Draws each line segment into the image
  ushort Draw(general_image * the_image);
  void SetColor(uchar in_r, uchar in_g, uchar in_b);
  inline vPolygon * GetPolygon(vint8 index)
  {
    if (index < 0) return 0;
    else if (polygons.size() <= (vint8) index) return 0;
    else return (vPolygon *) polygons[(vector_size) index];
  }
  inline vint8 NumberOfPolygons()
  {
    return (vint8) polygons.size();
  }
};



class vColorMap
{
public:
  vint8 number_of_colors;
  std::vector<uchar> R;
  std::vector<uchar> G;
  std::vector<uchar> B;

  vColorMap();

  // Build a color map for color_image, creates an image with the size
  // of color_image and puts it into color_ids, and stores in that image,
  // in every pixel, the color map ID of the color of that pixel in color_image.
  vColorMap(color_image * the_image, v3dMatrix<vint8> ** color_ids);
  ~vColorMap();

  // Writes the color map to a file
  ushort Write(FILE * fp);

  // Reads a color map from a file
  ushort Read(FILE * fp);

  // Apply the colormap to the gray image.
  color_image * Convert(grayscale_image * gray);

  vColorMap * Copy();

  inline vint8 Color(vint8 index, vint8 * r, vint8 * g, vint8 * b)
  {
    if ((index < 0) || (index >= number_of_colors))
    {
      return 0;
    }
    *r = R[(vector_size) index];
    *g = G[(vector_size) index];
    *b = B[(vector_size) index];
    return 1;
  }
};

ushort vColorMapEqual(vColorMap * map1, vColorMap * map2);

grayscale_image * vCopyGrayImage(grayscale_image * source);

void vCopyGrayImage(grayscale_image * target, grayscale_image * source);

color_image * vReadBMP(const char * filename);

inline color_image * function_read_bitmap (const char * filename)
{
  return vReadBMP(filename);
}

grayscale_image * vReadGrayBMP(const char * filename);

inline grayscale_image * function_read_gray_bitmap (const char * filename)
{
  return vReadGrayBMP(filename);
}

short vWriteColorBMP(color_image * the_image, const char * filename);

inline short function_save_bitmap(color_image * image, const char * filename)
{
  return vWriteColorBMP(image, filename);
}

short vWriteGrayBMP(grayscale_image * the_image, const char * filename);

inline short function_save_grayscale_bitmap(grayscale_image * image, const char * filename)
{
  return vWriteGrayBMP(image, filename);
}


void inline vLineSteps(double row1, double col1, double row2, double col2,
                        double * row_step, double * col_step, vint8 * steps)
{
  double row_diff = row2 - row1;
  double col_diff = col2 - col1;

  if (vAbs(row_diff) > vAbs(col_diff))
  {
    *row_step = 1.0;
    *col_step = vAbs(col_diff / row_diff);
    *steps = round_number(vAbs(row_diff) + 1);
  }
  else
  {
    *col_step = 1.0;
    *row_step = vAbs(row_diff / col_diff);
    *steps = round_number(vAbs(col_diff) + 1);
  }

  if (row_diff < 0) *row_step = -*row_step;
  if (col_diff < 0) *col_step = -*col_step;
}


void line_endpoints(vint8 center_row, vint8 center_col, 
                    float orientation, vint8 length,
                    vint8 * row1, vint8 * col1, 
                    vint8 * row2, vint8 * col2);


void inline vLineEnds(double center_row, double center_col, 
                       double orientation, double length,
                       double * row1, double * col1, 
                       double * row2, double * col2,
                       double * row_step, double * col_step,
                       vint8 * steps)
{
  static const double junk1 = vPrecomputedSin::PrecomputeValues();
  static const double junk2 = vPrecomputedCos::PrecomputeValues();

  double dx = vPrecomputedCos::Cos(orientation);
  double dy = vPrecomputedSin::Sin(orientation);

  double abs_dx = fabs(dx);
  double abs_dy = fabs(dy); 
  double x_direction, y_direction;
  if (dx < 0) x_direction = -1;
  else x_direction = 1;
  if (dy < 0) y_direction = -1;
  else y_direction = 1;

  if (abs_dx >= abs_dy) 
  {
    *col_step = x_direction;
    *row_step = abs_dy / abs_dx * y_direction;
  }

  else
  {
    *col_step = abs_dx / abs_dy * x_direction;
    *row_step = y_direction;
  }

  double half_length = length/2.0;
  double end_x = half_length * dx;
  double end_y = half_length * dy;
  *row1 = center_row - end_y;
  *col1 = center_col - end_x;
  *row2 = center_row + end_y;
  *col2 = center_col + end_x;
  if (abs_dy > abs_dx)
  {
    *steps = round_number((*row2 - *row1) / *row_step);
  }
  else
  {
    *steps = round_number((*col2 - *col1) / *col_step);
  }
}


// Returns an angle between 0 and pi. In general the values are 
// the same as in the standard trigonometric circle:
// vAngle(1, 0) = 0
// vAngle(1, 1) = PI/4
// vAngle(0, 1) = PI/2
// vAngle(-1, 1) = 3PI/4
// vAngle(-1, 0) = 0
// vAngle(-1, -1) = PI/4
// vAngle(0, -1) = PI/2
// vAngle(1, -1) = 3PI/4
inline double vAngle(double x, double y)
{
  double result;
  if (x == 0)
  {
    if (y != 0) result = (double) (M_PI / 2.0);
    else result = 0;
  }
  else
  {
    double tangent = y / x;
    result = atan(tangent);
    // Make sure the result is not negative
    if (result < 0) result = result + (double) M_PI;
  }
  return result;
}


// Returns an angle between 0 and 2*pi. In general the values are 
// the same as in the standard trigonometric circle:
// vAngle(1, 0) = 0
// vAngle(1, 1) = PI/4
// vAngle(0, 1) = 2PI/4
// vAngle(-1, 1) = 3PI/4
// vAngle(-1, 0) = PI
// vAngle(-1, -1) = 5PI/4
// vAngle(0, -1) = 6PI/4
// vAngle(1, -1) = 7PI/4
inline double vAngle2(double x, double y)
{
  static const double double_pi = (double) (2.0 * M_PI);
  if ((x == 0) && (y == 0)) return 0;
  double angle = atan2(y, x);
  if (y < 0) angle += double_pi;
  return angle;
}


inline void vRotatePoint(double x, double y, double angle, 
                          double & new_x, double & new_y)
{
  // Get the length of the point
  double length = sqrt(x*x + y*y);
  // Get the angle of the line connecting the origin to the point.
  double point_angle = vAngle2(x, y);
  // Rotate the angle of the line connecting the origin to the point.
  double new_angle = point_angle + angle;
  // Get values for new_x and new_y corresponding to new angle
  new_y = sin(new_angle) * length;
  new_x = cos(new_angle) * length;
}


inline double vBilinearInterpolation(double x, double y, 
			                               double top_left, double top_right, 
			                               double bottom_left, double bottom_right)
{
    // I looked up the formulas from "Numerical Recipes in C".

    double result;
    result = (1 - x) * (1 - y) * top_left;
    result += x * (1 - y) * top_right;
    result += x * y * bottom_right;
    result += (1 - x) * y * bottom_left;
    return result;
}


inline float vBilinearInterpolation(float x, float y, 
  	                                 float top_left, float top_right, 
 		                                 float bottom_left, float bottom_right)
{
    // I looked up the formulas from "Numerical Recipes in C".

    float result;
    result = (1 - x) * (1 - y) * top_left;
    result += x * (1 - y) * top_right;
    result += x * y * bottom_right;
    result += (1 - x) * y * bottom_left;
    return result;
}


double vBilinearInterpolation(const v3dMatrix<double> * the_image, 
                              double row, double col);

float vBilinearInterpolation(const v3dMatrix<float> * the_image, 
                             float row, float col);

double vBilinearInterpolation(grayscale_image * gray, double row, double col);

inline float vBilinearInterpolation(vArray2(uchar) values, float row, float col)
{
  float top_left, top_right, bottom_left, bottom_right;
  vint8 top, bottom, left, right;
  float x, y;

  // Get the bounding square.
  top = (vint8) floor(row);
  left = (vint8) floor(col);
  bottom = top + 1;
  right = left + 1;
  
  // Get values at the corners of the square
  top_left = values[top][left];
  top_right = values[top][right];
  bottom_left = values[bottom][left];
  bottom_right = values[bottom][right];

  x = col - left;
  y = row - top;
  
  float result;
  result = vBilinearInterpolation(x, y, top_left, top_right, 
				   bottom_left, bottom_right);

  return result;
}

inline double vBilinearInterpolationD(vArray2(double) values, double row, double col)
{
  double top_left, top_right, bottom_left, bottom_right;
  vint8 top, bottom, left, right;
  double x, y;

  // Get the bounding square.
  top = (vint8) floor(row);
  left = (vint8) floor(col);
  bottom = top + 1;
  right = left + 1;
  
  // Get values at the corners of the square
  top_left = values[top][left];
  top_right = values[top][right];
  bottom_left = values[bottom][left];
  bottom_right = values[bottom][right];

  x = col - left;
  y = row - top;
  
  double result;
  result = vBilinearInterpolation(x, y, top_left, top_right, 
				   bottom_left, bottom_right);

  return result;
}

template <class type>
inline double vBilinearInterpolationD(vArray2(type) values, double row, double col)
{
  double top_left, top_right, bottom_left, bottom_right;
  vint8 top, bottom, left, right;
  double x, y;

  // Get the bounding square.
  top = (vint8) floor(row);
  left = (vint8) floor(col);
  bottom = top + 1;
  right = left + 1;
  
  // Get values at the corners of the square
  top_left = values[top][left];
  top_right = values[top][right];
  bottom_left = values[bottom][left];
  bottom_right = values[bottom][right];

  x = col - left;
  y = row - top;
  
  double result;
  result = vBilinearInterpolation(x, y, top_left, top_right, 
				   bottom_left, bottom_right);

  return result;
}

float vBilinearInterpolation(vArray2(float) values, float row, float col);

// result[row][col] = (row2, col2) iff rotating point (row, col)
// by given degrees around center (center_row, center_col) moves
// (row, col) to (row2, col2);
v3dMatrix<short> * vRotationTemplate(vint8 rows, vint8 cols, 
                                        vint8 center_row, vint8 center_col,
                                        double degrees);

// Same as vRotationTemplate, but here the angle is specified in radians.
v3dMatrix<short> * vRotationTemplateR(vint8 rows, vint8 cols, 
                                         vint8 center_row, vint8 center_col,
                                         double radians);

// here we return floats, so that we can do bilinear interpolation.
v3dMatrix<float> * vRotationTemplatef(vint8 rows, vint8 cols, 
                                         vint8 center_row, vint8 center_col,
                                         double degrees);

v3dMatrix<float> * vRotationTemplateRf(vint8 rows, vint8 cols, 
                                          vint8 center_row, vint8 center_col,
                                          double radians);


// the cropped versions put -1 in entries that fall outside the bounds
// of an image with the specified rows and columns.
v3dMatrix<float> * cropped_rotation_template_degrees(vint8 rows, vint8 cols, 
                                                     vint8 center_row, vint8 center_col,
                                                     double degrees);

v3dMatrix<float> * cropped_rotation_template(vint8 rows, vint8 cols, 
                                             vint8 center_row, vint8 center_col,
                                             double radians);

// this function puts -1 in all entries of warping_template that
// are outside the image bounds of an image of the same size
// as warping template.
vint8 crop_warping_template(float_image * warping_template);

v3dMatrix<uchar> * vRotateImage(v3dMatrix<uchar> * the_image, 
                                   vint8 center_row, vint8 center_col,
                                   double degrees);

v3dMatrix<uchar> * vRotateImageR(v3dMatrix<uchar> * the_image, 
                                    vint8 center_row, vint8 center_col,
                                    double radians);

static inline v3dMatrix<uchar> * function_rotate_image(v3dMatrix<uchar> * the_image, 
                                   vint8 center_row, vint8 center_col,
                                   double degrees)
{
  return vRotateImage(the_image, center_row, center_col, degrees);
}

static inline v3dMatrix<uchar> * function_rotate_image_radians(v3dMatrix<uchar> * the_image, 
                                    vint8 center_row, vint8 center_col,
                                    double radians)
{
  return vRotateImageR(the_image, center_row, center_col, radians);
}


// vQuadPixels returns the pixels (i.e. points with integer coordinates)
// that bevint8 to the quadrilateral specified by the given rows and cols.
// It should also work for a triangle (where we just repeat the last
// vertex twice to make it a quadrilateral).
// We assume that the quadrilateral is convex.
vint8 vQuadPixels(class_pointer (vint8) rows, class_pointer (vint8) cols, 
                  vector<vPoint> * result);


// vTrianglePixels returns the pixels (i.e. points with integer coordinates)
// that bevint8 to the triangle specified by the given rows and cols.
vint8 vTrianglePixelsSlow(class_pointer (vint8) rows, class_pointer (vint8) cols, 
                          vector<vPoint> * result);

vint8 vTrianglePixels(class_pointer (vint8) rows, class_pointer (vint8) cols, 
                      vector<vPoint> * result);

vint8 vTrianglePixels2(class_pointer (vint8) rows, class_pointer (vint8) cols, 
                       vector<vPoint> * result);


vint8 vCutTriangle(class_pointer (vint8), class_pointer (vint8) cols, 
                  matrix_pointer (vint8) result_rows, matrix_pointer (vint8) result_cols,
                   vint8 max_length);

vint8 vCutTriangle3D(class_pointer (double) xs, class_pointer (double) ys, class_pointer (double) zs, 
                     matrix_pointer (double) result_xs, matrix_pointer (double) result_ys, 
                     matrix_pointer (double) result_zs, double max_length);

vint8 vDrawTriangles1(vint8 number, matrix_pointer (vint8) rows, matrix_pointer (vint8) cols, 
                      color_image * the_image, uchar i);

vint8 vDrawTriangles3(vint8 number, matrix_pointer (vint8) rows, matrix_pointer (vint8) cols, 
                      color_image * the_image, uchar r, uchar g, uchar b);

vint8 vDrawTriangle1(class_pointer (vint8) rows, class_pointer (vint8) cols, color_image * the_image, uchar i);

vint8 vDrawTriangle3(class_pointer (vint8) rows, class_pointer (vint8) cols, color_image * the_image, 
                     uchar r, uchar g, uchar b);

vint8 vLinePoints(vint8 row, vint8 col, double angle, double length,
                  vector<vint8> * rows, vector<vint8> * cols);

vint8 vLinePoints2(double row, double col, double angle, double length, double step,
                   vector<double> * rows, vector<double> * cols);

vint8 vLinePoints5(vint8 row1, vint8 col1, vint8 row2, vint8 col2,
                  vector<vPoint> * points);

vint8 vLinePoints5(vint8 row1, vint8 col1, vint8 row2, vint8 col2,
                  vector<class_pixel> * points);

// For the line connecting (0, 0) to (row, col), return
// the leftmost pixel col at each row, from 0 to row.
vArray(short) vLeftCols(vint8 row, vint8 col);

// For the line connecting (0, 0) to (row, col), return
// the righttmost pixel col at each row, from 0 to row.
vArray(short) vRightCols(vint8 row, vint8 col);

ushort vTranslatePolygon(vPolygon * polygon, vint8 y, vint8 x);

double vVerticalClockwise(double angle);

double vVerticalCounterclockwise(double angle);

ushort vLocatePoint(double y, double x, double angle, double length,
		               double & x1, double & x2);


inline ushort function_translate_polygon(vPolygon * polygon, vint8 y, vint8 x)
{
  return vTranslatePolygon(polygon, y, x);
}

inline double function_vertical_clockwise(double angle)
{
  return vVerticalClockwise(angle);
}

inline double vertical_counterclockwise(double angle)
{
  return vVerticalCounterclockwise(angle);
}

inline ushort function_locate_point(double y, double x, double angle, double length,
		               double & x1, double & x2)
{
  return vLocatePoint(y, x, angle, length, x1, x2);
}


grayscale_image * function_histogram_equalization (ushort_image* input, vint8 window_size);

grayscale_image * function_histogram_equalization_offset (ushort_image* input, vint8 window_size,
                                                 vint8 vertical_offset, vint8 horizontal_offset);


float_image * histogram_equalization_weights(vint8 window_size);



#endif //  VASSILIS_DRAWING_H




