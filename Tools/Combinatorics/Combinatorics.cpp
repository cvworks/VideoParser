/* 24.12.2008 last modification: 26.04.2009
Copyright (c) 2009 by Siegfried Koepf

This file is distributed under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.
For information on usage and redistribution and for a disclaimer of all
warranties, see the file COPYING in this distribution.
*/
#include "Combinatorics.h"

/*!
	Variations with repetition in lexicographic order

	Inspired by Algorithm M (Mixed-radix generation) in: Knuth, Donald E.: 
	The Art of Computer Programming, Vol. 4: Fascicle 2. Generating All 
	Tuples and Permutations. Upper Saddle River, NJ 2005.

	Functions:
	int gen_vari_rep_init(unsigned char *vector, const unsigned char m, const unsigned char n)
	Test for special cases
	Initialization of vector
	Possible return values are: GEN_EMPTY, GEN_NEXT

	int gen_vari_rep_next(unsigned char *vector, const unsigned char m, const unsigned char n)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char m; //length of alphabet
	const unsigned char n; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process

	Cardinality:
	m^n
*/
int gen_vari_rep_init(unsigned char *vector, const unsigned char m, const unsigned char n)
{
	int j; //index

	//test for special cases
	if(m == 0 || n == 0)
		return(GEN_EMPTY);

	//initialize: vector[0, ..., n - 1] are zero
	for(j = 0; j < n; j++)
		vector[j] = 0;

	return(GEN_NEXT);
}

int gen_vari_rep_next(unsigned char *vector, const unsigned char m, const unsigned char n)
{
	int j; //index

	//easy case, increase rightmost element
	if(vector[n - 1] < m - 1)
	{
		vector[n - 1]++;
		return(GEN_NEXT);
	}

	//find rightmost element to increase and reset right-hand elements
	for(j = n - 1; j >= 0 && vector[j] == m - 1; j--)
		vector[j] = 0;

	//terminate if all elements are m - 1
	if(j < 0)
		return(GEN_TERM);

	//increase
	vector[j]++;

	return(GEN_NEXT);
}

/*!
	k-combinations without repetition in lexicographic order

	Inspired by Donald Knuth and many others

	Functions:
	int gen_comb_norep_init(unsigned char *vector, const unsigned char n, const unsigned char k)
	Test for special cases
	Initialization of vector
	Possible return values are: GEN_ERROR, GEN_EMPTY, GEN_NEXT

	int gen_comb_norep_next(unsigned char *vector, const unsigned char n, const unsigned char k)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char n; //length of alphabet
	const unsigned char k; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process
	k must be <= n

	Cardinality:
	n! / ((n - k)! * k!) == Binomial(n, k)
*/
int gen_comb_norep_init(unsigned char *vector, const unsigned char n, const unsigned char k)
{
	int j; //index

	//test for special cases
	if(k > n)
		return(GEN_ERROR);

	if(k == 0)
		return(GEN_EMPTY);

	//initialize: vector[0, ..., k - 1] are 0, ..., k - 1
	for(j = 0; j < k; j++)
		vector[j] = j;

	return(GEN_NEXT);
}


int gen_comb_norep_next(unsigned char *vector, const unsigned char n, const unsigned char k)
{
	int j; //index

	//easy case, increase rightmost element
	if(vector[k - 1] < n - 1)
	{
		vector[k - 1]++;
		return(GEN_NEXT);
	}

	//find rightmost element to increase
	j = k - 1;
	while(j >= 0 && vector[j] == n - k + j)
		j--;

	//terminate if vector[0] == n - k
	if(j < 0)
		return(GEN_TERM);

	//increase
	vector[j]++;

	//set right-hand elements
	for(j += 1; j < k; j++)
		vector[j] = vector[j - 1] + 1;

	return(GEN_NEXT);
}

/*!
	k-combinations with repetition in lexicographic order

	Functions:
	int gen_comb_rep_init(unsigned char *vector, const unsigned char n, const unsigned char k)
	Test for special cases
	Initialization of vector
	Possible return values are: GEN_EMPTY, GEN_NEXT

	int gen_comb_rep_next(unsigned char *vector, const unsigned char n, const unsigned char k)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char n; //length of alphabet
	const unsigned char k; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process

	Cardinality:
	(n + k - 1)! / (k! * (n - 1)!) == Binomial(n + k - 1, k)
*/
int gen_comb_rep_init(unsigned char *vector, const unsigned char n, const unsigned char k)
{
	int j; //index

	//test for special cases
	if(n == 0 || k == 0)
		return(GEN_EMPTY);

	//initialize: vector[0, ..., k - 1] are zero
	for(j = 0; j < k; j++)
		vector[j] = 0;

	return(GEN_NEXT);
}

