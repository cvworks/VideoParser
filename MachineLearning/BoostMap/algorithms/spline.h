#ifndef VASSILIS_SPLINE_H
#define VASSILIS_SPLINE_H


#include "basics/algebra.h"
#include "math.h"
#include "basics/angles.h"
#include "spline_auxiliaries.h"

//(category == 0) // Unknown curve location, Red
//(category == 1) // Wire tip (high-contrast), Blue 
//(category == 2) // Regular wire (low-contrast), Yellow
//(category == 3) // Cathether, Green
//(category == 4) // Stent, PINK
//(category == 5) // Known curve location but unknown curve type, Light blue
//(category == 6) // Catheter & wire, Light Green
//(category == 7) // Wire (low or high density) NOT to be used for training, 



//#define BG_DBG_OUT
      
float evaluate_spline(float * xa, float * ya, float * y2a, 
			                 int n, float x);

void initialize_spline(float * x, float * y, int n, float * y2);

// this is an initial implementation of splines, designed so that I use
// the code from numerical recipes with as few changes as possible.
// essentially this is a model of 2-D splines
class class_spline_first
{
public:
  long number;
  float_matrix control_points;
  float_matrix estimated_parameters;

  class_spline_first ()
  {
    number = 0;
  }

  class_spline_first (float_matrix in_control_points);
  ~class_spline_first();

  void get_point (float argument, float * horizontal, float * vertical);
  color_image* draw_image (general_image* image);
};


// this is an improved implementation of splines, with some changes
// to the code from numerical recipes.
class class_spline
{
public:
  // using the contents of horizontal_vector and vertical_vector
  // it creates the various matrices (deleting the existing ones),
  // and it computes the derivatives.
  long update();
  long compute_derivatives();

public:
  // this is the number of points that the user actually specified.
  long number;

  // data_matrix stores extra points
  // at the beginning and the end, so it has two more points than 
  // the corresponding vector
  float_matrix data_matrix;

  // the second derivative information, computed as specified in 
  // numerical recipes.
  float_matrix derivative_matrix;

  // scratch matrix, used in the computations
  float_matrix scratch_matrix;

  // data_vector stores the points that the user
  // specifies
  std::vector <float > data_vector;

public:
  class_spline()
  {
    number = 0;
  }

  class_spline(float_image* user_data);
  ~class_spline();

  // argument is a number between 0 and number - 1.
  // We compute the coordinates that respond to that argument
  float get_point (const float argument) const;

// insert data AFTER the current index (NOT at the current index)
  long insert_data (const long index, const float point);

  // deletes the point specified by index
  long delete_data(const long index);
};




class class_belt_spline
{
protected:
  // using the contents of horizontal_vector, vertical_vector,
  // and width_vector,
  // it creates the various matrices (deleting the existing ones).
  //long update();
  long from_matrix(float_matrix representation);
  long extract_widths(float_matrix representation);
  long extract_categories(float_matrix representation);
  long extract_control_points(float_matrix representation);

  // this is the number of points that the user actually clicked on.
  long number;

public:
	long update();

  class_spline horizontal_spline;
  class_spline vertical_spline;
  float_matrix orientations;

  std::vector<long> width_vector;
  vint8_matrix widths;

  std::vector<long> category_vector;
  vint8_matrix categories;

  //  float_matrix responses;

  // current_index indicates the position where insertions (after it)
  // and deletions (on it)
  // should occur next.  Usually that should be the end of the sequence,
  // but it changes when select_index is called.
  long current_index;


  class_belt_spline()
  {
    number = 0;
    current_index = -1;
  }

  class_belt_spline(float_image * in_control_points);

  // this is legacy code, format should be 1 to specify the old format.
  class_belt_spline(const char* filename, long format);

  class_belt_spline(const char* filename);
  class_belt_spline(FILE* file_pointer);
  //class_belt_spline (file_handle * file_pointer);
  ~class_belt_spline();

