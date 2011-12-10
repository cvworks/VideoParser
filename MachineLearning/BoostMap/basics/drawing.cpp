
#include <math.h>
#include <string.h>

#include "vplatform.h"

#include "pc_aux.h"

#include "matrix.h"
#include "drawing.h"
#include "drawing_temp.h"
#include "auxiliaries.h"
#include "precomputed.h"

#include "definitions.h"

using std::vector;

///////////////////////////////////////////////////////////////////////
// Implementation of the class vColorMap
///////////////////////////////////////////////////////////////////////


vColorMap::vColorMap()
{
  number_of_colors = 0;
}


// Build a color map for color_image, creates an image with the size
// of color_image and puts it into color_ids, and stores in that image,
// in every pixel, the color map ID of the color of that pixel in color_image.
vColorMap::vColorMap(color_image * the_image, v3dMatrix<vint8> ** color_ids)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 size = rows * cols;
  vArray(uchar) r_band = the_image->GetR();
  vArray(uchar) g_band = the_image->GetG();
  vArray(uchar) b_band = the_image->GetB();

  v3dMatrix<vint8> * result_matrix = new v3dMatrix<vint8>(rows, cols, 1);
  vArray(vint8) result = result_matrix->Matrix(0);

  // create a scratch array of colors, where we will mark the colors that
  // exist in color_image.
  const vint8 total_colors = 256;
  v3dMatrix<vint8> colors_matrix(total_colors, total_colors, total_colors);
  vArray3(vint8) colors = colors_matrix.Matrix3();

  // Go through the image once, and put zeros in locations corresponding
  // to image colors.
  vint8 i;
  for (i = 0; i < size; i++)
  {
    uchar r = r_band[i];
    uchar g = g_band[i];
    uchar b = b_band[i];

    colors[r][g][b] = 0;
  }

  // Now go through the image again, and store all unique colors into
  // colormap. For each color, the corresponding bin in colors will
  // get its color ID plus 1 (so that zero remains reserved for
  // colors not seen so far).
  number_of_colors = 0;
  for (i = 0; i < size; i++)
  {
    uchar r = r_band[i];
    uchar g = g_band[i];
    uchar b = b_band[i];

    vint8 current_id = colors[r][g][b];
    // if current_id is 0, then it is the first occurence of r,g,b,
    // so we should assign an id to it, and add it to the colormap.
    if (current_id == 0)
    {
      current_id = number_of_colors;
      colors[r][g][b] = current_id + 1;
      R.push_back(r);
      G.push_back(g);
      B.push_back(b);
      number_of_colors++;
    }
    else
    {
      current_id--;
    }
    result[i] = current_id;
  }

  *color_ids = result_matrix;
}


vColorMap::~vColorMap()
{
}

// Writes the color map to a file
ushort vColorMap::Write(FILE * fp)
{
  // Write the number of colors
  vint8 items = store_vint8(fp, number_of_colors);
  if (items != 1) return 0;

  // Put all the color values into one big array, so that we can
  // write it all at once (for efficiency).
  uchar * entries = new uchar[(vector_size) (3 * number_of_colors)];
  vint8 i;
  vint8 index = 0;
  for (i = 0; i < number_of_colors; i++)
  {
    entries[index++] = R[(vector_size) i];
    entries[index++] = G[(vector_size) i];
    entries[index++] = B[(vector_size) i];
  }
  // Write the array
  items = fwrite(entries, sizeof(uchar), (long) (3 * number_of_colors), fp);
  vdelete2(entries);
  if (items != 3 * number_of_colors) return 0;
  return 1;
}


// Reads a color map from a file
ushort vColorMap::Read(FILE * fp)
{
  // Read the number of colors
  vint8 items = read_vint8(fp, &number_of_colors);
  if (items != 1) return 0;
  
  // Read all the values into one big array (for efficiency)
  uchar * entries = new uchar[(vector_size) (3 * number_of_colors)];
  items = fread(entries, sizeof(uchar), (long) (3 * number_of_colors), fp);
  if (items != 3 * number_of_colors) 
  {
    vdelete2(entries);
    return 0;
  }

  // If anything was in the colormap before, we must erase it.
  R.erase(R.begin(), R.end());
  G.erase(G.begin(), G.end());
  B.erase(B.begin(), B.end());

  // Copy the values from the array into the R, G, B vectors.
  vint8 i;
  vint8 index = 0;
  for (i = 0; i < number_of_colors; i++)
  {
    R.push_back(entries[index++]);
    G.push_back(entries[index++]);
    B.push_back(entries[index++]);
  }
  vdelete2(entries);
  return 1;
}

// We create a color image. For every pixel in the gray
// image, we use its value as an index into the color map,
// and we put the corresponding color into the color image.
// If the index is illegal (too high) we return null.
color_image * vColorMap::Convert(grayscale_image * gray)
{
  vint8 rows = gray->Rows();
  vint8 cols = gray->Cols();
  color_image * the_image = new color_image(rows, cols);

  vArray(uchar) I = gray->GetI();
  vArray(uchar) red = the_image->GetR();
  vArray(uchar) green = the_image->GetG();
  vArray(uchar) blue = the_image->GetB();
  ushort error = 0;

  vint8 size = rows * cols;
  vint8 i;
  for (i = 0; i < size; i++)
  {
    uchar index = I[i];

    if (index >= number_of_colors)
    {
      error = 1;
      break;
    }
    red[i] = R[index];
    green[i] = G[index];
    blue[i] = B[index];
  }

  if (error == 1)
  {
    vdelete(the_image);
    return 0;
  }
  else return the_image;
}


vColorMap * vColorMap::Copy()
{
  vColorMap * result = new vColorMap;
  result->number_of_colors = number_of_colors;
  vint8 i;
  for (i = 0; i < number_of_colors; i++)
  {
    result->R.push_back(R[(vector_size) i]);
    result->G.push_back(G[(vector_size) i]);
    result->B.push_back(B[(vector_size) i]);
  }

  return result;
}

// Returns true if the number of colors is the same in both maps
// and the entries in the color maps are identical.
ushort vColorMapEqual(vColorMap * map1, vColorMap * map2)
{
  if ((map1 == 0) && (map2 == 0)) return 1;
  if (map1 == 0) return 0;
  if (map2 == 0) return 0;

  vint8 number_of_colors = map1->number_of_colors;
  if (number_of_colors != map2->number_of_colors) return(0);

  vint8 i;
  for (i = 0; i < number_of_colors; i++)
  {
    if (map1->R[(vector_size) i] != map2->R[(vector_size) i]) return(0);
    if (map1->G[(vector_size) i] != map2->G[(vector_size) i]) return(0);
    if (map1->B[(vector_size) i] != map2->B[(vector_size) i]) return(0);
  }

  return(1);
}


grayscale_image * vCopyGrayImage(grayscale_image * source)
{
  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  grayscale_image * result = new grayscale_image(rows, cols);
  vint8 size = rows * cols;
  memcpy(vData(result->GetI()), vData(source->GetI()), (vector_size) size);
  return result;
}


void vCopyGrayImage(grayscale_image * target, grayscale_image * source)
{
  memcpy(vData(target->GetI()), vData(source->GetI()), (vector_size) source->Size());
}

/////////////////////////////////////////////////////////////////////
// Implementation of the class vPolygon
/////////////////////////////////////////////////////////////////////

vPolygon::vPolygon()
{
  num_vertices = 0;
}


vPolygon::~vPolygon()
{
}


void vPolygon::Print()
{
  printf("num_vertices = %li\n", num_vertices);
  vint8 i;
  for (i = 0; i < num_vertices; i++)
  {
    printf("%li %li %li\n", i, x_coordinates[(vector_size) i], y_coordinates[(vector_size) i]);
  }
  fflush(stdout);
}


void vPolygon::AddVertex(vint8 row, vint8 col)
{
  y_coordinates.push_back(row);
  x_coordinates.push_back(col);
  num_vertices++;
}


// Removes the last vertex (if any).
void vPolygon::DeleteVertex()
{
  if (num_vertices == 0) return;
  y_coordinates.pop_back();
  x_coordinates.pop_back();
  num_vertices--;
}


ushort vPolygon::GetBounds(vint8 * top, vint8 * bottom, vint8 * left, vint8 * right)
{
  if (num_vertices == 0) return 0;
  *top = vTopEnd(this);
  *bottom = vBottomEnd(this);
  *left = vLeftEnd(this);
  *right = vRightEnd(this);
  return 1;
}


/////////////////////////////////////////////////////////////////////
// Implementation of the class vLineSet
/////////////////////////////////////////////////////////////////////


vLineSet::vLineSet()
{
  r = 255;
  g = 255;
  b = 255;

  StartPolygon();
}


vLineSet::~vLineSet()
{
  vint8 number = polygons.size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vPolygon * polygon = (vPolygon *) polygons[(vector_size) i];
    vdelete(polygon);
  }
}


vPolygon * vLineSet::CurrentPolygon()
{
  vint8 number = polygons.size();
  if (number == 0) return 0;
  return (vPolygon *) polygons[(vector_size) (number-1)];
}


// Adds a new polygon to the list of polygons, unless the 
// current polygon is empty.
ushort vLineSet::StartPolygon()
{
  vPolygon * current = CurrentPolygon();
  if ((current != 0) && (current->num_vertices == 0)) return 0;
  vPolygon * polygon = new vPolygon;
  polygons.push_back(polygon);
  return 1;
}


// Adds a vertex to the most recent polygon
void vLineSet::AddVertex(vint8 row, vint8 col)
{
  vPolygon * polygon = CurrentPolygon();
  if (polygon == 0) return;
  polygon->AddVertex(row, col);
}


// Deletes the last vertex of the most recent polygon.
void vLineSet::DeleteVertex()
{
  vPolygon * polygon = CurrentPolygon();
  if (polygon == 0) return;
  polygon->DeleteVertex();
  if ((polygon->num_vertices == 0) && (polygons.size() > 1))
  {
    vdelete(polygon);
    polygons.pop_back();
    vPrint("Deleted polygon\n");
  }
}


// Writes the four bounds of the line segment set into
// the arguments. We assume a matrix coordinate system,
// where row coordinates increase from top to bottom.
ushort vLineSet::GetBounds(vint8 * top, vint8 * bottom, vint8 * left, vint8 * right)
{
  vint8 number = polygons.size();
  if (number == 0) return 0;

  // Initialize bounds.
  // The "temp" variables will hold the topmost, leftmost etc. values found so far.
  vint8 temp_top, temp_bottom, temp_left, temp_right;
  vPolygon * polygon = (vPolygon *) polygons[0];
  ushort success = polygon->GetBounds(&temp_top, &temp_bottom, 
                                      &temp_left, &temp_right);
  
  // success could be 0 if the polygon has no vertices. That should
  // only happen with the last polygon
  if (success == 0) 
  {
    assert(number == 1);
    return 0;
  }

  vint8 i;
  // The "current" variables will hold the bounds of the current
  // polygon (based on which the "temp" variables get updated).
  vint8 current_top, current_bottom, current_left, current_right;

  for (i = 1; i < number; i++)
  {
    polygon = (vPolygon *) polygons[(vector_size) i];
    success = polygon->GetBounds(&current_top, &current_bottom, 
                                 &current_left, &current_right);
    if (success == 0)
    {
      assert(i == number - 1);
      return 1;
    }
    if (current_top < temp_top) temp_top = current_top;
    if (current_bottom > temp_bottom) temp_bottom = current_bottom;
    if (current_left < temp_left) temp_left = current_left;
    if (current_right > temp_right) temp_right = current_right;
  }
  return 1;
}


