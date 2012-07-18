/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Chris Whiten
 *-----------------------------------------------------------------------*/

#include "GAPhenotype.h"

GAPhenotype::GAPhenotype()
{
	length = 0;
	fitness = 0;
}
GAPhenotype::GAPhenotype(vector<unsigned int> a, unsigned int l)
{
	alleles = a;
	length = l;
	fitness = 0;

	init();
}

void GAPhenotype::init()
{
	// random initialization.
	for (unsigned i = 0; i < length; ++i)
	{
		chromosome.push_back(randomAllele());
	}
}

unsigned int GAPhenotype::randomAllele()
{
	unsigned int index = floor((double) (rand() % alleles.size()));
	unsigned int allele =  alleles[index];
	return allele;
}

string GAPhenotype::toString()
{
	string ret = "";
	for (unsigned i = 0; i < length; ++i)
	{
		ret += chromosome[i];
	}
	return ret;
}

void GAPhenotype::mutate(double mutation_rate)
{
	for (unsigned i = 0; i < length; ++i)
	{
		double mutation_attempt = rand() / (RAND_MAX + 1);
		if (mutation_attempt < mutation_rate)
		{
			chromosome[i] = randomAllele();
		}
	}
}

pair<GAPhenotype, GAPhenotype> GAPhenotype::crossover(GAPhenotype mate)
{
	unsigned int slice_point = (int)floor((double) (rand() % length)) + 1;
	GAPhenotype child1(alleles, length);
	GAPhenotype child2(alleles, length);
	for (unsigned i = 0; i < slice_point; ++i)
	{
		child1.chromosome[i] = chromosome[i];
		child2.chromosome[i] = mate.chromosome[i];
	}

	for (unsigned i = slice_point; i < length; ++i)
	{
		child1.chromosome[i] = mate.chromosome[i];
		child2.chromosome[i] = chromosome[i];
	}

	pair<GAPhenotype, GAPhenotype> ret;
	ret.first = child1;
	ret.second = child2;
	return ret;
}