int gen_comb_rep_next(unsigned char *vector, const unsigned char n, const unsigned char k)
{
	int j; //index

	//find rightmost element to increase
	j = k - 1;
	while(j >= 0 && vector[j] == n - 1)
		j--;

	//terminate if all elements are n - 1
	if(j < 0)
		return(GEN_TERM);

	//increase
	vector[j]++;

	//set right-hand elements
	for(j += 1; j < k; j++)
		vector[j] = vector[j - 1];

	return(GEN_NEXT);
}

/*!
	k-partitions in colex order

	Based on Algorithm H (Partitions into m parts) in: Knuth, Donald E.: The Art 
	of Computer Programming, Vol. 4: Fascicle 3. Generating All Combinations and 
	Partitions. Upper Saddle River, NJ 2005.

	Functions:
	int gen_k_part_init(unsigned char *vector, const unsigned char n, const unsigned char k)
	Test for special cases
	Initialization of vector
	Possible return values are: GEN_ERROR, GEN_EMPTY, GEN_NEXT

	int gen_k_part_next(unsigned char *vector, const unsigned char k)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char n; //length of alphabet
	const unsigned char k; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process
	k must be <= n

	Cardinality see:
	Sloane, N.J.A. et al.: The On-Line Encyclopedia of Integer Sequences, 2008.
	www.research.att.com/~njas/sequences/A008284
*/
int gen_k_part_init(unsigned char *vector, const unsigned char n, const unsigned char k)
{
	int j; //index

	//test for special cases
	if(k > n)
		return(GEN_ERROR);

	if(n == 0)
		return(GEN_EMPTY);

	if(k == 0)
		return(GEN_ERROR);

	//initialize: vector[0] = n - k + 1, vector[1, ..., k - 1] are 1
	vector[0] = n - k + 1;

	for(j = 1; j < k; j++)
		vector[j] = 1;

	return(GEN_NEXT);
}

int gen_k_part_next(unsigned char *vector, const unsigned char k)
{
	int j;    //index
	int r;    //temporary remainder
	int temp; //auxiliary element

	//terminate, this is only needed to allow k == 1
	if(k <= 1)
		return(GEN_TERM);

	//easy case
	if(vector[0] - 1 > vector[1])
	{
		vector[0]--;
		vector[1]++;
		return(GEN_NEXT);
	}

	//find leftmost element to increase and set r
	r = vector[0] - 1;
	for(j = 1; j < k && vector[j] >= vector[0] - 1; j++)
		r += vector[j];

	//terminate if no part is less than vector[0] - 1
	if(j >= k)
		return(GEN_TERM);

	//increase
	vector[j]++;

	//set left-hand elements
	temp = vector[j];

	for(j -= 1; j > 0; j--)
	{
		vector[j] = temp;
		r -= temp;
	}

	vector[0] = r;

	return(GEN_NEXT);
}

/*!
	Necklaces in lexicographic order

	Based on FKM-algorithm by Fredricksen, Kessler, Maiorana as discussed by 
	Donald Knuth as Algorithm F (Prime and preprime string generation) in: 
	Knuth, Donald E.: The Art of Computer Programming, Vol. 4: Fascicle 2. 
	Generating All Tuples and Permutations. Upper Saddle River, NJ 2005.

	Functions:
	int gen_neck_init(unsigned char *vector, const unsigned char m, const unsigned char n)
	Test for special cases
	Initialization of vector
	Possible return values are: GEN_EMPTY, GEN_NEXT

	int gen_neck_next(unsigned char *vector, const unsigned char m, const unsigned char n)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char m; //length of alphabet
	const unsigned char n; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process

	Cardinality see:
	http://mathworld.wolfram.com/Necklace.html
*/
int gen_neck_init(unsigned char *vector, const unsigned char m, const unsigned char n)
{
	int j; //index

	//test for special cases
	if(m == 0 || n == 0)
		return(GEN_EMPTY);

	//initialize: vector[0, ..., n - 1] are zero
	for(j = 0; j < n; j++)
		vector[j] = 0;

	return(GEN_NEXT);
}


int gen_neck_next(unsigned char *vector, const unsigned char m, const unsigned char n)
{
	int j; //index
	int i; //help index
	int r; //temporary remainder, for hand made modulo computation only

SKIP: //previous prenecklace skipped

	//find rightmost element to increase
	j = n - 1;
	while(j >= 0 && vector[j] == m - 1)
		j--;

	//terminate if all elements are m - 1
	if(j < 0)
		return(GEN_TERM);

	//increase
	vector[j]++;

	//set right-hand elements
	for(i = j + 1; i < n; i++)
		vector[i] = vector[i - j - 1];

	//necklaces only
	r = n;
	j++;

	while(r >= j)
		r -= j;

	if(r != 0)
		goto SKIP; //skip this prenecklace

	return(GEN_NEXT);
}