// Draws each line segment into the image
ushort vLineSet::DrawColor(color_image * the_image)
{
  ushort success;
  vector<uchar> colors;
  colors.push_back(r);
  colors.push_back(g);
  colors.push_back(b);
  vint8 number = polygons.size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vPolygon * polygon = (vPolygon *) polygons[(vector_size) i];
    success = vDrawOpenPolygon(the_image, polygon, &colors);
    if (success == 0) return 0;
  }
  return 1;
}


ushort vLineSet::DrawGray(grayscale_image * gray)
{
  double double_r, double_g, double_b;
  double_r = r;
  double_g = g;
  double_b = b;
  double double_gray = 0.3 * double_r + 0.59 * double_g + 0.11 * double_b;
  uchar gray_color = (uchar) double_gray;
  vector<uchar> colors;
  colors.push_back(gray_color);

  vint8 number = polygons.size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vPolygon * polygon = (vPolygon *) polygons[(vector_size) i];
    ushort success = vDrawOpenPolygon(gray, polygon, &colors);
    if (success == 0) return 0;
  }
  return 1;
}


ushort vLineSet::Draw(general_image * the_image)
{
  if (predicate_color_image(the_image))
  {
    return DrawColor((color_image *) the_image);
  }
  else if (predicate_gray_image(the_image))
  {
    return DrawGray((grayscale_image *) the_image);
  }
  else return 0;
}


void vLineSet::SetColor(uchar in_r, uchar in_g, uchar in_b)
{
  r = in_r;
  g = in_g;
  b = in_b;
}



/////////////////////////////////////////////////////////////////////
// End of implementation of class vLineSet
/////////////////////////////////////////////////////////////////////


vint8 vLeftEnd(vPolygon * polygon)
{
  vint8 vertices = polygon->num_vertices;
  vector<vint8> xs = polygon->x_coordinates;

  vint8 result = xs[0];
  vint8 i;
  vint8 x;

  for (i = 1; i < vertices; i++)
    {
      x = xs[(vector_size) i];
      if (x < result) result = x;
    }

  return result;
}


vint8 vRightEnd(vPolygon * polygon)
{
  vint8 vertices = polygon->num_vertices;
  vector<vint8> xs = polygon->x_coordinates;

  vint8 result = xs[0];
  vint8 i;
  vint8 x;

  for (i = 1; i < vertices; i++)
    {
      x = xs[(vector_size) i];
      if (x > result) result = x;
    }

  return result;
}


vint8 vTopEnd(vPolygon * polygon)
{
  vint8 vertices = polygon->num_vertices;
  vector<vint8> ys = polygon->y_coordinates;

  vint8 result = ys[0];
  vint8 i;
  vint8 y;

  for (i = 1; i < vertices; i++)
    {
      y = ys[(vector_size) i];
      if (y < result) result = y;
    }

  return result;
}


vint8 vBottomEnd(vPolygon * polygon)
{
  vint8 vertices = polygon->num_vertices;
  vector<vint8> ys = polygon->y_coordinates;

  vint8 result = ys[0];
  vint8 i;
  vint8 y;

  for (i = 1; i < vertices; i++)
    {
      y = ys[(vector_size) i];
      if (y > result) result = y;
    }

  return result;
}

ushort vTranslatePolygon(vPolygon * polygon, vint8 x, vint8 y)
{
  vint8 i;
  vint8 vs = polygon->num_vertices;
  vector<vint8> xs = polygon->x_coordinates;
  vector<vint8> ys = polygon->y_coordinates;

  for (i = 0; i < vs; i++)
   {
     xs[(vector_size) i] = xs[(vector_size) i] + x;
     ys[(vector_size) i] = ys[(vector_size) i] + y;
   }
  return 1;
}

double vVerticalClockwise(double angle)
{
  const double HALF_PI = M_PI / 2;
  double result = angle - HALF_PI;
  return result;
}


double vVerticalCounterclockwise(double angle)
{
  const double HALF_PI = M_PI / 2;
  double result = angle + HALF_PI;
  return result;
}


ushort vLocatePoint(double x0, double y0, double angle, double length,
            		   double & x1, double & y1)
{
  x1 = x0 + cos(angle) * length;
  y1 = y0 + sin(angle) * length;
  return 1;
}


typedef struct tempBITMAPFILEHEADER 
{ // bmfh 
    char    bfType[2]; 
    vuint4  bfSize; 
    vuint2  bfReserved1; 
    vuint2  bfReserved2; 
    vuint4  bfOffBits; 
} WindowsBitmapFileHeader;


// Returns 1 or greater if successful, 0 or less if it fails.
short S_ReadBitmapFileHeader(FILE * fp, WindowsBitmapFileHeader * header)
{
  vint8 items;
  items = fread(header->bfType, sizeof(char), 2, fp);
  if (items != 2) return 0;
  if (header->bfType[0] != 'B') return -1;
  if (header->bfType[1] != 'M') return -2;

  items = read_vint8(fp, (vint8 *) &(header->bfSize));
  if (items != 1) return -3;

  items = vReadPCUshorts(fp, &(header->bfReserved1), 1);
  if (items != 1) return -4;
  if (header->bfReserved1 != 0) return -5;

  items = vReadPCUshorts(fp, &(header->bfReserved2), 1);
  if (items != 1) return -6;
  if (header->bfReserved1 != 0) return -7;

  items = read_vint8(fp, (vint8 *) &(header->bfOffBits));
  if (items != 1) return -8;
  if (header->bfOffBits < 54) return -9;

  return 1;
}


short S_WriteBitmapFileHeader(FILE * fp, WindowsBitmapFileHeader * header)
{
  vint8 items;
  items = fwrite(header->bfType, sizeof(char), 2, fp);
  if (items != 2) return 0;

  items = store_vint8(fp, (vint8) header->bfSize);
  if (items != 1) return -3;

  items = vWritePCUshorts(fp, &(header->bfReserved1), 1);
  if (items != 1) return -4;

  items = vWritePCUshorts(fp, &(header->bfReserved2), 1);
  if (items != 1) return -6;

  items = store_vint8(fp, (vint8) header->bfOffBits);
  if (items != 1) return -8;

  return 1;
}


short S_MakeBitmapFileHeader(v3dMatrix<uchar> * image, 
                             WindowsBitmapFileHeader * header)
{
  vint8 size = image->AllBandSize();
  vint8 bands = image->Bands();

  vint8 colormap_bytes;
  if (bands == 1) colormap_bytes = 1024;
  else if (bands == 3) colormap_bytes = 0;
  else
  {
    return 0;
  }
  
  memcpy(header->bfType, "BM", 2);
  header->bfSize = (vuint4) (54 + colormap_bytes + size);
  header->bfReserved1 = 0;
  header->bfReserved2 = 0;
  header->bfOffBits = (vuint4) (54 + colormap_bytes);

  return 1;
}


typedef struct tempBITMAPINFOHEADER
{ // bmih 
    vuint4  biSize; 
    vint4   biWidth; 
    vint4   biHeight; 
    vuint2  biPlanes; 
    vuint2  biBitCount;
    vuint4  biCompression; 
    vuint4  biSizeImage; 
    vint4   biXPelsPerMeter; 
    vint4   biYPelsPerMeter; 
    vuint4  biClrUsed; 
    vuint4  biClrImportant; 
} WindowsBitmapInfoHeader; 
 

short S_ReadBitmapInfoHeader(FILE * fp, WindowsBitmapInfoHeader * header)
{
  vint8 items;
  items = read_vint8(fp, (vint8 *) &(header->biSize));
  if (items != 1) return 0;
  if (header->biSize != 40) return -1;

  items = read_vint8(fp, (vint8 *) &(header->biWidth));
  if (items != 1) return -2;

  items = read_vint8(fp, (vint8 *) &(header->biHeight));
  if (items != 1) return -3;

  items = vReadPCUshorts(fp, (ushort *) &(header->biPlanes), 1);
  if (items != 1) return -4;
  if (header->biPlanes != 1) return -5;

  items = vReadPCUshorts(fp, (ushort *) &(header->biBitCount), 1);
  if (items != 1) return -6;
  if ((header->biBitCount != 1) && (header->biBitCount != 8) 
      && (header->biBitCount != 24)) return -7;

  items = read_vint8(fp, (vint8 *) &(header->biCompression));
  if (items != 1) return -8;
  // biCompression should be 0 if no compression is used.
  if (header->biCompression != 0) return -9;

  items = read_vint8(fp, (vint8 *) &(header->biSizeImage));
  if (items != 1) return -10;

  items = read_vint8(fp, (vint8 *) &(header->biXPelsPerMeter));
  if (items != 1) return 0;

  items = read_vint8(fp, (vint8 *) &(header->biYPelsPerMeter));
  if (items != 1) return 0;
  
  items = read_vint8(fp, (vint8 *) &(header->biClrUsed));
  if (items != 1) return 0;
  if (header->biBitCount == 24)
  {
    if (header->biClrUsed != 0) return 0;
  }

  items = read_vint8(fp, (vint8 *) &(header->biClrImportant));
  if (items != 1) return 0;
  if (header->biBitCount == 24)
  {
    if (header->biClrImportant != 0) return 0;
  }

  return 1;
}


short S_WriteBitmapInfoHeader(FILE * fp, WindowsBitmapInfoHeader * header)
{
  vint8 items;
  items = store_vint8s(fp, (vint8 *) &(header->biSize), 1);
  if (items != 1) return 0;
  if (header->biSize != 40) return -1;

  items = store_vint8s(fp, (vint8 *) &(header->biWidth), 1);
  if (items != 1) return -2;

  items = store_vint8s(fp, (vint8 *) &(header->biHeight), 1);
  if (items != 1) return -3;

  items = vWritePCUshorts(fp, (ushort *) &(header->biPlanes), 1);
  if (items != 1) return -4;
  if (header->biPlanes != 1) return -5;

  items = vWritePCUshorts(fp, (ushort *) &(header->biBitCount), 1);
  if (items != 1) return -6;
  if ((header->biBitCount != 8) && (header->biBitCount != 24)) return -7;

  items = store_vint8s(fp, (vint8 *) &(header->biCompression), 1);
  if (items != 1) return -8;
  // biCompression should be 0 if no compression is used.
  if (header->biCompression != 0) return -9;

  items = store_vint8s(fp, (vint8 *) &(header->biSizeImage), 1);
  if (items != 1) return -10;

  items = store_vint8s(fp, (vint8 *) &(header->biXPelsPerMeter), 1);
  if (items != 1) return 0;

  items = store_vint8s(fp, (vint8 *) &(header->biYPelsPerMeter), 1);
  if (items != 1) return 0;
  
  items = store_vint8s(fp, (vint8 *) &(header->biClrUsed), 1);
  if (items != 1) return 0;
  if (header->biBitCount == 24)
  {
    if (header->biClrUsed != 0) return 0;
  }

  items = store_vint8s(fp, (vint8 *) &(header->biClrImportant), 1);
  if (items != 1) return 0;
  if (header->biBitCount == 24)
  {
    if (header->biClrImportant != 0) return 0;
  }

  return 1;
}


