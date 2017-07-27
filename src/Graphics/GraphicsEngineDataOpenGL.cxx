
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

#define __GRAPHICS_ENGINE_DATA_OPEN_G_L_DECLARE__
#include "GraphicsEngineDataOpenGL.h"
#undef __GRAPHICS_ENGINE_DATA_OPEN_G_L_DECLARE__

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "EventGraphicsOpenGLCreateBufferObject.h"
#include "EventGraphicsOpenGLCreateTextureName.h"
#include "EventManager.h"
#include "GraphicsOpenGLBufferObject.h"
#include "GraphicsOpenGLTextureName.h"
#include "GraphicsPrimitive.h"
#include "GraphicsPrimitiveSelectionHelper.h"

using namespace caret;


    
/**
 * \class caret::GraphicsEngineDataOpenGL 
 * \brief Contains data the OpenGL Graphics Engine may associate with primtive data.
 * \ingroup Graphics
 */

/**
 * Constructor.
 *
 * @param openglContextPointer
 *     Context to which the OpenGL buffers apply.  The current usage of QGLWidget
 *     shares the OpenGL context among all QGLWidgets.  However, in the event
 *     OpenGL contexts are not shared, this implementation will need to be 
 *     updated.
 */
GraphicsEngineDataOpenGL::GraphicsEngineDataOpenGL(const void* openglContextPointer)
: GraphicsEngineData(),
m_openglContextPointer(openglContextPointer)
{
    
}

/**
 * Destructor.
 */
GraphicsEngineDataOpenGL::~GraphicsEngineDataOpenGL()
{
    deleteBuffers();
}

/**
 * Delete an OpenGL buffer object.
 *
 * @param bufferObject
 *     Reference to pointer of buffer object for deletion.
 *     If NULL, no action is taken.
 *     Will be NULL upon exit.
 */
void
GraphicsEngineDataOpenGL::deleteBufferObjectHelper(GraphicsOpenGLBufferObject* &bufferObject)
{
    if (bufferObject != NULL) {
        delete bufferObject;
        bufferObject = NULL;
    }
}

/**
 * Delete the OpenGL buffer objects.
 */
void
GraphicsEngineDataOpenGL::deleteBuffers()
{
    deleteBufferObjectHelper(m_coordinateBufferObject);
    
    deleteBufferObjectHelper(m_normalVectorBufferObject);
    
    deleteBufferObjectHelper(m_colorBufferObject);
    
    deleteBufferObjectHelper(m_textureCoordinatesBufferObject);
    
    for (auto mapBuff: m_alternativeColorBufferObjectMap) {
        deleteBufferObjectHelper(mapBuff.second);
    }
    m_alternativeColorBufferObjectMap.clear();
    
    if (m_textureImageDataName != NULL) {
        delete m_textureImageDataName;
        m_textureImageDataName = NULL;
    }
}

/**
 * Invalidate the coordinates after they have
 * changed in the graphics primitive.
 */
void
GraphicsEngineDataOpenGL::invalidateCoordinates()
{
    m_reloadCoordinatesFlag = true;
}

/**
 * Get the OpenGL Buffer Usage Hint from the primitive.
 *
 * @param primitive
 *     The graphics primitive
 * @return
 *     The OpenGL Buffer Usage Hint.
 */
GLenum
GraphicsEngineDataOpenGL::getOpeGLBufferUsageHint(const GraphicsPrimitive* primitive) const
{
    GLenum usageHint = GL_STREAM_DRAW;
    switch (primitive->getUsageType()) {
        case GraphicsPrimitive::UsageType::MODIFIED_ONCE_DRAWN_FEW_TIMES:
            usageHint = GL_STREAM_DRAW;
            break;
        case GraphicsPrimitive::UsageType::MODIFIED_ONCE_DRAWN_MANY_TIMES:
            usageHint = GL_STATIC_DRAW;
            break;
        case GraphicsPrimitive::UsageType::MODIFIED_MANY_DRAWN_MANY_TIMES:
            usageHint = GL_DYNAMIC_DRAW;
            break;
    }
    
    return usageHint;
}

/**
 * Load the coordinate buffer.
 *
 * @param primitive
 *     The graphics primitive that will be drawn.
 */
