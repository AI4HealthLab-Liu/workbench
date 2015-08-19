
/*LICENSE_START*/
/*
 *  Copyright (C) 2015 Washington University School of Medicine
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

#include <cmath>

#define __BRAIN_OPEN_G_L_ANNOTATION_DRAWING_FIXED_PIPELINE_DECLARE__
#include "BrainOpenGLAnnotationDrawingFixedPipeline.h"
#undef __BRAIN_OPEN_G_L_ANNOTATION_DRAWING_FIXED_PIPELINE_DECLARE__

#include "AnnotationBox.h"
#include "AnnotationCoordinate.h"
#include "AnnotationFile.h"
#include "AnnotationImage.h"
#include "AnnotationLine.h"
#include "AnnotationManager.h"
#include "AnnotationOval.h"
#include "AnnotationText.h"
#include "Brain.h"
#include "DisplayPropertiesAnnotation.h"
#include "Brain.h"
#include "BrainOpenGLFixedPipeline.h"
#include "BrainOpenGLPrimitiveDrawing.h"
#include "BrainOpenGLTextRenderInterface.h"
#include "CaretAssert.h"
#include "CaretColorEnum.h"
#include "CaretLogger.h"
#include "IdentificationWithColor.h"
#include "MathFunctions.h"
#include "Matrix4x4.h"
#include "SelectionManager.h"
#include "SelectionItemAnnotation.h"
#include "Surface.h"

using namespace caret;


    
/**
 * \class caret::BrainOpenGLAnnotationDrawingFixedPipeline 
 * \brief OpenGL Fixed Pipeline drawing of Annotations.
 * \ingroup Brain
 */

/**
 * Constructor.
 * 
 * @param brainOpenGLFixedPipeline
 *    Fixed pipeline drawing.
 */
BrainOpenGLAnnotationDrawingFixedPipeline::BrainOpenGLAnnotationDrawingFixedPipeline(BrainOpenGLFixedPipeline* brainOpenGLFixedPipeline)
: CaretObject(),
m_brainOpenGLFixedPipeline(brainOpenGLFixedPipeline),
m_volumeSpacePlaneValid(false)
{
    CaretAssert(brainOpenGLFixedPipeline);
}

/**
 * Destructor.
 */
BrainOpenGLAnnotationDrawingFixedPipeline::~BrainOpenGLAnnotationDrawingFixedPipeline()
{
}

/**
 * Get a description of this object's content.
 * @return String describing this object's content.
 */
AString 
BrainOpenGLAnnotationDrawingFixedPipeline::toString() const
{
    return "BrainOpenGLAnnotationDrawingFixedPipeline";
}

/**
 * Get the window coordinate for display of the annotation.
 *
 * @param coordinate
 *     The annotation coordinate whose window coordinate is computed.
 * @param surfaceDisplayed
 *     Surface that is displayed (may be NULL !)
 * @param windowXYZOut
 *     Output containing the window coordinate.
 * @return
 *     True if the window coordinate is valid, else false.
 */
bool
BrainOpenGLAnnotationDrawingFixedPipeline::getAnnotationWindowCoordinate(const AnnotationCoordinate* coordinate,
                                                                         const AnnotationCoordinateSpaceEnum::Enum annotationCoordSpace,
                                                                          const Surface* surfaceDisplayed,
                                                                          float windowXYZOut[3]) const
{
    float stereotaxicXYZ[3]  = { 0.0, 0.0, 0.0 };
    bool stereotaxicXYZValid = false;
    
    float windowXYZ[3] = { 0.0, 0.0, 0.0 };
    bool windowXYZValid = false;
    
    float annotationXYZ[3];
    coordinate->getXYZ(annotationXYZ);
    
    switch (annotationCoordSpace) {
        case AnnotationCoordinateSpaceEnum::STEREOTAXIC:
            stereotaxicXYZ[0] = annotationXYZ[0];
            stereotaxicXYZ[1] = annotationXYZ[1];
            stereotaxicXYZ[2] = annotationXYZ[2];
            stereotaxicXYZValid = true;
            
            if (m_volumeSpacePlaneValid) {
                float xyzFloat[3] = {
                    stereotaxicXYZ[0],
                    stereotaxicXYZ[1],
                    stereotaxicXYZ[2]
                };
                const float distToPlaneAbs = std::fabs(m_volumeSpacePlane.signedDistanceToPlane(xyzFloat));
                if (distToPlaneAbs < 1.5) {
                    stereotaxicXYZValid = true;
                }
                else {
                    stereotaxicXYZValid = false;
                }
            }
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            windowXYZ[0] = annotationXYZ[0];
            windowXYZ[1] = annotationXYZ[1];
            windowXYZ[2] = annotationXYZ[2];
            windowXYZValid = true;
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            if (surfaceDisplayed != NULL) {
                StructureEnum::Enum annotationStructure = StructureEnum::INVALID;
                int32_t annotationNumberOfNodes  = -1;
                int32_t annotationNodeIndex      = -1;
                float annotationOffsetLength     = AnnotationCoordinate::getDefaultSurfaceOffsetLength();
                coordinate->getSurfaceSpace(annotationStructure,
                                            annotationNumberOfNodes,
                                            annotationNodeIndex,
                                            annotationOffsetLength);
                
                const StructureEnum::Enum surfaceStructure = surfaceDisplayed->getStructure();
                const int32_t surfaceNumberOfNodes = surfaceDisplayed->getNumberOfNodes();
                if ((surfaceStructure == annotationStructure)
                    && (surfaceNumberOfNodes == annotationNumberOfNodes)) {
                    if ((annotationNodeIndex >= 0)
                        && (annotationNodeIndex < surfaceNumberOfNodes)) {
                        float nodeXYZ[3];
                        surfaceDisplayed->getCoordinate(annotationNodeIndex,
                                                        nodeXYZ);
                        stereotaxicXYZ[0] = nodeXYZ[0];
                        stereotaxicXYZ[1] = nodeXYZ[1];
                        stereotaxicXYZ[2] = nodeXYZ[2];
                        
                        
                        
                        float offsetUnitVector[3] = { 0.0, 0.0, 0.0 };
                        const bool useNormalVectorForOffsetFlag = false;
                        if (useNormalVectorForOffsetFlag) {
                            const float* normalVector = surfaceDisplayed->getNormalVector(annotationNodeIndex);
                            offsetUnitVector[0] = normalVector[0];
                            offsetUnitVector[1] = normalVector[1];
                            offsetUnitVector[2] = normalVector[2];
                        }
                        else {
                            BoundingBox boundingBox;
                            surfaceDisplayed->getBounds(boundingBox);
                            float surfaceCenter[3] = { 0.0, 0.0, 0.0 };
                            boundingBox.getCenter(surfaceCenter);
                            
                            MathFunctions::subtractVectors(nodeXYZ,
                                                           surfaceCenter,
                                                           offsetUnitVector);
                            MathFunctions::normalizeVector(offsetUnitVector);
                        }
                        
                        stereotaxicXYZ[0] += (offsetUnitVector[0] * annotationOffsetLength);
                        stereotaxicXYZ[1] += (offsetUnitVector[1] * annotationOffsetLength);
                        stereotaxicXYZ[2] += (offsetUnitVector[2] * annotationOffsetLength);
                        stereotaxicXYZValid = true;
                    }
                }
            }
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
        {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT,
                          viewport);
            windowXYZ[0] = viewport[2] * annotationXYZ[0];
            windowXYZ[1] = viewport[3] * annotationXYZ[1];
            windowXYZ[2] = 0.0;
            windowXYZValid = true;
        }
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
        {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT,
                          viewport);
            windowXYZ[0] = viewport[2] * annotationXYZ[0];
            windowXYZ[1] = viewport[3] * annotationXYZ[1];
            windowXYZ[2] = 0.0;
            windowXYZValid = true;
        }
            break;
    }
    
    if (stereotaxicXYZValid) {
        /*
         * Convert model space coordinates to window coordinates
         * as all annotations are drawn in window coordinates.
         */
        
        if (convertModelToWindowCoordinate(stereotaxicXYZ, windowXYZ)) {
            stereotaxicXYZValid  = false;
            windowXYZValid = true;
        }
        else {
            CaretLogSevere("Failed to convert model coordinates to window coordinates for annotation drawing.");
        }
    }
    
    if (windowXYZValid) {
        windowXYZOut[0] = windowXYZ[0];
        windowXYZOut[1] = windowXYZ[1];
        windowXYZOut[2] = windowXYZ[2];
    }
    
    return windowXYZValid;
}