short S_MakeBitmapInfoHeader(v3dMatrix<uchar> * the_image, 
                             WindowsBitmapInfoHeader * header)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  if ((bands != 1) && (bands != 3)) return 0;

  header->biSize = 40;
  header->biWidth = (long) cols;
  header->biHeight = (long) rows;
  header->biPlanes = 1;
  header->biBitCount = (vuint2) (bands * 8);
  header->biCompression = 0;
  header->biSizeImage = 0;
  header->biXPelsPerMeter = 2925;
  header->biYPelsPerMeter = 2925;
  header->biClrUsed = 0;
  header->biClrImportant = 0;

  return 1;
}


vColorMap * vReadBMPColormap(FILE * fp, WindowsBitmapInfoHeader * header)
{
  if ((header->biBitCount != 8) &&
      (header->biBitCount != 1))
  {
    return 0;
  }
  vint8 number_of_colors = (vint8) header->biClrUsed;
  if (number_of_colors < 0) return 0;
  if (number_of_colors == 0) number_of_colors = 256;
  if (number_of_colors > 256) return 0;
  vint8 i;
  short success = 1;
  vColorMap * result = new vColorMap();
  vector<uchar> & R = result->R;
  vector<uchar> & G = result->G;
  vector<uchar> & B = result->B;

  for (i = 0; i < number_of_colors; i++)
  {
    uchar colors[4];
    vint8 items = fread(colors, sizeof(uchar), 4, fp);
    if (items != 4)
    {
      success = 0;
      break;
    }
    R.push_back(colors[2]);
    G.push_back(colors[1]);
    B.push_back(colors[0]);
  }

  if (success == 0)
  {
    vdelete(result);
    return 0;
  }
  result->number_of_colors = number_of_colors;
  return result;
}


color_image * vReadBMP(const char * filename)
{
  // Open the file
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;

  WindowsBitmapFileHeader file_header;
  WindowsBitmapInfoHeader info_header;
  short success = 1;
  color_image * result = 0;
  color_image * temp_image = 0;
  vector<vint8> band_order(3);
  vColorMap * color_table = 0;
  grayscale_image * pixel_values = 0;
  grayscale_image * temp_gray = 0;
  vint8 rows, cols;

  // Read BITMAPFILEHEADER
  success = S_ReadBitmapFileHeader(fp, &file_header);
  if (success <= 0) goto vReadBMPExit;

  // Read BITMAPINFOHEADER
  success = S_ReadBitmapInfoHeader(fp, &info_header);
  if (success <= 0) goto vReadBMPExit;
  // If the bitmap is not 24 bits per pixel or 8 bits per pixel, I
  // don't handle it, we just return 0

  if ((info_header.biBitCount != 24) &&
      (info_header.biBitCount != 8) &&
      (info_header.biBitCount != 1)) 
  {
    goto vReadBMPExit;
  }

  rows = info_header.biHeight;
  cols = info_header.biWidth;

  // If pixel values are saved in the BGR format, read them directly
  if (info_header.biBitCount == 24)
  {
    // Create result image
    result = new color_image(rows, cols);

    // Read image data. It should be read in BGR order, so we
    // need to set up the appropriate band order to pass
    // to ReadInterlaced.
    band_order[0] = 2;
    band_order[1] = 1;
    band_order[2] = 0;
    success = result->ReadInterlaced(fp, &band_order);
    if (success <= 0)
    {
      vdelete(result);
      fclose(fp);
      return 0;
    }

    // Rearrange rows of result, so that top row goes to bottom.
    temp_image = result;
    result = (color_image *) vSwitchTopBottom(temp_image);
    vdelete(temp_image);
  }
  // Otherwise, pixel values should be stored as colormap indices. 
  // Read the color table and the pixel values, and convert the
  // pixel values to RGB
  else if (info_header.biBitCount == 8)
  {
    color_table = vReadBMPColormap(fp, &info_header);
    if (color_table == 0)
    {
      goto vReadBMPExit;
    }
    vint8 cols2;
    if (cols % 4 == 0)
    {
      cols2 = cols;
    }
    else
    {
      cols2 = cols - (cols % 4) + 4;
    }

    temp_gray = new grayscale_image(rows, cols2);
    // Read image data. Bandwise and interlaced is the same, since
    // there is only one band, bandwise is more efficient.
    short success = temp_gray->ReadBandwise(fp);
    if (success <= 0)
    {
      vdelete(temp_gray);
      goto vReadBMPExit;
    }

    // Reverse bottom and top of image data (in BMP the bottom 
    // row is stored first, my v3dMatrix class assumes the top
    // row is stored first).
    pixel_values = (grayscale_image *) vSwitchTopBottom(temp_gray);
    vdelete(temp_gray);

    if (cols2 != cols)
    {
      grayscale_image * pixel_values2 = pixel_values;
      pixel_values = (grayscale_image *) vCopyRectangle(pixel_values2, 0, cols-1, 0, rows-1);
      vdelete(pixel_values2);
    }

    result = color_table->Convert(pixel_values);
    vdelete(pixel_values);
    vdelete(color_table);
  }

  else
  {
    color_table = vReadBMPColormap(fp, &info_header);
    if (color_table == 0)
    {
      goto vReadBMPExit;
    }
    vint8 cols2;
    if (cols % 32 == 0)
    {
      cols2 = cols;
    }
    else
    {
      cols2 = cols - (cols % 32) + 32;
    }

    temp_gray = new grayscale_image(rows, cols);
    vArray2(uchar) gray_values = temp_gray->Matrix2(0);
    // Read image data. Bandwise and interlaced is the same, since
    // there is only one band, bandwise is more efficient.

    vint8 row;
    Bitmap vbitmap(cols2);
    for (row = 0; row < rows; row++)
    {
      vint8 success = vbitmap.LoadData(fp);
      if (success <= 0)
      {
        vdelete(temp_gray);
        goto vReadBMPExit;
      }
      vint8 col2;
      for (col2 = 0; col2 < cols2; col2 += 8)
      {
        vint8 col;
        for (col = 0; col < 8; col++)
        {
          if (col2+7-col < cols)
          {
            gray_values[row][col2+7-col] = vbitmap.Read(col2+col);
          }
        }
      }
    }


    // Reverse bottom and top of image data (in BMP the bottom 
    // row is stored first, my v3dMatrix class assumes the top
    // row is stored first).
    pixel_values = (grayscale_image *) vSwitchTopBottom(temp_gray);
    vdelete(temp_gray);

    result = color_table->Convert(pixel_values);
    vdelete(pixel_values);
    vdelete(color_table);
  }

  // Finish up
vReadBMPExit:
  fclose(fp);
  return result;
}


grayscale_image * vReadGrayBMP(const char * filename)
{
  // Open the file
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;

  vint8 rows = 0, cols = 0;
  
  WindowsBitmapFileHeader file_header;
  WindowsBitmapInfoHeader info_header;
  short success = 1;
  grayscale_image * result = 0;
  grayscale_image * temp_image = 0;
  int error = 0;

  grayscale_image * pixel_values = 0;
  grayscale_image * temp_gray = 0;

  // Read BITMAPFILEHEADER
  success = S_ReadBitmapFileHeader(fp, &file_header);
  if (success <= 0) goto vReadBMPExit;

  // Read BITMAPINFOHEADER
  success = S_ReadBitmapInfoHeader(fp, &info_header);
  if (success <= 0) goto vReadBMPExit;
  // If the bitmap is not 8 bits per pixel, we return 0
  if ((info_header.biBitCount != 8) &&
      (info_header.biBitCount != 1)) 

  // Create result image
  result = new grayscale_image(info_header.biHeight, info_header.biWidth);

  rows = info_header.biHeight;
  cols = info_header.biWidth;

  // Read image data. Bandwise and interlaced is the same, since
  // there is only one band, bandwise is more efficient.
  // Otherwise, pixel values should be stored as colormap indices. 
  // Read the color table and the pixel values, and convert the
  // pixel values to RGB
  if (info_header.biBitCount == 8)
  {
    vColorMap * color_table = vReadBMPColormap(fp, &info_header);
    if (color_table == 0)
    {
      goto vReadBMPExit;
    }
    vint8 cols2;
    if (cols % 4 == 0)
    {
      cols2 = cols;
    }
    else
    {
      cols2 = cols - (cols % 4) + 4;
    }

    temp_gray = new grayscale_image(rows, cols2);
    // Read image data. Bandwise and interlaced is the same, since
    // there is only one band, bandwise is more efficient.
    short success = temp_gray->ReadBandwise(fp);
    if (success <= 0)
    {
      vdelete(temp_gray);
      goto vReadBMPExit;
    }

    // Reverse bottom and top of image data (in BMP the bottom 
    // row is stored first, my v3dMatrix class assumes the top
    // row is stored first).
    pixel_values = (grayscale_image *) vSwitchTopBottom(temp_gray);
    vdelete(temp_gray);

    if (cols2 != cols)
    {
      grayscale_image * pixel_values2 = pixel_values;
      pixel_values = (grayscale_image *) vCopyRectangle(pixel_values2, 0, cols-1, 0, rows-1);
      vdelete(pixel_values2);
    }

    color_image * result2 = color_table->Convert(pixel_values);
    vdelete(pixel_values);
    vdelete(color_table);

    result = result2->ToGray();
    vdelete(result2);
  }

  else
  {
    vColorMap * color_table = vReadBMPColormap(fp, &info_header);
    if (color_table == 0)
    {
      goto vReadBMPExit;
    }
    vint8 cols2;
    if (cols % 32 == 0)
    {
      cols2 = cols;
    }
    else
    {
      cols2 = cols - (cols % 32) + 32;
    }

    temp_gray = new grayscale_image(rows, cols);
    vArray2(uchar) gray_values = temp_gray->Matrix2(0);
    // Read image data. Bandwise and interlaced is the same, since
    // there is only one band, bandwise is more efficient.

    vint8 row;
    Bitmap vbitmap(cols2);
    for (row = 0; row < rows; row++)
    {
      vint8 success = vbitmap.LoadData(fp);
      if (success <= 0)
      {
        vdelete(temp_gray);
        goto vReadBMPExit;
      }
      vint8 col2;
      for (col2 = 0; col2 < cols2; col2 += 8)
      {
        vint8 col;
        for (col = 0; col < 8; col++)
        {
          if (col2+7-col < cols)
          {
            gray_values[row][col2+7-col] = vbitmap.Read(col2+col);
          }
        }
      }
    }


    // Reverse bottom and top of image data (in BMP the bottom 
    // row is stored first, my v3dMatrix class assumes the top
    // row is stored first).
    pixel_values = (grayscale_image *) vSwitchTopBottom(temp_gray);
    vdelete(temp_gray);

    color_image * result2 = color_table->Convert(pixel_values);
    vdelete(pixel_values);
    vdelete(color_table);
    result = result2->ToGray();
    vdelete(result2);
  }

  // Finish up
vReadBMPExit:
  fclose(fp);
  return result;
}


