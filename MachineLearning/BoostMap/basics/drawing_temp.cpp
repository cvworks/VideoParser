
#ifdef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#endif // VASSILIS_SGI_PLATFORM

#ifdef VASSILIS_PROCESS_FILE
#undef VASSILIS_PROCESS_FILE

#include "drawing.h"
#include "drawing_temp.h"
#include "matrix.h"

template<class type>
static vArray2(uchar) S_PolygonMask(const v3dMatrix<type> * source, 
                                    const vPolygon * polygon);

template<class type>
v3dMatrix<type> * vCopyImage(const v3dMatrix<type> * source)
{
  const vint8 rows = source->Rows();
  const vint8 cols = source->Cols();
  const vint8 bands = source->Bands();
  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vCopyImage(result, source);
  return result;
}

template<class type>
ushort vCopyImage(v3dMatrix<type> * target, const v3dMatrix<type> * source)
{
  vint8 byte_size = source->AllBandSize() * sizeof(type);
  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  vint8 bands = source->Bands();
  assert(rows == target->Rows());
  assert(cols == target->Cols());
  assert(bands == target->Bands());
  
  vArray(type) source_matrix = source->Matrix();
  vArray(type) target_matrix = target->Matrix();
  memcpy(vData(target_matrix), vData(source_matrix), (vector_size)byte_size);
  
  return 1;
}


template<class type>
v3dMatrix<type> * vCopyBand2(const v3dMatrix<type> * source, const vint8 band)
{
  if ((band < 0) || (band >= source->Bands()))
  {
    return 0;
  }
  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, 1);
  vint8 success = vCopyBand3(result, source, band);
  if (success <= 0)
  {
    vdelete(result);
    return 0;
  }
  else
  {
    return result;
  }
}


template<class type>
vint8 vCopyBand3(v3dMatrix<type> * target, const v3dMatrix<type> * source, const vint8 band)
{
  if ((band < 0) || (band >= source->Bands()))
  {
    return 0;
  }
  vint8 size = source->Size();
  if (size != target->Size()) return 0;
  vArray(type) target1 = target->Matrix();
  vArray(type) source1 = source->Matrix(band);
  vint8 byte_size = sizeof(type) * source->Size();
  memcpy(vData(target1), vData(source1), (vector_size)byte_size);
  return 1;
}


template<class type>
vint8 function_copy_band(v3dMatrix<type> * target, const vint8 target_band, 
                        const v3dMatrix<type> * source, const vint8 source_band)
{
  if ((source_band < 0) || (source_band >= source->Bands()))
  {
    return 0;
  }
  if ((target_band < 0) || (target_band >= target->Bands()))
  {
    return 0;
  }
  vint8 size = source->Size();
  if (size != target->Size()) 
  {
    return 0;
  }
  vArray(type) target1 = target->Matrix(target_band);
  vArray(type) source1 = source->Matrix(source_band);
  vint8 byte_size = sizeof(type) * source->Size();
  memcpy(vData(target1), vData(source1), byte_size);
  return 1;
}

template<class type>
v3dMatrix<type> * vCopyAndDrawRectangle(const v3dMatrix<type> * source, 
					                                 const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
					                                 const uchar r, const uchar g, const uchar b)
{
  v3dMatrix<type> * result = vCopyImage(source);
  vDrawRectangle(result, y1, x1, y2, x2, r, g, b);
  return result;
}

template<class type>
ushort vDrawRectangle1(v3dMatrix<type> * the_image,
		                  const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
		                  const type value)
{
  vector<type> values;
  vint8 bands = the_image->Bands();
  vint8 i;
  for (i = 0; i < bands; i++)
    values.push_back(value);
  return vDrawRectangle(the_image, y1, x1, y2, x2, values);
}


template<class type>
ushort vDrawRectangle(v3dMatrix<type>* the_image,
		                 const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
		                 const vector<type> & values)
{
  vint8 i1 = Min(y1, y2);
  vint8 i2 = Max(y1, y2);
  vint8 j1 = Min(x1, x2);
  vint8 j2 = Max(x1, x2);

  // Check that endpoints lie within the image.
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  assert(bands == values.size());
  if (i1 < 0) i1 = 0;
  else if (i1 >= rows) i1 = rows-1;
  
  if (i2 < 0) i2 = 0;
  else if (i2 >= rows) i2 = rows-1;
  
  if (j1 < 0) j1 = 0;
  else if (j1 >= cols) j1 = cols-1;

  if (j2 < 0) j2 = 0;
  else if (j2 >= cols) j2 = cols-1;
	   
  vint8 row, col, band;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix = the_image->Matrix2(band);
    type value = values[(vector_size)band];
    for (row = i1; row <= i2; row++)
    {
      matrix[row][j1] = value;
      matrix[row][j2] = value;
    }

  for (col = j1; col <= j2; col++)
    {
      matrix[i1][col] = value;
      matrix[i2][col] = value;
    }
  }
  return 1;
}


template<class type>
ushort vDrawFilledRectangle1(v3dMatrix<type> * the_image,
			                     const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
			                     const type value)
{
  vector<type> values;
  vint8 i;
  vint8 bands = the_image->Bands();
  for (i = 0; i < bands; i++)
    values.push_back(value);
  return vDrawFilledRectangle(the_image, y1, x1, y2, x2, values);
}

  
template<class type>
ushort vDrawFilledRectangle(v3dMatrix<type> * the_image,
			                     const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
			                     const vector<type> & values)
{
  vint8 i1 = Min(y1, y2);
  vint8 i2 = Max(y1, y2);
  vint8 j1 = Min(x1, x2);
  vint8 j2 = Max(x1, x2);

  // Check that endpoints lie within the image.
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  assert(bands == values.size());
  if (i1 < 0) i1 = 0;
  else if (i1 >= rows) i1 = rows-1;
  
  if (i2 < 0) i2 = 0;
  else if (i2 >= rows) i2 = rows-1;
  
  if (j1 < 0) j1 = 0;
  else if (j1 >= cols) j1 = cols-1;

  if (j2 < 0) j2 = 0;
  else if (j2 >= cols) j2 = cols-1;
	   
  vint8 row, col, band;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix = the_image->Matrix2(band);
    type value = values[(vector_size)band];
    for (row = i1; row <= i2; row++)
      for (col = j1; col <= j2; col++)
	      matrix[row][col] = value;
  }
  return 1;
}