void
GraphicsEngineDataOpenGL::loadCoordinateBuffer(GraphicsPrimitive* primitive)
{
    CaretAssert(primitive);
    
    GLenum usageHint = getOpeGLBufferUsageHint(primitive);
    
    
    GLsizei coordinateCount = 0;
    switch (primitive->m_vertexType) {
        case GraphicsPrimitive::VertexType::FLOAT_XYZ:
            m_coordinateDataType = GL_FLOAT;
            m_coordinatesPerVertex = 3; // X, Y, Z
            
            coordinateCount = primitive->m_xyz.size();
            const GLuint xyzSizeBytes = coordinateCount * sizeof(float);
            CaretAssert(xyzSizeBytes > 0);
            const GLvoid* xyzDataPointer = (const GLvoid*)&primitive->m_xyz[0];
            
            /*
             * Coordinate buffer may have been created.
             * Coordinates may change but the number of coordinates does not change.
             */
            if (m_coordinateBufferObject == NULL) {
                EventGraphicsOpenGLCreateBufferObject createEvent;
                EventManager::get()->sendEvent(createEvent.getPointer());
                m_coordinateBufferObject = createEvent.getOpenGLBufferObject();
                CaretAssert(m_coordinateBufferObject);
            }
            
            CaretAssert(m_coordinateBufferObject->getBufferObjectName());
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         m_coordinateBufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         xyzSizeBytes,
                         xyzDataPointer,
                         usageHint);
            break;
    }
    
    if (m_coordinatesPerVertex > 0) {
        m_arrayIndicesCount = coordinateCount / m_coordinatesPerVertex;
    }
    else {
        m_arrayIndicesCount = 0;
    }
    
    m_reloadCoordinatesFlag = false;
}

/**
 * Load the normal vector buffer.
 *
 * @param primitive
 *     The graphics primitive that will be drawn.
 */
void
GraphicsEngineDataOpenGL::loadNormalVectorBuffer(GraphicsPrimitive* primitive)
{
    CaretAssert(primitive);
    
    GLenum usageHint = getOpeGLBufferUsageHint(primitive);
    
    
    switch (primitive->m_normalVectorType) {
        case GraphicsPrimitive::NormalVectorType::NONE:
            break;
        case GraphicsPrimitive::NormalVectorType::FLOAT_XYZ:
        {
            m_normalVectorDataType = GL_FLOAT;
            const GLuint normalSizeBytes = primitive->m_floatNormalVectorXYZ.size() * sizeof(float);
            CaretAssert(normalSizeBytes > 0);
            const GLvoid* normalDataPointer = (const GLvoid*)&primitive->m_floatNormalVectorXYZ[0];
            
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            m_normalVectorBufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(m_normalVectorBufferObject->getBufferObjectName());
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         m_normalVectorBufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         normalSizeBytes,
                         normalDataPointer,
                         usageHint);
        }
            break;
    }
}

/**
 * Load the color buffer.
 * @param primitive
 *     The graphics primitive that will be drawn.
 */
void
GraphicsEngineDataOpenGL::loadColorBuffer(GraphicsPrimitive* primitive)
{
    CaretAssert(primitive);
    
    GLenum usageHint = getOpeGLBufferUsageHint(primitive);
    
    
    switch (primitive->m_colorType) {
        case GraphicsPrimitive::ColorType::NONE:
            break;
        case GraphicsPrimitive::ColorType::FLOAT_RGBA:
        {
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            m_colorBufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(m_colorBufferObject->getBufferObjectName());
            
            m_componentsPerColor = 4;
            m_colorDataType = GL_FLOAT;
            
            const GLuint colorSizeBytes = primitive->m_floatRGBA.size() * sizeof(float);
            CaretAssert(colorSizeBytes > 0);
            const GLvoid* colorDataPointer = (const GLvoid*)&primitive->m_floatRGBA[0];
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         m_colorBufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         colorSizeBytes,
                         colorDataPointer,
                         usageHint);
        }
            break;
        case GraphicsPrimitive::ColorType::UNSIGNED_BYTE_RGBA:
        {
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            m_colorBufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(m_colorBufferObject->getBufferObjectName());
            
            m_componentsPerColor = 4;
            m_colorDataType = GL_UNSIGNED_BYTE;
            
            const GLuint colorSizeBytes = primitive->m_unsignedByteRGBA.size() * sizeof(uint8_t);
            CaretAssert(colorSizeBytes > 0);
            const GLvoid* colorDataPointer = (const GLvoid*)&primitive->m_unsignedByteRGBA[0];
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         m_colorBufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         colorSizeBytes,
                         colorDataPointer,
                         usageHint);
        }
            break;
    }
}

/**
 * Load the texture coordinate buffer.
 * @param primitive
 *     The graphics primitive that will be drawn.
 */