short vWriteColorBMP(color_image * in_color_image, const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0) return 0;
  WindowsBitmapFileHeader file_header;
  WindowsBitmapInfoHeader info_header;
  short success = S_MakeBitmapFileHeader(in_color_image, &file_header);
  if (success <= 0) return 0;
  success = S_MakeBitmapInfoHeader(in_color_image, &info_header);
  if (success <= 0) return 0;
  success = S_WriteBitmapFileHeader(fp, &file_header);
  success = S_WriteBitmapInfoHeader(fp, &info_header);

  vector<vint8> band_order(3);
  band_order[0] = 2;
  band_order[1] = 1;
  band_order[2] = 0;

  color_image * the_image = (color_image *) vSwitchTopBottom(in_color_image);
  success = the_image->WriteInterlaced(fp, &band_order);

  vdelete(the_image);
  fclose(fp);
  return success;
}


short vWriteGrayBMP(grayscale_image * in_color_image, const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0) return 0;
  WindowsBitmapFileHeader file_header;
  WindowsBitmapInfoHeader info_header;
  short success = S_MakeBitmapFileHeader(in_color_image, &file_header);
  if (success <= 0) return 0;
  success = S_MakeBitmapInfoHeader(in_color_image, &info_header);
  if (success <= 0) return 0;
  success = S_WriteBitmapFileHeader(fp, &file_header);
  success = S_WriteBitmapInfoHeader(fp, &info_header);

  char colormap[1024];
  vint8 i;
  vint8 index = 0;
  for (i = 0; i < 256; i++)
  {
    colormap[index++] = (char) i;
    colormap[index++] = (char) i;
    colormap[index++] = (char) i;
    colormap[index++] = 0;
  }

  vint8 items = fwrite(colormap, sizeof(char), 1024, fp);
  if (items != 1024) return 0;
  grayscale_image * the_image = (grayscale_image *) vSwitchTopBottom(in_color_image);
  success = the_image->WriteBandwise(fp);

  vdelete(the_image);
  fclose(fp);
  return success;
}



// result[row][col] = (row2, col2) iff rotating point (row2, col2)
// by given degrees around center (center_row, center_col) moves
// (row, col) to (row2, col2);
v3dMatrix<short> * vRotationTemplate(vint8 rows, vint8 cols, 
                                        vint8 center_row, vint8 center_col,
                                        double degrees)
{
  const double degrees_to_radians = M_PI / 180.0;
  double radians = degrees * degrees_to_radians;
  return vRotationTemplateR(rows, cols, center_row, center_col, radians);
}

  
// Same as vRotationTemplate, but here the angle is specified in radians.
v3dMatrix<short> * vRotationTemplateR(vint8 rows, vint8 cols, 
                                          vint8 center_row, vint8 center_col, 
                                          double radians)
{
  double trash1 = vPrecomputedSin::PrecomputeValues();
  double trash2 = vPrecomputedSin::PrecomputeValues();
  long trash3 = vPrecomputedLengths::PrecomputeValues((long) rows, (long) cols);
  long trash4 = vPrecomputedAngles2::PrecomputeValues((long) rows, (long) cols);

  if ((rows > 30000) || (cols > 30000))
  {
    return 0;
  }
  v3dMatrix<short> * result_matrix = new v3dMatrix<short>(rows, cols, 2);
  vArray2(short) source_rows = result_matrix->Matrix2(0);
  vArray2(short) source_cols = result_matrix->Matrix2(1);

  vint8 row, col;
  for (row = 0; row < (vint8) rows; row++)
  {
    for (col = 0; col < (vint8) cols; col++)
    {
      vint8 row_diff = row - center_row;
      vint8 col_diff = col - center_col;
      double length = vPrecomputedLengths::Length((long) vAbs(row_diff), (long) vAbs(col_diff));
      double angle = vPrecomputedAngles2::Angle((long) row_diff, (long) col_diff);

      double new_angle = angle - radians;
      if (new_angle < 0) new_angle += 2*M_PI;
      double new_cos = vPrecomputedCos::Cos(new_angle);
      double new_sin = vPrecomputedSin::Sin(new_angle);
      vint8 new_row_diff = round_number(new_sin * length);
      vint8 new_col_diff = round_number(new_cos * length);
      vint8 new_row = center_row + new_row_diff;
      vint8 new_col = center_col + new_col_diff;

      source_rows[row][col] = (short) new_row;
      source_cols[row][col] = (short) new_col;
    }
  }

  return result_matrix;
}


// here we return floats, so that we can do bilinear interpolation.
v3dMatrix<float> * vRotationTemplatef(vint8 rows, vint8 cols, 
                                         vint8 center_row, vint8 center_col,
                                         double degrees)
{
  const double degrees_to_radians = M_PI / 180.0;
  double radians = degrees * degrees_to_radians;
  return vRotationTemplateRf(rows, cols, center_row, center_col, radians);
}

  
v3dMatrix<float> * vRotationTemplateRf(vint8 rows, vint8 cols, 
                                          vint8 center_row, vint8 center_col, 
                                          double radians)
{
  if ((center_row >= rows) || (center_col >= cols))
  {
    return 0;
  }

  static double trash1 = vPrecomputedSin::PrecomputeValues();
  static double trash2 = vPrecomputedSin::PrecomputeValues();
  vint8 trash3 = vPrecomputedLengths::PrecomputeValues((long) rows, (long) cols);
  vint8 trash4 = vPrecomputedAngles2::PrecomputeValues((long) rows, (long) cols);

  if ((rows > 30000) || (cols > 30000))
  {
    return 0;
  }
  v3dMatrix<float> * result_matrix = new v3dMatrix<float>(rows, cols, 2);
  vArray2(float) source_rows = result_matrix->Matrix2(0);
  vArray2(float) source_cols = result_matrix->Matrix2(1);

  vint8 row, col;
  for (row = 0; row < (vint8) rows; row++)
  {
    for (col = 0; col < (vint8) cols; col++)
    {
      vint8 row_diff = row - center_row;
      vint8 col_diff = col - center_col;
      float length = vPrecomputedLengths::Lengthf((long) vAbs(row_diff), (long) vAbs(col_diff));
//      float length = sqrt(row_diff * row_diff + col_diff * col_diff);
      double angle = vPrecomputedAngles2::Angle((long) row_diff, (long) col_diff);

      double new_angle = angle - radians;
      if (new_angle < 0) new_angle += 2*M_PI;
      double new_cos = vPrecomputedCos::Cos(new_angle);
      double new_sin = vPrecomputedSin::Sin(new_angle);
      double new_row_diff = new_sin * length;
      double new_col_diff = new_cos * length;
      double new_row = center_row + new_row_diff;
      double new_col = center_col + new_col_diff;

      source_rows[row][col] = (float) new_row;
      source_cols[row][col] = (float) new_col;
    }
  }

  return result_matrix;
}


// the cropped versions put -1 in entries that fall outside the bounds
// of an image with the specified rows and columns.
v3dMatrix<float> * cropped_rotation_template_degrees(vint8 rows, vint8 cols, 
                                                     vint8 center_row, vint8 center_col,
                                                     double degrees)
{
  float_image * result = vRotationTemplatef(rows, cols, center_row, center_col, degrees);
  crop_warping_template(result);
  return result;
}


v3dMatrix<float> * cropped_rotation_template(vint8 rows, vint8 cols, 
                                             vint8 center_row, vint8 center_col,
                                             double radians)
{
  float_image * result = vRotationTemplateRf(rows, cols, center_row, center_col, radians);
  crop_warping_template(result);
  return result;
}


// this function puts -1 in all entries of warping_template that
// are outside the image bounds of an image of the same size
// as warping template.
vint8 crop_warping_template(float_image * warping_template)
{
  vArray3(float) entries = warping_template->Matrix3();
  matrix_pointer(float) row_entries = entries[0];
  matrix_pointer(float) col_entries = entries[1];

  vint8 rows = warping_template->Rows();
  vint8 cols = warping_template->Cols();
  
  vint8 row, col;
  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      float rowf = row_entries[row][col];
      float colf = col_entries[row][col];

      if ((rowf < 0) || (colf < 0) ||
          (rowf >= rows-1) || (colf >= cols-1))
      {
        row_entries[row][col] = -1;
        col_entries[row][col] = -1;
      }
    }
  }

  return 1;
}

v3dMatrix<uchar> * vRotateImage(v3dMatrix<uchar> * the_image, 
                                   vint8 center_row, vint8 center_col,
                                   double degrees)
{
  const double degrees_to_radians = M_PI / 180.0;
  double radians = degrees * degrees_to_radians;
  return vRotateImageR(the_image, center_row, center_col, radians);
}


v3dMatrix<uchar> * vRotateImageR(v3dMatrix<uchar> * the_image, 
                                    vint8 center_row, vint8 center_col,
                                    double radians)
{
  vint8 rows = the_image->Rows();
  vint8 cols = the_image->Cols();
  vint8 bands = the_image->Bands();
  v3dMatrix<short> * sources = vRotationTemplateR(rows, cols, 
                                                  center_row, center_col, radians);
  if (sources == 0) return 0;
  vArray2(short) source_rows = sources->Matrix2(0);
  vArray2(short) source_cols = sources->Matrix2(1);

  v3dMatrix<uchar> * result = new v3dMatrix<uchar>(rows, cols, bands);
  vArray3(uchar) result3 = result->Matrix3();
  vArray3(uchar) source3 = the_image->Matrix3();
  function_enter_value(result, (uchar) 0);

  vint8 row, col;

  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 source_row = source_rows[row][col];
      vint8 source_col = source_cols[row][col];
      if (!result->check_bounds(source_row, source_col)) continue;

      vint8 band;
      for (band = 0; band < bands; band++)
      {
        result3[band][row][col] = source3[band][source_row][source_col];
      }
    }
  }

  vdelete(sources);
  return result;
}


double vBilinearInterpolation(grayscale_image * gray, double row, double col)
{
    double top_left, top_right, bottom_left, bottom_right;
    vint8 top, bottom, left, right;
    double x, y;

    vint8 rows, cols;
    rows = gray->Rows();
    cols = gray->Cols();

    // Get the bounding square.
    top = (vint8) floor(row);
    left = (vint8) floor(col);
    bottom = top + 1;
    right = left + 1;
    
    // Make sure rectangle is completely inside the image
    if ((top < 0) || (bottom >= rows) || (left < 0) || (right >= cols))
    {
      return 0;
    }

    vArray2(uchar) I = gray->Matrix2(0);;

    // Get values at the corners of the square
    top_left = I[top][left];
    top_right = I[top][right];
    bottom_left = I[bottom][left];
    bottom_right = I[bottom][right];

    x = col - left;
    y = row - top;
    
    double result;
    result = vBilinearInterpolation(x, y, top_left, top_right, 
				     bottom_left, bottom_right);

    return result;
}


double vBilinearInterpolation(const v3dMatrix<double> * the_image, double row, double col)
{
    double top_left, top_right, bottom_left, bottom_right;
    vint8 top, bottom, left, right;
    double x, y;

    vint8 rows, cols;
    rows = the_image->Rows();
    cols = the_image->Cols();

    // Get the bounding square.
    top = (vint8) floor(row);
    left = (vint8) floor(col);
    bottom = top + 1;
    right = left + 1;
    
    // Make sure rectangle is completely inside the image
    if ((top < 0) || (bottom >= rows) || (left < 0) || (right >= cols))
    {
      return 0;
    }

    vArray2(double) I = the_image->Matrix2(0);;

    // Get values at the corners of the square
    top_left = I[top][left];
    top_right = I[top][right];
    bottom_left = I[bottom][left];
    bottom_right = I[bottom][right];

    x = col - left;
    y = row - top;
    
    double result;
    result = vBilinearInterpolation(x, y, top_left, top_right, 
				     bottom_left, bottom_right);

    return result;
}