/**
 * Convert the given model coordinate into a window coordinate.
 *
 * @param modelXYZ
 *    The model coordinate.
 * @param windowXYZOut
 *    The window coordinate
 * @return true if the conversion is valid, else false.
 */
bool
BrainOpenGLAnnotationDrawingFixedPipeline::convertModelToWindowCoordinate(const float modelXYZ[3],
                                                                          float windowXYZOut[3]) const
{
    /*
     * Convert model space coordinates to window coordinates
     * as all annotations are drawn in window coordinates.
     */
    GLdouble modelDoubleXYZ[3] = { modelXYZ[0], modelXYZ[1], modelXYZ[2] };
    GLdouble windowX, windowY, windowZ;
    if (gluProject(modelDoubleXYZ[0], modelDoubleXYZ[1], modelDoubleXYZ[2],
                   m_modelSpaceModelMatrix, m_modelSpaceProjectionMatrix, m_modelSpaceViewport,
                   &windowX, &windowY, &windowZ) == GL_TRUE) {
        windowXYZOut[0] = windowX - m_modelSpaceViewport[0];
        windowXYZOut[1] = windowY - m_modelSpaceViewport[1];
        
        /*
         * From OpenGL Programming Guide 3rd Ed, p 133:
         *
         * If the near value is 1.0 and the far value is 3.0,
         * objects must have z-coordinates between -1.0 and -3.0 in order to be visible.
         * So, negate the Z-value to be negative.
         */
        windowXYZOut[2] = -windowZ;
        
        return true;
    }
    
    return false;
}

/**
 * Get the bounds for a two-dimensional shape annotation.
 *
 * @param annotation
 *     The annotation whose bounds is computed.
 * @param windowXYZ
 *     Window coordinates of the annotation.
 * @param bottomLeftOut
 *     The bottom left corner of the annotation bounds.
 * @param bottomRightOut
 *     The bottom right corner of the annotation bounds.
 * @param topRightOut
 *     The top right corner of the annotation bounds.
 * @param topLeftOut
 *     The top left corner of the annotation bounds.
 */
bool
BrainOpenGLAnnotationDrawingFixedPipeline::getAnnotationTwoDimShapeBounds(const AnnotationTwoDimensionalShape* annotation2D,
                                                                          const float windowXYZ[3],
                                                                          float bottomLeftOut[3],
                                                                          float bottomRightOut[3],
                                                                          float topRightOut[3],
                                                                          float topLeftOut[3]) const
{
    float viewportWidth  = m_modelSpaceViewport[2];
    float viewportHeight = m_modelSpaceViewport[3];
    if (annotation2D->getCoordinateSpace() == AnnotationCoordinateSpaceEnum::STEREOTAXIC) {
        viewportWidth  = m_tabViewport[2];
        viewportHeight = m_tabViewport[3];
    }
    
    /*
     * Only use text characters when the text is NOT empty
     */
    const AnnotationText* textAnnotation = dynamic_cast<const AnnotationText*>(annotation2D);
    bool textFlag = false;
    if (textAnnotation != NULL) {
        if ( ! textAnnotation->getText().trimmed().isEmpty()) {
            textFlag = true;
        }
    }
    
    bool boundsValid = false;
    if (textFlag) {
        m_brainOpenGLFixedPipeline->getTextRenderer()->getBoundsForTextAtViewportCoords(*textAnnotation,
                                                                                   windowXYZ[0], windowXYZ[1], windowXYZ[2],
                                                                                   bottomLeftOut, bottomRightOut, topRightOut, topLeftOut);
        
        boundsValid = true;
    }
    else {
        boundsValid = annotation2D->getShapeBounds(viewportWidth,
                                                   viewportHeight,
                                                   windowXYZ,
                                                   bottomLeftOut,
                                                   bottomRightOut,
                                                   topRightOut,
                                                   topLeftOut);
    }
    
    return boundsValid;
}