void
GraphicsEngineDataOpenGL::loadTextureCoordinateBuffer(GraphicsPrimitive* primitive)
{
    CaretAssert(primitive);
    
    GLenum usageHint = getOpeGLBufferUsageHint(primitive);
    
    switch (primitive->m_textureType) {
        case GraphicsPrimitive::TextureType::FLOAT_STR:
        {
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            m_textureCoordinatesBufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(m_textureCoordinatesBufferObject->getBufferObjectName());
            
            m_textureCoordinatesDataType = GL_FLOAT;
            const GLuint textureSizeBytes = primitive->m_floatTextureSTR.size() * sizeof(float);
            CaretAssert(textureSizeBytes > 0);
            const GLvoid* textureDataPointer = (const GLvoid*)&primitive->m_floatTextureSTR[0];
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         m_textureCoordinatesBufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         textureSizeBytes,
                         textureDataPointer,
                         usageHint);
            
        }
            break;
        case GraphicsPrimitive::TextureType::NONE:
            break;
    }    
}


/**
 * Load the buffers with data from the grpahics primitive.
 *
 * @param primitive
 *     The graphics primitive that will be drawn.
 */
void
GraphicsEngineDataOpenGL::loadAllBuffers(GraphicsPrimitive* primitive)
{
    loadCoordinateBuffer(primitive);
    loadNormalVectorBuffer(primitive);
    loadColorBuffer(primitive);
    loadTextureCoordinateBuffer(primitive);    
}

/**
 * If needed, load the texture buffer.
 * Note that the texture buffer may be invalidated when
 * an image is captures as the image capture wipes out all
 * textures.
 *
 * @param primitive
 *     The graphics primitive that will be drawn.
*/
void
GraphicsEngineDataOpenGL::loadTextureImageDataBuffer(GraphicsPrimitive* primitive)
{
    if (m_textureImageDataName != NULL) {
        return;
    }
    
    switch (primitive->m_textureType) {
        case GraphicsPrimitive::TextureType::FLOAT_STR:
        {
            const int32_t imageWidth  = primitive->m_textureImageWidth;
            const int32_t imageHeight = primitive->m_textureImageHeight;
            const int32_t imageNumberOfBytes = imageWidth * imageHeight * 4;
            if (imageNumberOfBytes <= 0) {
                CaretLogWarning("Invalid texture (empty data) for drawing primitive with texture");
                return;
            }
            if (imageNumberOfBytes != static_cast<int32_t>(primitive->m_textureImageBytesRGBA.size())) {
                CaretLogWarning("Image bytes size incorrect for image width/height");
                return;
            }
            
            glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
            
            const GLubyte* imageBytesRGBA = &primitive->m_textureImageBytesRGBA[0];
            EventGraphicsOpenGLCreateTextureName createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            m_textureImageDataName = createEvent.getOpenGLTextureName();
            CaretAssert(m_textureImageDataName);
            
            const GLuint openGLTextureName = m_textureImageDataName->getTextureName();
            
            glBindTexture(GL_TEXTURE_2D, openGLTextureName);
            
            bool useMipMapFlag = true;
            if (useMipMapFlag) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                
                /*
                 * This code seems to work if OpenGL 3.0 or later and
                 * replaces gluBuild2DMipmaps()
                 *
                 * glTexImage2D(GL_TEXTURE_2D,     // MUST BE GL_TEXTURE_2D
                 *            0,                 // level of detail 0=base, n is nth mipmap reduction
                 *            GL_RGBA,           // number of components
                 *            imageWidth,        // width of image
                 *            imageHeight,       // height of image
                 *            0,                 // border
                 *            GL_RGBA,           // format of the pixel data
                 *            GL_UNSIGNED_BYTE,  // data type of pixel data
                 *            imageBytesRGBA);   // pointer to image data
                 * glGenerateMipmap(GL_TEXTURE_2D);
                 */
                
                
                const int errorCode = gluBuild2DMipmaps(GL_TEXTURE_2D,     // MUST BE GL_TEXTURE_2D
                                                        GL_RGBA,           // number of components
                                                        imageWidth,        // width of image
                                                        imageHeight,       // height of image
                                                        GL_RGBA,           // format of the pixel data
                                                        GL_UNSIGNED_BYTE,  // data type of pixel data
                                                        imageBytesRGBA);    // pointer to image data
                if (errorCode != 0) {
                    useMipMapFlag = false;
                    
                    const GLubyte* errorChars = gluErrorString(errorCode);
                    if (errorChars != NULL) {
                        const QString errorText = ("ERROR building mipmaps for annotation image: "
                                                   + AString((char*)errorChars));
                        CaretLogSevere(errorText);
                    }
                }
            }
            
            if ( ! useMipMapFlag) {
                CaretAssert(0);   // image must be 2^N by 2^M
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                
                glTexImage2D(GL_TEXTURE_2D,     // MUST BE GL_TEXTURE_2D
                             0,                 // level of detail 0=base, n is nth mipmap reduction
                             GL_RGBA,           // number of components
                             imageWidth,        // width of image
                             imageHeight,       // height of image
                             0,                 // border
                             GL_RGBA,           // format of the pixel data
                             GL_UNSIGNED_BYTE,  // data type of pixel data
                             imageBytesRGBA);   // pointer to image data
            }

            glBindTexture(GL_TEXTURE_2D, 0);
            glPopClientAttrib();
        }
            break;
        case GraphicsPrimitive::TextureType::NONE:
            break;
    }
}