float vBilinearInterpolation(const v3dMatrix<float> * the_image, float row, float col)
{
    float top_left, top_right, bottom_left, bottom_right;
    vint8 top, bottom, left, right;
    float x, y;

    vint8 rows, cols;
    rows = the_image->Rows();
    cols = the_image->Cols();

    // Get the bounding square.
    top = (vint8) floor(row);
    left = (vint8) floor(col);
    bottom = top + 1;
    right = left + 1;
    
    // Make sure rectangle is completely inside the image
    if ((top < 0) || (bottom >= rows) || (left < 0) || (right >= cols))
    {
      return 0;
    }

    vArray2(float) I = the_image->Matrix2(0);;

    // Get values at the corners of the square
    top_left = I[top][left];
    top_right = I[top][right];
    bottom_left = I[bottom][left];
    bottom_right = I[bottom][right];

    x = col - left;
    y = row - top;
    
    float result;
    result = vBilinearInterpolation(x, y, top_left, top_right, 
				     bottom_left, bottom_right);

    return result;
}

float vBilinearInterpolation(vArray2(float) values, float row, float col)
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


// vQuadPixels returns the pixels (i.e. points with integer coordinates)
// that bevint8 to the quadrilateral specified by the given rows and cols.
// It should also work for a triangle (where we just repeat the last
// vertex twice to make it a quadrilateral).
// We assume that the quadrilateral is convex.
vint8 vQuadPixels(class_pointer(vint8) rows, class_pointer(vint8) cols, 
                  vector<vPoint> * result)
{
  vint8 min_row, max_row, min_col, max_col;
  min_row = max_row = rows[0];
  min_col = max_col = cols[0];
  vint8 i;
  for (i = 1; i < 4; i++)
  {
    if (rows[i] < min_row) min_row = rows[i];
    else if (rows[i] > max_row) max_row = rows[i];
    if (cols[i] < min_col) min_col = cols[i];
    else if (cols[i] > max_col) max_col = cols[i];
  }

  vint8 height = max_row - min_row + 1;
  vint8 width = max_col - min_col + 1;
  v3dMatrix<uchar> scratch_matrix(height, width, 1);
  vArray2(uchar) scratch = scratch_matrix.Matrix2(0);
  function_enter_value(&scratch_matrix, (uchar) 0);
  vDrawLine1(&scratch_matrix, rows[0] - min_row, cols[0] - min_col, 
              rows[1] - min_row, cols[1] - min_col, (uchar) 255);
  vDrawLine1(&scratch_matrix, rows[1] - min_row, cols[1] - min_col, 
              rows[2] - min_row, cols[2] - min_col, (uchar) 255);
  vDrawLine1(&scratch_matrix, rows[2] - min_row, cols[2] - min_col, 
              rows[3] - min_row, cols[3] - min_col, (uchar) 255);
  vDrawLine1(&scratch_matrix, rows[3] - min_row, cols[3] - min_col, 
              rows[0] - min_row, cols[0] - min_col, (uchar) 255);

  vint8 row, col;
  for (row = 0; row < height; row++)
  {
    vint8 left, right;
    for (col = 0; col < width; col++)
    {
      if (scratch[row][col] == 255) 
      { 
        left = col;
        break;
      }
    }

    for (col = width-1; col >= 0; col--)
    {
      if (scratch[row][col] == 255) 
      {
        right = col;
        break;
      }
    }

    for (col = left; col <= right; col++)
    {
      result->push_back(vPoint(row+min_row, col+min_col));
    }
  }

  return 1;
}


// vTrianglePixels returns the pixels (i.e. points with integer coordinates)
// that bevint8 to the triangle specified by the given rows and cols.
//vint8 vTrianglePixels(vint8 * rows, vint8 * cols, 
vint8 vTrianglePixelsSlow(class_pointer(vint8) rows, class_pointer(vint8) cols, 
                          vector<vPoint> * result)
{
  vint8 min_row, max_row, min_col, max_col;
  min_row = max_row = rows[0];
  min_col = max_col = cols[0];
  vint8 i;
  for (i = 1; i < 3; i++)
  {
    if (rows[i] < min_row) min_row = rows[i];
    else if (rows[i] > max_row) max_row = rows[i];
    if (cols[i] < min_col) min_col = cols[i];
    else if (cols[i] > max_col) max_col = cols[i];
  }

  vint8 height = max_row - min_row + 1;
  vint8 width = max_col - min_col + 1;
  v3dMatrix<uchar> scratch_matrix(height, width, 1);
  vArray2(uchar) scratch = scratch_matrix.Matrix2(0);
  function_enter_value(&scratch_matrix, (uchar) 0);
  vDrawLine1(&scratch_matrix, rows[0] - min_row, cols[0] - min_col, 
              rows[1] - min_row, cols[1] - min_col, (uchar) 255);
  vDrawLine1(&scratch_matrix, rows[1] - min_row, cols[1] - min_col, 
              rows[2] - min_row, cols[2] - min_col, (uchar) 255);
  vDrawLine1(&scratch_matrix, rows[2] - min_row, cols[2] - min_col, 
              rows[0] - min_row, cols[0] - min_col, (uchar) 255);

  vint8 row, col;
  for (row = 0; row < height; row++)
  {
    vint8 left, right;
    for (col = 0; col < width; col++)
    {
      if (scratch[row][col] == 255) 
      { 
        left = col;
        break;
      }
    }

    for (col = width-1; col >= 0; col--)
    {
      if (scratch[row][col] == 255) 
      {
        right = col;
        break;
      }
    }

    for (col = left; col <= right; col++)
    {
      result->push_back(vPoint(row+min_row, col+min_col));
    }
  }

  return 1;
}



