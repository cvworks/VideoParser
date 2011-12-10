#include <Tools/MathUtils.h>
#include "io.h"
#include "genrl.h"



void float2rgb(float& value,float& R,float& G,float& B)	//simple color-coding routine
{
   const float dx=0.8f;

   value = (6-2*dx)*value+dx;
   R = max(0.0f,(3-(float)fabs(value-4)-(float)fabs(value-5))/2);
   G = max(0.0f,(4-(float)fabs(value-2)-(float)fabs(value-4))/2);
   B = max(0.0f,(3-(float)fabs(value-1)-(float)fabs(value-2))/2);
}

		
