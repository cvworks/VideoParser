#ifndef VASSILIS_DRAWING_TEMP_H
#define VASSILIS_DRAWING_TEMP_H

#include "drawing.h"
#include "image.h"

template<class type>
v3dMatrix<type> * vCopyImage(const v3dMatrix<type> * source);

template<class type> 
ushort vCopyImage(v3dMatrix<type> * result, const v3dMatrix<type> * source);

template<class type>
v3dMatrix<type> * vCopyBand2(const v3dMatrix<type> * source, const vint8 band);


template<class type>
vint8 vCopyBand3(v3dMatrix<type> * target, const v3dMatrix<type> * source, const vint8 band);

template<class type>
vint8 function_copy_band(v3dMatrix<type> * target, const vint8 target_band, 
                        const v3dMatrix<type> *source, const vint8 source_band);

// Returns a result that is a copy of the source, except that it
// also has a rectangle drawn on it, with top y1, left x1, bottom y2,
// right x2. The rectangle sides are colored as specified by values.
template<class type>
v3dMatrix<type> * vCopyAndDrawRectangle(const v3dMatrix<type> * source, 
					                              const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
					                              const vector<type> & values);

// Similar as vCopyAndDrawRectangle, but here we just draw the rectangle 
// directly onto the source.
template<class type>
ushort vDrawRectangle(const v3dMatrix<type> * source, 
                      const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
                      const vector<type> & values);

// Similar to vDrawRectangle, but here the color of the rectangle sides is
// specified by value (value is used in all bands of the image).
template<class type>
ushort vDrawRectangle1(v3dMatrix<type> * const source, 
                       const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
                       const type value);

template<class type>
ushort vDrawSquare(const v3dMatrix<type> * source, 
                   const vint8 y, const vint8 x, const double angle, const vint8 size,
                   const vector<type> & values);

template<class type>
ushort vDrawFilledRectangle(const v3dMatrix<type> * source, 
                            const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
                            const vector<type> & values);

template<class type>
ushort vDrawFilledRectangle1(const v3dMatrix<type> * source, 
                             const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
                             const type value);

template<class type>
ushort vDrawCircle(const v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
                   const vector<type> & values);

template<class type>
ushort vDrawCircle1(const v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
                    const type value);

template<class type>
ushort vDrawFilledCircle(const v3dMatrix<type> * source, 
                         const vint8 y1, const vint8 x1, const vint8 radius,
                         const vector<type> & values);

template<class type>
ushort vDrawFilledCircle1(const v3dMatrix<type> * source, 
                          const vint8 y1, const vint8 x1, const vint8 radius,
                          const type value);

template<class type>
v3dMatrix<type> * vCopyPolygon(const v3dMatrix<type> * the_image, 
				                       const vPolygon * polygon);

template<class type>
v3dMatrix<type> * vSubtractPolygon(const v3dMatrix<type> * the_image, 
                        			     const vPolygon * polygon);

template<class type>
v3dMatrix<type> * vCopyRectangle(const v3dMatrix<type> * source, 
				                         vint8 left, vint8 right, 
				                         vint8 top, vint8 bottom);


template<class type>
ushort vCropImage(v3dMatrix<type> * the_image, const vPolygon * polygon);

template<class type>
ushort vDrawLine(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
             		 const vector<type> * values);

template<class type>
ushort vDrawCross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                  const vint8 width, const vint8 length, const type value);

template<class type>
ushort vDrawCrossv(const v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                   const vint8 width, const vint8 length, const vector<type> * values);

// Draws a line starting at (y,x) (y is row, x is col), whose
// slope is specified by angle and has the given length. Notice
// that the image coordinates are not the standard coordinate 
// system (y increases as we go down in image coordinates). Therefore
// a line with an orientation of PI/2 points downwards.
template<class type>
ushort vDrawLineA(v3dMatrix<type> * the_image, 
                  const vint8 y, const vint8 x, const double angle, const vint8 length,
                  const vector<type> * values);

template<class type>
ushort vDrawLineB(v3dMatrix<type> * the_image, 
                  const vint8 y, const vint8 x, const double angle, const vint8 length,
                  const type value);

