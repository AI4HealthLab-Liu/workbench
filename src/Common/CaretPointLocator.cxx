/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
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

#include "CaretPointLocator.h"
#include "CaretHeap.h"
#include <cmath>

using namespace caret;
using namespace std;

void CaretPointLocator::addPoint(Oct<LeafVector<Point> >* thisOct, const float point[3], const int32_t index, const int32_t pointSet)
{
    if (thisOct->m_leaf)
    {
        thisOct->m_data.m_vector->push_back(Point(point, index, pointSet));
        int curSize = (int)thisOct->m_data.m_vector->size();
        if (curSize > NUM_POINTS_SPLIT)
        {//test that not all points are the same, or that they have some minimum percentage spread, or...
            vector<Point>& myVecRef = *(thisOct->m_data.m_vector);
            Vector3D minBox = myVecRef[0].m_point, maxBox = myVecRef[0].m_point, tempvec;
            tempvec[0] = thisOct->m_bounds[0][2] - thisOct->m_bounds[0][0];
            tempvec[1] = thisOct->m_bounds[1][2] - thisOct->m_bounds[1][0];
            tempvec[2] = thisOct->m_bounds[2][2] - thisOct->m_bounds[2][0];
            float diagonal = tempvec.length();
            bool safeToSplit = false;
            for (int i = 1; i < curSize; ++i)
            {//this will slow down if lots of stuff is continuously put in an Oct that is far too big - use random sampling?
                if (myVecRef[i].m_point[0] < minBox[0]) minBox[0] = myVecRef[i].m_point[0];
                if (myVecRef[i].m_point[1] < minBox[1]) minBox[1] = myVecRef[i].m_point[1];
                if (myVecRef[i].m_point[2] < minBox[2]) minBox[2] = myVecRef[i].m_point[2];
                if (myVecRef[i].m_point[0] > maxBox[0]) maxBox[0] = myVecRef[i].m_point[0];
                if (myVecRef[i].m_point[1] > maxBox[1]) maxBox[1] = myVecRef[i].m_point[1];
                if (myVecRef[i].m_point[2] > maxBox[2]) maxBox[2] = myVecRef[i].m_point[2];
                tempvec = minBox - maxBox;
                if (tempvec.length() > 0.01f * diagonal)//make sure points aren't all identical, would go to infinity recursively
                {
                    safeToSplit = true;
                    break;
                }
            }
            if (safeToSplit)
            {
                thisOct->makeChildren();
                for (int i = 0; i < curSize; ++i)
                {
                    addPoint(thisOct->containingChild(myVecRef[i].m_point), myVecRef[i].m_point, myVecRef[i].m_index, myVecRef[i].m_mySet);
                }
                thisOct->m_data.freeData();
            }
        }
    } else {
        addPoint(thisOct->containingChild(point), point, index, pointSet);
    }
}

int32_t CaretPointLocator::addPointSet(const float* coordsIn, const int32_t numCoords)
{
    int32_t setNum = newIndex();
    if (numCoords < 1) return setNum;
    if (m_tree == NULL)
    {
        Vector3D minBox, maxBox;
        minBox = maxBox = coordsIn;//hack - first triple
        for (int32_t i = 1; i < numCoords; ++i)
        {
            int32_t i3 = i * 3;
            if (coordsIn[i3] < minBox[0]) minBox[0] = coordsIn[i3];
            if (coordsIn[i3 + 1] < minBox[1]) minBox[1] = coordsIn[i3 + 1];
            if (coordsIn[i3 + 2] < minBox[2]) minBox[2] = coordsIn[i3 + 2];
            if (coordsIn[i3] > maxBox[0]) maxBox[0] = coordsIn[i3];
            if (coordsIn[i3 + 1] > maxBox[1]) maxBox[1] = coordsIn[i3 + 1];
            if (coordsIn[i3 + 2] > maxBox[2]) maxBox[2] = coordsIn[i3 + 2];
        }
        m_tree = new Oct<LeafVector<Point> >(minBox, maxBox);
    }
    for (int32_t i = 0; i < numCoords; ++i)
    {
        int32_t i3 = i * 3;
        m_tree = m_tree->makeContains(coordsIn + i3);//make new root if needed
        addPoint(m_tree, coordsIn + i3, i, setNum);//and add the point
    }
    return setNum;
}