/**
 * Load the buffer for the given alternative color identifier.  If the buffer
 * was previously loaded, no action is taken.
 *
 * @param primitive
 *     The graphics primitive that will be drawn.
 * @param alternativeColorIdentifier
 *     Identifier for the alternative color.
 * @return
 *     True if the alternative color buffer is valid for the given alternative 
 *     color identifier so drawing can be performed.  Otherwise, false is returned.
 */
GraphicsOpenGLBufferObject*
GraphicsEngineDataOpenGL::loadAlternativeColorBuffer(GraphicsPrimitive* primitive,
                                                     const int32_t alternativeColorIdentifier)
{
    std::map<int32_t, GraphicsOpenGLBufferObject*>::iterator iter = m_alternativeColorBufferObjectMap.find(alternativeColorIdentifier);
    if (iter != m_alternativeColorBufferObjectMap.end()) {
        return iter->second;
    }
    
    GraphicsOpenGLBufferObject* outputOpenGLBufferObject = NULL;
    
    GLenum usageHint = getOpeGLBufferUsageHint(primitive);
    
    switch (primitive->m_colorType) {
        case GraphicsPrimitive::ColorType::NONE:
            CaretAssert(0);
            break;
        case GraphicsPrimitive::ColorType::FLOAT_RGBA:
        {
            const std::vector<float>& alternativeFloatRGBA = primitive->getAlternativeFloatRGBAProtected(alternativeColorIdentifier);
            if ((alternativeFloatRGBA.size() / 4) != (primitive->m_xyz.size() / 3)) {
                const AString msg("Number of alternative RGBA values is for "
                                  + AString::number((alternativeFloatRGBA.size() / 4))
                                  + " vertices but should be for "
                                  + AString::number((primitive->m_xyz.size() / 3))
                                  + " vertices.");
                CaretAssertMessage(0, msg);
                CaretLogSevere(msg);
                return NULL;
            }
            
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            GraphicsOpenGLBufferObject* bufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(bufferObject->getBufferObjectName());
            
            m_componentsPerColor = 4;
            m_colorDataType = GL_FLOAT;
            
            const GLuint colorSizeBytes = alternativeFloatRGBA.size() * sizeof(float);
            const GLvoid* colorDataPointer = (const GLvoid*)&alternativeFloatRGBA[0];
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         bufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         colorSizeBytes,
                         colorDataPointer,
                         usageHint);
            
            m_alternativeColorBufferObjectMap.insert(std::make_pair(alternativeColorIdentifier,
                                                                    bufferObject));
            outputOpenGLBufferObject = bufferObject;
        }
            break;
        case GraphicsPrimitive::ColorType::UNSIGNED_BYTE_RGBA:
        {
            const std::vector<uint8_t>& alternativeUnsignedByteRGBA = primitive->getAlternativeUnsignedByteRGBAProtected(alternativeColorIdentifier);
            if ((alternativeUnsignedByteRGBA.size() / 4) != (primitive->m_xyz.size() / 3)) {
                const AString msg("Number of alternative RGBA values is for "
                                  + AString::number((alternativeUnsignedByteRGBA.size() / 4))
                                  + " vertices but should be for "
                                  + AString::number((primitive->m_xyz.size() / 3))
                                  + " vertices.");
                CaretAssertMessage(0, msg);
                CaretLogSevere(msg);
                return NULL;
            }
            
            EventGraphicsOpenGLCreateBufferObject createEvent;
            EventManager::get()->sendEvent(createEvent.getPointer());
            GraphicsOpenGLBufferObject* bufferObject = createEvent.getOpenGLBufferObject();
            CaretAssert(bufferObject->getBufferObjectName());
            
            m_componentsPerColor = 4;
            m_colorDataType = GL_UNSIGNED_BYTE;
            
            const GLuint colorSizeBytes = alternativeUnsignedByteRGBA.size() * sizeof(uint8_t);
            const GLvoid* colorDataPointer = (const GLvoid*)&alternativeUnsignedByteRGBA[0];
            
            glBindBuffer(GL_ARRAY_BUFFER,
                         bufferObject->getBufferObjectName());
            glBufferData(GL_ARRAY_BUFFER,
                         colorSizeBytes,
                         colorDataPointer,
                         usageHint);
            
            m_alternativeColorBufferObjectMap.insert(std::make_pair(alternativeColorIdentifier,
                                                                    bufferObject));
            outputOpenGLBufferObject = bufferObject;
        }
            break;
    }
    
    return outputOpenGLBufferObject;
}