template<class type>
ushort vDrawLine1(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
                  const type value);

template<class type>
ushort vDrawCenteredLine(v3dMatrix<type> * the_image, const vint8 center_row, const vint8 center_col,
                         const double angle, const vint8 length, const type value);


template<class type>
ushort vDrawArrayLine(type ** array, const vint8 rows, const vint8 cols,
                      const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
                      const type value);

// Draws a polygon onto an image. The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort vDrawPolygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                const vector<type> * values);


// Draws an open polygon onto an image. "Open polygon" is my own terminology
// for a polygon in which the line segment connecting the last vertex back
// to the first vertex is not drawn.
// The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort vDrawOpenPolygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                    const vector<type> * values);


template<class type>
v3dMatrix<type> * vMaskFromPolygon(const v3dMatrix<type> * the_image, 
				                           const  vPolygon * polygon);

template<class type>
v3dMatrix<type> * vRotateRight(const v3dMatrix<type> * fimage);

// Works only if degrees is a multiple of 90.
template<class type>
v3dMatrix<type> * vRotateRight(const v3dMatrix<type> * fimage, const vint8 degrees);

template<class type>
v3dMatrix<type> * vRotateLeft(const v3dMatrix<type> * fimage);

template<class type>
v3dMatrix<type> * vUpsideDown(const v3dMatrix<type> * fimage);

// This function is useful for raster images that are stored on 
// file with their bottom row first. This way I can read them
// using v3dMatrix::ReadInterlaced and then call this function
// to flip them 
template<class type>
v3dMatrix<type> * vSwitchTopBottom(const v3dMatrix<type> * the_image);

template<class type>
v3dMatrix<type> * vReverseRowsCols(const v3dMatrix<type> * fimage);

template<class type>
v3dMatrix<type> * function_reverse_rows_columns(const v3dMatrix<type> * fimage)
{
  return vReverseRowsCols(fimage);
}


template<class type>
void vWriteValue2(v3dMatrix<type> * the_image, 
                  const vector<vint8> * rows, const vector<vint8> * cols, 
                  const type value);

// Same as vWriteValue2, but point coordinates here are passed
// in in arrays, not in vectors.
template<class type>
void vWriteValue3(v3dMatrix<type> * the_image, 
                  const vArray(uchar) point_rows, 
                  const vArray(uchar) point_cols, 
                  const vint8 number, const type value);

template<class type>
void vWriteValue3(v3dMatrix<type> * the_image, 
                  const vArray(vint8) point_rows, 
                  const vArray(vint8) point_cols, 
                  const vint8 number, type value);

template<class type>
void vWriteValue4(v3dMatrix<type> * the_image, 
                  const vector<vPoint> * points, 
                  const type value);

// Writes the specified value to the border pixels of the image
// (pixels in the top and bottom rows and left and right columns).
template<class type>
ushort vWriteBorderValue(v3dMatrix<type> * the_image, const type value);

// Writes the specified value to the border pixels of the image
// (pixels whose distance from the border is <= width).
template<class type>
ushort vWriteBorderValue3(v3dMatrix<type> * the_image, const type value, vint8 width);


// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row][col] = (row2, col2).
template<class type>
v3dMatrix<type> * vWarpImage(const v3dMatrix<type> * the_image, 
                             const v3dMatrix<short> * warp_template);

// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row2][col2] = (row, col).
template<class type>
v3dMatrix<type> * inverse_warp(const v3dMatrix<type> * image, 
                               const v3dMatrix<short> * warp_template);


template<class type>
v3dMatrix<type> * function_warp_template(const v3dMatrix<type> * the_image, 
                                         const v3dMatrix<short> * warp_template);


template <class type >
v3dMatrix<type> * rotate_image_exact(const v3dMatrix<type> * the_image, 
                                     const vint8 center_row, const vint8 center_col,
                                     const double radians);

// here we do bilinear interpolation
template <class type >
v3dMatrix<type> * rotate_image_exact_degrees(const v3dMatrix<type> * the_image, 
                                             const vint8 center_row, const vint8 center_col,
                                             const double degrees);