template<class type>
ushort vDrawSquare(v3dMatrix<type> * the_image, const vint8 y, const vint8 x, const double angle, const vint8 size,
		               const vector<type> & values)
{
  angle = -angle; // This adjusts the co-ordinates;
  double diag1 = angle + M_PI / 4;
  double diag2 = angle - M_PI / 4;
  double diag_length = sqrt(2) * ((double) size) / 2.0;
  double x0, y0, x1, y1, x2, y2, x3, y3;
  vLocatePoint(y, x, diag1, diag_length, y0, x0);
  vLocatePoint(y, x, diag2, -diag_length, y1, x1);
  vLocatePoint(y, x, diag1, -diag_length, y2, x2);
  vLocatePoint(y, x, diag2, diag_length, y3, x3);
  
  vDrawLine(the_image, y0, x0, y1, x1, values);
  vDrawLine(the_image, y1, x1, y2, x2, values);
  vDrawLine(the_image, y2, x2, y3, x3, values);
  vDrawLine(the_image, y3, x3, y0, x0, values);
  return 1;
}


template<class type>
ushort vDrawCircle1(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
			              const type value)
{
  vector<type> values;
  vint8 i;
  vint8 bands = source->Bands();
  for (i = 0; i < bands; i++)
    values.push_back(value);
  vDrawCircle(source, y1, x1, radius, values);
  return 1;
}


template<class type>
ushort vDrawCircle(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
	                 const vector<type> & values)
{
  vint8 rows, cols, bands;
  rows = source->Rows();
  cols = source->Cols();
  bands = source->Bands();
  assert(bands == values.size());
  vArray3(type) matrix = source->Matrix3();

  vint8 top, bottom, left, right;

  vint8 row, col, band;
  double row_dist, col_dist;

  top = y1 - radius;
  bottom = y1 + radius;
  if (top < 0) top = 0;
  if (bottom >= rows) bottom = rows - 1;

  for (row = top; row <= bottom; row++)
  {
	  row_dist = (double) (row - y1);
	  col_dist = sqrt((double) (radius * radius - row_dist * row_dist));
	  left = (vint8) (x1 - col_dist);
	  right = (vint8) (x1 + col_dist);
	  if (left < 0) left = 0;
	  if (right >= cols) right = cols - 1;
	  for (band = 0; band < bands; band++)
	  {
       matrix[band][row][left] = values[(vector_size) band];
       matrix[band][row][right] = values[(vector_size) band];
	  }
  }

  left = x1 - radius;
  right = x1 + radius;
  if (left < 0) left = 0;
  if (right >= cols) right = cols - 1;

  for (col = left; col <= right; col++)
  {
	  col_dist = (double) (col - x1);
	  row_dist = sqrt((double) (radius * radius - col_dist * col_dist));
	  top = (vint8) (y1 - row_dist);
	  bottom = (vint8) (y1 + row_dist);
	  if (top < 0) top = 0;
	  if (bottom >= rows) bottom = rows- 1;
	  for (band = 0; band < bands; band++)
	  {
       matrix[band][top][col] = values[(vector_size) band];
       matrix[band][bottom][col] = values[(vector_size) band];
	  }
  }
  return 1;
}


template<class type>
ushort vDrawFilledCircle1(v3dMatrix<type> * source, 
			                    const vint8 y1, const vint8 x1, const vint8 radius,
			                    const type value)
{
  vector<type> values;
  vint8 i;
  vint8 bands = source->Bands();
  for (i = 0; i < bands; i++)
    values.push_back(value);
  vDrawFilledCircle(source, y1, x1, radius, values);
  return 1;
}


template<class type>
ushort vDrawFilledCircle(v3dMatrix<type> * source, 
			                   const vint8 y1, const vint8 x1, const vint8 radius,
			                   const vector<type> & values)
{
    vint8 rows, cols, bands;
    rows = (vint8) source->Rows();
    cols = (vint8) source->Cols();
    bands = (vint8) source->Bands();
    assert(bands == values.size());

    vint8 top, bottom, left, right;

    top = y1 - radius;
    bottom = y1 + radius;
    if (top < 0) top = 0;
    if (bottom >= rows) bottom = rows - 1;
    vint8 row, col, band;
    double row_dist, col_dist;

    for (row = top; row <= bottom; row++)
    {
	    row_dist = (double) (row - y1);
	    col_dist = sqrt((double) (radius * radius - row_dist * row_dist));
	    left = (vint8) (x1 - col_dist);
	    right = (vint8) (x1 + col_dist);
	    if (left < 0) left = 0;
	    if (right >= cols) right = cols - 1;
	    for (band = 0; band < bands; band++)
	    {
	      vArray2(type) matrix = source->Matrix2(band);
	      type value = values[(vector_size) band];
	      for (col = left; col <= right; col++)
	        matrix[row][col] = value;
	    }
    }
    return 1;
}

template<class type>
v3dMatrix<type> * vCopyPolygon(const v3dMatrix<type> * the_image, const vPolygon * polygon)
{
  if (polygon == 0) return 0;
  const vint8 top = vTopEnd(polygon) - 1; 
  const vint8 left = vLeftEnd(polygon) - 1;
  const vint8 right = vRightEnd(polygon) + 1;
  const vint8 bottom = vBottomEnd(polygon) + 1;

  v3dMatrix<type> * result = vCopyRectangle(the_image, left, right, top, bottom);
  vTranslatePolygon(polygon, 0 - left, 0 - top);
  vCropImage(result, polygon);
  return result;
}

// Copies the source region with the specified top left corner and the specified
// rows and cols to the target region whose top left corner is 
// (target_top, target_left)

template <class type>
ushort vCopyRectangle2(const v3dMatrix<type> * source, v3dMatrix<type> * target,
                       const vint8 top, const vint8 left, const vint8 rows, const vint8 cols,
                       const vint8 target_top, const vint8 target_left)
{ 
  // Get the size of the source and target
  vint8 source_rows = source->Rows();
  vint8 source_cols = source->Cols();
  vint8 target_rows = target->Rows();
  vint8 target_cols = target->Cols();
  vint8 min_rows = Min(source_rows, target_rows);
  vint8 min_cols = Min(source_cols, target_cols);

  // Get the boundaries of the source and target regions
  vint8 bottom = top + rows - 1;
  vint8 right = left + cols - 1;
  vint8 target_bottom = target_top + rows - 1;
  vint8 target_right = target_left + cols - 1;

  // Return false if one of the specified regions exceeds the boundaries
  // of its corresponding image
  if (top < 0) return 0;
  if (bottom >= source_rows) return 0;
  if (target_bottom >= target_rows) return 0;
  if (left < 0) return 0;
  if (right >= source_cols) return 0;;
  if (target_right >= target_cols) return 0;

  vint8 bands = source->Bands();
  if (bands != target->Bands()) return 0;
 
  vint8 i, j, row, col, band;
  
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix1 = source->Matrix2(band);
    vArray2(type) matrix2 = target->Matrix2(band);
    for (row = top; row <= bottom; row++)
    {
      i = row - top + target_top;
      for (col = left; col <= right; col++)
      {
	      j = col - left + target_left;
	      matrix2[i][j] = matrix1[row][col];
      }
    }
  }

  return 1;
}