/**
 * Draw the given graphics primitive.
 *
 * @param openglContextPointer
 *     Pointer to the active OpenGL context.
 * @param primitive
 *     Primitive that is drawn.
 */
void
GraphicsEngineDataOpenGL::draw(void* openglContextPointer,
                               GraphicsPrimitive* primitive)
{
    float dummyRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    drawPrivate(PrivateDrawMode::DRAW_NORMAL,
                openglContextPointer,
                primitive,
                NULL,
                -1,
                dummyRGBA);
}

/**
 * Draw to determine the index of primitive at the given (X, Y) location
 *
 * @param openglContextPointer
 *     Pointer to the active OpenGL context.
 * @param primitive
 *     Primitive that is drawn.
 * @param pixelX
 *     X location for selection.
 * @param pixelY
 *     Y location for selection.
 * @param selectedPrimitiveIndexOut
 *     Output with the index of the primitive (invalid no primitve at location
 * @param selectedPrimitiveDepthOut
 *     Output with depth at the location.
 */
void
GraphicsEngineDataOpenGL::drawWithSelection(void* openglContextPointer,
                                            GraphicsPrimitive* primitive,
                                            const int32_t pixelX,
                                            const int32_t pixelY,
                                            int32_t& selectedPrimitiveIndexOut,
                                            float&   selectedPrimitiveDepthOut)
{
    CaretAssert(primitive);
    
    selectedPrimitiveIndexOut = -1;
    selectedPrimitiveDepthOut = 0.0;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,
                  viewport);
    if ((pixelX    >= viewport[0])
        && (pixelX <  (viewport[0] + viewport[2]))
        && (pixelY >= viewport[1])
        && (pixelY <  (viewport[1] + viewport[3]))) {
        /*
         * Saves glPixelStore parameters
         */
        glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
        glPushAttrib(GL_LIGHTING_BIT);
        
        glShadeModel(GL_FLAT);
        glDisable(GL_LIGHTING);
        
        float dummyRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        GraphicsPrimitiveSelectionHelper selectionHelper(primitive);
        selectionHelper.setupSelectionBeforeDrawing();
        drawPrivate(PrivateDrawMode::DRAW_SELECTION,
                    openglContextPointer,
                    primitive,
                    &selectionHelper,
                    -1,
                    dummyRGBA);
        
        /*
         * QOpenGLWidget Note: The QOpenGLWidget always renders in a
         * frame buffer object (see its documentation).  This is
         * probably why calls to glReadBuffer() always cause an
         * OpenGL error.
         */
#ifdef WORKBENCH_USE_QT5_QOPENGL_WIDGET
        /* do not call glReadBuffer() */
#else
        glReadBuffer(GL_BACK);
#endif
        uint8_t pixelRGBA[4];

        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1); // bytes
        glReadPixels(pixelX,
                     pixelY,
                     1,
                     1,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     pixelRGBA);
        
        /*
         * Get depth from depth buffer
         */
        glPixelStorei(GL_PACK_ALIGNMENT, 4); // float
        glReadPixels(pixelX,
                     pixelY,
                     1,
                     1,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     &selectedPrimitiveDepthOut);
        
        glPopAttrib();
        glPopClientAttrib();
        
        selectedPrimitiveIndexOut = selectionHelper.getPrimitiveIndexFromEncodedRGBA(pixelRGBA);
    }
}


/**
 * Draw the given graphics primitive using a solid color.
 *
 * @param openglContextPointer
 *     Pointer to the active OpenGL context.
 * @param primitive
 *     Primitive that is drawn.
 * @param solidColorOverrideRGBA
 *     Color that is applied to all vertices.
 */