/////////////////////////////////////////////////////////////////////////

template<class type>
v3dMatrix<type> * function_copy_image(const v3dMatrix<type> * source);

template<class type> 
ushort function_copy_image(v3dMatrix<type> * result, const v3dMatrix<type> * source);

template<class type>
v3dMatrix<type> * function_copy_band(const v3dMatrix<type> * source, const vint8 band);

template<class type>
vint8 function_copy_band(v3dMatrix<type> * target, const v3dMatrix<type> * source, const vint8 band);

// Returns a result that is a copy of the source, except that it
// also has a rectangle drawn on it, with top y1, left x1, bottom y2,
// right x2. The rectangle sides are colored as specified by values.
template<class type>
v3dMatrix<type> * copy_draw_rectangle(const v3dMatrix<type> * source, 
					                            const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
					                            const vector<type> & values);

// Similar as vCopyAndDrawRectangle, but here we just draw the rectangle 
// directly onto the source.
template<class type>
ushort function_draw_rectangle(v3dMatrix<type> * source, 
		                           const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
		                           const vector<type> & values);

// Similar to vDrawRectangle, but here the color of the rectangle sides is
// specified by value (value is used in all bands of the image).
template<class type>
ushort function_draw_rectangle(v3dMatrix<type> * source, 
  		                vint8 y1, vint8 x1, vint8 y2, vint8 x2,
  		                type value);

template<class type>
ushort function_draw_square(v3dMatrix<type> * source, 
  		            const vint8 y, const vint8 x, const double angle, const vint8 size,
		              vector<type> & values);

template<class type>
ushort draw_full_rectangle(v3dMatrix<type> * source, 
			                     const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
			                     const vector<type> & values);

template<class type>
ushort draw_full_rectangle(v3dMatrix<type> * source, 
			                     const vint8 y1, const vint8 x1, const vint8 y2, const vint8 x2,
			                     const type value);


template<class type>
ushort function_draw_circle(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
			                      const vector<type> & values);

template<class type>
ushort function_draw_circle(v3dMatrix<type> * source, const vint8 y1, const vint8 x1, const vint8 radius,
			               const type value);


template<class type>
ushort draw_full_circle(v3dMatrix<type> * source, 
			                  const vint8 y1, const vint8 x1, const vint8 radius,
			                  const vector<type> & values);

template<class type>
ushort draw_full_circle(v3dMatrix<type> * source, 
 			                   const vint8 y1, const vint8 x1, const vint8 radius,
			                   const type value);

template<class type>
v3dMatrix<type> * function_copy_polygon(const v3dMatrix<type> * the_image, 
				                                const vPolygon * polygon);

template<class type>
v3dMatrix<type> * function_subtract_polygon(const v3dMatrix<type> * the_image, 
                        				            const vPolygon * polygon);

template<class type>
v3dMatrix<type> * function_copy_rectangle(const v3dMatrix<type> * source, 
                                          const vint8 left, const vint8 right, 
                                          const vint8 top, const vint8 bottom);

template <class type>
ushort function_copy_rectangle(const v3dMatrix<type> * source, v3dMatrix<type> * target,
                               const vint8 left, const vint8 right, const vint8 top, const vint8 bottom);

template<class type>
ushort function_crop_image(v3dMatrix<type> * the_image, const vPolygon * polygon);

template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, 
                          const vint8 y1, const vint8 x1, const vector<type> * values);

template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, 
                          const vint8 y1, const vint8 x1, const type red, const type green, const type blue);

template<class type>
ushort function_draw_cross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                           const vint8 width, const vint8 length, const type value);

template<class type>
ushort function_draw_cross(v3dMatrix<type> * the_image, const vint8 row, const vint8 col, 
                           const vint8 width, const vint8 length, const vector<type> * values);


// Draws a line starting at (y,x); (y is row, x is col), whose
// slope is specified by angle and has the given length. Notice
// that the image coordinates are not the standard coordinate 
// system (y increases as we go down in image coordinates);. Therefore
// a line with an orientation of PI/2 points downwards.
template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, 
		                      const vint8 y, const vint8 x, const double angle, const vint8 length,
		                      const vector<type> * values);