// Returns an image that is a copy of the specified region in the source image
// If the specified region exceeds the boundaries of the source image, 
// the function returns 0
template<class type>
v3dMatrix<type> * vCopyRectangle(const v3dMatrix<type> * source, 
				                         vint8 left, vint8 right, vint8 top, vint8 bottom)
{ 
  vint8 result_rows = bottom - top + 1;
  vint8 result_cols = right - left + 1;
  if (left < 0) return 0;
  if (right >= source->Cols()) right = source->Cols() - 1;
  if (top < 0) top = 0;
  if (bottom >= source->Rows()) bottom = source->Rows() - 1;
  
  vint8 rows = bottom - top + 1;
  vint8 cols = right - left + 1;
  vint8 bands = source->Bands();
  v3dMatrix<type> * target = new v3dMatrix<type>(result_rows, result_cols, bands);

//  ushort vCopyRectangle2(v3dMatrix<type> * source, v3dMatrix<type> * target,
//                       vint8 top, vint8 left, vint8 rows, vint8 cols,
//                       vint8 target_top, vint8 target_left)

  vCopyRectangle2(source, target, top, left, rows, cols, 0, 0);
  vint8 row, col, band;

  // Write zeros to uninitialized pixels of target.
  for (band = 0; band < bands; band++)
  {
    vArray2(type) array = target->Matrix2(band);
    for (row = rows; row < result_rows; row++)
    {
      for (col = 0; col < result_cols; col++)
      {
        array[row][col] = 0;
      }
    }
    for (col = cols; col < result_cols; col++)
    {
      for (row = 0; row < result_rows; row++)
      {
        array[row][col] = 0;
      }
    }
  }

  return target;
}


template<class type>
ushort vCropImage(v3dMatrix<type> * the_image, const vPolygon * polygon)
{
  static const unsigned char OUTSIDE = 2;
  vArray2(uchar) scratch = S_PolygonMask(the_image, polygon);

  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  vint8 size = rows * cols;
  vint8 band;

  vint8 i, j;
  for (band = 0; band < bands; band++)
  {
    type ** matrix = the_image->Matrix2(band);
    for (i = 0; i < rows; i++)
      for (j = 0; j < cols; j++)
      {
	      if (scratch[i][j] == OUTSIDE)
	        matrix[i][j] = 0;
      }
  }
   
  for (i = 0; i < rows; i++)
  {
    vdelete2(scratch[i]);
  }

  vdelete2(scratch);
  return 1;
}


// Draws a line between (y0, x0) and (y1, x1). (Coordinates are in
// (row, col) format). If an endpoint is outside the boundary of the
// image, the line is only drawn up to the boundary.
template<class type>
ushort vDrawLine(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
		             const vector<type> * values)
{
  vint8 rows = (vint8) the_image->Rows();
  vint8 cols = (vint8) the_image->Cols();
  vint8 bands = (vint8) the_image->Bands();
  assert(bands == values->size());

  vint8 band;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix = the_image->Matrix2(band);
    vDrawArrayLine(matrix, rows, cols, y0, x0, y1, x1, (*values)[(vector_size) band]);
  }
  return 1;
}
template<class type>
ushort vDrawLine1(v3dMatrix<type> * the_image, 
            		  const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
		              const type value)
{
  vector<type> values;
  vint8 i;
  vint8 bands = the_image->Bands();
  for (i = 0; i < bands; i++)
    values.push_back(value);
  return vDrawLine(the_image, y0, x0, y1, x1, &values);
}


// Draws a line starting at (y,x) (y is row, x is col), whose
// slope is specified by angle and has the given length. Notice
// that the image coordinates are not the standard coordinate 
// system (y increases as we go down in image coordinates). Therefore
// a line with an orientation of PI/2 points downwards.
template<class type>
ushort vDrawLineA(v3dMatrix<type> * the_image, 
		              const vint8 y, const vint8 x, const double angle, const vint8 length,
		              const vector<type> * values)
{
  vint8 x1 = (vint8) ((double) x + (cos(angle) * (double) length));
  vint8 y1 = (vint8) ((double) y + (sin(angle) * (double) length));
  return vDrawLine(the_image, y, x, y1, x1, values);
}


template<class type>
ushort vDrawLineB(v3dMatrix<type> * the_image, 
		              const vint8 y, const vint8 x, const double angle, const vint8 length, 
                  const type value)
{
  vint8 bands = the_image->Bands();
  vector<type> values(bands);
  vint8 i;
  for (i = 0; i < bands; i++)
  {
    values[i] = value;
  }
  return vDrawLineA(the_image, y, x, angle, length, &values);
}


template<class type>
ushort vDrawCenteredLine(v3dMatrix<type> * the_image, const vint8 center_row, const vint8 center_col,
                         const double angle, const vint8 length, const type value)
{
  static const double junk1 = vPrecomputedSin::PrecomputeValues();
  static const double junk2 = vPrecomputedCos::PrecomputeValues();

  if (the_image == 0) return 0;
  vint8 half_length = length/2;
  double row_diff = half_length * sin(angle);
  double col_diff = half_length * cos(angle);
  vint8 start_row = round_number(center_row - row_diff);
  vint8 start_col = round_number(center_col - col_diff);
  vint8 end_row = round_number(center_row + row_diff);
  vint8 end_col = round_number(center_col + col_diff);
  ushort result = vDrawLine1(the_image, start_row, start_col, end_row, end_col, value);
  return result;
}


template<class type>
ushort vDrawPolygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                const vector<type> & values)
{
  vint8 * xs = polygon->x_coordinates;
  vint8 * ys = polygon->y_coordinates;
  vint8 num_vertices = polygon->num_vertices;

  vint8 i;
  vint8 x1, y1, x2, y2;
  for (i = 0; i < num_vertices; i++)
  {
	  x1 = xs[i];
	  y1 = ys[i];
	  x2 = xs[(i+1) % num_vertices];
	  y2 = ys[(i+1) % num_vertices];
	  vDrawLine(the_image, y1, x1, y2, x2, values);
	  vDrawLine(the_image, y1, (x1-3), y1, (x1+3), values);
	  vDrawLine(the_image, y1 - 3, x1, y1 + 3, x1, values);
  }
  return 1;
}    