void
GraphicsEngineDataOpenGL::drawWithOverrideColor(void* openglContextPointer,
                                                GraphicsPrimitive* primitive,
                                                const float solidColorOverrideRGBA[4])
{
    drawPrivate(PrivateDrawMode::DRAW_COLOR_SOLID,
                openglContextPointer,
                primitive,
                NULL,
                -1,
                solidColorOverrideRGBA);
}

/**
 * Draw the given graphics primitive using an alternative color.
 *
 * @param openglContextPointer
 *     Pointer to the active OpenGL context.
 * @param primitive
 *     Primitive that is drawn.
 * @param alternativeColorIdentifier
 *     Identifier for the alternative color.
 */
void
GraphicsEngineDataOpenGL::drawWithAlternativeColor(void* openglContextPointer,
                                                   GraphicsPrimitive* primitive,
                                                   const int32_t alternativeColorIdentifier)
{
    float dummyRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    drawPrivate(PrivateDrawMode::DRAW_COLOR_ALTERNATIVE,
                openglContextPointer,
                primitive, NULL,
                alternativeColorIdentifier,
                dummyRGBA);
}


/**
 * Draw the graphics primitive.
 *
 * @param drawMode
 *     Mode for drawing.
 * @param openglContextPointer
 *     Pointer to the active OpenGL context.
 * @param primitive
 *     Primitive that is drawn.
 * @param primitiveSelectionHelper
 *     Selection helper when draw mode is selection.
 * @param alternativeColorIdentifier
 *     Identifier for the alternative color.
 * @param solidColorRGBA
 *     Color for solid color mode.
 */