// vTrianglePixels returns the pixels (i.e. points with integer coordinates)
// that bevint8 to the triangle specified by the given rows and cols.
// We assume that result already has enough capacity to receive all the pixels.
vint8 vTrianglePixels(class_pointer(vint8) in_rows, class_pointer(vint8) in_cols, 
                      vector<vPoint> * result)
{
  static v3dMatrix<vint8> triangle_matrix(10000, 3, 2);
  static vArray2(vint8) triangle_rows = triangle_matrix.Matrix2(0);
  static vArray2(vint8) triangle_cols = triangle_matrix.Matrix2(1);
  static const vint8 max_side = 128;
  static const vint8 precomputed_side = 5;
  static vint8 called_before = 0;
  if (called_before == 0)
  {
    vPrecomputedLengths::PrecomputeValues(500, 500);
    precomputed_line_columns::PrecomputeValues(max_side);
    vPrecomputedTriangles::PrecomputeValues(precomputed_side);
    called_before = 1;
  }
  
  // Figure out if we need to cut the triangle or not.
  vint8 max_value = vAbs(in_rows[0] - in_rows[1]);
  max_value = Max(max_value, vAbs(in_rows[1] - in_rows[2]));
  max_value = Max(max_value, vAbs(in_rows[2] - in_rows[0]));

  max_value = Max(max_value, vAbs(in_cols[0] - in_cols[1]));
  max_value = Max(max_value, vAbs(in_cols[1] - in_cols[2]));
  max_value = Max(max_value, vAbs(in_cols[2] - in_cols[0]));

  vector<vPoint> & pixels = *result;

  if (max_value <= precomputed_side)
  {
    vint8 i1 = in_rows[1] - in_rows[0];
    vint8 j1 = in_cols[1] - in_cols[0];
    vint8 i2 = in_rows[2] - in_rows[0];
    vint8 j2 = in_cols[2] - in_cols[0];

    vArray(vPoint) triangle_pixels = vPrecomputedTriangles::Pixels((long) i1, (long) j1, (long) i2, (long) j2);
    vint8 number = triangle_pixels[0].row;
    vint8 i;
    for (i = 0; i < number; i++)
    {
      vint8 j = i+1;
      pixels[(vector_size) i].row = triangle_pixels[(vector_size) j].row + in_rows[0];
      pixels[(vector_size) i].col = triangle_pixels[(vector_size) j].col + in_cols[0];
    }
    return number;
  }

  vint8 number;
  if (max_value >= max_side)
  {
    number = vCutTriangle(in_rows, in_cols, triangle_rows, 
                           triangle_cols, max_side-3);
  }
  else
  {
    number = 1;
    triangle_rows[0][0] = in_rows[0];
    triangle_rows[0][1] = in_rows[1];
    triangle_rows[0][2] = in_rows[2];

    triangle_cols[0][0] = in_cols[0];
    triangle_cols[0][1] = in_cols[1];
    triangle_cols[0][2] = in_cols[2];
  }

  vint8 counter = 0;
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vArray(vint8) rows = triangle_rows[i];
    vArray(vint8) cols = triangle_cols[i];
    vint8 row0 = rows[0];
    vint8 row1 = rows[1];
    vint8 row2 = rows[2];
  
    vint8 test1 = (row0 < row1);
    vint8 test2 = (row0 < row2);
    vint8 test3 = (row1 < row2);
    vint8 case_number = 4 * test1 + 2 * test2 + test3;
    static vint8 a_row = 0, b_row = 0, c_row = 0;
    static vint8 a_col = 0, b_col = 0, c_col = 0;

    switch(case_number)
    {
    case 0: // 000, row0 >= row1, row0 >= row2, row1 >= row2
            // order row2, row1, row0
      a_row = row2;
      b_row = row1;
      c_row = row0;

      a_col = cols[2];
      b_col = cols[1];
      c_col = cols[0];
      break;

    case 1: // 001, row0 >= row1, row0 >= row2, row1 < row2
            // order row1, row2, row0
      a_row = row1;
      b_row = row2;
      c_row = row0;
    
      a_col = cols[1];
      b_col = cols[2];
      c_col = cols[0];
      break;

    case 2: // 010, row0 >= row1, row0 < row2, row1 >= row2, impossible, 
            // row1 <= row0 < row2 <= row1
      exit_error("Impossible order: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;

    case 3: // 011, row0 >= row1, row0 < row2, row1 < row2
            // order row1, row0, row2
      a_row = row1;
      b_row = row0;
      c_row = row2;

      a_col = cols[1];
      b_col = cols[0];
      c_col = cols[2];
      break;

    case 4: // 100, row0 < row1, row0 >= row2, row1 >= row2
            // order row2, row0, row1
      a_row = row2;
      b_row = row0;
      c_row = row1;

      a_col = cols[2];
      b_col = cols[0];
      c_col = cols[1];
      break;

    case 5: // 101, row0 < row1, row0 >= row2, row1 < row2;
            // impossible: row0 < row1 < row2 <= row0
      exit_error("Impossible order: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;

    case 6: // 110, row0 < row1, row0 < row2, row1 >= row2
            // order row0, row2, row1
      a_row = row0;
      b_row = row2;
      c_row = row1;

      a_col = cols[0];
      b_col = cols[2];
      c_col = cols[1];
      break;

    case 7: // 111, row0 < row1, row0 < row2, row1 < row2
            // order row0, row1, row2
      a_row = row0;
      b_row = row1;
      c_row = row2;

      a_col = cols[0];
      b_col = cols[1];
      c_col = cols[2];
      break;

    default:
      exit_error("Impossible: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;
    }

    vint8 ab_row = b_row - a_row;
    vint8 ac_row = c_row - a_row;
    vint8 bc_row = c_row - b_row;

    vint8 ab_col = b_col - a_col;
    vint8 ac_col = c_col - a_col;
    vint8 bc_col = c_col - b_col;

    // Figure out if ab is to the left or right of ac.
    vint8 ac_left;
    if (ab_row != 0)
    {
      float ab_col2 = ((float) ab_col) * ((float) ac_row) / ((float) ab_row);
      if (ab_col2 < ac_col)
      {
        ac_left = -1;
      }
      else 
      {
        ac_left = 1;
      }
    }
    else
    {
      if (a_col < b_col)
      {
        ac_left = 1;
      }
      else
      {
        ac_left = -1;
      }
    }

    class_pointer(short) ab_cols;
    class_pointer(short) ac_cols;
    class_pointer(short) bc_cols;
    if (ac_left == 1)
    {
      ac_cols = precomputed_line_columns::LeftCols((long) ac_row, (long) ac_col);
      ab_cols = precomputed_line_columns::RightCols((long) ab_row, (long) ab_col);
      bc_cols = precomputed_line_columns::RightCols((long) bc_row, (long) bc_col);
    }
    else
    {
      ac_cols = precomputed_line_columns::RightCols((long) ac_row, (long) ac_col);
      ab_cols = precomputed_line_columns::LeftCols((long) ab_row, (long) ab_col);
      bc_cols = precomputed_line_columns::LeftCols((long) bc_row, (long) bc_col);
    }

    if (ac_row != ab_row + bc_row)
    {
      exit_error("Error in computing line cols: %li %li %li\n",
                       ac_row, ab_row, bc_row);
    }

    vint8 i;
    vint8 row = a_row;
    vint8 col;

    vint8 start_col, end_col;
    // Process common rows of ac and ab.
    for (i = 0; i < ab_row; i++)
    {
      vint8 ac_col = ac_cols[i] + a_col;
      vint8 ab_col = ab_cols[i] + a_col;
      if (ab_col < ac_col)
      {
        start_col = ab_col;
        end_col = ac_col;
      }
      else
      {
        start_col = ac_col;
        end_col = ab_col;
      }

      for (col = start_col; col <= end_col; col++)
      {
        pixels[(vector_size) counter] = vPoint(row, col);
        counter++;
      }
      row++;
    }

    vint8 ac_index = ab_row;
    vint8 bc_limit = bc_row + 1;
    // Process common rows of ac and bc
    for (i = 0; i < bc_limit; i++)
    {
      vint8 ac_col = ac_cols[ac_index] + a_col;
      ac_index++;
      vint8 bc_col = bc_cols[i] + b_col;
      if (bc_col < ac_col)
      {
        start_col = bc_col;
        end_col = ac_col;
      }
      else
      {
        start_col = ac_col;
        end_col = bc_col;
      }
      for (col = start_col; col <= end_col; col++)
      {
        pixels[(vector_size) counter] = vPoint(row, col);
        counter++;
      }
      row++;
    }
  }

  return counter;
}


// vTrianglePixels2 duplicates a lot of the code of vTrianglePixels. The
// main difference is that vTrianglePixels2 does not use 
// vPrecomputedTriangles at all, and therefore it can be used 
// to initialize vPrecomputedTriangles.
vint8 vTrianglePixels2(class_pointer(vint8) in_rows, class_pointer(vint8) in_cols, 
                      vector<vPoint> * result)
{
  static v3dMatrix<vint8> triangle_matrix(10000, 3, 2);
  static vArray2(vint8) triangle_rows = triangle_matrix.Matrix2(0);
  static vArray2(vint8) triangle_cols = triangle_matrix.Matrix2(1);
  static const vint8 max_side = 128;
  static vint8 called_before = 0;
  if (called_before == 0)
  {
    vPrecomputedLengths::PrecomputeValues(500, 500);
    precomputed_line_columns::PrecomputeValues(max_side);
  }
  
  // Figure out if we need to cut the triangle or not.
  vint8 max_value = vAbs(in_rows[0] - in_rows[1]);
  max_value = Max(max_value, vAbs(in_rows[1] - in_rows[2]));
  max_value = Max(max_value, vAbs(in_rows[2] - in_rows[0]));

  max_value = Max(max_value, vAbs(in_cols[0] - in_cols[1]));
  max_value = Max(max_value, vAbs(in_cols[1] - in_cols[2]));
  max_value = Max(max_value, vAbs(in_cols[2] - in_cols[0]));

  vint8 number;

  if (max_value >= max_side)
  {
    number = vCutTriangle(in_rows, in_cols, triangle_rows, 
                           triangle_cols, max_side-3);
  }
  else
  {
    number = 1;
    triangle_rows[0][0] = in_rows[0];
    triangle_rows[0][1] = in_rows[1];
    triangle_rows[0][2] = in_rows[2];

    triangle_cols[0][0] = in_cols[0];
    triangle_cols[0][1] = in_cols[1];
    triangle_cols[0][2] = in_cols[2];
  }

  vint8 counter = 0;
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vArray(vint8) rows = triangle_rows[i];
    vArray(vint8) cols = triangle_cols[i];
    vint8 row0 = rows[0];
    vint8 row1 = rows[1];
    vint8 row2 = rows[2];
  
    vint8 test1 = (row0 < row1);
    vint8 test2 = (row0 < row2);
    vint8 test3 = (row1 < row2);
    vint8 case_number = 4 * test1 + 2 * test2 + test3;
    static vint8 a_row = 0, b_row = 0, c_row = 0;
    static vint8 a_col = 0, b_col = 0, c_col = 0;

    switch(case_number)
    {
    case 0: // 000, row0 >= row1, row0 >= row2, row1 >= row2
            // order row2, row1, row0
      a_row = row2;
      b_row = row1;
      c_row = row0;

      a_col = cols[2];
      b_col = cols[1];
      c_col = cols[0];
      break;

    case 1: // 001, row0 >= row1, row0 >= row2, row1 < row2
            // order row1, row2, row0
      a_row = row1;
      b_row = row2;
      c_row = row0;
    
      a_col = cols[1];
      b_col = cols[2];
      c_col = cols[0];
      break;

    case 2: // 010, row0 >= row1, row0 < row2, row1 >= row2, impossible, 
            // row1 <= row0 < row2 <= row1
      exit_error("Impossible order: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;

    case 3: // 011, row0 >= row1, row0 < row2, row1 < row2
            // order row1, row0, row2
      a_row = row1;
      b_row = row0;
      c_row = row2;

      a_col = cols[1];
      b_col = cols[0];
      c_col = cols[2];
      break;

    case 4: // 100, row0 < row1, row0 >= row2, row1 >= row2
            // order row2, row0, row1
      a_row = row2;
      b_row = row0;
      c_row = row1;

      a_col = cols[2];
      b_col = cols[0];
      c_col = cols[1];
      break;

    case 5: // 101, row0 < row1, row0 >= row2, row1 < row2;
            // impossible: row0 < row1 < row2 <= row0
      exit_error("Impossible order: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;

    case 6: // 110, row0 < row1, row0 < row2, row1 >= row2
            // order row0, row2, row1
      a_row = row0;
      b_row = row2;
      c_row = row1;

      a_col = cols[0];
      b_col = cols[2];
      c_col = cols[1];
      break;

    case 7: // 111, row0 < row1, row0 < row2, row1 < row2
            // order row0, row1, row2
      a_row = row0;
      b_row = row1;
      c_row = row2;

      a_col = cols[0];
      b_col = cols[1];
      c_col = cols[2];
      break;

    default:
      exit_error("Impossible: case %li for %li %li %li\n", 
                      case_number, row0, row1, row2);
      break;
    }

    vint8 ab_row = b_row - a_row;
    vint8 ac_row = c_row - a_row;
    vint8 bc_row = c_row - b_row;

    vint8 ab_col = b_col - a_col;
    vint8 ac_col = c_col - a_col;
    vint8 bc_col = c_col - b_col;

    // Figure out if ab is to the left or right of ac.
    vint8 ac_left;
    if (ab_row != 0)
    {
      float ab_col2 = ((float) ab_col) * ((float) ac_row) / ((float) ab_row);
      if (ab_col2 < ac_col)
      {
        ac_left = -1;
      }
      else 
      {
        ac_left = 1;
      }
    }
    else
    {
      if (a_col < b_col)
      {
        ac_left = 1;
      }
      else
      {
        ac_left = -1;
      }
    }

    class_pointer(short) ab_cols;
    class_pointer(short) ac_cols;
    class_pointer(short) bc_cols;
    if (ac_left == 1)
    {
      ac_cols = precomputed_line_columns::LeftCols((long) ac_row, (long) ac_col);
      ab_cols = precomputed_line_columns::RightCols((long) ab_row, (long) ab_col);
      bc_cols = precomputed_line_columns::RightCols((long) bc_row, (long) bc_col);
    }
    else
    {
      ac_cols = precomputed_line_columns::RightCols((long) ac_row, (long) ac_col);
      ab_cols = precomputed_line_columns::LeftCols((long) ab_row, (long) ab_col);
      bc_cols = precomputed_line_columns::LeftCols((long) bc_row, (long) bc_col);
    }

    if (ac_row != ab_row + bc_row)
    {
      exit_error("Error in computing line cols: %li %li %li\n",
                       ac_row, ab_row, bc_row);
    }

    vint8 i;
    vint8 row = a_row;
    vint8 col;

    vector<vPoint> & pixels = *result;
    vint8 start_col, end_col;
    // Process common rows of ac and ab.
    for (i = 0; i < ab_row; i++)
    {
      vint8 ac_col = ac_cols[i] + a_col;
      vint8 ab_col = ab_cols[i] + a_col;
      if (ab_col < ac_col)
      {
        start_col = ab_col;
        end_col = ac_col;
      }
      else
      {
        start_col = ac_col;
        end_col = ab_col;
      }

      for (col = start_col; col <= end_col; col++)
      {
        pixels[(vector_size) counter] = vPoint(row, col);
        counter++;
      }
      row++;
    }

    vint8 ac_index = ab_row;
    vint8 bc_limit = bc_row + 1;
    // Process common rows of ac and bc
    for (i = 0; i < bc_limit; i++)
    {
      vint8 ac_col = ac_cols[ac_index] + a_col;
      ac_index++;
      vint8 bc_col = bc_cols[i] + b_col;
      if (bc_col < ac_col)
      {
        start_col = bc_col;
        end_col = ac_col;
      }
      else
      {
        start_col = ac_col;
        end_col = bc_col;
      }
      for (col = start_col; col <= end_col; col++)
      {
        pixels[(vector_size) counter] = vPoint(row, col);
        counter++;
      }
      row++;
    }
  }

  return counter;
}


// It is assumed that vPrecomputedLenghts::PrecomputeValues
// has already been called with an appropriate argument to
// cover the lengths that will be encountered here.
vint8 vCutTriangle(class_pointer(vint8) rows, class_pointer(vint8) cols, matrix_pointer(vint8) result_rows, 
                   matrix_pointer(vint8) result_cols, vint8 max_length)
{
  vint8 a_row = rows[0];
  vint8 b_row = rows[1];
  vint8 c_row = rows[2];

  vint8 a_col = cols[0];
  vint8 b_col = cols[1];
  vint8 c_col = cols[2];

  vint8 ab_row = a_row - b_row;
  vint8 ac_row = a_row - c_row;
  vint8 bc_row = b_row - c_row;

  vint8 pab_row = vAbs(ab_row);
  vint8 pac_row = vAbs(ac_row);
  vint8 pbc_row = vAbs(bc_row);

  vint8 ab_col = a_col - b_col;
  vint8 ac_col = a_col - c_col;
  vint8 bc_col = b_col - c_col;

  vint8 pab_col = vAbs(ab_col);
  vint8 pac_col = vAbs(ac_col);
  vint8 pbc_col = vAbs(bc_col);

  vint8 ab_length = vPrecomputedLengths::Length((long) pab_row, (long) pab_col);
  vint8 ac_length = vPrecomputedLengths::Length((long) pac_row, (long) pac_col);
  vint8 bc_length = vPrecomputedLengths::Length((long) pbc_row, (long) pbc_col);

  vint8 max_side = Max(ab_length, Max(ac_length, bc_length));
  vint8 factor = (max_side + max_length - 1) / max_length;

  vint8 i, j, counter;
  counter = 0;
  float ab_row_step = ((float) ab_row) / (float) factor;
  float bc_row_step = ((float) bc_row) / (float) factor;
  float ab_col_step = ((float) ab_col) / (float) factor;
  float bc_col_step = ((float) bc_col) / (float) factor;

  for (i = 0; i < factor; i++)
  {
    float di = (float) i;
    float start_a_row = a_row - di * ab_row_step;
    float start_b_row = start_a_row - ab_row_step;
    float start_c_row = start_b_row - bc_row_step;

    float start_a_col = a_col - di * ab_col_step;
    float start_b_col = start_a_col - ab_col_step;
    float start_c_col = start_b_col - bc_col_step;

    for (j = 0; j <= i; j++)
    {
      float dj = (float) j;
      float temp_a_row = start_a_row - dj * bc_row_step;
      float temp_b_row = start_b_row - dj * bc_row_step;
      float temp_c_row = start_c_row - dj * bc_row_step;

      result_rows[counter][0] = round_number(temp_a_row);
      result_rows[counter][1] = round_number(temp_b_row);
      result_rows[counter][2] = round_number(temp_c_row);

      float temp_a_col = start_a_col - dj * bc_col_step;
      float temp_b_col = start_b_col - dj * bc_col_step;
      float temp_c_col = start_c_col - dj * bc_col_step;

      result_cols[counter][0] = round_number(temp_a_col);
      result_cols[counter][1] = round_number(temp_b_col);
      result_cols[counter][2] = round_number(temp_c_col);

      counter++;
    }

    for (j = 0; j < i; j++)
    {
      float dj = (float) j;
      float temp_a_row = start_a_row - dj * bc_row_step;
      float temp_b_row = temp_a_row - bc_row_step;
      float temp_c_row = start_c_row - dj * bc_row_step;

      result_rows[counter][0] = round_number(temp_a_row);
      result_rows[counter][1] = round_number(temp_b_row);
      result_rows[counter][2] = round_number(temp_c_row);

      float temp_a_col = start_a_col - dj * bc_col_step;
      float temp_b_col = temp_a_col - bc_col_step;
      float temp_c_col = start_c_col - dj * bc_col_step;

      result_cols[counter][0] = round_number(temp_a_col);
      result_cols[counter][1] = round_number(temp_b_col);
      result_cols[counter][2] = round_number(temp_c_col);

      counter++;
    }
  }

  return counter;
}



vint8 vCutTriangle3D(class_pointer(double) xs, class_pointer(double) ys, class_pointer(double) zs, 
                    matrix_pointer(double) result_xs, matrix_pointer(double)  result_ys, 
                    matrix_pointer(double) result_zs, double max_length)
{
  double a_x = xs[0];
  double b_x = xs[1];
  double c_x = xs[2];

  double a_y = ys[0];
  double b_y = ys[1];
  double c_y = ys[2];

  double a_z = zs[0];
  double b_z = zs[1];
  double c_z = zs[2];

  double ab_x = a_x - b_x;
  double ac_x = a_x - c_x;
  double bc_x = b_x - c_x;

  double ab_y = a_y - b_y;
  double ac_y = a_y - c_y;
  double bc_y = b_y - c_y;

  double ab_z = a_z - b_z;
  double ac_z = a_z - c_z;
  double bc_z = b_z - c_z;

  double ab_length = sqrt(ab_x * ab_x + ab_y * ab_y + ab_z * ab_z);
  double ac_length = sqrt(ac_x * ac_x + ac_y * ac_y + ac_z * ac_z);
  double bc_length = sqrt(bc_x * bc_x + bc_y * bc_y + bc_z * bc_z);

  double max_side = Max(ab_length, Max(ac_length, bc_length));
  double factor = ceil(max_side / max_length);

  vint8 i, j, counter;
  counter = 0;
  double ab_x_step = ab_x /  factor;
  double bc_x_step = bc_x /  factor;
  double ab_y_step = ab_y /  factor;
  double bc_y_step = bc_y /  factor;
  double ab_z_step = ab_z /  factor;
  double bc_z_step = bc_z /  factor;

  for (i = 0; i < factor; i++)
  {
    double di = (double) i;
    double start_a_x = a_x - di * ab_x_step;
    double start_b_x = start_a_x - ab_x_step;
    double start_c_x = start_b_x - bc_x_step;

    double start_a_y = a_y - di * ab_y_step;
    double start_b_y = start_a_y - ab_y_step;
    double start_c_y = start_b_y - bc_y_step;

    double start_a_z = a_z - di * ab_z_step;
    double start_b_z = start_a_z - ab_z_step;
    double start_c_z = start_b_z - bc_z_step;

    for (j = 0; j <= i; j++)
    {
      double dj = (double) j;
      double temp_a_x = start_a_x - dj * bc_x_step;
      double temp_b_x = start_b_x - dj * bc_x_step;
      double temp_c_x = start_c_x - dj * bc_x_step;

      result_xs[counter][0] = (double) round_number(temp_a_x);
      result_xs[counter][1] = (double) round_number(temp_b_x);
      result_xs[counter][2] = (double) round_number(temp_c_x);

      double temp_a_y = start_a_y - dj * bc_y_step;
      double temp_b_y = start_b_y - dj * bc_y_step;
      double temp_c_y = start_c_y - dj * bc_y_step;

      result_ys[counter][0] = (double) round_number(temp_a_y);
      result_ys[counter][1] = (double) round_number(temp_b_y);
      result_ys[counter][2] = (double) round_number(temp_c_y);

      double temp_a_z = start_a_z - dj * bc_z_step;
      double temp_b_z = start_b_z - dj * bc_z_step;
      double temp_c_z = start_c_z - dj * bc_z_step;

      result_zs[counter][0] = (double) round_number(temp_a_z);
      result_zs[counter][1] = (double) round_number(temp_b_z);
      result_zs[counter][2] = (double) round_number(temp_c_z);

      counter++;
    }

    for (j = 0; j < i; j++)
    {
      double dj = (double) j;
      double temp_a_x = start_a_x - dj * bc_x_step;
      double temp_b_x = temp_a_x - bc_x_step;
      double temp_c_x = start_c_x - dj * bc_x_step;

      result_xs[counter][0] = (double) round_number(temp_a_x);
      result_xs[counter][1] = (double) round_number(temp_b_x);
      result_xs[counter][2] = (double) round_number(temp_c_x);

      double temp_a_y = start_a_y - dj * bc_y_step;
      double temp_b_y = temp_a_y - bc_y_step;
      double temp_c_y = start_c_y - dj * bc_y_step;

      result_ys[counter][0] = (double) round_number(temp_a_y);
      result_ys[counter][1] = (double) round_number(temp_b_y);
      result_ys[counter][2] = (double) round_number(temp_c_y);

      double temp_a_z = start_a_z - dj * bc_z_step;
      double temp_b_z = temp_a_z - bc_z_step;
      double temp_c_z = start_c_z - dj * bc_z_step;

      result_zs[counter][0] = (double) round_number(temp_a_z);
      result_zs[counter][1] = (double) round_number(temp_b_z);
      result_zs[counter][2] = (double) round_number(temp_c_z);

      counter++;
    }
  }

  return counter;
}




vint8 vDrawTriangles1(vint8 number, matrix_pointer(vint8) rows, matrix_pointer(vint8) cols, 
                      color_image * the_image, uchar i)
{
  return vDrawTriangles3(number, rows, cols, the_image, i, i, i);
}


vint8 vDrawTriangles3(vint8 number, matrix_pointer(vint8) rows, matrix_pointer(vint8) cols, 
                      color_image * the_image, uchar r, uchar g, uchar b)
{
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vDrawTriangle3(rows[i], cols[i], the_image, r, g, b);
  }

  return 1;
}


vint8 vDrawTriangle1(class_pointer (vint8) rows, class_pointer (vint8) cols, color_image * the_image, uchar i)
{
  return vDrawTriangle3(rows, cols, the_image, i, i, i);
}


vint8 vDrawTriangle3(class_pointer (vint8) rows, class_pointer (vint8) cols, color_image * the_image, 
                     uchar r, uchar g, uchar b)
{
  vector<uchar> values(3);
  values[0] = r;
  values[1] = g;
  values[2] = b;
  vint8 i, j;
  for (i = 0; i < 3; i++)
  {
    j = (i + 1) % 3;
    vDrawLine(the_image, rows[i], cols[i], rows[j], cols[j], &values);
  }

  return 1;
}


// Get the integer coordinates of points on the line with
// specified center (row, col) and slope (angle) and length.
// Those points are stored in rows and cols. The increment
// between consecutive points in rows and cols is 1 in either
// the row or the col dimension, and not greater than 1 in
// the other dimension.
vint8 vLinePoints(vint8 row, vint8 col, double angle, double length,
                  vector<vint8> * rows, vector<vint8> * cols)
{
  double row1, col1, row2, col2, row_step, col_step;
  vint8 steps = 0;
  row1 = col1 = row2 = col2 = row_step = col_step = 0;
  // Get the line endpoints, and the increments.
  vLineEnds((double) row, (double) col, angle, length, &row1, 
             &col1, &row2, &col2, &row_step, &col_step, &steps);

  // Store the line points.
  vint8 i;
  for (i = 0; i <= steps; i++)
  {
    vint8 current_row = round_number(row1 + row_step * (double) i);
    vint8 current_col = round_number(col1 + col_step * (double) i);
    rows->push_back(current_row);
    cols->push_back(current_col);
  }
  return 1;
}


// inputs:
// row, col: center of the line segment
// angle, length: they specify the angle and length of the segment
// step: it specifies the maximum difference in either the rows or
// the cols direction between consecutive line points stored in the result.
// outputs:
// rows, cols: coordinates of line points sampled uniformly from that 
// segment, at the density specified by step.
// The main differences between vLinePoints2 and vLinePoints is that
// here we can specify the maximum increment (so that it does not
// have to be 1, which is its value at vLinePoints), and that the 
// coordinates are saved as doubles, and not as integers.
vint8 vLinePoints2(double row, double col, double angle, double length, double step,
                   vector<double> * rows, vector<double> * cols)
{
  double row1, col1, row2, col2, row_step, col_step;
  vint8 steps = 0;
  row1 = col1 = row2 = col2 = row_step = col_step = 0;

  // Get the segment endpoints, the row and col increments
  // to get from one line point to the next, and the steps
  // that we need to take to go from one endpoint to the 
  // other using those increments.
  vLineEnds(row, col, angle, length, &row1, &col1, 
             &row2, &col2, &row_step, &col_step, &steps);

  // Now start at one endpoint, and move avint8 the line segment
  // till we get to the other endpoint. Note that we move
  // in increments that are specified not only by row_step
  // and col_step, but also by step.
  double i;
  for (i = 0; i <= steps + step/1000; i += step)
  {
    double current_row = row1 + row_step * (double) i;
    double current_col = col1 + col_step * (double) i;
    rows->push_back(current_row);
    cols->push_back(current_col);
  }
  return 1;
}


vint8 vLinePoints5(vint8 y0, vint8 x0, vint8 y1, vint8 x1,
                  vector<vPoint> * points)
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
    points->push_back(vPoint(y0, x0));
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
    int_row = round_number(row);
    int_col = round_number(col);
    points->push_back(vPoint(int_row, int_col));
  }

  return points->size();
}


