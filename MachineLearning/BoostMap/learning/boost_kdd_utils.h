#pragma once

// this is an auxiliary class used to sort distances and labels 
// in CascadeStats
class sResultPair
{
public:
	float distance;
	vint8 label;

	sResultPair()
	{
		distance = 0;
		label = -1;
	}

	sResultPair(float in_distance, vint8 in_label)
	{
		distance = in_distance;
		label = in_label;
	}
};


class sResultPairLess
{
public:
	ushort operator() (const sResultPair & score1, const sResultPair & score2);
};

vMatrix<float> vSquareEntries(vMatrix<float> input);	
vint8 vIndexRanks(vector<float> * values, vector<vint8> * result);
vint8 vSortedRanks(vector<float> * values, vector<vint8> * result);
vint8_matrix vGetIndices(v3dMatrix<float> * source, float value);
vint8 vGetIndicesValues(v3dMatrix<float> * sourcem, 
	v3dMatrix<vint8> * indicesm, 
	vector<class_couple> * pairs);
v3dMatrix<float> vSelectGreater(v3dMatrix<float> * input, 
	float threshold);
