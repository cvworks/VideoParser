/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>

#include "FilteredPoint.h"

using namespace vpl;

FilteredPoint::FilteredPoint()
{
}

void FilteredPoint::Initialize(double time_step, XYCoord init_position, XYCoord init_velocity)
{

	//For the x component...

	//Initialize the parameters of the Kalman Filter
	cvSetIdentity(x->transition_matrix, cvRealScalar(1));
	cvmSet(x->transition_matrix,0,1,time_step);

	cvSetIdentity(x->measurement_matrix, cvRealScalar(1));

	cvSetIdentity(x->process_noise_cov, cvRealScalar(1E-3));
	cvSetIdentity(x->measurement_noise_cov, cvRealScalar(1E-2));


	//Set the initial conditions
	cvSetIdentity(x->state_post, cvRealScalar(0));
	cvmSet(x->state_post,0,0,init_position.x);
	cvmSet(x->state_post,1,0,init_velocity.x);

	cvSetIdentity(x->error_cov_post, cvRealScalar(1));


	//For the y component...

	//Initialize the parameters of the Kalman Filter
	cvSetIdentity(y->transition_matrix, cvRealScalar(1));
	cvmSet(y->transition_matrix,0,1,time_step);

	cvSetIdentity(y->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(y->process_noise_cov, cvRealScalar(1E-3));
	cvSetIdentity(y->measurement_noise_cov, cvRealScalar(1E-2));


	//Set the initial conditions
	cvSetIdentity(y->state_post, cvRealScalar(0));
	cvmSet(y->state_post,0,0,init_position.y);
	cvmSet(y->state_post,1,0,init_velocity.y);

	cvSetIdentity(y->error_cov_post, cvRealScalar(1));
}

XYCoord FilteredPoint::State(const unsigned & index) const
{
	XYCoord p;
	p.x = (int)cvmGet(x->state_post,index,0);
	p.y = (int)cvmGet(y->state_post,index,0);
	return p;
}

XYCoord FilteredPoint::Predicted_state(const unsigned & index) const
{
	XYCoord p;
	p.x = (int)cvmGet(x->state_pre,index,0);
	p.y = (int)cvmGet(y->state_pre,index,0);
	return p;
}

void FilteredPoint::Next_Measurement(XYCoord & measurement) const
{
	CvMat* z = cvCreateMat(1,1,CV_32FC1);

	//For the x component...
	cvmSet(z,0,0,measurement.x);
	cvKalmanPredict(x, NULL);
	cvKalmanCorrect(x, z);

	//For the y component...
	cvmSet(z,0,0,measurement.y);
	cvKalmanPredict(y, NULL);
	cvKalmanCorrect(y, z);

	cvReleaseMat(&z);
}

void FilteredPoint::Evolve()
{
	CvMat* z = cvCreateMat(1,1,CV_32FC1);


	//For the x component...
	cvKalmanPredict(x, NULL);
	cvmSet(z,0,0,cvmGet(x->state_pre,0,0));
	cvKalmanCorrect(x, z);

	//For the y component...
	cvKalmanPredict(y, NULL);
	cvmSet(z,0,0,cvmGet(y->state_pre,0,0));
	cvKalmanCorrect(y, z);

	cvReleaseMat(&z);


}