template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, 
		                      const vint8 y, const vint8 x, const double angle, const vint8 length,
		                      const type value);


template<class type>
ushort function_draw_line(v3dMatrix<type> * the_image, const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
                          const type value);

template<class type>
ushort draw_centered_line(v3dMatrix<type> * the_image, const vint8 center_row, const vint8 center_col,
                          const double angle, const vint8 length, const type value);


template<class type>
ushort draw_array_line(type ** array, const vint8 rows, const vint8 cols,
	                     const vint8 y0, const vint8 x0, const vint8 y1, const vint8 x1,
	                     const type value);

// Draws a polygon onto an image. The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort function_draw_polygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                         const vector<type> * values);

// Draws an open polygon onto an image. "Open polygon" is my own terminology
// for a polygon in which the line segment connecting the last vertex back
// to the first vertex is not drawn.
// The color of the polygon is specified
// by the "values" argument, which should have one entry for each band
// in the target image.
template<class type>
ushort draw_open_polygon(v3dMatrix<type> * the_image, const vPolygon * polygon,
		                     const vector<type> * values);


template<class type>
v3dMatrix<type> * mask_from_polygon(const v3dMatrix<type> * the_image, 
				                            const vPolygon * polygon);

template<class type>
v3dMatrix<type> * function_rotate_right(const v3dMatrix<type> * fimage);

// Works only if degrees is a multiple of 90.
template<class type>
v3dMatrix<type> * function_rotate_right(const v3dMatrix<type> * fimage, 
                                        const vint8 degrees);

template<class type>
v3dMatrix<type> * function_rotate_left(const v3dMatrix<type> * fimage);

template<class type>
v3dMatrix<type> * function_upside_down(const v3dMatrix<type> * fimage);

// This function is useful for raster images that are stored on 
// file with their bottom row first. This way I can read them
// using v3dMatrix::ReadInterlaced and then call this function
// to flip them 
template<class type>
v3dMatrix<type> * switch_top_bottom(const v3dMatrix<type> * the_image);

template<class type>
v3dMatrix<type> * reverse_rows_columns(const v3dMatrix<type> * fimage);


template<class type>
void function_enter_value(v3dMatrix<type> * the_image, const type value);


template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                   const vector<vint8> * rows, const vector<vint8> * cols, 
                   const type value);

// Same as vWriteValue2, but point coordinates here are passed
// in in arrays, not in vectors.
template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                   const vArray(uchar) point_rows, 
                   const vArray(uchar) point_cols, 
                   vint8 number, type value);

template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                   const vArray(vint8) point_rows, 
                   const vArray(vint8) point_cols, 
                   const vint8 number, const type value);


// matrix has two columns, and each row represents a pixel:
// col 0 is pixel row, col1 is pixel col
template<class type>
void function_enter_values(v3dMatrix<type> * the_image, 
                           const vint8_matrix pixels, const type value);


// Writes the specified value to the border pixels of the image
// (pixels in the top and bottom rows and left and right columns);.
template<class type>
ushort enter_border_value(v3dMatrix<type> * the_image, const type value);

// Writes the specified value to the border pixels of the image
// (pixels whose distance from the border is <= width);.
template<class type>
ushort enter_border_value(v3dMatrix<type> * the_image, const type value, vint8 width);


// result[row][col] will get the values of color_image[row2][col2]
// iff warp_template[row][col] = (row2, col2);.
template<class type>
v3dMatrix<type> * function_warp_template(const v3dMatrix<type> * the_image, 
                                const v3dMatrix<short> * warp_template);

// here we do bilinear interpolation
template <class type >
v3dMatrix<type> * rotate_image_exact_degrees(const v3dMatrix<type> * the_image, 
                                    const vint8 center_row, const vint8 center_col,
                                    const double degrees);

// write a value to a specific band
template<class type>
void function_channel_value(v3dMatrix<type> * the_image, vint8 channel, type value);

#ifndef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#include "drawing_temp.cpp"
#endif // VASSILIS_SGI_PLATFORM


#endif //  VASSILIS_DRAWING_TEMP_H