// Draws an open polygon onto an image. "Open polygon" is my own terminology
// for a polygon in which the line segment connecting the last vertex back
// to the first vertex is not drawn.
// The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort vDrawOpenPolygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                    const vector<type> * values)
{
  const vector<vint8> & xs = polygon->x_coordinates;
  const vector<vint8> & ys = polygon->y_coordinates;
  const vint8 num_vertices = polygon->num_vertices;

  vint8 i;
  vint8 x1, y1, x2, y2;
  for (i = 0; i < num_vertices - 1; i++)
  {
	  x1 = xs[(vector_size) i];
	  y1 = ys[(vector_size) i];
	  x2 = xs[(vector_size) (i+1)];
	  y2 = ys[(vector_size) (i+1)];
	  vDrawLine(the_image, y1, x1, y2, x2, values);
	  //V_DrawLine(the_image, y1, (x1-3), y1, (x1+3), values);
	  //V_DrawLine(the_image, y1 - 3, x1, y1 + 3, x1, values);
  }
  return 1;
}



template<class type>
v3dMatrix<type> * vMaskFromPolygon(const v3dMatrix<type> * source, 
				   const vPolygon * polygon)
{
  static const unsigned char OUTSIDE = 2;
  vArray2(uchar) scratch = S_PolygonMask(source, polygon);

  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  vint8 bands = source->Bands();
  vint8 band, i, j;

  v3dMatrix<type> * the_image = new v3dMatrix<type>(rows, cols, bands);

  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix = the_image->Matrix2(band);
    for (i = 0; i < rows; i++)
      for (j = 0; j < cols; j++)
      {
	      if (scratch[i][j] == OUTSIDE) matrix[i][j] = 0;
	      else matrix[i][j] = 255;
      }
  }

  for (i = 0; i < rows; i++)
  {
    vdelete2(scratch[i]);
  }

  vdelete2(scratch);
  return the_image;
}    


template<class type>
static vArray2(uchar) S_PolygonMask(const v3dMatrix<type> * source, 
                                    const vPolygon * polygon)
{
  static const unsigned char INSIDE  = 0;
  static const unsigned char TO_VISIT = 1;
  static const unsigned char OUTSIDE = 2;
  
  vint8 vertices = polygon->num_vertices;
  vint8 * xs = polygon->x_coordinates;
  vint8 * ys = polygon->y_coordinates;

  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  vint8 bands = source->Bands();
  vint8 size = rows * cols;

  vArray2(uchar) scratch = vnew(vArray(uchar), rows);
  vArray(vint8) to_visit_i = vnew(vint8, size);
  vArray(vint8) to_visit_j = vnew(vint8, size);

  vint8 i, j;
  for (i = 0; i < rows; i++)
  {
    scratch[i] = vnew(uchar, cols);
    bzero(vData(scratch[i]), cols);
  }

  for (i = 0; i < vertices - 1; i++)
  {
    vDrawArrayLine(scratch, rows, cols, 
            		    ys[i], xs[i], ys[i+1], xs[i+1], (uchar) 255);
  }
  
  vDrawArrayLine(scratch, rows, cols,
		              ys[vertices - 1], xs[vertices - 1], ys[0], xs[0], 
                  (uchar) 255);
 
  // We assume that the top left pixel is outside the polygon.
  to_visit_i[0] = 0;
  to_visit_j[0] = 0;
  scratch[0][0] = TO_VISIT;
  vint8 back_index = 0;
  vint8 front_index = 1;

  vint8 row, col;

  while (back_index < front_index)
  {
    row = to_visit_i[back_index];
    col = to_visit_j[back_index];

    scratch[row][col] = OUTSIDE;

    i = row+1;
    j = col;
    if ((i < rows) && (scratch[i][j] == INSIDE))
    {
      to_visit_i[front_index] = i;
      to_visit_j[front_index] = j;
      scratch[i][j] = TO_VISIT;
      front_index++;
    }

    i = row-1;
    j = col;
    if ((i >= 0) && (scratch[i][j] == INSIDE))
    {
      to_visit_i[front_index] = i;
      to_visit_j[front_index] = j;
      scratch[i][j] = TO_VISIT;
      front_index++;
    }

    i = row;
    j = col+1;
    if ((j < cols) && (scratch[i][j] == INSIDE))
    {
      to_visit_i[front_index] = i;
      to_visit_j[front_index] = j;
      scratch[i][j] = TO_VISIT;
      front_index++;
    }

    i = row;
    j = col-1;
    if ((j >= 0) && (scratch[i][j] == INSIDE))
    {
      to_visit_i[front_index] = i;
      to_visit_j[front_index] = j;
      scratch[i][j] = TO_VISIT;
      front_index++;
    }

    back_index++;
  }

  vdelete2(to_visit_i);
  vdelete2(to_visit_j);

  return scratch;
}

template<class type>
ushort vDrawArrayLine(vArray2(type) array, const vint8 rows, const vint8 cols, 
		                  const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
		                  const type value)
{
  double dx = x1 - (double) x0;
  double dy = y1 - (double) y0;

  double abs_dx = fabs(dx);
  double abs_dy = fabs(dy); 
  double times, step_x, step_y;

  if (abs_dx >= abs_dy) times = abs_dx + 1;
  else times = abs_dy + 1;

  if (times == 1)
  {
    array[y0][x0] = value;
    return 1;
  }

  step_x = dx / (times-1);
  step_y = dy / (times-1);
  
  vint8 i;
  double col = x0 - step_x;
  double row = y0 - step_y;
  vint8 int_row, int_col;
  for (i = 0; i < times; i++)
  {
    row = row + step_y;
    col = col + step_x;
    int_row = (vint8) round_number(row);
    int_col = (vint8) round_number(col);
    if (int_row >= rows) continue;
    if (int_row < 0) continue;
    if (int_col >= cols) continue;
    if (int_col < 0) continue;
    array[int_row][int_col] = value;
  }
  return 1;
}


template<class type>
ushort vDrawCross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                  const vint8 width, const vint8 length, const type value)
{
  vint8 bands = the_image->Bands();
  vector<type> values;
  vint8 i;
  for (i = 0; i < bands; i++)
  {
    values.push_back(value);
  }
  //return 1;
  vint8 left, right, top, bottom;
  left = col - width;
  right = col + width;
  top = row - length;
  bottom = row + length;
  vint8 col_index, row_index;
  for (col_index = left; col_index <= right; col_index++)
  {
    vDrawLine(the_image, top, col_index, bottom, col_index, &values);
  }
  left = col - length;
  right = col + length;
  top = row - width;
  bottom = row + width;
  for (row_index = top; row_index <= bottom; row_index++)
  {
    vDrawLine(the_image, row_index, left, row_index, right, &values);
  }
  return 1;
}