/**
 * Apply rotation to the shape's bounding coordinates.
 *
 * @param rotationAngle
 *     The rotation angle.
 * @param bottomLeftOut
 *     The bottom left corner of the annotation bounds.
 * @param bottomRightOut
 *     The bottom right corner of the annotation bounds.
 * @param topRightOut
 *     The top right corner of the annotation bounds.
 * @param topLeftOut
 *     The top left corner of the annotation bounds.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::applyRotationToShape(const float rotationAngle,
                                                                const float rotationPoint[3],
                                                                float bottomLeftOut[3],
                                                                float bottomRightOut[3],
                                                                float topRightOut[3],
                                                                float topLeftOut[3]) const
{
    if (rotationAngle != 0) {
        Matrix4x4 matrix;
        matrix.translate(-rotationPoint[0], -rotationPoint[1], -rotationPoint[2]);
        matrix.rotateZ(-rotationAngle);
        matrix.translate(rotationPoint[0], rotationPoint[1], rotationPoint[2]);
        matrix.multiplyPoint3(bottomLeftOut);
        matrix.multiplyPoint3(bottomRightOut);
        matrix.multiplyPoint3(topRightOut);
        matrix.multiplyPoint3(topLeftOut);
    }
}

/**
 * Draw model space annotations on the volume slice with the given plane.
 *
 * @param plane
 *     The volume slice's plane.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawModelSpaceAnnotationsOnVolumeSlice(const Plane& plane,
                                                                                  const int tabViewport[4])
{
    if (plane.isValidPlane()) {
        m_volumeSpacePlane = plane;
        m_volumeSpacePlaneValid = true;
        
        drawAnnotations(AnnotationCoordinateSpaceEnum::STEREOTAXIC,
                        tabViewport,
                        NULL);
    }
    
    m_volumeSpacePlaneValid = false;
}


/**
 * Draw the annotations in the given coordinate space.
 *
 * @param drawingCoordinateSpace
 *     Coordinate space of annotation that are drawn.
 * @param surfaceDisplayed
 *     Surface that is displayed.  May be NULL in some instances.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawAnnotations(const AnnotationCoordinateSpaceEnum::Enum drawingCoordinateSpace,
                                                           const int tabViewport[4],
                                                           const Surface* surfaceDisplayed)
{
    if (m_brainOpenGLFixedPipeline->m_brain == NULL) {
        return;
    }
    
    m_tabViewport[0] = tabViewport[0];
    m_tabViewport[1] = tabViewport[1];
    m_tabViewport[2] = tabViewport[2];
    m_tabViewport[3] = tabViewport[3];
    
    m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL, ("At beginning of annotation drawing in space "
                                                           + AnnotationCoordinateSpaceEnum::toName(drawingCoordinateSpace)));
    
    AnnotationManager* annotationManager = m_brainOpenGLFixedPipeline->m_brain->getAnnotationManager();
    
    /*
     * When user is drawing an annotation by dragging the mouse, it is always in window space.
     */
    const Annotation* annotationBeingDrawn = ((drawingCoordinateSpace == AnnotationCoordinateSpaceEnum::WINDOW)
                                              ? annotationManager->getAnnotationBeingDrawnInWindow(m_brainOpenGLFixedPipeline->windowIndex)
                                              : NULL);
    
    bool drawAnnotationsFromFilesFlag = true;
    
    const DisplayPropertiesAnnotation* dpa = m_brainOpenGLFixedPipeline->m_brain->getDisplayPropertiesAnnotation();
    switch (drawingCoordinateSpace) {
        case AnnotationCoordinateSpaceEnum::STEREOTAXIC:
            if ( ! dpa->isDisplayModelAnnotations(m_brainOpenGLFixedPipeline->windowTabIndex)) {
                return;
            }
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            CaretAssertMessage(0, "Never draw annotations in pixel space.");
            return;
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            if ( ! dpa->isDisplaySurfaceAnnotations(m_brainOpenGLFixedPipeline->windowTabIndex)) {
                return;
            }
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
            if ( ! dpa->isDisplayTabAnnotations(m_brainOpenGLFixedPipeline->windowTabIndex)) {
                return;
            }
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
            if ( ! dpa->isDisplayWindowAnnotations(m_brainOpenGLFixedPipeline->windowIndex)) {
                if (annotationBeingDrawn != NULL) {
                    /*
                     * Window annotation are not being drawn.
                     * BUT, the annotation being drawn by the user is always in window space
                     * so we need to continue but skip drawing the window annotations.
                     */
                    drawAnnotationsFromFilesFlag = false;
                }
                else {
                    return;
                }
            }
//            if (annotationBeingDrawn != NULL) {
//                if ( ! dpa->isDisplayWindowAnnotations(m_brainOpenGLFixedPipeline->windowIndex)) {
//                    return;
//                }
//            }
            break;
    }
    SelectionItemAnnotation* annotationID = m_brainOpenGLFixedPipeline->m_brain->getSelectionManager()->getAnnotationIdentification();
    
    GLint savedShadeModel;
    glGetIntegerv(GL_SHADE_MODEL,
                  &savedShadeModel);
    /*
     * Check for a 'selection' type mode
     */
    m_selectionModeFlag = false;
    m_selectionInfo.clear();
    switch (m_brainOpenGLFixedPipeline->mode) {
        case BrainOpenGLFixedPipeline::MODE_DRAWING:
            break;
        case BrainOpenGLFixedPipeline::MODE_IDENTIFICATION:
            if (annotationID->isEnabledForSelection()) {
                m_selectionModeFlag = true;
            }
            else {
                return;
            }
            
            /*
             * Need flag shading for identification
             */
            glShadeModel(GL_FLAT);
            break;
        case BrainOpenGLFixedPipeline::MODE_PROJECTION:
            return;
            break;
    }
    
    /*
     * When selection is performed, annoations in model space need
     * to be converted to window coordinates.  However, when 
     * selecting, all annotations are drawn in WINDOW SPACE 
     * as a rectangle in a solid color so that the color selector
     * can be used.
     *
     * So, when selecting:
     *    (1) Save the matrices and viewewport if drawing in
     *        model space.
     *    (2) Setup matrices for pixel (window) coordinates.
     *        Since we are changing the matrices, they must
     *        be saved as is done in (1).
     */
    glGetDoublev(GL_MODELVIEW_MATRIX,
                 m_modelSpaceModelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX,
                 m_modelSpaceProjectionMatrix);
    glGetIntegerv(GL_VIEWPORT,
                  m_modelSpaceViewport);

    GLdouble depthRange[2];
    glGetDoublev(GL_DEPTH_RANGE,
                 depthRange);
    
    /*
     * All drawing is in window space
     */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_modelSpaceViewport[2],
            0.0, m_modelSpaceViewport[3],
            depthRange[0], depthRange[1]);  // -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    /*
     * Enable anti-aliasing for lines
     */
    m_brainOpenGLFixedPipeline->enableLineAntiAliasing();
    
    /*
     * When selecting, clear out all previous drawing
     * since we identify via colors in each pixel.
     */
    if (m_selectionModeFlag) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    std::vector<AnnotationFile*> allAnnotationFiles;
    if (drawAnnotationsFromFilesFlag) {
        m_brainOpenGLFixedPipeline->m_brain->getAllAnnotationFilesIncludingSceneAnnotationFile(allAnnotationFiles);
    }

    m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL, ("Before draw annotations loop in space: "
                                                           + AnnotationCoordinateSpaceEnum::toName(drawingCoordinateSpace)));
    for (std::vector<AnnotationFile*>::iterator fileIter = allAnnotationFiles.begin();
         fileIter != allAnnotationFiles.end();
         fileIter++) {
        AnnotationFile* annotationFile = *fileIter;
        const std::vector<Annotation*> annotationsFromFile = annotationFile->getAllAnnotations();
        
        const int32_t annotationCount = static_cast<int32_t>(annotationsFromFile.size());
        for (int32_t iAnn = 0; iAnn < annotationCount; iAnn++) {
            CaretAssertVectorIndex(annotationsFromFile, iAnn);
            Annotation* annotation = annotationsFromFile[iAnn];
            CaretAssert(annotation);
            
            AnnotationOneDimensionalShape* oneDimAnn = dynamic_cast<AnnotationOneDimensionalShape*>(annotation);
            AnnotationTwoDimensionalShape* twoDimAnn = dynamic_cast<AnnotationTwoDimensionalShape*>(annotation);
            
            /*
             * Limit drawing of annotations to those in the
             * selected coordinate space.
             */
            const AnnotationCoordinateSpaceEnum::Enum annotationCoordinateSpace = annotation->getCoordinateSpace();
            if (annotationCoordinateSpace != drawingCoordinateSpace) {
                continue;
            }
            
            if (oneDimAnn != NULL) {
            }
            else if (twoDimAnn != NULL) {
            }
            else {
                CaretAssertMessage(0, ("Annotation is not derived from One or Two Dim Annotation classes: "
                                       + annotation->toString()));
                continue;
            }
            
            /*
             * Skip annotation in a different window
             */
            if (annotationCoordinateSpace == AnnotationCoordinateSpaceEnum::WINDOW) {
                const int32_t annotationWindowIndex = annotation->getWindowIndex();
                if ((annotationWindowIndex < 0)
                    || (annotationWindowIndex >= BrainConstants::MAXIMUM_NUMBER_OF_BROWSER_WINDOWS)) {
                    CaretLogSevere("Annotation has invalid window index="
                                   + AString::number(annotationWindowIndex)
                                   + " "
                                   + annotation->toString());
                }
                if (m_brainOpenGLFixedPipeline->windowIndex != annotationWindowIndex) {
                    continue;
                }
            }
            
            /*
             * Skip annotations in a different tab
             */
            if (annotationCoordinateSpace == AnnotationCoordinateSpaceEnum::TAB) {
                const int32_t annotationTabIndex = annotation->getTabIndex();
                if ((annotationTabIndex < 0)
                    || (annotationTabIndex >= BrainConstants::MAXIMUM_NUMBER_OF_BROWSER_TABS)) {
                    CaretLogSevere("Annotation has invalid tab index="
                                   + AString::number(annotationTabIndex)
                                   + " "
                                   + annotation->toString());
                }
                if (m_brainOpenGLFixedPipeline->windowTabIndex != annotationTabIndex) {
                    continue;
                }
            }
            
            drawAnnotation(annotationFile,
                           annotation,
                           surfaceDisplayed);
        }
    }
    m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL, ("After draw annotations loop in space: "
                                                           + AnnotationCoordinateSpaceEnum::toName(drawingCoordinateSpace)));
    
    if (m_selectionModeFlag) {
        CaretAssert(annotationID);
        int32_t annotationIndex = -1;
        float depth = -1.0;
        m_brainOpenGLFixedPipeline->getIndexFromColorSelection(SelectionItemDataTypeEnum::ANNOTATION,
                                                               m_brainOpenGLFixedPipeline->mouseX,
                                                               m_brainOpenGLFixedPipeline->mouseY,
                                                               annotationIndex,
                                                               depth);
        
        
        if (annotationIndex >= 0) {
            if (annotationID != NULL) {
                CaretAssertVectorIndex(m_selectionInfo, annotationIndex);
                const SelectionInfo& selectionInfo = m_selectionInfo[annotationIndex];
                
                if (annotationID->isOtherScreenDepthCloserToViewer(depth)) {
                    
                    annotationID->setAnnotation(selectionInfo.m_annotationFile,
                                                selectionInfo.m_annotation,
                                                selectionInfo.m_sizingHandle);
                    annotationID->setBrain(m_brainOpenGLFixedPipeline->m_brain);
                    annotationID->setScreenXYZ(selectionInfo.m_windowXYZ);
                    annotationID->setScreenDepth(depth);
                    CaretLogFine("Selected Annotation: " + annotationID->toString());
                }
            }
        }
    }
    else {
        /*
         * Annotation being drawn by the user.
         */
        m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL,
                                                        "Start of annotation drawn by user model space.");
        if (annotationBeingDrawn != NULL) {
            AnnotationFile dummyFile;
            drawAnnotation(&dummyFile,
                           const_cast<Annotation*>(annotationBeingDrawn),
                           surfaceDisplayed);
        }
        m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL,
                                                        "End of annotation drawn by user model space.");
    }
    
    /*
     * Disable anti-aliasing for lines
     */
    m_brainOpenGLFixedPipeline->disableLineAntiAliasing();
    
    glShadeModel(savedShadeModel);
    
    /*
     * Restore the matrices since we were drawing in window space
     */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    m_brainOpenGLFixedPipeline->checkForOpenGLError(NULL, ("At end of annotation drawing in space "
                                                           + AnnotationCoordinateSpaceEnum::toName(drawingCoordinateSpace)));
}