CaretPointLocator::CaretPointLocator(const float* coordsIn, const int32_t numCoords)
{
    m_nextSetIndex = 0;
    m_tree = NULL;
    if (numCoords >= 1)
    {
        Vector3D minBox, maxBox;
        minBox = maxBox = coordsIn;//hack - first triple
        for (int32_t i = 1; i < numCoords; ++i)
        {
            int32_t i3 = i * 3;
            if (coordsIn[i3] < minBox[0]) minBox[0] = coordsIn[i3];
            if (coordsIn[i3 + 1] < minBox[1]) minBox[1] = coordsIn[i3 + 1];
            if (coordsIn[i3 + 2] < minBox[2]) minBox[2] = coordsIn[i3 + 2];
            if (coordsIn[i3] > maxBox[0]) maxBox[0] = coordsIn[i3];
            if (coordsIn[i3 + 1] > maxBox[1]) maxBox[1] = coordsIn[i3 + 1];
            if (coordsIn[i3 + 2] > maxBox[2]) maxBox[2] = coordsIn[i3 + 2];
        }
        m_tree = new Oct<LeafVector<Point> >(minBox, maxBox);
        int32_t setNum = m_nextSetIndex;
        for (int32_t i = 0; i < numCoords; ++i)
        {
            int32_t i3 = i * 3;
            addPoint(m_tree, coordsIn + i3, i, setNum);
        }
    }
}

CaretPointLocator::CaretPointLocator(const float minBounds[3], const float maxBounds[3])
{
    m_nextSetIndex = 0;
    m_tree = new Oct<LeafVector<Point> >(minBounds, maxBounds);
}

int32_t CaretPointLocator::closestPoint(const float target[3], LocatorInfo* infoOut) const
{
    if (m_tree == NULL) return -1;
    CaretSimpleMinHeap<Oct<LeafVector<Point> >*, float> myHeap;
    bool first = true;
    float bestDist2 = -1.0f, bestDist = -1.0f, tempf, curDist = m_tree->distToPoint(target);
    Vector3D bestPoint;
    int32_t bestSet = -1, bestIndex = -1;
    myHeap.push(m_tree, curDist);
    while (curDist < bestDist || first)
    {
        Oct<LeafVector<Point> >* thisOct = myHeap.pop();
        if (thisOct->m_leaf)
        {
            vector<Point>& myVecRef = *(thisOct->m_data.m_vector);
            int curSize = (int)myVecRef.size();
            for (int i = 0; i < curSize; ++i)
            {
                tempf = MathFunctions::distanceSquared3D(myVecRef[i].m_point, target);
                if (tempf < bestDist2 || first)
                {
                    first = false;
                    bestDist2 = tempf;
                    bestPoint = myVecRef[i].m_point;
                    bestSet = myVecRef[i].m_mySet;
                    bestIndex = myVecRef[i].m_index;
                }
            }
            bestDist = sqrt(bestDist2);
        } else {
            for (int ii = 0; ii < 2; ++ii)
            {
                for (int ij = 0; ij < 2; ++ij)
                {
                    for (int ik = 0; ik < 2; ++ik)
                    {
                        tempf = thisOct->m_children[ii][ij][ik]->distToPoint(target);
                        if (tempf < bestDist || first)
                        {
                            myHeap.push(thisOct->m_children[ii][ij][ik], tempf);
                        }
                    }
                }
            }
        }
        if (myHeap.isEmpty())
        {
            break;//allows us to use top() without violating an assertion
        }
        myHeap.top(&curDist);//get the key for the next item
    }
    if (infoOut != NULL)
    {
        infoOut->whichSet = bestSet;
        infoOut->coords = bestPoint;
        infoOut->index = bestIndex;
    }
    return bestIndex;
}