vint8 vLinePoints5(vint8 y0, vint8 x0, vint8 y1, vint8 x1,
                  vector<class_pixel> * points)
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
    points->push_back(class_pixel(y0, x0));
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
    int_row = round_number(row);
    int_col = round_number(col);
    points->push_back(class_pixel(int_row, int_col));
  }

  return points->size();
}


// For the line connecting (0, 0) to (row, col), return
// the leftmost pixel col at each row, from 0 to row.
vArray(short) vLeftCols(vint8 row, vint8 col)
{
  vint8 abs_row = vAbs(row);
  vArray(short) result = vnew(short, (long) (abs_row+1));
  vector<vPoint> line_pixels;
  vLinePoints5(0, 0, row, col, &line_pixels);
  
  vint8 i;
  vint8 max_value = 2 * (vAbs(col) + 10);
  for (i = 0; i <= abs_row; i++)
  {
    result[i] = (short) max_value;
  }

  vint8 number = line_pixels.size();
  for (i = 0; i < number; i++)
  {
    vPoint & p = line_pixels[(vector_size) i];
    vint8 row = vAbs(p.row);
    vint8 col = p.col;
    if (col < result[row]) 
    {
      result[row] = (short) col;
    }
  }

  // For debugging
  for (i = 0; i <= abs_row; i++)
  {
    if (result[i] == max_value)
    {
      exit_error("Bad result in vLeftCols(%li, %li)\n", row, col);
    }
  }

  return result;
}