/**
 * Get color used for identification when drawing an annotation.
 *
 * @param identificationColorOut
 *     Color components encoding identification.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::getIdentificationColor(uint8_t identificationColorOut[4])
{
    const int32_t annotationDrawnIndex = static_cast<int32_t>(m_selectionInfo.size());
    m_brainOpenGLFixedPipeline->colorIdentification->addItem(identificationColorOut,
                                                             SelectionItemDataTypeEnum::ANNOTATION,
                                                             annotationDrawnIndex);
}

/**
 * Draw an annotation.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param annotation
 *    Annotation to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawAnnotation(AnnotationFile* annotationFile,
                                                          Annotation* annotation,
                                                          const Surface* surfaceDisplayed)
{
    CaretAssert(annotation);
    
    switch (annotation->getType()) {
        case AnnotationTypeEnum::BOX:
            drawBox(annotationFile,
                    dynamic_cast<AnnotationBox*>(annotation),
                    surfaceDisplayed);
            break;
        case AnnotationTypeEnum::IMAGE:
            drawImage(annotationFile,
                      dynamic_cast<AnnotationImage*>(annotation),
                      surfaceDisplayed);
            break;
        case AnnotationTypeEnum::LINE:
            drawLine(annotationFile,
                     dynamic_cast<AnnotationLine*>(annotation),
                     surfaceDisplayed);
            break;
        case AnnotationTypeEnum::OVAL:
            drawOval(annotationFile,
                     dynamic_cast<AnnotationOval*>(annotation),
                     surfaceDisplayed);
            break;
        case AnnotationTypeEnum::TEXT:
            drawText(annotationFile,
                     dynamic_cast<AnnotationText*>(annotation),
                     surfaceDisplayed);
            break;
    }
}


/**
 * Draw an annotation box.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param box
 *    box to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawBox(AnnotationFile* annotationFile,
                                                   AnnotationBox* box,
                                                   const Surface* surfaceDisplayed)
{
    CaretAssert(box);
    CaretAssert(box->getType() == AnnotationTypeEnum::BOX);
    
    float windowXYZ[3];
    if ( ! getAnnotationWindowCoordinate(box->getCoordinate(),
                                         box->getCoordinateSpace(),
                                         surfaceDisplayed,
                                         windowXYZ)) {
        return;
    }
    
    float bottomLeft[3];
    float bottomRight[3];
    float topRight[3];
    float topLeft[3];
    if ( ! getAnnotationTwoDimShapeBounds(box, windowXYZ,
                               bottomLeft, bottomRight, topRight, topLeft)) {
        return;
    }
    
    const float selectionCenterXYZ[3] = {
        (bottomLeft[0] + bottomRight[0] + topRight[0] + topLeft[0]) / 4.0,
        (bottomLeft[1] + bottomRight[1] + topRight[1] + topLeft[1]) / 4.0,
        (bottomLeft[2] + bottomRight[2] + topRight[2] + topLeft[2]) / 4.0
    };
    
    const float outlineWidth = box->getForegroundLineWidth();
    
    const bool depthTestFlag = isDrawnWithDepthTesting(box);
    const bool savedDepthTestStatus = setDepthTestingStatus(depthTestFlag);

    std::vector<float> coords;
    coords.insert(coords.end(), bottomLeft,  bottomLeft + 3);
    coords.insert(coords.end(), bottomRight, bottomRight + 3);
    coords.insert(coords.end(), topRight,    topRight + 3);
    coords.insert(coords.end(), topLeft,     topLeft + 3);
    
    std::vector<float> dummyNormals;
    
    float backgroundRGBA[4];
    box->getBackgroundColorRGBA(backgroundRGBA);
    float foregroundRGBA[4];
    box->getForegroundColorRGBA(foregroundRGBA);

    const bool drawBackgroundFlag = (backgroundRGBA[3] > 0.0);
    const bool drawForegroundFlag = (foregroundRGBA[3] > 0.0);
    const bool drawAnnotationFlag = (drawBackgroundFlag || drawForegroundFlag);
    
    if (drawAnnotationFlag) {
        if (m_selectionModeFlag) {
            uint8_t selectionColorRGBA[4];
            getIdentificationColor(selectionColorRGBA);
            BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                     dummyNormals,
                                                     selectionColorRGBA);
            m_selectionInfo.push_back(SelectionInfo(annotationFile,
                                                    box,
                                                    AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_NONE,
                                                    selectionCenterXYZ));
        }
        else {
            if (drawBackgroundFlag) {
                BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                         dummyNormals,
                                                         backgroundRGBA);
            }
            
            if (drawForegroundFlag) {
                BrainOpenGLPrimitiveDrawing::drawLineLoop(coords,
                                                          foregroundRGBA,
                                                          outlineWidth);
            }
        }
        if (box->isSelected()) {
            drawAnnotationTwoDimSizingHandles(annotationFile,
                                              box,
                                              bottomLeft,
                                              bottomRight,
                                              topRight,
                                              topLeft,
                                              outlineWidth,
                                              box->getRotationAngle());
        }
    }

    setDepthTestingStatus(savedDepthTestStatus);
}

/**
 * Draw an annotation oval.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param oval
 *    Annotation oval to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawOval(AnnotationFile* annotationFile,
                                                    AnnotationOval* oval,
                                                    const Surface* surfaceDisplayed)
{
    CaretAssert(oval);
    CaretAssert(oval->getType() == AnnotationTypeEnum::OVAL);
    
    float windowXYZ[3];
    
    if ( ! getAnnotationWindowCoordinate(oval->getCoordinate(),
                                         oval->getCoordinateSpace(),
                                         surfaceDisplayed,
                                         windowXYZ)) {
        return;
    }
    
    float bottomLeft[3];
    float bottomRight[3];
    float topRight[3];
    float topLeft[3];
    if ( ! getAnnotationTwoDimShapeBounds(oval, windowXYZ,
                                          bottomLeft, bottomRight, topRight, topLeft)) {
        return;
    }
    
    const float majorAxis     = (oval->getWidth()  * m_modelSpaceViewport[2]) / 2.0;
    const float minorAxis     = (oval->getHeight() * m_modelSpaceViewport[3]) / 2.0;
    const float rotationAngle = oval->getRotationAngle();
    const float outlineWidth  = oval->getForegroundLineWidth();
    
    const float selectionCenterXYZ[3] = {
        (bottomLeft[0] + bottomRight[0] + topRight[0] + topLeft[0]) / 4.0,
        (bottomLeft[1] + bottomRight[1] + topRight[1] + topLeft[1]) / 4.0,
        (bottomLeft[2] + bottomRight[2] + topRight[2] + topLeft[2]) / 4.0
    };
    
    const bool depthTestFlag = isDrawnWithDepthTesting(oval);
    const bool savedDepthTestStatus = setDepthTestingStatus(depthTestFlag);
    
    uint8_t backgroundRGBA[4];
    oval->getBackgroundColorRGBA(backgroundRGBA);
    uint8_t foregroundRGBA[4];
    oval->getForegroundColorRGBA(foregroundRGBA);
    
    const bool drawBackgroundFlag = (backgroundRGBA[3] > 0.0);
    const bool drawForegroundFlag = (foregroundRGBA[3] > 0.0);
    const bool drawAnnotationFlag = (drawBackgroundFlag || drawForegroundFlag);
    
    if (drawAnnotationFlag) {
        glPushMatrix();
        glTranslatef(windowXYZ[0], windowXYZ[1], windowXYZ[2]);
        if (rotationAngle != 0.0) {
            glRotatef(-rotationAngle, 0.0, 0.0, 1.0);
        }
        
        if (m_selectionModeFlag) {
            uint8_t selectionColorRGBA[4];
            getIdentificationColor(selectionColorRGBA);
            m_brainOpenGLFixedPipeline->drawEllipseFilled(selectionColorRGBA,
                                                          majorAxis,
                                                          minorAxis);
            m_selectionInfo.push_back(SelectionInfo(annotationFile,
                                                    oval,
                                                    AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_NONE,
                                                    selectionCenterXYZ));
        }
        else {
            if (drawBackgroundFlag) {
                m_brainOpenGLFixedPipeline->drawEllipseFilled(backgroundRGBA,
                                                              majorAxis,
                                                              minorAxis);
            }
            
            if (drawForegroundFlag) {
                m_brainOpenGLFixedPipeline->drawEllipseOutline(foregroundRGBA,
                                                               majorAxis,
                                                               minorAxis,
                                                               outlineWidth);
            }
        }
        glPopMatrix();
        
        if (oval->isSelected()) {
            drawAnnotationTwoDimSizingHandles(annotationFile,
                                              oval,
                                              bottomLeft,
                                              bottomRight,
                                              topRight,
                                              topLeft,
                                              outlineWidth,
                                              oval->getRotationAngle());
        }
    }
    
    
    
    
    setDepthTestingStatus(savedDepthTestStatus);
}

/**
 * Get coordinate for drawing a line that connects text to brainordinate.
 *
 * @param text
 *    The text annotation.
 * @param surfaceDisplayed
 *    Surface for text
 * @param lineCoordinatesOut
 *    Output with coordinates for drawing line.  Will be EMPTY if line is not drawn.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::getTextLineToBrainordinateLineCoordinates(const AnnotationText* text,
                                                                                     const Surface* surfaceDisplayed,
                                                                                     std::vector<float>& lineCoordinatesOut) const
{
    lineCoordinatesOut.clear();
    
    if (text->isConnectToBrainordinateValid()) {
        bool showLineFlag = false;
        bool showArrowFlag = false;
        
        if (text->getCoordinateSpace() == AnnotationCoordinateSpaceEnum::SURFACE) {
            if (surfaceDisplayed != NULL) {
                switch (text->getConnectToBrainordinate()) {
                    case AnnotationTextConnectTypeEnum::ANNOTATION_TEXT_CONNECT_NONE:
                        break;
                    case AnnotationTextConnectTypeEnum::ANNOTATION_TEXT_CONNECT_ARROW:
                        showArrowFlag = true;
                        break;
                    case AnnotationTextConnectTypeEnum::ANNOTATION_TEXT_CONNECT_LINE:
                        showLineFlag = true;
                        break;
                }
            }
        }
        
        if (showLineFlag
            || showArrowFlag) {
            const AnnotationCoordinate* coord = text->getCoordinate();
            StructureEnum::Enum structure = StructureEnum::INVALID;
            int32_t numNodes  = 0;
            int32_t nodeIndex = 0;
            float offset      = 0.0;
            coord->getSurfaceSpace(structure,
                                   numNodes,
                                   nodeIndex,
                                   offset);
            
            if (offset > 0.0) {
                AnnotationCoordinate noOffsetCoord = *coord;
                noOffsetCoord.setSurfaceSpace(structure,
                                              numNodes,
                                              nodeIndex,
                                              0.0);
                
                float brainordinateXYZ[3];
                if (getAnnotationWindowCoordinate(&noOffsetCoord,
                                                  text->getCoordinateSpace(),
                                                  surfaceDisplayed,
                                                  brainordinateXYZ)) {
                    float windowXYZ[3];
                    if (getAnnotationWindowCoordinate(coord,
                                                      text->getCoordinateSpace(),
                                                      surfaceDisplayed,
                                                      windowXYZ)) {
                        
                        createLineCoordinates(windowXYZ,
                                              brainordinateXYZ,
                                              text->getForegroundLineWidth(),
                                              false,
                                              showArrowFlag,
                                              lineCoordinatesOut);
                    }
                }
            }
        }
    }
}

/**
 * Draw an annotation text.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param text
 *    Annotation text to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawText(AnnotationFile* annotationFile,
                                                    AnnotationText* text,
                                                    const Surface* surfaceDisplayed)
{
    CaretAssert(text);
    CaretAssert(text->getType() == AnnotationTypeEnum::TEXT);
    
    

    float windowXYZ[3];
    
    if ( ! getAnnotationWindowCoordinate(text->getCoordinate(),
                                         text->getCoordinateSpace(),
                                         surfaceDisplayed,
                                         windowXYZ)) {
        return;
    }
    
    float bottomLeft[3];
    float bottomRight[3];
    float topRight[3];
    float topLeft[3];
    m_brainOpenGLFixedPipeline->getTextRenderer()->getBoundsForTextAtViewportCoords(*text,
                                                                               windowXYZ[0], windowXYZ[1], windowXYZ[2],
                                                                               bottomLeft, bottomRight, topRight, topLeft);

    std::vector<float> coords;
    coords.insert(coords.end(), bottomLeft,  bottomLeft + 3);
    coords.insert(coords.end(), bottomRight, bottomRight + 3);
    coords.insert(coords.end(), topRight,    topRight + 3);
    coords.insert(coords.end(), topLeft,     topLeft + 3);
    
    std::vector<float> dummyNormals;

    const float selectionCenterXYZ[3] = {
        (bottomLeft[0] + bottomRight[0] + topRight[0] + topLeft[0]) / 4.0,
        (bottomLeft[1] + bottomRight[1] + topRight[1] + topLeft[1]) / 4.0,
        (bottomLeft[2] + bottomRight[2] + topRight[2] + topLeft[2]) / 4.0
    };

    const bool depthTestFlag = isDrawnWithDepthTesting(text);
    const bool savedDepthTestStatus = setDepthTestingStatus(depthTestFlag);
    
    
    float backgroundRGBA[4];
    text->getBackgroundColorRGBA(backgroundRGBA);
    uint8_t foregroundRGBA[4];
    text->getForegroundColorRGBA(foregroundRGBA);
    
    const bool drawBackgroundFlag = (backgroundRGBA[3] > 0.0);
    const bool drawForegroundFlag = (foregroundRGBA[3] > 0.0);
    const bool drawAnnotationFlag = (drawBackgroundFlag || drawForegroundFlag);
    
    if (drawAnnotationFlag) {
        if (m_selectionModeFlag) {
            uint8_t selectionColorRGBA[4];
            getIdentificationColor(selectionColorRGBA);
            BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                     dummyNormals,
                                                     selectionColorRGBA);
            m_selectionInfo.push_back(SelectionInfo(annotationFile,
                                                    text,
                                                    AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_NONE,
                                                    selectionCenterXYZ));
        }
        else {
            std::vector<float> connectLineCoordinates;

            getTextLineToBrainordinateLineCoordinates(text,
                                                      surfaceDisplayed,
                                                      connectLineCoordinates);
            
            if ( ! connectLineCoordinates.empty()) {
                BrainOpenGLPrimitiveDrawing::drawLines(connectLineCoordinates,
                                                       foregroundRGBA,
                                                       text->getForegroundLineWidth()); // 2.0);
            }
            
            if (drawBackgroundFlag) {
                BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                         dummyNormals,
                                                         backgroundRGBA);
            }
            
            if (drawForegroundFlag) {
                if (depthTestFlag) {
                    m_brainOpenGLFixedPipeline->getTextRenderer()->drawTextAtViewportCoords(windowXYZ[0],
                                                                                       windowXYZ[1],
                                                                                       windowXYZ[2],
                                                                                       *text);
                }
                else {
                    if (text->getText().trimmed().isEmpty()) {
                        /*
                         * Text is empty when user is dragging mouse to create a
                         * text region.  In this case, use the bounds of the 
                         * two-dim shape.
                         */
                        float bl[3];
                        float br[3];
                        float tr[3];
                        float tl[3];
                        getAnnotationTwoDimShapeBounds(text, windowXYZ, bl, br, tr, tl);
                        
                        std::vector<float> boxCoords;
                        boxCoords.insert(boxCoords.end(), bl, bl + 3);
                        boxCoords.insert(boxCoords.end(), br, br + 3);
                        boxCoords.insert(boxCoords.end(), tr, tr + 3);
                        boxCoords.insert(boxCoords.end(), tl, tl + 3);
                        
                        if (boxCoords.size() == 12) {
                            BrainOpenGLPrimitiveDrawing::drawLineLoop(boxCoords,
                                                                      foregroundRGBA,
                                                                      text->getForegroundLineWidth());
                        }
                    }
                    else {
                        m_brainOpenGLFixedPipeline->getTextRenderer()->drawTextAtViewportCoords(windowXYZ[0],
                                                                                           windowXYZ[1],
                                                                                           *text);
                    }
                }
                
                setDepthTestingStatus(depthTestFlag);
                
                if (text->isUnderlineEnabled()) {
                    if (text->getOrientation() == AnnotationTextOrientationEnum::HORIZONTAL) {
                        std::vector<float> underlineCoords;
                        underlineCoords.insert(underlineCoords.end(), bottomLeft,  bottomLeft + 3);
                        underlineCoords.insert(underlineCoords.end(), bottomRight, bottomRight + 3);
                        
                        BrainOpenGLPrimitiveDrawing::drawLines(underlineCoords,
                                                               foregroundRGBA,
                                                               2.0);
                    }
                }
            }
        }
        
        if (text->isSelected()) {
            const float outlineWidth = 2.0;
            drawAnnotationTwoDimSizingHandles(annotationFile,
                                              text,
                                              bottomLeft,
                                              bottomRight,
                                              topRight,
                                              topLeft,
                                              outlineWidth,
                                              text->getRotationAngle());
        }
    }




    setDepthTestingStatus(savedDepthTestStatus);
}