template<class type>
ushort vDrawCrossv(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                   const vint8 width, const vint8 length, const vector<type> * values)
{
  vint8 left, right, top, bottom;
  left = col - width;
  right = col + width;
  top = row - length;
  bottom = row + length;
  vint8 col_index;
  for (col_index = left; col_index <= right; col_index++)
  {
    vDrawLine(the_image, top, col_index, bottom, col_index, values);
  }
  left = col - length;
  right = col + length;
  top = row - width;
  bottom = row + width;
  for (vint8 row_index = top; row_index <= bottom; row_index++)
  {
    vDrawLine(the_image, row_index, left, row_index, right, values);
  }
  return 1;
}


template<class type>
v3dMatrix<type> * vRotateRight(const v3dMatrix<type> * fimage)
{
  vint8 rows = fimage->Rows();
  vint8 cols = fimage->Cols();
  vint8 bands = fimage->Bands();
  
  // the result image must have as many rows as fimage->cols, since it
  // will be fimage rotated.

  v3dMatrix<type> * result = new v3dMatrix<type>(cols, rows, bands);
  vint8 i;

  for (i = 0; i < bands; i++)
  {
    vArray2(type) I1 = fimage->Matrix2(i);
    vArray2(type) I2 = result->Matrix2(i);
  
    vint8 row, col;
    for (row = 0; row < rows; row++)
      for (col = 0; col < cols; col++)
      {
	      I2[col][rows - 1 - row] = I1[row][col];
      }
  }

  return result;
}


template<class type>
v3dMatrix<type> * vRotateRight(const v3dMatrix<type> * fimage, const vint8 degrees)
{
  vint8 remainder = degrees % 90;
  if (remainder != 0) return 0;
  vint8 real_degrees = degrees % 360;
  while(real_degrees < 0) real_degrees += 360;
  if (real_degrees == 0) return vCopyImage(fimage);
  else if (real_degrees == 90) return vRotateRight(fimage);
  else if (real_degrees == 180) return vUpsideDown(fimage);
  else if (real_degrees == 270) return vRotateLeft(fimage);
  else 
  {
    assert(0);
    return 0;
  }
}


template<class type>
v3dMatrix<type> * vRotateLeft(const v3dMatrix<type> * fimage)
{
  vint8 rows = fimage->Rows();
  vint8 cols = fimage->Cols();
  vint8 bands = fimage->Bands();
  
  // the result image must have as many rows as fimage->cols, since it
  // will be fimage rotated.

  v3dMatrix<type> * result = new v3dMatrix<type>(cols, rows, bands);
  vint8 i;

  for (i = 0; i < bands; i++)
  {
    vArray2(type) I1 = fimage->Matrix2(i);
    vArray2(type) I2 = result->Matrix2(i);
  
    vint8 row, col;
    for (row = 0; row < rows; row++)
      for (col = 0; col < cols; col++)
      {
	      I2[cols - 1 - col][row] = I1[row][col];
      }
  }

  return result;
}


template<class type>
v3dMatrix<type> * vUpsideDown(const v3dMatrix<type> * fimage)
{
  vint8 rows = fimage->Rows();
  vint8 cols = fimage->Cols();
  vint8 bands = fimage->Bands();
  
  // the result image must have as many rows as fimage->cols, since it
  // will be fimage rotated.

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vint8 i;

  for (i = 0; i < bands; i++)
  {
    vArray2(type) I1 = fimage->Matrix2(i);
    vArray2(type) I2 = result->Matrix2(i);
  
    vint8 row, col;
    for (row = 0; row < rows; row++)
      for (col = 0; col < cols; col++)
      {
	      I2[rows - 1 - row][cols - 1 - col] = I1[row][col];
      }
  }

  return result;
}


// This function is useful for raster images that are stored on 
// file with their bottom row first. This way I can read them
// using v3dMatrix::ReadInterlaced and then call this function
// to flip them 
template<class type>
v3dMatrix<type> * vSwitchTopBottom(const v3dMatrix<type> * the_image)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vint8 row_byte_size = cols * sizeof(type);
  vArray3(type) in_matrix3 = the_image->Matrix3();
  vArray3(type) out_matrix3 = result->Matrix3();
  vint8 band, row;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) in_matrix2 = in_matrix3[band];
    vArray2(type) out_matrix2 = out_matrix3[band];
    for (row = 0; row < rows; row++)
    {
      vint8 in_row = rows - row - 1;
      memcpy(vData(out_matrix2[row]), vData(in_matrix2[in_row]), 
             (vector_size) row_byte_size);
    }
  }
  return result;
}


template<class type>
v3dMatrix<type> * vReverseRowsCols(const v3dMatrix<type> * fimage)
{
  vint8 rows = fimage->Rows();
  vint8 cols = fimage->Cols();
  vint8 bands = fimage->Bands();

  // the result image must have as many rows as fimage->cols, since it
  // will be fimage rotated.

  v3dMatrix<type> * result = new v3dMatrix<type>(cols, rows, bands);
  vint8 i;

  for (i = 0; i < bands; i++)
  {
    vArray2(type) I1 = fimage->Matrix2(i);
    vArray2(type) I2 = result->Matrix2(i);
    
    vint8 row, col;
    for (row = 0; row < rows; row++)
      for (col = 0; col < cols; col++)
      {
	      I2[col][row] = I1[row][col];
      }
  }

  return result;
}


template<class type>
void function_enter_value(v3dMatrix<type> * the_image, const type value)
{
  vArray(type) matrix = the_image->Matrix();
  vint8 size = the_image->AllBandSize();
  for (vint8 i = 0; i < size; i++)
  {
    matrix[i] = value;
  }
}


template<class type>
void vWriteValue2(v3dMatrix<type> * the_image, 
                  const vector<vint8> * point_rows, const vector<vint8> * point_cols, 
                  const type value)
{
  vArray3(type) matrix = the_image->Matrix3();
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  vint8 band, i;
  vint8 size = point_rows->size();
  for (i = 0; i < size; i++)
  {
    vint8 row = (*point_rows)[(vector_size)i];
    vint8 col = (*point_cols)[(vector_size)i];
    if (the_image->check_bounds(row, col) <= 0) continue;
    for (band = 0; band < bands; band++)
    {
      matrix[(vector_size)band][(vector_size)row][(vector_size)col] = value;
    }
  }
}


template<class type>
void vWriteValue3(v3dMatrix<type> * the_image, 
                  const vArray(uchar) point_rows, 
                  const vArray(uchar) point_cols, 
                  const vint8 number, const type value)
{
  vArray3(type) matrix = the_image->Matrix3();
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  vint8 band, i;
  for (i = 0; i < number; i++)
  {
    vint8 row = point_rows[i];
    vint8 col = point_cols[i];
    if (the_image->check_bounds(row, col) <= 0) continue;
    for (band = 0; band < bands; band++)
    {
      matrix[band][row][col] = value;
    }
  }
}


