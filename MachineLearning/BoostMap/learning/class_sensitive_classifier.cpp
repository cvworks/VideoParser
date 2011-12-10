#include "boost_map.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"
#include "basics/definitions.h"

#include "class_sensitive_classifier.h"

// decides if the splitter accepts this value (in which case
// we return 1) or not (we return 0).
vint8 class_sensitive_classifier::Split(float normalized_value)
{
	vint8 result = 0;
	switch(split_type)
	{
	case 0:
		if (normalized_value >= low)
		{
			result = 1;
		}
		break;

	case 1:
		if (normalized_value < high)
		{
			result = 1;
		}
		break;

	case 2:
		if ((low <= normalized_value) && (normalized_value < high))
		{
			result = 1;
		}
		break;

	case 3:
		if ((normalized_value < low) || (high <= normalized_value))
		{
			result = 1;
		}
		break;

	case 4:
		result = 1;
		break;

	default:
		exit_error("error: split, impossible type = %li\n",
			(long) split_type);
		break;
	}

	return result;
}


vint8 class_sensitive_classifier::Print()
{
	vPrint("class_sensitive_classifier:\n");
	vPrint("splitter: ");
	splitter.Print();
	vPrint("classifier: ");
	classifier.Print();
	vPrint("type = %li, low = %f, high = %f, range = %li\n", 
		(long) split_type, low * distance_factor, high * distance_factor, (long) range);

	return 1;
}
