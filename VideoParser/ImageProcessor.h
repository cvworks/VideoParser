/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IMAGE_PREPROCESSOR_H_
#define _IMAGE_PREPROCESSOR_H_

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>
#include <Tools/DirWalker.h>
#include <Tools/BasicTypes.h>

namespace vpl {

/*!
	Image processor with "memory" of previous frames processed.
*/
class ImageProcessor : public VisSysComponent
{
	typedef std::list<InputImageInfo> ImgList;

	struct Params
	{
		unsigned int maxBufferSize; //!< Maximum size of the image stack
		float darkPixelValue;
		double darknessThreshold;
		double pixelDiffThreshold;
		double maxTolerableNoiseLevel;
	};

	struct Stats
	{
		double darknessLevel;
		double noiseLevel;

		void Clear()
		{
			darknessLevel = 0;
			noiseLevel = 0;
		}
	};

	ImgList m_imgBuffer; //!< Stack of video parsing data. Current frame is at the front.

	Params m_params;
	Stats m_stats;

	double ComputeDarknessLevel(FloatImg img) const;
	double ComputeNoiseLevel(FloatImg img, FloatImg meanImg) const;

	bool IsOfftime() const
	{
		Time tm(Timestamp());

		return (tm.hour_between(1, 8) ||
			tm.weekday() == Time::SATURDAY ||
			tm.weekday() == Time::SUNDAY);
	}

public:
	//! Specifies that an image metadata is a list of string-value pairs
	typedef DirWalker::KeyValueList ImageMetadata;

public:
	unsigned int BufferSize() const
	{
		return m_imgBuffer.size();
	}

	bool IsTooDark() const
	{
		return DarknessLevel() >= m_params.darknessThreshold;
	}

	bool IsNoisy() const
	{
		return m_stats.noiseLevel > m_params.maxTolerableNoiseLevel;
	}

	double DarknessLevel() const
	{
		return m_stats.darknessLevel;
	}

	double NoiseLevel() const
	{
		return m_stats.noiseLevel;
	}

	bool ROIChanged() const
	{
		if (m_imgBuffer.empty())
			return false;
		else if (m_imgBuffer.size() == 1)
			return !GetROISequence().empty(); // changed if not empty
		else
			return GetROISequence(0) != GetROISequence(1);
	}

	size_t ROICount() const
	{
		return (m_imgBuffer.empty()) ? 0 : GetROISequence().size();
	}

	RGBImg GetRGBImage() const
	{
		return (m_imgBuffer.empty()) ? RGBImg() : m_imgBuffer.front().rgbFrame;
	}

	FloatImg GetGreyImage() const
	{
		return (m_imgBuffer.empty()) ? FloatImg() : m_imgBuffer.front().greyFrame;
	}

	double GetFramePosition() const
	{
		return (m_imgBuffer.empty()) ? 0 : m_imgBuffer.front().framePos;
	}

	ROISequence& GetROISequence() 
	{
		ASSERT(!m_imgBuffer.empty());

		return m_imgBuffer.front().roiSequence;
	}

	ROISequence GetROISequence() const
	{
		ASSERT(!m_imgBuffer.empty());

		return m_imgBuffer.front().roiSequence;
	}

	RGBImg GetRGBImage(unsigned int i) const
	{
		ASSERT(!m_imgBuffer.empty());

		return element_at(m_imgBuffer, i).rgbFrame;
	}

	FloatImg GetGreyImage(unsigned int i) const
	{
		ASSERT(!m_imgBuffer.empty());

		return element_at(m_imgBuffer, i).greyFrame;
	}

	ROISequence GetROISequence(unsigned i) const
	{
		ASSERT(!m_imgBuffer.empty());

		return element_at(m_imgBuffer, i).roiSequence;
	}

	virtual void Run();

	virtual void Clear()
	{
		VisSysComponent::Clear();

		m_imgBuffer.clear();
	}

	virtual void Initialize(graph::node v);
	virtual void ReadParamsFromUserArguments();

	virtual std::string ClassName() const
	{
		return "ImageProcessor";
	}

	virtual int NumOutputImages() const
	{
		return 3;
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Current RGB image";
			case 1: return "Current grey image";
			case 2: return "ROI Masks";
		}

		ASSERT(false);
	}

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		if (i == 2 && !m_imgBuffer.empty() && !GetROISequence().empty())
		{
			InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 
				GetROISequence().size() - 1, 1);
		}
		else
		{
			VisSysComponent::GetParameterInfo(i, pMinVals, pMaxVals, pSteps);
		}
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	/*!
		Returns the metadata associated with the current video frame. 
		It is the empty if there is no metadata.

		It recovers the video metadata from its filename
		using the DirWalker::ParseFileNameParams() function.

		If the file name has no KeyValue pairs, it is assumed that the
		name of the file (with no extension) is a key and its value is -1.
	*/
	virtual ImageMetadata FrameMetadata() const
	{
		if (m_imgBuffer.empty())
		{
			return ImageMetadata();
		}
		else
		{
			DirWalker dw;

			dw.ParseFilePath(m_imgBuffer.front().frameInfo.c_str());
			
			dw.ParseFileNameParams(true); // true => include incomplete key-val pairs

			return dw.Params();
		}
	}

	virtual bool OnGUIEvent(const UserEventInfo& uei);
	virtual void Draw(const DisplayInfoIn& dii) const;
};

} // namespace vpl

#endif //_IMAGE_PREPROCESSOR_H_
