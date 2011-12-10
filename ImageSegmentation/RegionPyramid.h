/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Region.h"

namespace vpl {

typedef std::vector<Region> RegionArray;

/*!
	Set of groups of regions. Each level corresponds
	to a particular image segmentation.
*/
class RegionPyramid
{
private:
	/*! 
		Basic non-deferrable iterator over all the regions 
		in the pyramid. It is inherited by the const and 
		non-const deferrable iterator defined bellow.
	*/
	template <typename L, typename R> class basic_iterator 
	{
	protected:
		L levIt;
		L endLevIt;
		R regIt;

		/*! 
			Finds the first region within the first non-empty
			level, starting from the current level.

			It does nothing if the current level is the "end" level.
		*/
		void GoToFirstRegion()
		{
			// Find the first region...
			while (levIt != endLevIt) 
			{
				regIt = levIt->begin(); // see if first region in level

				// If level is not empty
				if (regIt != levIt->end())
					break; // it's the first region
				else
					++levIt; // go to next level
			}
		}


	public:
		basic_iterator(const L& lit, const L& elit)
			: levIt(lit), endLevIt(elit)
		{
			GoToFirstRegion();
		}

		/*!
			Returns true if both iterators refer to the same region
			or if both iterators refer to the "end" of the pyramid.
		*/
		bool operator==(const basic_iterator& rhs) const
		{
			ASSERT(endLevIt == rhs.endLevIt);

			// Note, the region iterators can only be compared
			// if both level iterators don't refer to the "end" levels
			if (levIt == endLevIt || rhs.levIt == rhs.endLevIt)
			{
				// Buth must be the "end" level to be equal
				return (levIt == rhs.levIt);
			}
			else 
			{
				// Neither level is the "end", so check regions...
				return (levIt == rhs.levIt && regIt == rhs.regIt);
			}
		}

		/*!
			Returns the negation of operator==.
		*/
		bool operator!=(const basic_iterator& rhs) const
		{
			return !operator==(rhs);
		}

		basic_iterator& operator++()
		{
			ASSERT(levIt != endLevIt);
			ASSERT(regIt != levIt->end());
			
			// Move to next region
			++regIt;

			// If it is the "end" region of the level...
			if (regIt == levIt->end())
			{
				// Move to next level
				++levIt;

				// Move to first region in the first non-empty level
				GoToFirstRegion();
			}

			return *this;
		}
	};

protected:
	typedef std::list<RegionArray> PyramidLevels;

	PyramidLevels m_levels;

public:
	typedef RegionArray::const_iterator const_region_iterator;
	typedef PyramidLevels::const_iterator const_level_iterator;
	
	//! Deferrable, constant-region iterator
	class const_iterator : public 
		basic_iterator<const_level_iterator, const_region_iterator>
	{
	public:
		const_iterator(const const_level_iterator& lit, 
			const const_level_iterator& elit) : basic_iterator(lit, elit)
		{
		}

		const Region& operator*() const
		{
			return regIt.operator*();
		}

		const Region* operator->() const
		{
			return regIt.operator->();
		}
	};

	typedef RegionArray::iterator region_iterator;
	typedef PyramidLevels::iterator level_iterator;

	//! Deferrable region iterator
	class iterator : public 
		basic_iterator<level_iterator, region_iterator>
	{
	public:
		iterator(const level_iterator& lit, 
			const level_iterator& elit) : basic_iterator(lit, elit)
		{
		}

		Region& operator*() const
		{
			return regIt.operator*();
		}
		
		Region* operator->() const
		{
			return regIt.operator->();
		}
	};

public:
	void operator=(const RegionPyramid& rhs)
	{
		m_levels = rhs.m_levels;
	}

	/*!
		Adds a level to the pyramid. The first level added has 
		index 0, the second level has index 1 and so on. That is,
		the index of the level added is equal to the value size() 
		prior to adding the level.
	*/
	void AddLevel(const RegionArray& ra)
	{
		m_levels.push_back(ra);
	}

	void clear() 
	{ 
		m_levels.clear(); 
	}

	bool empty() const 
	{ 
		return m_levels.empty(); 
	}

	/*! 
		Returns an iterator set to the first region
		in the first non-empty level.
	*/
	iterator begin() 
	{ 
		return iterator(m_levels.begin(), m_levels.end()); 
	}

	/*! 
		Returns an iterator set to the "end" level.
	*/
	iterator end() 
	{ 
		return iterator(m_levels.end(), m_levels.end()); 
	}

	/*! 
		Returns a const iterator set to the first region
		in the first non-empty level.
	*/
	const_iterator begin() const
	{ 
		return const_iterator(m_levels.begin(), m_levels.end()); 
	}

	/*! 
		Returns an iterator set to the "end" level.
	*/
	const_iterator end() const
	{ 
		return const_iterator(m_levels.end(), m_levels.end()); 
	}

	void Serialize(OutputStream& os) const
	{
		::Serialize(os, m_levels);
	}

	void Deserialize(InputStream& is) 
	{
		::Deserialize(is, m_levels);
	}

	const RegionArray& level(unsigned i) const
	{
		return element_at(m_levels, i);
	}

	const Region& region(unsigned i, unsigned regionId) const
	{
		return element_at(m_levels, i).at(regionId);
	}

	Region& region(unsigned i, unsigned regionId)
	{
		return element_at(m_levels, i).at(regionId);
	}

	//! Number of levels in the pyramid
	unsigned height() const 
	{ 
		return m_levels.size(); 
	}

	//! Width (num regions) of the widest level
	unsigned maxWidth() const
	{
		unsigned mw = 0;

		for (auto it = m_levels.begin(); it != m_levels.end(); ++it)
		{
			if (it->size() > mw)
				mw = it->size();
		}

		return mw;
	}

	/*!
		Returns true iff the sets have the same number of groups
		listed in the same order and each level has the same number
		of regions, in the same order and with the same boundary.

		Note: region attributes other than boundary can be different.
	*/
	bool operator==(const RegionPyramid& rhs) const
	{
		auto it0 = begin();
		auto it1 = rhs.begin();

		for (; it0 != end() && it1 != rhs.end(); ++it0, ++it1)
		{
			if (it0->boundaryPts != it1->boundaryPts)
				return false;
		}

		// If we reached the end of both pyramids, then
		// they must be equal
		return (it0 == end() && it1 == rhs.end());
	}

	/*!	
		Returns the negation of operator==.

		@see operator==
	*/
	bool operator!=(const RegionPyramid& rhs) const
	{
		return !operator==(rhs);
	}
};

} // namespace vpl

