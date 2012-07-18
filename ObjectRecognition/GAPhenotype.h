/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Chris Whiten
 *-----------------------------------------------------------------------*/
#ifndef GAPHENOTYPE_H
#define GAPHENOTYPE_H

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;
class GAPhenotype
{
public:
	GAPhenotype(vector<unsigned int> a, unsigned int l);
	GAPhenotype();

	void mutate(double mutation_rate = 0.1);
	pair<GAPhenotype, GAPhenotype> crossover(GAPhenotype mate);
	string toString();

	vector<unsigned int> chromosome;
	double fitness;

	bool operator < (const GAPhenotype &pheno) const
	{
		return (fitness < pheno.fitness);
	}


private:
	void init();
	unsigned int randomAllele();

	
	
	vector<unsigned int> alleles;
	unsigned int length;
};

#endif