  inline long get_number() const
  {
    return number;
  }
  
  inline long valid() const
  {
    return (number > 1);
  }

  float_matrix matrix_representation() const;
  float_matrix control_points() const;
  long enter_control_points(float_matrix representation) const;
  long enter_widths(float_matrix representation) const;
  long enter_categories(float_matrix representation) const;

  class_belt_spline * copy() const;

  static char * filename(const char * sequence_name, long frame);
  static char * old_filename(const char * sequence_name, long frame);
  static char * old_tracking_filename(const char * sequence_name, long frame);

  // save in the old format.
  long save_old(const char* filename) const;

  long save(const char* filename) const;
  long save(file_handle * file_pointer) const;
  long print() const;

  // argument is a number between 0 and number - 1.
  // We compute the coordinates that respond to that argument
  void get_point(float argument, float * horizontal, float * vertical) const;
  spline_pixel get_pixel(float argument) const;

  // draw the control points and the points in between.
  color_image* draw_image2(general_image* image) const;

  // draw the control points and the points in between.
  color_image* draw_image(general_image* image) const;

  // new draw image
  void draw_on_image(color_image* image) const;
  void draw_on_image2(color_image* image) const;
  void draw_on_image_sparse(color_image* image) const;

  inline float pixel_distance (long index, float horizontal, float vertical) const;
  float pixel_distance2(float parameter, float horizontal, float vertical) const;

  // finds the index of the control point nearest to the passed coordinates
  long closest_index(float horizontal, float vertical) const;
  float closest_distance(float horizontal, float vertical) const;

  // selects the index of the point nearest to the passed coordinates
  long select_index(float horizontal, float vertical);

  long set_index(long index_num);
  long insert_pixel(float horizontal, float vertical, long category = 0);
  long move_pixel(long index, float horizontal, float vertical);
  long set_category(long index, long category);

  // deletes the point specified by current index
  long delete_pixel();

  long compute_orientations();

  float get_orientation(float argument) const;

  long horizontal(long index) const;
  long vertical(long index) const;

  // scales the coordinates of all control points by the given factor
  long scale(float factor);

  long specify_widths(long start, long finish, long width);
  long specify_categories(long start, long finish, long category);

  // the maximum distance between consecutive control points in the spline
  float maximum_distance() const;

  // scores, in output arguments xs and ys, a list of points on the spline. Parameter "spacing"
  // indicates the desired distance between consecutive points on the list.  However,
  // the actual distance is expected to be greater than "spacing".  Parameter "spacing"
  // is used to determine the increment that we add to the variable that parameterizes the spline.
  // However, the correct increment, for each spline segment between to control points,
  // also depends on the distance, along the spline curve, between the two control points.
  // In this function, we do not try to estimate the distance, and instead we use the 
  // Euclidean distance between each two consecutive control points as an approximation.
  // Since the distance along the curve is never less than the Euclidean distance,
  // the increment that we compute is usually larger, and the spacing that we obtain
  // is also larger than the desired one.
  // Note that this function is NOT designed for sub pixel accuracy.
  long resample(float spacing, std::vector <long> * xs, std::vector<long> * ys) const;

  // similar to resample, but this function is designed for sub pixel accuracy
  long resample_points (float spacing, std::vector <spline_point> * points) const;

  bool bg_resample_spline(float spacing, class_belt_spline& resampled_spline) const;

  // similar to resample, and resample_points, but this function is designed to
  // get the pixels (in integer coordinates) corresponding to the spline,
  // and avoid skipping pixels and duplications of pixels (unless a pixel,
  // corresponds to more than one spline segment).
  long get_pixels (std::vector <spline_pixel> * pixels, long spline_id) const;
  long get_pixels (std::vector <class_pixel> * pixels, long spline_id) const;