/**
 * Draw an annotation image.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param image
 *    Annotation image to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawImage(AnnotationFile* /*annotationFile*/,
                                                     AnnotationImage* /*image*/,
                                                    const Surface* /*surfaceDisplayed*/)
{
    CaretAssertMessage(0, "BrainOpenGLAnnotationDrawingFixedPipeline::drawImage() needs to be implemented.");
}

/**
 * Create the coordinates for drawing a line with optional arrows at the end points.
 *
 * @param lineHeadXYZ
 *     Start of the line
 * @param lineTailXYZ
 *     End of the line
 * @param lineThickness
 *     Thickness of the line that affects size of the optional arrow heads.
 * @param validStartArrow
 *     Add an arrow at the start of the line
 * @param validEndArrow
 *     Add an arrow at the end of the line
 * @param coordinatesOut
 *     Output containing coordinates for drawing the line
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::createLineCoordinates(const float lineHeadXYZ[3],
                                                                 const float lineTailXYZ[3],
                                                                 const float lineThickness,
                                                                 const bool validStartArrow,
                                                                 const bool validEndArrow,
                                                                 std::vector<float>& coordinatesOut) const
{
    coordinatesOut.clear();
    
//    /*
//     * Length of arrow's line
//     */
//    const float lineLength = MathFunctions::distance3D(lineHeadXYZ,
//                                                       lineTailXYZ);
    
