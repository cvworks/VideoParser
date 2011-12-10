#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"

#include "basics/definitions.h"
#include "boost_kdd_utils.h"

ushort sResultPairLess::operator () (const sResultPair & object1, 
	const sResultPair & object2)
{
	return (object1.distance < object2.distance);
}

// this function should become a template. Takes a matrix and
// returns another matrix where each entry in the first matrix
// is squared.
vMatrix<float> vSquareEntries(vMatrix<float> input)
{
	if (input.valid() <= 0)
	{
		exit_error("error: invalid input in vSquareEntries\n");
	}

	vint8 rows = input.Rows();
	vint8 cols = input.Cols();
	vMatrix<float> result(rows, cols);

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			float entry = input(row, col);
			result(row, col) = entry * entry;
		}
	}

	return result;
}


/*!
	Returns the position of each elements in 'values' that results from
	sorting 'values'. ie, results[i] = j means that elements 'i' is in
	position 'j' when 'values' is sorted.

	Eventually this function should go to algo_templates or general_templates.
	result[i] = j if values[i] is the j-th smallest entry in values.
*/
vint8 vIndexRanks(vector<float> * values, vector<vint8> * result)
{
	vint8 number = values->size();
	if (number == 0)
	{
		return 0;
	}

	// create a vector that holds value-index pairs, so that we 
	// can sort based on value, but still know the associated index.
	vector<class_couple> pairs((vector_size) number);
	vint8 i;

	// basically, save the current index of each value, then sort by value
	for (i = 0; i < number; i++)
	{
		pairs[(vector_size) i] = class_couple((*values)[(vector_size) i], 
			(void *) (long) i); // 'i' casting is required, but value of i is stored
	}

	std::sort(pairs.begin(), pairs.end(), couple_less());

	result->erase(result->begin(), result->end());
	result->reserve((vector_size) number);
	result->insert(result->begin(), (vector_size) number, 0);

	// let each value know its position in the ranking
	for (i = 0; i < number; i++)
	{
		vint8 index = (vint8) (long) (pairs[(vector_size) i].object);
		(*result)[(vector_size) index] = i;
	}

	return 1;
}


// eventually this function should go to algo_templates or general_templates.
// result[i] = j if values[j] is the i-th smallest entry in values.
vint8 vSortedRanks(vector<float> * values, vector<vint8> * result)
{
	vint8 number = values->size();
	if (number == 0)
	{
		return 0;
	}

	// create a vector that holds value-index pairs, so that we 
	// can sort based on value, but still know the associated index.
	vector<class_couple> pairs((vector_size) number);
	vint8 i;

	for (i = 0; i < number; i++)
	{
		pairs[(vector_size) i] = class_couple((*values)[(vector_size) i], (void *) (long) i);
	}
	std::sort(pairs.begin(), pairs.end(), couple_less());

	result->erase(result->begin(), result->end());
	result->reserve((vector_size) number);
	result->insert(result->begin(), (vector_size) number, 0);

	for (i = 0; i < number; i++)
	{
		vint8 index = (vint8) (long) (pairs[(vector_size) i].object);
		(*result)[(vector_size) i] = index;
	}

	return 1;
}


// this function should also be a template function. It is kind of
// the reverse of copy_horizontal_line. Assumes that source only has one row,
// and writes that row into row row of target.
/*vint8 function_put_row(v3dMatrix<float> * source, v3dMatrix<float> * target, vint8 row)
{
if (source->Rows() != 1)
{
exit_error("error: in function_put_row, source->rows = %li\n",
source->Rows());
}

if (source->Bands() != target->Bands())
{
exit_error("error: in function_put_row, different bands: %li %li\n",
source->Bands(), target->Bands());
}

if (source->Cols() != target->Cols())
{
exit_error("error: in function_put_row, different cols: %li %li\n",
source->Cols(), target->Cols());
}

if (target->Rows() <= row)
{
exit_error("error: in function_put_row, row = %li, target rows = %li\n",
row, target->Rows());
}

vArray3(float) source_data = source->Matrix3();
vArray3(float) target_data = target->Matrix3();

vint8 bands = source->Bands();
vint8 cols = source->Cols();
vint8 band, col;
for (band = 0; band < bands; band++)
{
vArray(float) source_row = source_data[band][0];
vArray(float) target_row = target_data[band][row];
for (col = 0; col < cols; col++)
{
target_row[col] = source_row[col];
}
}

return 1;
}
*/

// some more functions that should be templates
// returns a matrix containing all the indices
// i such that source->matrix[i] == value.
vint8_matrix vGetIndices(v3dMatrix<float> * source, float value)
{
	vector<vint8> indices_vector;
	vint8 size = source->Size();
	vArray(float) entries = source->Matrix();

	vint8 i;
	for (i = 0; i < size; i++)
	{
		if (entries[i] == value)
		{
			indices_vector.push_back(i);
		}
	}

	if (indices_vector.size() == 0)
	{
		return vint8_matrix();
	}

	vint8_matrix result = matrix_from_vector(&indices_vector);
	return result;
}


// this should also be a template. pairs is an output
// argument. For each index occurring in indicesm, 
// we store into pairs the pair (sourcem[index], index).
vint8 vGetIndicesValues(v3dMatrix<float> * sourcem, 
	v3dMatrix<vint8> * indicesm, 
	vector<class_couple> * pairs)
{
	vArray(float) source = sourcem->Matrix();
	vArray(vint8) indices = indicesm->Matrix();
	vint8 size = indicesm->Size();
	vint8 source_size = sourcem->Size();

	vint8 i;
	for (i = 0; i < size; i++)
	{
		vint8 index = indices[i];
		if ((index < 0) || (index >= source_size))
		{
			exit_error("error: vGetIndicesValues, index = %li, source_size = %li\n",
				(long) index, (long) source_size);
		}
		float value = source[index];
		pairs->push_back(class_couple(value, (void *) (long) index));
	}

	return 1;
}

// this function returns a binary matrix, which is
// zero at the i-th position iff input[i] (i.e.
// input->matrix[i]) <= threshold.
v3dMatrix<float> vSelectGreater(v3dMatrix<float> * input, 
	float threshold)
{
	vint8 rows = input->Rows();
	vint8 cols = input->Cols();
	vint8 bands = input->Bands();
	vint8 size = input->Size();

	v3dMatrix<float> result(rows, cols, bands);
	vArray(float) in = input->Matrix();
	vArray(float) out = result.Matrix();

	vint8 i;
	for (i = 0; i < size; i++)
	{
		if (in[i] > threshold)
		{
			out[i] = (float) 1;
		}
		else
		{
			out[i] = (float) 0;
		}
	}

	return result;
}