int32_t CaretPointLocator::closestPointLimited(const float target[3], const float& maxDist, LocatorInfo* infoOut) const
{
    if (m_tree == NULL) return -1;
    float curDist2 = m_tree->distSquaredToPoint(target), maxDist2 = maxDist * maxDist;
    if (curDist2 > maxDist2)
    {
        if (infoOut != NULL)
        {
            infoOut->whichSet = -1;
            infoOut->index = -1;
        }
        return -1;
    }
    CaretSimpleMinHeap<Oct<LeafVector<Point> >*, float> myHeap;
    bool first = true;
    float bestDist2 = -1.0f, tempf;
    Vector3D bestPoint;
    int32_t bestSet = -1, bestIndex = -1;
    myHeap.push(m_tree, curDist2);
    while (curDist2 < bestDist2 || first)
    {
        Oct<LeafVector<Point> >* thisOct = myHeap.pop();
        if (thisOct->m_leaf)
        {
            vector<Point>& myVecRef = *(thisOct->m_data.m_vector);
            int curSize = (int)myVecRef.size();
            for (int i = 0; i < curSize; ++i)
            {
                tempf = MathFunctions::distanceSquared3D(myVecRef[i].m_point, target);
                if (tempf < bestDist2 || (first && tempf <= maxDist2))
                {
                    first = false;
                    bestDist2 = tempf;
                    bestPoint = myVecRef[i].m_point;
                    bestSet = myVecRef[i].m_mySet;
                    bestIndex = myVecRef[i].m_index;
                }
            }
        } else {
            for (int ii = 0; ii < 2; ++ii)
            {
                for (int ij = 0; ij < 2; ++ij)
                {
                    for (int ik = 0; ik < 2; ++ik)
                    {
                        tempf = thisOct->m_children[ii][ij][ik]->distSquaredToPoint(target);
                        if (tempf < bestDist2 || (first && tempf <= maxDist2))
                        {
                            myHeap.push(thisOct->m_children[ii][ij][ik], tempf);
                        }
                    }
                }
            }
        }
        if (myHeap.isEmpty())
        {
            break;//allows us to use top() without violating an assertion
        }
        myHeap.top(&curDist2);//get the key for the next item
    }
    if (infoOut != NULL)
    {
        infoOut->whichSet = bestSet;
        infoOut->coords = bestPoint;
        infoOut->index = bestIndex;
    }
    return bestIndex;
}

set<LocatorInfo> CaretPointLocator::pointsInRange(const float target[3], const float& maxDist) const
{
    set<LocatorInfo> ret;
    if (m_tree == NULL) return ret;
    float curDist2 = m_tree->distSquaredToPoint(target), maxDist2 = maxDist * maxDist;
    if (curDist2 > maxDist2) return ret;
    vector<Oct<LeafVector<Point> >*> myStack;//since we don't need the points sorted by distance
    myStack.push_back(m_tree);
    while (!myStack.empty())
    {
        Oct<LeafVector<Point> >* thisOct = myStack.back();
        myStack.pop_back();
        if (thisOct->m_leaf)
        {
            vector<Point>& myVecRef = *(thisOct->m_data.m_vector);
            int curSize = (int)myVecRef.size();
            for (int i = 0; i < curSize; ++i)
            {
                float tempf = MathFunctions::distanceSquared3D(myVecRef[i].m_point, target);
                if (tempf <= maxDist2)
                {
                    ret.insert(LocatorInfo(myVecRef[i].m_index, myVecRef[i].m_mySet, myVecRef[i].m_point));//let std::set sort out uniqueness
                }
            }
        } else {
            for (int ii = 0; ii < 2; ++ii)
            {
                for (int ij = 0; ij < 2; ++ij)
                {
                    for (int ik = 0; ik < 2; ++ik)
                    {
                        float tempf = thisOct->m_children[ii][ij][ik]->distSquaredToPoint(target);
                        if (tempf <= maxDist2)
                        {
                            myStack.push_back(thisOct->m_children[ii][ij][ik]);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

int32_t CaretPointLocator::newIndex()
{
    if (m_unusedIndexes.empty())
    {
        return m_nextSetIndex++;
    } else {
        int32_t ret = m_unusedIndexes[m_unusedIndexes.size() - 1];
        m_unusedIndexes.pop_back();
        return ret;
    }
}

void CaretPointLocator::removePointSet(int32_t whichSet)
{
    m_unusedIndexes.push_back(whichSet);
    removeSetHelper(m_tree, whichSet);
}

void CaretPointLocator::removeSetHelper(Oct<LeafVector<CaretPointLocator::Point> >* thisOct, int32_t thisSet)
{
    if (thisOct == NULL) return;
    if (thisOct->m_leaf)
    {
        vector<Point>& myVecRef = *(thisOct->m_data.m_vector);
        int curSize = (int)myVecRef.size();
        bool match = false;
        for (int i = 0; i < curSize; ++i)//make sure something gets removed, so we don't have to do an allocation if it isn't needed
        {
            if (myVecRef[i].m_mySet == thisSet)
            {
                match = true;
                break;
            }
        }
        if (match)
        {
            vector<Point> tempvec;
            tempvec.reserve(curSize - 1);//because at least one is getting removed
            for (int i = 0; i < curSize; ++i)
            {
                if (myVecRef[i].m_mySet != thisSet)
                {
                    tempvec.push_back(myVecRef[i]);
                }
            }
            myVecRef = tempvec;
        }
    } else {
        for (int ii = 0; ii < 2; ++ii)
        {
            for (int ij = 0; ij < 2; ++ij)
            {
                for (int ik = 0; ik < 2; ++ik)
                {
                    removeSetHelper(thisOct->m_children[ii][ij][ik], thisSet);
                }
            }
        }
    }
}