//    /*
//     * Length of arrow's right and left pointer tips
//     */
//    const float pointerPercent = 0.2;
//    const float tipLength = lineLength * pointerPercent;
    
    /*
     * Length of arrow's tips is function of line thickness
     */
    const float tipScale = 3.0;
    const float tipLength = lineThickness * tipScale;
    
    /*
     * Point on arrow's line that is between the arrow's left and right arrow tips
     */
    float headToTailVector[3];
    MathFunctions::createUnitVector(lineHeadXYZ, lineTailXYZ, headToTailVector);
    const float startArrowTipsOnLine[3] = {
        lineHeadXYZ[0] + (headToTailVector[0] * tipLength),
        lineHeadXYZ[1] + (headToTailVector[1] * tipLength),
        lineHeadXYZ[2] + (headToTailVector[2] * tipLength)
    };
    const float tailArrowTipsOnLine[3] = {
        lineTailXYZ[0] - (headToTailVector[0] * tipLength),
        lineTailXYZ[1] - (headToTailVector[1] * tipLength),
        lineTailXYZ[2] - (headToTailVector[2] * tipLength)
    };
    
    /*
     * Vector for arrow tip's on left and right
     *
     * Create a perpendicular vector by swapping first two elements
     * and negating the second element.
     */
    float headLeftRightTipOffset[3] = {
        headToTailVector[0],
        headToTailVector[1],
        headToTailVector[2]
    };
    MathFunctions::normalizeVector(headLeftRightTipOffset);
    std::swap(headLeftRightTipOffset[0],
              headLeftRightTipOffset[1]);
    headLeftRightTipOffset[1] *= -1;
    headLeftRightTipOffset[0] *= tipLength;
    headLeftRightTipOffset[1] *= tipLength;
    headLeftRightTipOffset[2] *= tipLength;
    
    /*
     * Tip of arrow's head pointer on the right
     */
    const float headRightTipEnd[3] = {
        startArrowTipsOnLine[0] - headLeftRightTipOffset[0],
        startArrowTipsOnLine[1] - headLeftRightTipOffset[1],
        startArrowTipsOnLine[2] - headLeftRightTipOffset[2]
    };
    
    /*
     * Tip of arrow's head pointer on the left
     */
    const float headLeftTipEnd[3] = {
        startArrowTipsOnLine[0] + headLeftRightTipOffset[0],
        startArrowTipsOnLine[1] + headLeftRightTipOffset[1],
        startArrowTipsOnLine[2] + headLeftRightTipOffset[2]
    };
    
    /*
     * Tip of arrow tail pointer on the right
     */
    const float tailRightTipEnd[3] = {
        tailArrowTipsOnLine[0] + headLeftRightTipOffset[0],
        tailArrowTipsOnLine[1] + headLeftRightTipOffset[1],
        tailArrowTipsOnLine[2] + headLeftRightTipOffset[2]
    };
    
    /*
     * Tip of arrow tail pointer on the right
     */
    const float tailLeftTipEnd[3] = {
        tailArrowTipsOnLine[0] - headLeftRightTipOffset[0],
        tailArrowTipsOnLine[1] - headLeftRightTipOffset[1],
        tailArrowTipsOnLine[2] - headLeftRightTipOffset[2]
    };
    
    coordinatesOut.insert(coordinatesOut.end(), lineHeadXYZ, lineHeadXYZ + 3);
    coordinatesOut.insert(coordinatesOut.end(), lineTailXYZ, lineTailXYZ + 3);
    if (validStartArrow) {
        coordinatesOut.insert(coordinatesOut.end(), lineHeadXYZ,     lineHeadXYZ + 3);
        coordinatesOut.insert(coordinatesOut.end(), headRightTipEnd, headRightTipEnd + 3);
        coordinatesOut.insert(coordinatesOut.end(), lineHeadXYZ,     lineHeadXYZ + 3);
        coordinatesOut.insert(coordinatesOut.end(), headLeftTipEnd,  headLeftTipEnd + 3);
    }
    if (validEndArrow) {
        coordinatesOut.insert(coordinatesOut.end(), lineTailXYZ,     lineTailXYZ + 3);
        coordinatesOut.insert(coordinatesOut.end(), tailRightTipEnd, tailRightTipEnd + 3);
        coordinatesOut.insert(coordinatesOut.end(), lineTailXYZ,     lineTailXYZ + 3);
        coordinatesOut.insert(coordinatesOut.end(), tailLeftTipEnd,  tailLeftTipEnd + 3);
    }
}

