
/*LICENSE_START*/
/*
 *  Copyright (C) 2017 Washington University School of Medicine
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

#define __GRAPHICS_PRIMITIVE_V3F_C4F_DECLARE__
#include "GraphicsPrimitiveV3fC4f.h"
#undef __GRAPHICS_PRIMITIVE_V3F_C4F_DECLARE__

#include "CaretAssert.h"
using namespace caret;


    
/**
 * \class caret::GraphicsPrimitiveV3fC4f 
 * \brief Primitive containing XYZ and Float RGBA.
 * \ingroup Graphics
 */

/**
 * Constructor.
 * 
 * @param primitiveType
 *     Type of primitive drawn (triangles, lines, etc.)
 */
GraphicsPrimitiveV3fC4f::GraphicsPrimitiveV3fC4f(const PrimitiveType primitiveType)
: GraphicsPrimitive(VertexType::FLOAT_XYZ,
                    NormalVectorType::NONE,
                    ColorType::FLOAT_RGBA,
                    primitiveType)
{
    
}

/**
 * Destructor.
 */
GraphicsPrimitiveV3fC4f::~GraphicsPrimitiveV3fC4f()
{
}

/**
 * Copy constructor.
 * @param obj
 *    Object that is copied.
 */
GraphicsPrimitiveV3fC4f::GraphicsPrimitiveV3fC4f(const GraphicsPrimitiveV3fC4f& obj)
: GraphicsPrimitive(obj)
{
    this->copyHelperGraphicsPrimitiveV3fC4f(obj);
}

/**
 * Helps with copying an object of this type.
 * @param obj
 *    Object that is copied.
 */
void 
GraphicsPrimitiveV3fC4f::copyHelperGraphicsPrimitiveV3fC4f(const GraphicsPrimitiveV3fC4f& obj)
{
    
}

/**
 * Add a vertex.
 * 
 * @param xyz
 *     Coordinate of vertex.
 * @param rgba
 *     RGBA color components ranging 0.0 to 1.0.
 */
void
GraphicsPrimitiveV3fC4f::addVertex(const float xyz[3],
                                   const float rgba[4])
{
    m_xyz.insert(m_xyz.end(),
                 xyz, xyz + 3);
    m_floatRGBA.insert(m_floatRGBA.end(),
                       rgba, rgba + 4);
}

/**
 * Add a vertex.
 *
 * @param x
 *     X-coordinate of vertex.
 * @param y
 *     Y-coordinate of vertex.
 * @param z
 *     Z-coordinate of vertex.
 * @param rgba
 *     RGBA color components ranging 0.0 to 1.0.
 */
void
GraphicsPrimitiveV3fC4f::addVertex(const float x,
                                   const float y,
                                   const float z,
                                   const float rgba[4])
{
    m_xyz.push_back(x);
    m_xyz.push_back(y);
    m_xyz.push_back(z);
    m_floatRGBA.insert(m_floatRGBA.end(),
                       rgba, rgba + 4);
}

/**
 * Add a 2D vertex.  Z will be zero.
 *
 * @param x
 *     X-coordinate of vertex.
 * @param y
 *     Y-coordinate of vertex.
 * @param rgba
 *     RGBA color components ranging 0.0 to 1.0.
 */
void
GraphicsPrimitiveV3fC4f::addVertex(const float x,
                                   const float y,
                                   const float rgba[4])
{
    m_xyz.push_back(x);
    m_xyz.push_back(y);
    m_xyz.push_back(0.0);
    m_floatRGBA.insert(m_floatRGBA.end(),
                       rgba, rgba + 4);
}

/**
 * Clone this primitive.
 */
GraphicsPrimitive*
GraphicsPrimitiveV3fC4f::clone() const
{
    GraphicsPrimitiveV3fC4f* obj = new GraphicsPrimitiveV3fC4f(*this);
    return obj;
}