template<class type>
void vWriteValue3(v3dMatrix<type> * the_image, 
                  const vArray(vint8) point_rows, 
                  const vArray(vint8) point_cols, 
                  const vint8 number, const type value)
{
  vArray3(type) matrix = the_image->Matrix3();
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  vint8 band, i;
  for (i = 0; i < number; i++)
  {
    vint8 row = point_rows[i];
    vint8 col = point_cols[i];
    if (the_image->check_bounds(row, col) <= 0) continue;
    for (band = 0; band < bands; band++)
    {
      matrix[band][row][col] = value;
    }
  }
}


template<class type>
void vWriteValue4(v3dMatrix<type> * the_image, 
                  const vector<vPoint> * pixels, 
                  const type value)
{
  vArray3(type) matrix = the_image->Matrix3();
  const vint8 rows = the_image->Rows();
  const vint8 cols = the_image->Cols();
  const vint8 bands = the_image->Bands();

  vint8 band, i;
  const vint8 size = pixels->size();
  for (i = 0; i < size; i++)
  {
    const vPoint & point = (*pixels)[i];
    const vint8 row = point.row;
    const vint8 col = point.col;
    if (the_image->check_bounds(row, col) <= 0) continue;
    for (band = 0; band < bands; band++)
    {
      matrix[band][row][col] = value;
    }
  }
}


// Writes the specified value to the border pixels of the image
// (pixels in the top and bottom rows and left and right columns.
template<class type>
ushort vWriteBorderValue(v3dMatrix<type> * the_image, const type value)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bottom = rows - 1;
  vint8 right = cols - 1;
  vint8 bands = the_image->Bands();
  vArray3(type) matrix3 = the_image->Matrix3();
  vint8 band, row, col;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix2 = matrix3[band];

    for (col = 0; col < cols; col++)
    {
      matrix2[0][col] = value;
      matrix2[bottom][col] = value;
    }

    for (row = 0; row < rows; row++)
    {
      matrix2[row][0] = value;
      matrix2[row][right] = value;
    }
  }
  return 1;
}


// Writes the specified value to the border pixels of the image
// (pixels whose distance from the border is <= width).
template<class type>
ushort vWriteBorderValue3(v3dMatrix<type> * the_image, const type value, vint8 width)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  if (width > rows) width = rows;
  if (width > cols) width = cols;
  vint8 bottom = rows - 1;
  vint8 right = cols - 1;
  vint8 bands = the_image->Bands();
  vArray3(type) matrix3 = the_image->Matrix3();
  vint8 band, row, col, i;
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix2 = matrix3[band];

    for (col = 0; col < cols; col++)
    {
      for (i = 0; i < width; i++)
      {
        matrix2[i][col] = value;
        matrix2[bottom-i][col] = value;
      }
    }

    for (row = 0; row < rows; row++)
    {
      for (i = 0; i < width; i++)
      {
        matrix2[row][i] = value;
        matrix2[row][right-i] = value;
      }
    }
  }
  return 1;
}


// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row][col] = (row2, col2), with one exception,
// of course, i.e. when two source points map to the same 
// result point, in which case only one of the source points will
// give its value to the result point.
template<class type>
v3dMatrix<type> * vWarpImage(const v3dMatrix<type> * the_image, 
                             const v3dMatrix<short> * warp_template)
{
  // Error checking:
  if (the_image == 0) return 0;
  if (warp_template == 0) return 0;
  if (warp_template->Rows() < the_image->Rows()) return 0;
  if (warp_template->Cols() < the_image->Cols()) return 0;
  if (warp_template->Bands() != 2) return 0;

  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vArray3(type) color_image3 = the_image->Matrix3();
  vArray3(type) result3 = result->Matrix3();
  vArray2(short) source_rows = warp_template->Matrix2(0);
  vArray2(short) source_cols = warp_template->Matrix2(1);

  vint8 row, col, band;
  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 source_row = source_rows[row][col];
      vint8 source_col = source_cols[row][col];
      if (the_image->check_bounds(source_row, source_col) <= 0)
      {
        for (band = 0; band < bands; band++)
        {
          result3[band][row][col] = (type) 0;
        }
      }
      else
      {
        for (band = 0; band < bands; band++)
        {
          result3[band][row][col] = color_image3[band][source_row][source_col];
        }
      }
    }
  }

  return result;
}


// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row2][col2] = (row, col).
template<class type>
v3dMatrix<type> * inverse_warp(const v3dMatrix<type> * image, 
                               const v3dMatrix<short> * warp_template)
{
  // Error checking:
  if (image == 0) return 0;
  if (warp_template == 0) return 0;
  if (warp_template->Rows() < image->Rows()) return 0;
  if (warp_template->Cols() < image->Cols()) return 0;
  if (warp_template->Bands() != 2) return 0;

  vint8 rows = image->Rows();
  vint8 cols = image->Cols();
  vint8 bands = image->Bands();

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  function_enter_value(result, (type) 0);
  vArray3(type) image3 = image->Matrix3();
  vArray3(type) result3 = result->Matrix3();
  vArray2(short) target_rows = warp_template->Matrix2(0);
  vArray2(short) target_cols = warp_template->Matrix2(1);

  vint8 row, col, band;
  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 target_row = target_rows[row][col];
      vint8 target_col = target_cols[row][col];
      if (result->check_bounds(target_row, target_col) > 0)
      {
        for (band = 0; band < bands; band++)
        {
          result3[band][target_row][target_col] = image3[band][row][col];
        }
      }
    }
  }

  return result;
}



// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row][col] = (row2, col2), as in vWarpImage,
// with one difference: if a zero source pixel and a non-zero source
// pixel map to the same result pixel, the non-zero pixel takes precedence.
template<class type>
v3dMatrix<type> * vWarpImageSparse(const v3dMatrix<type> * the_image, 
                                   const v3dMatrix<short> * warp_template)
{
  // Error checking:
  if (the_image == 0) return 0;
  if (warp_template == 0) return 0;
  if (warp_template->Rows() < the_image->Rows()) return 0;
  if (warp_template->Cols() < the_image->Cols()) return 0;
  if (warp_template->Bands() != 2) return 0;

  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vArray3(type) color_image3 = the_image->Matrix3();
  vArray3(type) result3 = result->Matrix3();
  vArray2(short) source_rows = warp_template->Matrix2(0);
  vArray2(short) source_cols = warp_template->Matrix2(1);
  function_enter_value(result, (type) 0);

  vint8 row, col, band;
  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 source_row = source_rows[row][col];
      vint8 source_col = source_cols[row][col];
      if (the_image->check_bounds(source_row, source_col) <= 0) continue;
      vint8 zero_flag = 1;
      for (band = 0; band < bands; band++)
      {
        if (color_image3[band][source_row][source_col] != 0) zero_flag = 0;
      }
      if (zero_flag == 1) continue;
      for (band = 0; band < bands; band++)
      {
        result3[band][row][col] = color_image3[band][source_row][source_col];
      }
    }
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////