/**
 * Draw an annotation line.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param line
 *    Annotation line to draw.
 * @param surfaceDisplayed
 *    Surface that is displayed (may be NULL).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawLine(AnnotationFile* annotationFile,
                                                    AnnotationLine* line,
                                                    const Surface* surfaceDisplayed)
{
    CaretAssert(line);
    CaretAssert(line->getType() == AnnotationTypeEnum::LINE);
    
    float lineHeadXYZ[3];
    float lineTailXYZ[3];
    
    if ( ! getAnnotationWindowCoordinate(line->getStartCoordinate(),
                                         line->getCoordinateSpace(),
                                         surfaceDisplayed,
                                         lineHeadXYZ)) {
        return;
    }
    if ( ! getAnnotationWindowCoordinate(line->getEndCoordinate(),
                                         line->getCoordinateSpace(),
                                         surfaceDisplayed,
                                         lineTailXYZ)) {
        return;
    }
    const float lineWidth = line->getForegroundLineWidth();
    const float backgroundLineWidth = lineWidth + 4;
    
    const float selectionCenterXYZ[3] = {
        (lineHeadXYZ[0] + lineTailXYZ[0]) / 2.0,
        (lineHeadXYZ[1] + lineTailXYZ[1]) / 2.0,
        (lineHeadXYZ[2] + lineTailXYZ[2]) / 2.0
    };
    
    const bool depthTestFlag = isDrawnWithDepthTesting(line);
    const bool savedDepthTestStatus = setDepthTestingStatus(depthTestFlag);
    
    std::vector<float> coords;
    createLineCoordinates(lineHeadXYZ,
                          lineTailXYZ,
                          line->getForegroundLineWidth(),
                          line->isDisplayStartArrow(),
                          line->isDisplayEndArrow(),
                          coords);
    
    uint8_t foregroundRGBA[4];
    line->getForegroundColorRGBA(foregroundRGBA);
    
    const bool drawForegroundFlag = (foregroundRGBA[3] > 0.0);
    
    if (drawForegroundFlag) {
        if (m_selectionModeFlag) {
            uint8_t selectionColorRGBA[4];
            getIdentificationColor(selectionColorRGBA);
            BrainOpenGLPrimitiveDrawing::drawLines(coords,
                                                   selectionColorRGBA,
                                                   backgroundLineWidth);
            m_selectionInfo.push_back(SelectionInfo(annotationFile,
                                                    line,
                                                    AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_NONE,
                                                    selectionCenterXYZ));
        }
        else {
            if (drawForegroundFlag) {
                BrainOpenGLPrimitiveDrawing::drawLines(coords,
                                                       foregroundRGBA,
                                                       lineWidth);
                
            }
        }
        
        if (line->isSelected()) {
            drawAnnotationOneDimSizingHandles(annotationFile,
                                              line,
                                              lineHeadXYZ,
                                              lineTailXYZ,
                                              lineWidth);
        }
    }
    
    setDepthTestingStatus(savedDepthTestStatus);
}

/**
 * Draw a sizing handle at the given coordinate.
 *
 * @param handleType
 *     Type of sizing handle.
 * @param annotationFile
 *    File containing the annotation.
 * @param annotation
 *    Annotation to draw.
 * @param xyz
 *     Center of square.
 * @param halfWidthHeight
 *     Half Width/height of square.
 * @param rotationAngle
 *     Rotation angle for the sizing handle.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawSizingHandle(const AnnotationSizingHandleTypeEnum::Enum handleType,
                                                            AnnotationFile* annotationFile,
                                                            Annotation* annotation,
                                                            const float xyz[3],
                                                            const float halfWidthHeight,
                                                            const float rotationAngle)
{
    glPushMatrix();
    
    glTranslatef(xyz[0], xyz[1], xyz[2]);
    if (rotationAngle != 0.0) {
        glRotatef(-rotationAngle, 0.0, 0.0, 1.0);
    }
    
    std::vector<float> coords;
    coords.push_back(-halfWidthHeight);
    coords.push_back(-halfWidthHeight);
    coords.push_back(0.0);
    coords.push_back( halfWidthHeight);
    coords.push_back(-halfWidthHeight);
    coords.push_back(0.0);
    coords.push_back( halfWidthHeight);
    coords.push_back( halfWidthHeight);
    coords.push_back(0.0);
    coords.push_back(-halfWidthHeight);
    coords.push_back( halfWidthHeight);
    coords.push_back(0.0);
    
    std::vector<float> dummyNormals;
    
    if (m_selectionModeFlag) {
        uint8_t identificationRGBA[4];
        getIdentificationColor(identificationRGBA);
        BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                 dummyNormals,
                                                 identificationRGBA);
        m_selectionInfo.push_back(SelectionInfo(annotationFile,
                                                annotation,
                                                handleType,
                                                xyz));
    }
    else {
        if (handleType == AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_ROTATION) {
            m_brainOpenGLFixedPipeline->drawCircleFilled(m_brainOpenGLFixedPipeline->m_foregroundColorByte,
                                                         halfWidthHeight);
        }
        else {
            BrainOpenGLPrimitiveDrawing::drawPolygon(coords,
                                                     dummyNormals,
                                                     m_brainOpenGLFixedPipeline->m_foregroundColorByte);
        }
    }

    glPopMatrix();
}

/**
 * Draw sizing handles around a one-dimensional annotation.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param annotation
 *    Annotation to draw.
 * @param firstPoint
 *     Top right corner of annotation.
 * @param secondPoint
 *     Top left corner of annotation.
 * @param lineThickness
 *     Thickness of line (when enabled).
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawAnnotationOneDimSizingHandles(AnnotationFile* annotationFile,
                                                                             Annotation* annotation,
                                                                             const float firstPoint[3],
                                                                        const float secondPoint[3],
                                                                        const float lineThickness)
{
    float lengthVector[3];
    MathFunctions::subtractVectors(secondPoint, firstPoint, lengthVector);
    MathFunctions::normalizeVector(lengthVector);
    
    const float dx = secondPoint[0] - firstPoint[0];
    const float dy = secondPoint[1] - firstPoint[1];
    
    const float cornerSquareSize = 3.0 + lineThickness;
    const float directionVector[3] = {
        lengthVector[0] * cornerSquareSize,
        lengthVector[1] * cornerSquareSize,
        0.0
    };
    
    const float firstPointSymbolXYZ[3] = {
        firstPoint[0] - directionVector[0],
        firstPoint[1] - directionVector[1],
        firstPoint[2] - directionVector[2]
    };
    
    const float secondPointSymbolXYZ[3] = {
        secondPoint[0] + directionVector[0],
        secondPoint[1] + directionVector[1],
        secondPoint[2] + directionVector[2]
    };
    
    float rotationAngle = 0.0;
    if ((dy != 0.0) && (dx != 0.0)) {
        const float angleRadians = std::atan2(dx, dy);
        rotationAngle = MathFunctions::toDegrees(angleRadians);
    }
    
    /*
     * Symbol for first coordinate is a little bigger
     */
    drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_LINE_START,
                     annotationFile,
                     annotation,
                     firstPointSymbolXYZ,
                     cornerSquareSize + 2.0,
                     rotationAngle);
    drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_LINE_END,
                     annotationFile,
                     annotation,
                     secondPointSymbolXYZ,
                     cornerSquareSize,
                     rotationAngle);
}