// For the line connecting (0, 0) to (row, col), return
// the righttmost pixel col at each row, from 0 to row.
vArray(short) vRightCols(vint8 row, vint8 col)
{
  vint8 abs_row = vAbs(row);
  vArray(short) result = vnew(short, (long) (abs_row+1));
  vector<vPoint> line_pixels;
  vLinePoints5(0, 0, row, col, &line_pixels);
  
  vint8 i;
  vint8 min_value = -2 * (vAbs(col) + 10);
  for (i = 0; i <= abs_row; i++)
  {
    result[i] = (short) min_value;
  }

  vint8 number = line_pixels.size();
  for (i = 0; i < number; i++)
  {
    vPoint & p = line_pixels[(vector_size) i];
    vint8 row = vAbs(p.row);
    vint8 col = p.col;
    if (col > result[row]) 
    {
      result[row] = (short) col;
    }
  }

  // For debugging
  for (i = 0; i <= abs_row; i++)
  {
    if (result[i] == min_value)
    {
      exit_error("Bad result in vRightCols(%li, %li)\n", row, col);
    }
  }

  return result;
}




grayscale_image * function_histogram_equalization (ushort_image* input_image, vint8 window_size)
{
  if (window_size % 2 != 1)
  {
    function_print("\nthe window size should be odd\n");
    return 0;
  }

  vint8 vertical_size = input_image->vertical ();
  vint8 horizontal_size = input_image->horizontal();
  grayscale_image* result_image = new grayscale_image (vertical_size, horizontal_size);

  vint8 half_size = window_size / 2;
  grayscale_image* top_left_image = function_histogram_equalization_offset(input_image, window_size, 0, 0);
  grayscale_image* top_right_image = function_histogram_equalization_offset(input_image, window_size, 0, half_size);
  grayscale_image* bottom_left_image = function_histogram_equalization_offset(input_image, window_size, half_size, 0);
  grayscale_image* bottom_right_image = function_histogram_equalization_offset(input_image, window_size, half_size, half_size);

  float_image* weights_image = histogram_equalization_weights(window_size);

  matrix_pointer(uchar) result = result_image->Matrix2(0);
  matrix_pointer(uchar) top_left = top_left_image->Matrix2(0);
  matrix_pointer(uchar) top_right = top_right_image->Matrix2(0);
  matrix_pointer(uchar) bottom_left = bottom_left_image->Matrix2(0);
  matrix_pointer(uchar) bottom_right = bottom_right_image->Matrix2(0);
  vArray3(float) weights = weights_image->Matrix3();

  vint8 vertical, horizontal;
  for (vertical = 0; vertical < vertical_size; vertical++)
  {
    for (horizontal = 0; horizontal < horizontal_size; horizontal++)
    {
      float entry1 = top_left[vertical][horizontal];
      float entry2 = top_right[vertical][horizontal];
      float entry3 = bottom_left[vertical][horizontal];
      float entry4 = bottom_right[vertical][horizontal];

      vint8 vertical2 = vertical % window_size;
      vint8 horizontal2 = horizontal % window_size;
      float weight1 = weights[0][vertical2][horizontal2];
      float weight2 = weights[1][vertical2][horizontal2];
      float weight3 = weights[2][vertical2][horizontal2];
      float weight4 = weights[3][vertical2][horizontal2];

      float average = entry1 * weight1 + entry2 * weight2 + entry3 * weight3 + entry4 * weight4;
      result[vertical][horizontal] = (uchar) round_number(average);
//      result[vertical][horizontal] = entry1;
    }
  }

  function_delete(top_left_image);
  function_delete(top_right_image);
  function_delete(bottom_left_image);
  function_delete(bottom_right_image);
  function_delete(weights_image);

  return result_image;
}


grayscale_image * function_histogram_equalization_offset (ushort_image* input_image, vint8 window_size,
                                                 vint8 vertical_offset, vint8 horizontal_offset)
{
  if ((vertical_offset < 0) || (vertical_offset >= window_size) ||
      (horizontal_offset < 0) || (horizontal_offset >= window_size))
  {
    function_print ("\nhorizontal and vertical offset should be in range [0, window_size]\n");
    return 0;
  }

  if (window_size % 2 != 1)
  {
    function_print("\nthe window size should be odd\n");
    return 0;
  }

  vint8 vertical_size = input_image->vertical ();
  vint8 horizontal_size = input_image->horizontal();
  grayscale_image* result_image = new grayscale_image (vertical_size, horizontal_size);
  vint8_matrix counters_matrix (1, 65536);
  function_enter_value (& counters_matrix, (vint8) 0);

  class_pointer(vint8) counters = counters_matrix.Matrix();
  matrix_pointer (ushort) input = input_image->Matrix2(0);
  matrix_pointer (uchar) result = result_image->Matrix2(0);

  vint8 vertical_top, horizontal_top, vertical, horizontal;
  for (vertical_top = -window_size+ vertical_offset; vertical_top < vertical_size; vertical_top += window_size)
  {
    for (horizontal_top = -window_size+ horizontal_offset; horizontal_top < horizontal_size; horizontal_top += window_size)
    {
      vint8 vertical_start = function_maximum ((vint8) 0, vertical_top);
      vint8 horizontal_start = function_maximum ((vint8) 0, horizontal_top);
      vint8 vertical_limit = Min(vertical_top + window_size, vertical_size);
      vint8 horizontal_limit = Min(horizontal_top + window_size, horizontal_size);

      // count the occurrences of each intensity value
      function_enter_value (& counters_matrix, (vint8) 0);
      for (vertical = vertical_start; vertical < vertical_limit; vertical ++)
      {
        for (horizontal = horizontal_start; horizontal < horizontal_limit; horizontal ++)
        {
          vint8 entry = input[vertical] [horizontal];
          counters[entry] ++;
        }
      }

      // accumulate the occurrences
      vint8 counter;
      for (counter = 1; counter < counters_matrix.Size(); counter ++)
      {
        counters [counter] += counters [counter -1];
      }

      // normalize so that all values are in the [0, 255] range
      vint8 maximum = counters [65535];
      if (maximum != 0)
      {
        for (counter = 1; counter < counters_matrix.Size(); counter ++)
        {
          vint8 entry = counters [counter];
          counters [counter] = entry*255/maximum;
        }
      }

      // store intensity values in the result image
      for (vertical = vertical_start; vertical < vertical_limit; vertical ++)
      {
        for (horizontal = horizontal_start; horizontal < horizontal_limit; horizontal ++)
        {
          vint8 entry = input[vertical] [horizontal];
          uchar converted = (uchar) counters[entry];
          result [vertical] [horizontal] = converted;
        }
      }
    }
  }

  return result_image;
}


float_image * histogram_equalization_weights(vint8 window_size)
{
  float_image * result_image = new float_image(window_size, window_size, 4);
  vArray3(float) result = result_image->Matrix3();
  vint8 half_size = window_size / 2;

  vint8 vertical, horizontal;
  for (vertical = 0; vertical < window_size; vertical++)
  {
    for (horizontal = 0; horizontal < window_size; horizontal++)
    {
      float y = (float) vAbs(vertical - half_size) / (float) half_size;
      float x = (float) vAbs(horizontal - half_size) / (float) half_size;

      result[0][vertical][horizontal] = (1.0f - x) * (1.0f - y);
      result[1][vertical][horizontal] = x * (1.0f - y);
      result[2][vertical][horizontal] = (1.0f - x) * y;
      result[3][vertical][horizontal] = x * y;
    }
  }

  return result_image;
}


void line_endpoints(vint8 center_row, vint8 center_col, 
                    float orientation, vint8 length,
                    vint8 * row1, vint8 * col1, 
                    vint8 * row2, vint8 * col2)
{
  static const double junk1 = vPrecomputedSin::PrecomputeValues();
  static const double junk2 = vPrecomputedCos::PrecomputeValues();

  float dx = (float) vPrecomputedCos::Cos(orientation);
  float dy = (float) vPrecomputedSin::Sin(orientation);

  float half_length = (float) ((length-1)/2);
  float end_x = half_length * dx;
  float end_y = half_length * dy;
  *row1 = round_number(center_row - end_y);
  *col1 = round_number(center_col - end_x);
  *row2 = round_number(center_row + end_y);
  *col2 = round_number(center_col + end_x);
}

