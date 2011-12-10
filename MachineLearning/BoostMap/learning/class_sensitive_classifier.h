#pragma once

/*!
	Class class_sensitive_classifier is a composite classifier, made of
	two parts: a splitter, which determines if the classifier should
	be applied to a given query or not (we output 0 if the splitter
	does not "accept" the query, where 0 is between -1 and 1, and 
	therefore it doesn't give any information), and a classifier
	(which maybe I should rename to decider or something like that) 
	which classifies the query, if the splitter accepts the query.
	Used in the BoostMap framework, sensitive classifiers are
	building blocks for query-sensitive embeddings, i.e. embeddings
	where the distance, which is used to find nearest neighbors of the
	query in the Euclidean space, is a query-sensitive distance, i.e.
	a weighted L1 distance whose weights depend on the query and
	are probably different for each query.
*/
class class_sensitive_classifier
{
public:
	// splitter decides whether to accept the query or not.
	class_triple_classifier splitter;

	// classifier classifies the query, if it is accepted by the
	// splitter.
	class_triple_classifier classifier;

	// types:
	// 0 if the splitter accepts values >= low
	// 1 if the splitter accepts values < high
	// 2 if the splitter accepts low <= values < high
	// 3 if the splitter accepts values < low OR >= high.
	// 4 if the splitter accepts everything.
	vint8 split_type;
	float low;
	float high;

	// for debugging, see how many training objects (or training triples,
	// depending on the implementation) were selected
	// by the splitter.
	vint8 range;

	// in the BoostMap framework, we have a list of classifiers we have 
	// already chosen (actually, we have two lists: classifiers, which 
	// allows repetitions, and unique_classifiers, which is a compact
	// representation of classifiers that does not have repetitions).
	// splitter_index and classifier_index are indices into unique_classifiers.
	vint8 splitter_index;
	vint8 classifier_index;

	/*!
	This factor records the distance_max_entry variable
	of the BoostMap object where this classifier is created.
	Not recording this factor caused a bug that it took me
	over 18 months to find. When, in class_BoostMap, we
	figure out the area of influence of the classifier, that
	area of influence is defined based on normalized distances,
	that have been divided by distance_max_entry. When we
	apply the classifier, for online retrieval, we have
	unnormalized distances, so we need to adjust the area
	of influence.
	*/
	float distance_factor;

	float distance_multiplier;

public:
	class_sensitive_classifier()
	{
		split_type = -1;
		low = (float) 0;
		high = (float) 0;
		distance_factor = -1;
		distance_multiplier = 1;
	}

	// decides if the splitter accepts this value (in which case
	// we return 1) or not (we return 0).
	vint8 Split(float value);

	vint8 Print();
};