/*!
	Partitions in reverse lexicographic order

	Based on Algorithm ZS1 in: Zoghbi, Antoine and Stojmenovic, Ivan: Fast 
	Algorithms for Generating Integer Partitions. International Journal of 
	Computer Mathematics, 70, 1998, 319-332.

	Functions:
	int gen_part_init(unsigned char *vector, const unsigned char n, unsigned char *k)
	Test for special cases
	Initialization of vector
	Initialization of k
	Possible return values are: GEN_EMPTY, GEN_NEXT

	int gen_part_next(unsigned char *vector, unsigned char *k)
	Transforms current figure in vector into its successor
	Sets k to the appropriate value
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char n; //length of alphabet
	const unsigned char k; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process

	Cardinality see:
	Sloane, N.J.A. et al.: The On-Line Encyclopedia of Integer Sequences, 2008.
	www.research.att.com/~njas/sequences/A000041
	see also:
	http://mathworld.wolfram.com/Partition.html
*/
int gen_part_init(unsigned char *vector, const unsigned char n, unsigned char *k)
{
	int j; //index

	//test for special cases
	if(n == 0)
	{
		(*k) = 0;
		return(GEN_EMPTY);
	}

	//initialize: vector[0] = n, vector[1, ..., n - 1] are 1
	vector[0] = n;

	for(j = 1; j < n; j++)
		vector[j] = 1;

	(*k) = 1;

	return(GEN_NEXT);
}


int gen_part_next(unsigned char *vector, unsigned char *k)
{
	static int j = 0; //remember index of the rightmost part which is greater than 1
	int        r;     //temporary remainder
	int        temp;  //auxiliary element

	//easy case
	if(vector[j] == 2)
	{
		vector[j] = 1;
		j--;
		(*k)++;
		return(GEN_NEXT);
	}

	//terminate if all parts are 1
	if(vector[0] == 1)
	{
		j = 0;
		return(GEN_TERM);
	}

	//decrease
	vector[j]--;
	temp = vector[j];
	r = *k - j;

	//set right-hand elements
	while(r > temp)
	{
		j++;
		vector[j] = temp;
		r -= temp;
	}

	*k = j + 2;

	//set rightmost element
	if(r > 1)
	{
		j++;
		vector[j] = r;
	}

	return(GEN_NEXT);
}

/*!
	Permutations with repetition in lexicographic order

	Based on Algorithm L (Lexicographic permutation generation) in: Knuth, 
	Donald E.: The Art of Computer Programming, Vol. 4: Fascicle 2. Generating 
	All Tuples and Permutations. Upper Saddle River, NJ 2005.

	Functions:
	int gen_perm_rep_init(const unsigned char n)
	Test for special cases
	Possible return values are: GEN_EMPTY, GEN_NEXT

	int gen_perm_rep_next(unsigned char *vector, const unsigned char n)
	Transforms current figure in vector into its successor
	Possible return values are: GEN_NEXT, GEN_TERM

	Arguments:
	unsigned char *vector; //pointer to the array where the current figure is stored
	const unsigned char n; //length of alphabet
	const unsigned char k; //length of figures

	Usage and restrictions:
	Arguments and elements in vector are restricted to the interval (0, 255)
	Memory allocation for vector must be provided by the calling process
	vector must be initialized by the calling process. At that point elements in 
	vector must be arranged in increasing order to obtain all possible permutations

	Cardinality:
	n! / ((s(1)! * ... * s(x)!)) with s(1) + ... + s(x) == n where s(x) is the number of occurrences of the xth disitinct element
*/
int gen_perm_rep_init(const unsigned char n)
{
	//test for special cases
	if(n == 0)
		return(GEN_EMPTY);

	//initialize: vector must be initialized by the calling process

	return(GEN_NEXT);
}


int gen_perm_rep_next(unsigned char *vector, const unsigned char n)
{
	int j = n - 2; //index
	int i = n - 1; //help index
	int temp;      //auxiliary element

	//find rightmost element to increase
	while(j >= 0 && vector[j + 1] <= vector[j])
		j--;

	//terminate if all elements are in decreasing order
	if(j < 0)
		return(GEN_TERM);

	//find i
	while(vector[i] <= vector[j])
		i--;

	//increase (swap)
	temp = vector[j];
	vector[j] = vector[i];
	vector[i] = temp;

	//reverse right-hand elements
	for(j += 1, i = n - 1; j < i;  j++, i--)
	{
		temp = vector[j];
		vector[j] = vector[i];
		vector[i] = temp;
	}

	return(GEN_NEXT);
}

