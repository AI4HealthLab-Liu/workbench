
/*LICENSE_START*/
/*
 *  Copyright (C) 2019 Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#define __VOLUME_SURFACE_OUTLINE_MODEL_CACHE_KEY_DECLARE__
#include "VolumeSurfaceOutlineModelCacheKey.h"
#undef __VOLUME_SURFACE_OUTLINE_MODEL_CACHE_KEY_DECLARE__

#include "CaretAssert.h"
using namespace caret;


    
/**
 * \class caret::VolumeSurfaceOutlineModelCacheKey 
 * \brief Key for a cached volume surface outline model
 * \ingroup Brain
 */

/**
 * Constructor.
 */
VolumeSurfaceOutlineModelCacheKey::VolumeSurfaceOutlineModelCacheKey(const VolumeSliceViewPlaneEnum::Enum slicePlane,
                                                                     const float sliceCoordinate)
: CaretObject(),
m_slicePlane(slicePlane),
m_sliceCoordinateScaled(static_cast<int32_t>(sliceCoordinate * 10.0))
{
}

/**
 * Destructor.
 */
VolumeSurfaceOutlineModelCacheKey::~VolumeSurfaceOutlineModelCacheKey()
{
}

/**
 * Copy constructor.
 * @param obj
 *    Object that is copied.
 */
VolumeSurfaceOutlineModelCacheKey::VolumeSurfaceOutlineModelCacheKey(const VolumeSurfaceOutlineModelCacheKey& obj)
: CaretObject(obj)
{
    this->copyHelperVolumeSurfaceOutlineModelCacheKey(obj);
}

/**
 * Assignment operator.
 * @param obj
 *    Data copied from obj to this.
 * @return 
 *    Reference to this object.
 */
VolumeSurfaceOutlineModelCacheKey&
VolumeSurfaceOutlineModelCacheKey::operator=(const VolumeSurfaceOutlineModelCacheKey& obj)
{
    if (this != &obj) {
        CaretObject::operator=(obj);
        this->copyHelperVolumeSurfaceOutlineModelCacheKey(obj);
    }
    return *this;    
}

/**
 * Helps with copying an object of this type.
 * @param obj
 *    Object that is copied.
 */
void 
VolumeSurfaceOutlineModelCacheKey::copyHelperVolumeSurfaceOutlineModelCacheKey(const VolumeSurfaceOutlineModelCacheKey& obj)
{
    m_slicePlane = obj.m_slicePlane;
    m_sliceCoordinateScaled = obj.m_sliceCoordinateScaled;
}

/**
 * Equality operator.
 * @param obj
 *    Instance compared to this for equality.
 * @return 
 *    True if this instance and 'obj' instance are considered equal.
 */
bool
VolumeSurfaceOutlineModelCacheKey::operator==(const VolumeSurfaceOutlineModelCacheKey& obj) const
{
    if (this == &obj) {
        return true;    
    }

    /* perform equality testing HERE and return true if equal ! */
    if ((m_slicePlane == obj.m_slicePlane)
        && (m_sliceCoordinateScaled == obj.m_sliceCoordinateScaled)) {
        return true;
    }
    
    return false;    
}

/**
 * Less than operator.
 *
 * @param obj
 *    Instance compared to this for equality.
 * @return
 *    True if this instance and 'obj' instance are considered equal.
 */
bool
VolumeSurfaceOutlineModelCacheKey::operator<(const VolumeSurfaceOutlineModelCacheKey& obj) const
{
    if (this == &obj) {
        return false;
    }
    
    /* perform equality testing HERE and return true if equal ! */
    if (static_cast<int32_t>(m_slicePlane) < static_cast<int32_t>(obj.m_slicePlane)) {
        return true;
    }
    if (m_sliceCoordinateScaled < obj.m_sliceCoordinateScaled) {
        return true;
    }
    
    return false;
}

/**
 * Get a description of this object's content.
 * @return String describing this object's content.
 */
AString 
VolumeSurfaceOutlineModelCacheKey::toString() const
{
    QString str("Plane="
                + VolumeSliceViewPlaneEnum::toName(m_slicePlane)
                + ", "
                + "ScaledCoordinate="
                + QString::number(m_sliceCoordinateScaled));
    return str;
}