  // returns a subset of the pixels collected by get_pixels, that satisfies
  // the following requirements:
  // - all pixels have the specified width.
  // - all pixels are interior pixels in a maximal connected part of the spline that has the specified
  // width.  Parameter "margin" specifies the threshold that is used in determining
  // if the pixel is interior or not.  To decide if a pixel is interior
  // we require that its distance along the curve to both the first and the last point
  // in the maximal connected part under consideration.
  // spline_id is useful for marking the pixels as coming from this spline,
  // if multiple splines are present.
  // this is the initial version, where the orientation for each 
  // spline pixel was computed locally at that pixel
  long interior_pixels_old(std::vector <spline_pixel> * pixels, const long width, 
                       const long margin, const long spline_id) const;
  
  // here the orientation at each pixel is determined based on an average
  // in the neighborhood around the pixel.
  long renamed_interior_pixels(std::vector <spline_pixel> * pixels, const long width, 
                       const long margin, const long spline_id) const;
  
  // calculates the maximum orientation change between two control points
  float maximum_orientation_change () const;

   bool bg_compute_score(const float_matrix& strength,const float_matrix& angles, float& result, long& length);
   bool bg_smoothness_score(float& score);
   bool bg_bending_pass(int idx);

  // the next function is used to create a spline corresponding to
  // the curve detection result obtained using dynamic programming.
  // the input matrix should have three columns, and each row should
  // contain the vertical, horizontal, and orientation of the midpoint
  // of a line segment.  The length of each line segment is the second
  // argument. orientation_factor is a factor we must multiply
  // each orientation with in the to get that orientation in radians.
  static class_belt_spline * detection_spline(vint8_matrix input_matrix, long segment_length,
                                              float orientation_factor);

  static long draw_detection_result(vint8_matrix input_matrix, long segment_length,
                               float orientation_factor, color_image * image);

  static vint8_matrix training_wire_categories();
  static vint8_matrix evaluation_wire_categories();
  static vint8_matrix training_catheter_categories();
  static vint8_matrix evaluation_catheter_categories();
  static vint8_matrix training_wiretip_categories();
  static vint8_matrix evaluation_wiretip_categories();

  // here the orientation at each pixel is determined based on an average
  // in the neighborhood around the pixel.
  long interior_category_pixels(std::vector<spline_pixel> * pixels, const vint8_matrix categories, 
                                const long margin, const long spline_id) const;

  long interior_category_pixels(std::vector<class_pixel> * pixels, const vint8_matrix categories, 
                                const long margin, const long spline_id) const;

  // identifies the first and last index of pixels belonging to the segment
  // between the control point specified by parameter and the next control point.
  // search_start is an optional parameter that specifies the index from which
  // we should start looking. If no points are found that belong to that segment
  // then the start index and the finish index are both equal to -1
  static void get_segment_indices(const std::vector<spline_pixel> & pixels, 
                                  const long parameter,
                                  long * start_index_pointer, long * finish_index_pointer, 
                                  const long search_start = 0);

  static void get_pixel_color(long category, uchar * red_pointer,
                              uchar * green_pointer, uchar * blue_pointer);

  static long legal_category(long category)
  {
    if ((category < 0) || (category >= 8))
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }
};

long print_spline_pixels (std::vector <spline_pixel> * pixels);

grayscale_image * spline_pixel_image(std::vector <spline_pixel> * pixels);

grayscale_image * spline_pixel_image(std::vector <spline_pixel> * pixels,
                                     long vertical_size, long horizontal_size);

float_matrix spline_orientation_image(std::vector <spline_pixel> * pixels_pointer,
                                      long vertical_size, long horizontal_size);


long adjust_pixel_orientations(std::vector <spline_pixel> * pixels, const long margin);

long extract_pixels(std::vector<spline_pixel> * input, std::vector<class_pixel> * output);

// replace the estimated orientations with new estimates, based on
// positions of neighboring pixels
long adjust_orientations(std::vector<spline_pixel> & pixels);




#endif // VASSILIS_SPLINE_H