void
GraphicsEngineDataOpenGL::drawPrivate(const PrivateDrawMode drawMode,
                                      void* openglContextPointer,
                                      GraphicsPrimitive* primitive,
                                      GraphicsPrimitiveSelectionHelper* primitiveSelectionHelper,
                                      const int32_t alternativeColorIdentifier,
                                      const float solidColorRGBA[4])
{
    CaretAssert(openglContextPointer);
    CaretAssert(primitive);
    if ( ! primitive->isValid()) {
        return;
    }
    
    switch (drawMode) {
        case PrivateDrawMode::DRAW_COLOR_ALTERNATIVE:
            if ( ! primitive->isAlternativeColoringValid(alternativeColorIdentifier)) {
                CaretLogSevere("Alternative coloring is invalid for index="
                               + AString::number(alternativeColorIdentifier));
                return;
            }
            break;
        case PrivateDrawMode::DRAW_COLOR_SOLID:
            break;
        case PrivateDrawMode::DRAW_NORMAL:
            break;
        case PrivateDrawMode::DRAW_SELECTION:
            break;
    }
    
    GraphicsEngineDataOpenGL* openglData = primitive->getGraphicsEngineDataForOpenGL();
    if (openglData == NULL) {
        openglData = new GraphicsEngineDataOpenGL(openglContextPointer);
        primitive->setGraphicsEngineDataForOpenGL(openglData);
        
        openglData->loadAllBuffers(primitive);
    }
    else {
        /*
         * Coordinates may get updated.
         */
        if (openglData->m_reloadCoordinatesFlag) {
            openglData->loadCoordinateBuffer(primitive);
        }
    }
    
    openglData->loadTextureImageDataBuffer(primitive);
    
    if (openglContextPointer != openglData->getOpenGLContextPointer()) {
        const AString msg("Only one OpenGL graphics context is allowed.  Have there been "
                          "changes made to QGLWidget creation?");
        CaretAssertMessage(0, msg);
        CaretLogSevere(msg);
        return;
    }
    
    const GLuint coordBufferID = openglData->m_coordinateBufferObject->getBufferObjectName();
    CaretAssert(coordBufferID);
    if ( ! glIsBuffer(coordBufferID)) {
        CaretAssertMessage(0, "Coordinate buffer is INVALID");
        CaretLogSevere("Coordinate buffer is INVALID");
        return;
    }
    
    GraphicsOpenGLBufferObject* alternativeColorBufferObject = NULL;
    switch (drawMode) {
        case PrivateDrawMode::DRAW_COLOR_ALTERNATIVE:
            alternativeColorBufferObject = openglData->loadAlternativeColorBuffer(primitive,
                                                                                  alternativeColorIdentifier);
            if (alternativeColorBufferObject == NULL) {
                CaretAssertMessage(0, "Coordinate buffer is INVALID");
                CaretLogSevere("Coordinate buffer is INVALID");
                return;
            }
            break;
        case PrivateDrawMode::DRAW_COLOR_SOLID:
            break;
        case PrivateDrawMode::DRAW_NORMAL:
            break;
        case PrivateDrawMode::DRAW_SELECTION:
            break;
    }
    
    /*
     * Setup vertices for drawing
     */
    glEnableClientState(GL_VERTEX_ARRAY);
    CaretAssert(glIsBuffer(coordBufferID));
    glBindBuffer(GL_ARRAY_BUFFER,
                 coordBufferID);
    glVertexPointer(openglData->m_coordinatesPerVertex,
                    openglData->m_coordinateDataType,
                    0,
                    (GLvoid*)0);
    
    /*
     * Setup normals for drawing
     */
    if (openglData->m_normalVectorBufferObject != NULL) {
        glEnableClientState(GL_NORMAL_ARRAY);
        CaretAssert(glIsBuffer(openglData->m_normalVectorBufferObject->getBufferObjectName()));
        glBindBuffer(GL_ARRAY_BUFFER, openglData->m_normalVectorBufferObject->getBufferObjectName());
        glNormalPointer(openglData->m_normalVectorDataType, 0, (GLvoid*)0);
    }
    else {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    
    bool hasColorFlag = false;
    switch (primitive->getColorType()) {
        case GraphicsPrimitive::ColorType::NONE:
            break;
        case GraphicsPrimitive::ColorType::FLOAT_RGBA:
            hasColorFlag = true;
            break;
        case GraphicsPrimitive::ColorType::UNSIGNED_BYTE_RGBA:
            hasColorFlag = true;
            break;
    }
    /*
     * Setup colors for drawing
     */
    GLuint localColorBufferName = 0;
    if (hasColorFlag) {
        switch (drawMode) {
            case PrivateDrawMode::DRAW_COLOR_ALTERNATIVE:
            {
                glEnableClientState(GL_COLOR_ARRAY);
                CaretAssert(glIsBuffer(alternativeColorBufferObject->getBufferObjectName()));
                glBindBuffer(GL_ARRAY_BUFFER, alternativeColorBufferObject->getBufferObjectName());
                glColorPointer(openglData->m_componentsPerColor, openglData->m_colorDataType, 0, (GLvoid*)0);
            }
                break;
            case PrivateDrawMode::DRAW_COLOR_SOLID:
            {
                const int32_t numberOfVertices = primitive->m_xyz.size() / 3;
                if (numberOfVertices > 0) {
                    glGenBuffers(1, &localColorBufferName);
                    CaretAssert(localColorBufferName > 0);
                    
                    const GLint  componentsPerColor = 4;
                    const GLenum colorDataType = GL_FLOAT;
                    
                    std::vector<float> colorRGBA(numberOfVertices * componentsPerColor);
                    for (int32_t i = 0; i < numberOfVertices; i++) {
                        const int32_t i4 = i * 4;
                        CaretAssertVectorIndex(colorRGBA, i4 + 3);
                        colorRGBA[i4]   = solidColorRGBA[0];
                        colorRGBA[i4+1] = solidColorRGBA[1];
                        colorRGBA[i4+2] = solidColorRGBA[2];
                        colorRGBA[i4+3] = solidColorRGBA[3];
                    }
                    
                    const GLuint colorSizeBytes = colorRGBA.size() * sizeof(float);
                    const GLvoid* colorDataPointer = (const GLvoid*)&colorRGBA[0];
                    glBindBuffer(GL_ARRAY_BUFFER,
                                 localColorBufferName);
                    glBufferData(GL_ARRAY_BUFFER,
                                 colorSizeBytes,
                                 colorDataPointer,
                                 GL_STREAM_DRAW);
                    glEnableClientState(GL_COLOR_ARRAY);
                    glColorPointer(componentsPerColor, colorDataType, 0, (GLvoid*)0);
                }
            }
                break;
            case PrivateDrawMode::DRAW_NORMAL:
            {
                glEnableClientState(GL_COLOR_ARRAY);
                CaretAssert(glIsBuffer(openglData->m_colorBufferObject->getBufferObjectName()));
                glBindBuffer(GL_ARRAY_BUFFER, openglData->m_colorBufferObject->getBufferObjectName());
                glColorPointer(openglData->m_componentsPerColor, openglData->m_colorDataType, 0, (GLvoid*)0);
            }
                break;
            case PrivateDrawMode::DRAW_SELECTION:
            {
                const int32_t numberOfVertices = primitive->m_xyz.size() / 3;
                if (numberOfVertices > 0) {
                    glGenBuffers(1, &localColorBufferName);
                    CaretAssert(localColorBufferName > 0);
                    
                    const GLint  componentsPerColor = 4;
                    const GLenum colorDataType = GL_UNSIGNED_BYTE;
                    
                    const std::vector<uint8_t> selectionRGBA = primitiveSelectionHelper->getSelectionEncodedRGBA();
                    CaretAssert((selectionRGBA.size() / 4) == numberOfVertices);
                    
                    const GLuint colorSizeBytes = selectionRGBA.size() * sizeof(GLubyte);
                    const GLvoid* colorDataPointer = (const GLvoid*)&selectionRGBA[0];
                    glBindBuffer(GL_ARRAY_BUFFER,
                                 localColorBufferName);
                    glBufferData(GL_ARRAY_BUFFER,
                                 colorSizeBytes,
                                 colorDataPointer,
                                 GL_STREAM_DRAW);
                    glEnableClientState(GL_COLOR_ARRAY);
                    glColorPointer(componentsPerColor, colorDataType, 0, (GLvoid*)0);
                }
            }
                break;
        }
    }
    
    bool hasTextureFlag = false;
    switch (primitive->getTextureType()) {
        case GraphicsPrimitive::TextureType::NONE:
            break;
        case GraphicsPrimitive::TextureType::FLOAT_STR:
            hasTextureFlag = true;
            break;
    }
    
    /*
     * Setup textures for drawing
     */
    if (hasTextureFlag
        && (drawMode == PrivateDrawMode::DRAW_NORMAL)
        && (openglData->m_textureImageDataName->getTextureName() > 0)) {
            glEnable(GL_TEXTURE_2D);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glBindTexture(GL_TEXTURE_2D, openglData->m_textureImageDataName->getTextureName());
            
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            CaretAssert(glIsBuffer(openglData->m_textureCoordinatesBufferObject->getBufferObjectName()));
            glBindBuffer(GL_ARRAY_BUFFER, openglData->m_textureCoordinatesBufferObject->getBufferObjectName());
        glTexCoordPointer(3, openglData->m_textureCoordinatesDataType, 0, (GLvoid*)0);
    }
     
    /*
     * Unbind buffers
     */
    glBindBuffer(GL_ARRAY_BUFFER,
                 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 0);
    
    /*
     * Draw the primitive
     */
    GLenum openGLPrimitiveType = GL_INVALID_ENUM;
    switch (primitive->m_primitiveType) {
        case GraphicsPrimitive::PrimitiveType::LINE_LOOP:
            openGLPrimitiveType = GL_LINE_LOOP;
            break;
        case GraphicsPrimitive::PrimitiveType::LINE_STRIP:
            openGLPrimitiveType = GL_LINE_STRIP;
            break;
        case GraphicsPrimitive::PrimitiveType::LINES:
            openGLPrimitiveType = GL_LINES;
            break;
        case GraphicsPrimitive::PrimitiveType::POINTS:
            openGLPrimitiveType = GL_POINTS;
            break;
        case GraphicsPrimitive::PrimitiveType::POLYGON:
            openGLPrimitiveType = GL_POLYGON;
            break;
        case GraphicsPrimitive::PrimitiveType::QUAD_STRIP:
            openGLPrimitiveType = GL_QUAD_STRIP;
            break;
        case GraphicsPrimitive::PrimitiveType::QUADS:
            openGLPrimitiveType = GL_QUADS;
            break;
        case GraphicsPrimitive::PrimitiveType::TRIANGLE_FAN:
            openGLPrimitiveType = GL_TRIANGLE_FAN;
            break;
        case GraphicsPrimitive::PrimitiveType::TRIANGLE_STRIP:
            openGLPrimitiveType = GL_TRIANGLE_STRIP;
            break;
        case GraphicsPrimitive::PrimitiveType::TRIANGLES:
            openGLPrimitiveType = GL_TRIANGLES;
            break;
    }
    
    CaretAssert(openGLPrimitiveType != GL_INVALID_ENUM);
    glDrawArrays(openGLPrimitiveType, 0, openglData->m_arrayIndicesCount);
    
    /*
     * Disable drawing
     */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    if (localColorBufferName > 0) {
        glDeleteBuffers(1, &localColorBufferName);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

/**
 * @return Pointer to the OpenGL graphics context to which the buffers belong.
 */
const void*
GraphicsEngineDataOpenGL::getOpenGLContextPointer() const
{
    return m_openglContextPointer;
}



