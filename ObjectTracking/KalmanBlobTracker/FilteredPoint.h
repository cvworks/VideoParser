/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/cv.h>
#include <Tools/CvUtils.h>
#include <Tools/BasicTypes.h>

namespace vpl {

class FilteredPoint
{
public:
    FilteredPoint();
    void Initialize(double time_step, XYCoord init_position, XYCoord init_velocity);
    void Next_Measurement(XYCoord & measurement) const;
    XYCoord State(const unsigned & index) const;
    XYCoord Predicted_state(const unsigned & index) const;
    void Evolve();

    CvKalman* x;
    CvKalman* y;
};

} // namespace vpl


