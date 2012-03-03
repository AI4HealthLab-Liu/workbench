#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#include <vector>
#include "stdint.h"

namespace caret
{
    
    class Histogram
    {
        std::vector<int64_t> m_buckets, m_cumulative;
        std::vector<float> m_display;
        float m_bucketMin, m_bucketMax;
        ///counts of each class of number
        int64_t m_posCount, m_zeroCount, m_negCount, m_infCount, m_negInfCount, m_nanCount;
        
        void resize(const int& buckets);
        
        void reset();
        
        void computeCumulative();
        
    public:
        Histogram(const int& numBuckets = 100);
        
        Histogram(const float* data, const int64_t& dataCount);//NOTE: automatically determines number of buckets by square root of dataCount, but with set minimum and maximum
        
        Histogram(const int& numBuckets, const float* data, const int64_t& dataCount);
        
        void update(const float* data, const int64_t& dataCount);
        
        void update(const int& numBuckets, const float* data, const int64_t& dataCount);
        
        void update(const float* data,
                    const int64_t& dataCount,
                    float mostPositiveValueInclusive,
                    float leastPositiveValueInclusive,
                    float leastNegativeValueInclusive,
                    float mostNegativeValueInclusive,
                    const bool& includeZeroValues);
        
        ///get raw counts (useful mathematically)
        const std::vector<int64_t>& getHistogramCounts() const { return m_buckets; }
        
        const std::vector<int64_t>& getHistogramCumulativeCounts() const { return m_cumulative; }
        
        ///get display values - counts divided by bucket widths - will be consistent on the same data regardless of number of buckets or 
        const std::vector<float>& getHistogramDisplay() const { return m_display; }
        
        int getNumberOfBuckets() const { return (int)m_buckets.size(); }
        
        void getCounts(int64_t& posCount, int64_t& zeroCount, int64_t& negCount, int64_t& infCount, int64_t& negInfCount, int64_t& nanCount) const
        {
            posCount = m_posCount;
            zeroCount = m_zeroCount;
            negCount = m_negCount;
            infCount = m_infCount;
            negInfCount = m_negInfCount;
            nanCount = m_nanCount;
        }
        
        ///returns the low edge of the low bucket, and the high edge of the high bucket
        void getRange(float& histMin, float& histMax) const
        {
            histMin = m_bucketMin;
            histMax = m_bucketMax;
        }
    };

}

#endif //__HISTOGRAM_H__
