/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

struct BackgroundStats
{
	enum Type {PIXEL_BASED, FEATURE_BASED};

	Type type;
	bool changeDetected;

	// Pixel based stats
	//int numberPixelsBelowThreshold;
	//int numberPixelsAboveThreshold;
	//int numberPixelsInThreshold;
	unsigned numberForegroundPixels;
	unsigned numberBackgroundPixels;

	// Feature based stats
	unsigned numGoodFeaturesMatched;
	double matchedGoodFeaturesRatio;
	double unmatchedQueryFeaturesRatio;

	BackgroundStats(Type t)
	{
		type = t;
	}

	double ChangeRatio() const
	{
		if (numberBackgroundPixels == 0)
			return 0;
		else
			return double(numberForegroundPixels) / double(numberBackgroundPixels);
	}

	void Clear()
	{
		changeDetected = false;

		numberForegroundPixels = 0;
		//numberPixelsBelowThreshold = 0;
		//numberPixelsAboveThreshold = 0;
		numberBackgroundPixels = 0;

		numGoodFeaturesMatched = 0;
		matchedGoodFeaturesRatio = 0;
		unmatchedQueryFeaturesRatio = 0;
	}
};