template<class type>
v3dMatrix<type> * function_copy_image(const v3dMatrix<type> * source)
{
  return vCopyImage(source);
}

template<class type> 
ushort function_copy_image(v3dMatrix<type> * result, const v3dMatrix<type> * source)
{
  return vCopyImage(result, source);
}

template<class type>
v3dMatrix<type> * function_copy_band(const v3dMatrix<type> * source, const vint8 band)
{
  return vCopyBand2(source, band);
}


template<class type>
vint8 function_copy_band(v3dMatrix<type> * target, const v3dMatrix<type> * source, const vint8 band)
{
  return vCopyBand3(target, source, band);
}

// Returns a result that is a copy of the source, except that it
// also has a rectangle drawn on it, with top y1, left x1, bottom y2,
// right x2. The rectangle sides are colored as specified by values.
template<class type>
v3dMatrix<type> * copy_draw_rectangle(const v3dMatrix<type> * source, 
					                            const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
					                            const vector<type> & values)
{
  return vCopyAndDrawRectangle(source, y1, x1, y2, x2, values);
}

// Similar as vCopyAndDrawRectangle, but here we just draw the rectangle 
// directly onto the source.
template<class type>
ushort function_draw_rectangle(v3dMatrix<type> * source, 
 	                             const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
	                             const vector<type> & values)
{
  return vDrawRectangle(source, y1, x1, y2, x2, values);
}

// Similar to vDrawRectangle, but here the color of the rectangle sides is
// specified by value (value is used in all bands of the image).
template<class type>
ushort function_draw_rectangle(v3dMatrix<type> * source, 
  		                         const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
  		                         const type value)
{
  return vDrawRectangle1(source, y1, x1, y2, x2, value);
}

template<class type>
ushort function_draw_square(v3dMatrix<type> * source, 
  		                      const vint8 y, vint8 x, double angle, vint8 size,
		                        const vector<type> & values)
{
  return vDrawSquare(source, y, x, angle, size, values);
}

template<class type>
ushort draw_full_rectangle(v3dMatrix<type> * source, 
			                     const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
			                     const vector<type> & values)
{
  return vDrawFilledRectangle(source, y1, x1, y2, x2, values);
}

template<class type>
ushort draw_full_rectangle(v3dMatrix<type> * source, 
		                       const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
		                       const type value)
{
  return vDrawFilledRectangle1(source, y1, x1, y2, x2, value);
}


template<class type>
ushort function_draw_circle(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
			                      const vector<type> & values)
{
  return vDrawCircle(source, y1, x1, radius, values);
}

template<class type>
ushort function_draw_circle(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius, const type value)
{                     
  return vDrawCircle1(source, y1, x1, radius, value);
}



template<class type>
ushort draw_full_circle(v3dMatrix<type> * source, 
			                  const vint8 y1, const vint8 x1, const vint8 radius,
			                  const vector<type> & values)
{
  return vDrawFilledCircle(source, y1, x1, radius, values);
}

template<class type>
ushort draw_full_circle(v3dMatrix<type> * source, 
 			                  const vint8 y1, const vint8 x1, const vint8 radius,
			                  const type value)
{
  return vDrawFilledCircle1(source, y1, x1, radius, value);
}



template<class type>
v3dMatrix<type> * function_copy_polygon(const v3dMatrix<type> * the_image, 
				                                const vPolygon * polygon)
{
  return vCopyPolygon(the_image, polygon);
}

template<class type>
v3dMatrix<type> * function_subtract_polygon(const v3dMatrix<type> * the_image, 
                        				            const vPolygon * polygon)
{
  return vSubtractPolygon(the_image, polygon);
}

template<class type>
v3dMatrix<type> * function_copy_rectangle(const v3dMatrix<type> * source, 
				                                  const vint8 left, const vint8 right, 
				                                  const vint8 top, const vint8 bottom)
{
  return vCopyRectangle(source, left, right, top, bottom);
}

template <class type>
ushort function_copy_rectangle(const v3dMatrix<type> * source, v3dMatrix<type> * target,
                               const vint8 left, const vint8 right, const vint8 top, const vint8 bottom)
{
  return vCopyRectangle2(source, target, top, left, bottom-top+1, right-left+1, 0, 0);
}

template <class type>
ushort function_copy_rectangle(const v3dMatrix<type> * source, v3dMatrix<type> * target,
                               const vint8 left, const vint8 right, const vint8 top, const vint8 bottom, 
                               const vint8 target_top, const vint8 target_left)
{
  return vCopyRectangle2(source, target, top, left, bottom-top+1, right-left+1, target_top, target_left);
}

template<class type>
ushort function_crop_image(v3dMatrix<type> * the_image, const vPolygon * polygon)
{
  return vCropImage(the_image, polygon);
}

template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
             		          const vector<type> * values)
{
  return vDrawLine(the_image, y0, x0, y1, x1, values);
}


template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
             		          const type red, const type green, const type blue)
{
  if (the_image->channels() != 3)
  {
    exit_error("\nerror in function_draw_line\n");
  }

  vector<type> values;
  values.push_back(red);
  values.push_back(green);
  values.push_back(blue);

  return function_draw_line(the_image, y0, x0, y1, x1, &values);
}




template<class type>
ushort function_draw_cross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                           const vint8 width, const vint8 length, const type value)
{
  return vDrawCross(the_image, row, col, width, length, value);
}

template<class type>
ushort function_draw_cross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                           const vint8 width, const vint8 length, const vector<type> * values)
{
  return vDrawCrossv(the_image, row, col, width, length, values);
}



// Draws a line starting at (y,x) (y is row, x is col), whose
// slope is specified by angle and has the given length. Notice
// that the image coordinates are not the standard coordinate 
// system (y increases as we go down in image coordinates). Therefore
// a line with an orientation of PI/2 points downwards.
template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, 
		                      const vint8 y, const vint8 x, const double angle, const vint8 length,
		                      const vector<type> * values)
{
  return vDrawLineA(the_image, y, x, angle, length, values);
}

template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, 
		                      const vint8 y, const vint8 x, const double angle, const vint8 length,
		                      const type value)
{
  return vDrawLineB(the_image, y, x, angle, length, value);
}


template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
                          const type value)
{
  return vDrawLine1(the_image, y0, x0, y1, x1, value);
}