/**
 * Draw sizing handles around a two-dimensional annotation.
 *
 * @param annotationFile
 *    File containing the annotation.
 * @param annotation
 *    Annotation to draw.
 * @param bottomLeft
 *     Bottom left corner of annotation.
 * @param bottomRight
 *     Bottom right corner of annotation.
 * @param topRight
 *     Top right corner of annotation.
 * @param topLeft
 *     Top left corner of annotation.
 * @param lineThickness
 *     Thickness of line (when enabled).
 * @param rotationAngle
 *     Rotation of the annotation.
 */
void
BrainOpenGLAnnotationDrawingFixedPipeline::drawAnnotationTwoDimSizingHandles(AnnotationFile* annotationFile,
                                                                             Annotation* annotation,
                                                                             const float bottomLeft[3],
                                                                        const float bottomRight[3],
                                                                        const float topRight[3],
                                                                        const float topLeft[3],
                                                                        const float lineThickness,
                                                                        const float rotationAngle)
{
    float widthVector[3];
    MathFunctions::subtractVectors(topRight, topLeft, widthVector);
    MathFunctions::normalizeVector(widthVector);
    
    float heightVector[3];
    MathFunctions::subtractVectors(topLeft, bottomLeft, heightVector);
    MathFunctions::normalizeVector(heightVector);
    
    const float innerSpacing = 2.0 + (lineThickness / 2.0);
    
    const float widthSpacingX = innerSpacing * widthVector[0];
    const float widthSpacingY = innerSpacing * widthVector[1];
    
    const float heightSpacingX = innerSpacing * heightVector[0];
    const float heightSpacingY = innerSpacing * heightVector[1];

    const float handleTopLeft[3] = {
        topLeft[0] - widthSpacingX + heightSpacingX,
        topLeft[1] - widthSpacingY + heightSpacingY,
        topLeft[2]
    };
    
    const float handleTopRight[3] = {
        topRight[0] + widthSpacingX + heightSpacingX,
        topRight[1] + widthSpacingY + heightSpacingY,
        topRight[2]
    };
    
    const float handleBottomLeft[3] = {
        bottomLeft[0] - widthSpacingX - heightSpacingX,
        bottomLeft[1] - widthSpacingY - heightSpacingY,
        bottomLeft[2]
    };
    const float handleBottomRight[3] = {
        bottomRight[0] + widthSpacingX - heightSpacingX,
        bottomRight[1] + widthSpacingY - heightSpacingY,
        bottomRight[2]
    };
    
    if (! m_selectionModeFlag) {
        std::vector<float> coords;
        coords.insert(coords.end(), handleBottomLeft,  handleBottomLeft + 3);
        coords.insert(coords.end(), handleBottomRight, handleBottomRight + 3);
        coords.insert(coords.end(), handleTopRight,    handleTopRight + 3);
        coords.insert(coords.end(), handleTopLeft,     handleTopLeft + 3);
        
        BrainOpenGLPrimitiveDrawing::drawLineLoop(coords,
                                                  m_brainOpenGLFixedPipeline->m_foregroundColorByte,
                                                  2.0);
    }
    
    const float handleLeft[3] = {
        (handleBottomLeft[0] + handleTopLeft[0]) / 2.0,
        (handleBottomLeft[1] + handleTopLeft[1]) / 2.0,
        (handleBottomLeft[2] + handleTopLeft[2]) / 2.0,
    };
    
    const float handleRight[3] = {
        (handleBottomRight[0] + handleTopRight[0]) / 2.0,
        (handleBottomRight[1] + handleTopRight[1]) / 2.0,
        (handleBottomRight[2] + handleTopRight[2]) / 2.0,
    };
    
    const float handleBottom[3] = {
        (handleBottomLeft[0] + handleBottomRight[0]) / 2.0,
        (handleBottomLeft[1] + handleBottomRight[1]) / 2.0,
        (handleBottomLeft[2] + handleBottomRight[2]) / 2.0,
    };
    
    const float handleTop[3] = {
        (handleTopLeft[0] + handleTopRight[0]) / 2.0,
        (handleTopLeft[1] + handleTopRight[1]) / 2.0,
        (handleTopLeft[2] + handleTopRight[2]) / 2.0,
    };
    
    const float sizeHandleSize = 5.0;
    
    const float rotationOffset = sizeHandleSize * 3.0;
    const float handleRotation[3] = {
        handleTop[0] + (rotationOffset * heightVector[0]),
        handleTop[1] + (rotationOffset * heightVector[1]),
        handleTop[2] + (rotationOffset * heightVector[2])
    };
    
    /*
     * Text does not receive resizing handles because the
     * text size is determined by the size of the text
     * characters.
     */
    if (annotation->getType() != AnnotationTypeEnum::TEXT) {
        if (annotation->isMovableOrResizableFromGUI()) {
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_BOTTOM_LEFT,
                             annotationFile,
                             annotation,
                             handleBottomLeft,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_BOTTOM_RIGHT,
                             annotationFile,
                             annotation,
                             handleBottomRight,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_TOP_RIGHT,
                             annotationFile,
                             annotation,
                             handleTopRight,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_TOP_LEFT,
                             annotationFile,
                             annotation,
                             handleTopLeft,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_TOP,
                             annotationFile,
                             annotation,
                             handleTop,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_BOTTOM,
                             annotationFile,
                             annotation,
                             handleBottom,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_RIGHT,
                             annotationFile,
                             annotation,
                             handleRight,
                             sizeHandleSize,
                             rotationAngle);
            drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_BOX_LEFT,
                             annotationFile,
                             annotation,
                             handleLeft,
                             sizeHandleSize,
                             rotationAngle);
        }
    }
    
    if (annotation->isMovableOrResizableFromGUI()) {
        /* 
         * Rotation handle and line connecting rotation handle to selection box
         */
        std::vector<float> coords;
        coords.insert(coords.end(), handleRotation, handleRotation + 3);
        coords.insert(coords.end(), handleTop, handleTop + 3);
        BrainOpenGLPrimitiveDrawing::drawLines(coords,
                                               m_brainOpenGLFixedPipeline->m_foregroundColorByte,
                                               2.0);
        
        drawSizingHandle(AnnotationSizingHandleTypeEnum::ANNOTATION_SIZING_HANDLE_ROTATION,
                         annotationFile,
                         annotation,
                         handleRotation,
                         sizeHandleSize,
                         rotationAngle);
    }
}

/**
 * Is the annotation drawn with depth testing enabled (based upon coordinate space)?
 *
 * @param annotation
 *     Annotation that will be drawn.
 * @return 
 *     True if the annotation is drawn with depth testing, else false.
 */
bool
BrainOpenGLAnnotationDrawingFixedPipeline::isDrawnWithDepthTesting(const Annotation* annotation)
{
    bool depthTestFlag = false;
    
    switch (annotation->getCoordinateSpace()) {
        case AnnotationCoordinateSpaceEnum::STEREOTAXIC:
            depthTestFlag = true;
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            depthTestFlag = true;
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
            break;
    }

    return depthTestFlag;
}

/**
 * Set the depth testing to the given status.
 *
 * @param newDepthTestingStatus
 *     New status for depth testing.
 * @return
 *     Depth testing status PRIOR to applying the new depth testing status.
 */
bool
BrainOpenGLAnnotationDrawingFixedPipeline::setDepthTestingStatus(const bool newDepthTestingStatus)
{
    GLboolean savedStatus = GL_FALSE;
    glGetBooleanv(GL_DEPTH_TEST, &savedStatus);
    
    if (newDepthTestingStatus) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    
    return (savedStatus == GL_TRUE);
}



