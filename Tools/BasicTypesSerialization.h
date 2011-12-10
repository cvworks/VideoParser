/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicTypes.h"
#include "Serialization.h"

DECLARE_BASIC_SERIALIZATION(vpl::UICoordinate);
DECLARE_BASIC_SERIALIZATION(vpl::UIBoundingBox);

inline void Serialize(OutputStream& os, const vpl::DiscreteXYArray& x)
{
	::Serialize(os, x.xa);
	::Serialize(os, x.ya);
}

inline void Deserialize(InputStream& is, vpl::DiscreteXYArray& x)
{
	::Deserialize(is, x.xa);
	::Deserialize(is, x.ya);
}