template<class type>
ushort draw_centered_line(v3dMatrix<type> * the_image, const vint8 center_row, const vint8 center_col,
                          const double angle, const vint8 length, type value)
{
  return vDrawCenteredLine(the_image, center_row, center_col, angle, length, value);
}


template<class type>
ushort draw_array_line(type ** array, const vint8 rows, const vint8 cols,
		                   const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
		                   const type value)
{
  return vDrawArrayLine(array, rows, cols, y0, x0, y1, x1, value);
}

// Draws a polygon onto an image. The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort function_draw_polygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                         const vector<type> * values)
{
  return vDrawPolygon(the_image, polygon, values);
}


// Draws an open polygon onto an image. "Open polygon" is my own terminology
// for a polygon in which the line segment connecting the last vertex back
// to the first vertex is not drawn.
// The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort draw_open_polygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                     const vector<type> * values)
{
  return vDrawOpenPolygon(the_image, polygon, values);
}


template<class type>
v3dMatrix<type> * mask_from_polygon(const v3dMatrix<type> * the_image, 
				                            const vPolygon * polygon)
{
  return vMaskFromPolygon(the_image, polygon);
}

template<class type>
v3dMatrix<type> * function_rotate_right(const v3dMatrix<type> * fimage)
{
  return vRotateRight(fimage);
}

// Works only if degrees is a multiple of 90.
template<class type>
v3dMatrix<type> * function_rotate_right(const v3dMatrix<type> * fimage, const vint8 degrees)
{
  return vRotateRight(fimage, degrees);
}

template<class type>
v3dMatrix<type> * function_rotate_left(const v3dMatrix<type> * fimage)
{
  return vRotateLeft(fimage);
}

template<class type>
v3dMatrix<type> * function_upside_down(const v3dMatrix<type> * fimage)
{
  return vUpsideDown(fimage);
}

// This function is useful for raster images that are stored on 
// file with their bottom row first. This way I can read them
// using v3dMatrix::ReadInterlaced and then call this function
// to flip them 
template<class type>
v3dMatrix<type> * switch_top_bottom(const v3dMatrix<type> * image)
{
  return vSwitchTopBottom(image);
}

template<class type>
v3dMatrix<type> * reverse_rows_columns(const v3dMatrix<type> * fimage)
{
  return vReverseRowsCols(fimage);
}


template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vector<vint8> * rows, const vector<vint8> * cols, 
                           const type value)
{
  return vWriteValue2(the_image, rows, cols, value);
}

// Same as vWriteValue2, but point coordinates here are passed
// in in arrays, not in vectors.
template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vArray(uchar) point_rows, 
                           const vArray(uchar) point_cols, 
                           const vint8 number, const type value)
{
  return vWriteValue3(the_image, point_rows, point_cols, number, value);
}

template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vArray(vint8) point_rows, 
                           const vArray(vint8) point_cols, 
                           const vint8 number, const type value)
{
  return vWriteValue3(the_image, point_rows, point_cols, number, value);
}


template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vector<vPoint> * points, 
                           const type value)
{
  return vWriteValue4(the_image, points, value);
}


// matrix has two columns, and each row represents a pixel:
// col 0 is pixel row, col1 is pixel col
template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vint8_matrix pixels, const type value)
{
  vint8 number = pixels.vertical();
  vint8 channels = the_image->channels();
  vint8 counter, channel;
  for (counter = 0; counter < number; counter++)
  {
    vint8 vertical = pixels(counter, 0);
    vint8 horizontal = pixels(counter, 1);
    if (the_image->check_bounds(vertical, horizontal) <= 0) 
    {
      continue;
    }

    for (channel = 0; channel < channels; channel++)
    {
      (*the_image)(channel, vertical, horizontal) = value;
    }
  }
}


// Writes the specified value to the border pixels of the image
// (pixels in the top and bottom rows and left and right columns).
template<class type>
ushort enter_border_value(v3dMatrix<type> * the_image, const type value)
{
  return vWriteBorderValue(the_image, value);
}

// Writes the specified value to the border pixels of the image
// (pixels whose distance from the border is <= width).
template<class type>
ushort enter_border_value(v3dMatrix<type> * the_image, const type value, vint8 width)
{
  return vWriteBorderValue3(the_image, value, width);
}


// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row][col] = (row2, col2).
template<class type>
v3dMatrix<type> * function_warp_template(const v3dMatrix<type> * the_image, 
                                         const v3dMatrix<short> * warp_template)
{
  return vWarpImage(the_image, warp_template);
}


// here we do bilinear interpolation
template <class type >
v3dMatrix<type> * rotate_image_exact_degrees(const v3dMatrix<type> * the_image, 
                                             const vint8 center_row, const vint8 center_col,
                                             const double degrees)
{
  const double degrees_to_radians = M_PI / 180.0;
  double radians = degrees * degrees_to_radians;
  return rotate_image_exact(the_image, center_row, center_col, radians);
}

template <class type >
v3dMatrix<type> * rotate_image_exact(const v3dMatrix<type> * the_image, 
                                     const vint8 center_row, const vint8 center_col,
                                     const double radians)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  v3dMatrix<float> * sources = vRotationTemplateRf(rows, cols, 
                                                   center_row, center_col, radians);
  if (sources == 0) return 0;
  vArray2(float) source_rows = sources->Matrix2(0);
  vArray2(float) source_cols = sources->Matrix2(1);

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  vArray3(type) result3 = result->Matrix3();
  vArray3(type) source3 = the_image->Matrix3();
  function_enter_value(result, (type) 0);

  vint8 row, col;

  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      double source_row = source_rows[row][col];
      double source_col = source_cols[row][col];
      if ((!result->check_bounds((vint8) source_row, (vint8) source_col)) ||
          (!result->check_bounds((vint8) (source_row+1), (vint8) (source_col+1))) ||
          (!result->check_bounds((vint8) (source_row-1), (vint8) (source_col-1))))
      {
        continue;
      }

      vint8 band;
      for (band = 0; band < bands; band++)
      {
      result3[band][row][col] = (type)
          vBilinearInterpolationD(source3[band], source_row, source_col);
      }
    }
  }

  vdelete(sources);
  return result;
}


// write a value to a specific band
template<class type>
void function_channel_value(v3dMatrix<type> * image, const vint8 channel, const type value)
{
  if ((image->Bands() >= channel) || (channel < 0))
  {
    return;
  }

  class_pointer(type) data = image->Matrix(channel);
  vint8 size = image->Size();
  vint8 counter = 0;
  for (counter = 0; counter < size; counter ++)
  {
    data [counter] = value;
  }
}





// If the cpp template file is included as part of the header file,
// we must undefine certain things.
#include "undefine.h"

#endif // VASSILIS_PROCESS_FILE
