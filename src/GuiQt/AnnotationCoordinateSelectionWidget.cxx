
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

#define __ANNOTATION_COORDINATE_SELECTION_WIDGET_DECLARE__
#include "AnnotationCoordinateSelectionWidget.h"
#undef __ANNOTATION_COORDINATE_SELECTION_WIDGET_DECLARE__

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVariant>

#include "Annotation.h"
#include "AnnotationCoordinate.h"
#include "AnnotationOneDimensionalShape.h"
#include "AnnotationTwoDimensionalShape.h"
#include "Brain.h"
#include "CaretAssert.h"
#include "CaretLogger.h"
#include "DisplayPropertiesAnnotation.h"
#include "GuiManager.h"
#include "MathFunctions.h"

using namespace caret;



/**
 * \class caret::AnnotationCoordinateSelectionWidget
 * \brief Widget for selection of an annotation coordinate.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param mouseEvent
 *     The mouse event indicating where user clicked in the window
 * @param annotationType
 *      Type of annotation that is being created.
 * @param parent
 *    The parent widget.
 */
AnnotationCoordinateSelectionWidget::AnnotationCoordinateSelectionWidget(const AnnotationTypeEnum::Enum annotationType,
                                                                         const UserInputModeAnnotations::CoordinateInformation& coordInfo,
                                                                         QWidget* parent)
: QWidget(parent),
m_annotationType(annotationType),
m_coordInfo(coordInfo)
{
    bool enableModelSpaceFlag   = false;
    bool enableSurfaceSpaceFlag = false;
    bool enableTabSpaceFlag     = true;
    bool enableWindowSpaceFlag  = true;
    
    switch (m_annotationType) {
        case AnnotationTypeEnum::BOX:
            enableModelSpaceFlag   = true;
            enableSurfaceSpaceFlag = true;
            break;
        case AnnotationTypeEnum::IMAGE:
            break;
        case AnnotationTypeEnum::LINE:
            break;
        case AnnotationTypeEnum::OVAL:
            enableModelSpaceFlag   = true;
            enableSurfaceSpaceFlag = true;
            break;
        case AnnotationTypeEnum::TEXT:
            enableModelSpaceFlag   = true;
            enableSurfaceSpaceFlag = true;
            break;
    }
    
    m_spaceButtonGroup = new QButtonGroup(this);
    
    int columnIndex = 0;
    const int COLUMN_RADIO_BUTTON = columnIndex++;
    const int COLUMN_COORD_X      = columnIndex++;
    const int COLUMN_COORD_Y      = columnIndex++;
    const int COLUMN_COORD_Z      = columnIndex++;
    const int COLUMN_EXTRA        = columnIndex++;
    
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setColumnStretch(COLUMN_RADIO_BUTTON, 0);
    gridLayout->setColumnStretch(COLUMN_COORD_X,      0);
    gridLayout->setColumnStretch(COLUMN_COORD_Y,      0);
    gridLayout->setColumnStretch(COLUMN_COORD_Z,      0);
    gridLayout->setColumnStretch(COLUMN_EXTRA,      100);
    
    const int titleRow = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel("Space"),
                          titleRow, COLUMN_RADIO_BUTTON,
                          Qt::AlignHCenter);
    gridLayout->addWidget(new QLabel("X"),
                          titleRow, COLUMN_COORD_X,
                          Qt::AlignHCenter);
    gridLayout->addWidget(new QLabel("Y"),
                          titleRow, COLUMN_COORD_Y,
                          Qt::AlignHCenter);
    gridLayout->addWidget(new QLabel("Z"),
                          titleRow, COLUMN_COORD_Z,
                          Qt::AlignHCenter);
    
    if (m_coordInfo.m_modelXYZValid
        && enableModelSpaceFlag) {
        QRadioButton* rb = createRadioButtonForSpace(AnnotationCoordinateSpaceEnum::MODEL);
        m_spaceButtonGroup->addButton(rb,
                                      AnnotationCoordinateSpaceEnum::toIntegerCode(AnnotationCoordinateSpaceEnum::MODEL));
        
        const int rowNum = gridLayout->rowCount();
        gridLayout->addWidget(rb,
                              rowNum, COLUMN_RADIO_BUTTON);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_modelXYZ[0], 'f', 3)),
                              rowNum, COLUMN_COORD_X,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_modelXYZ[1], 'f', 3)),
                              rowNum, COLUMN_COORD_Y,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_modelXYZ[2], 'f', 3)),
                              rowNum, COLUMN_COORD_Z,
                              Qt::AlignRight);
    }
    
    if ((m_coordInfo.m_tabIndex >= 0)
        && enableTabSpaceFlag) {
        QRadioButton* rb = createRadioButtonForSpace(AnnotationCoordinateSpaceEnum::TAB);
        rb->setText(rb->text()
                    + " "
                    + AString::number(m_coordInfo.m_tabIndex + 1));
        m_spaceButtonGroup->addButton(rb,
                                      AnnotationCoordinateSpaceEnum::toIntegerCode(AnnotationCoordinateSpaceEnum::TAB));
        
        const int rowNum = gridLayout->rowCount();
        gridLayout->addWidget(rb,
                              rowNum, COLUMN_RADIO_BUTTON);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_tabXYZ[0], 'f', 3)),
                              rowNum, COLUMN_COORD_X,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_tabXYZ[1], 'f', 3)),
                              rowNum, COLUMN_COORD_Y,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_tabXYZ[2], 'f', 3)),
                              rowNum, COLUMN_COORD_Z,
                              Qt::AlignRight);
    }
    
    if ((m_coordInfo.m_windowIndex >= 0)
        && enableWindowSpaceFlag) {
        QRadioButton* rb = createRadioButtonForSpace(AnnotationCoordinateSpaceEnum::WINDOW);
        rb->setText(rb->text()
                    + " "
                    + AString::number(m_coordInfo.m_windowIndex + 1));
        m_spaceButtonGroup->addButton(rb,
                                      AnnotationCoordinateSpaceEnum::toIntegerCode(AnnotationCoordinateSpaceEnum::WINDOW));
        
        
        const int rowNum = gridLayout->rowCount();
        gridLayout->addWidget(rb,
                              rowNum, COLUMN_RADIO_BUTTON);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_windowXYZ[0], 'f', 3)),
                              rowNum, COLUMN_COORD_X,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_windowXYZ[1], 'f', 3)),
                              rowNum, COLUMN_COORD_Y,
                              Qt::AlignRight);
        gridLayout->addWidget(new QLabel(AString::number(m_coordInfo.m_windowXYZ[2], 'f', 3)),
                              rowNum, COLUMN_COORD_Z,
                              Qt::AlignRight);
        
    }
    
    if (m_coordInfo.m_surfaceNodeValid
        && enableSurfaceSpaceFlag) {
        
        QRadioButton* rb = createRadioButtonForSpace(AnnotationCoordinateSpaceEnum::SURFACE);
        m_spaceButtonGroup->addButton(rb,
                                      AnnotationCoordinateSpaceEnum::toIntegerCode(AnnotationCoordinateSpaceEnum::SURFACE));
        
        
        const int rowNum = gridLayout->rowCount();
        gridLayout->addWidget(rb,
                              rowNum, COLUMN_RADIO_BUTTON);
        const AString infoText(StructureEnum::toGuiName(m_coordInfo.m_surfaceStructure)
                               + " Vertex: "
                               +AString::number(m_coordInfo.m_surfaceNodeIndex));
        gridLayout->addWidget(new QLabel(infoText),
                              rowNum, COLUMN_COORD_X, 1, 4);
    }
    
    /*
     * This switch statment does nothing.  But, if a new space is added
     * the missing enumerated value in the switch statement will cause a
     * compilation error which may indicate the code in this method
     * needs to be updated.
     */
    const AnnotationCoordinateSpaceEnum::Enum space = AnnotationCoordinateSpaceEnum::TAB;
    switch (space) {
        case AnnotationCoordinateSpaceEnum::MODEL:
        case AnnotationCoordinateSpaceEnum::PIXELS:
        case AnnotationCoordinateSpaceEnum::SURFACE:
        case AnnotationCoordinateSpaceEnum::TAB:
        case AnnotationCoordinateSpaceEnum::WINDOW:
            break;
    }
    
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
}

/**
 * Destructor.
 */
AnnotationCoordinateSelectionWidget::~AnnotationCoordinateSelectionWidget()
{
}

/**
 * Create a radio button that displays the text for and contains the
 * enumerated value in a property.
 *
 * @param space
 *     Coordinate space for button.
 */
QRadioButton*
AnnotationCoordinateSelectionWidget::createRadioButtonForSpace(const AnnotationCoordinateSpaceEnum::Enum space)
{
    const QString spaceGuiName = AnnotationCoordinateSpaceEnum::toGuiName(space);
    const QString spaceEnumName = AnnotationCoordinateSpaceEnum::toName(space);
    QRadioButton* rb = new QRadioButton(spaceGuiName);
    rb->setProperty(s_SPACE_PROPERTY_NAME.toAscii().constData(),
                    spaceEnumName);
    
    return rb;
}

/**
 * Select the given coordinate space.
 *
 * @param coordSpace
 *     The coordinate space.
 */
void
AnnotationCoordinateSelectionWidget::selectCoordinateSpace(const AnnotationCoordinateSpaceEnum::Enum coordSpace)
{
    QList<QAbstractButton*> buttons = m_spaceButtonGroup->buttons();
    QListIterator<QAbstractButton*> buttList(buttons);
    while (buttList.hasNext()) {
        QAbstractButton* button = buttList.next();
        CaretAssert(button);
        const int32_t buttonID = m_spaceButtonGroup->id(button);
        bool valid = false;
        const AnnotationCoordinateSpaceEnum::Enum buttSpace = AnnotationCoordinateSpaceEnum::fromIntegerCode(buttonID,
                                                                                                             &valid);
        if (valid) {
            if (buttSpace == coordSpace) {
                button->setChecked(true);
                break;
            }
        }
        else {
            CaretLogSevere("Invalid integer code for annotation coordinate space="
                           + QString::number(buttonID));
        }
    }
}

/**
 * Get the selected annotation coordinate space.
 *
 * @param validOut
 *     True if user has selected a coordinate space, else false.
 * @return
 *     Coordinate space selected by the user (valid if validOut is true).
 */
AnnotationCoordinateSpaceEnum::Enum
AnnotationCoordinateSelectionWidget::getSelectedCoordinateSpace(bool& validOut) const
{
    validOut = false;
    AnnotationCoordinateSpaceEnum::Enum space = AnnotationCoordinateSpaceEnum::PIXELS;
    
    QAbstractButton* button = m_spaceButtonGroup->checkedButton();
    if (button == NULL) {
        return space;
    }
    
    CaretAssert(button);
    const int32_t buttonID = m_spaceButtonGroup->id(button);
    space = AnnotationCoordinateSpaceEnum::fromIntegerCode(buttonID,
                                                           &validOut);
    return space;
    
}


/**
 * Set the coordinate with the current space selection.
 *
 * @param coordinate
 *     Coordinate that is set.
 * @param errorMessageOut
 *     Contains error information.
 * @return
 *     True if successful, else false.
 */
bool
AnnotationCoordinateSelectionWidget::changeAnnotationCoordinate(Annotation* annotation,
                                                                AnnotationCoordinate* coordinate,
                                                                AnnotationCoordinate* otherCoordinate,
                                                                QString& errorMessageOut)
{
    CaretAssert(annotation);
    CaretAssert(coordinate);
    
    errorMessageOut.clear();
    
    bool valid = false;
    const AnnotationCoordinateSpaceEnum::Enum newSpace = getSelectedCoordinateSpace(valid);
    if ( ! valid) {
        errorMessageOut = ("A coordinate space has not been selected.");
        return false;
    }
    
    const AnnotationCoordinateSpaceEnum::Enum oldSpace = annotation->getCoordinateSpace();
    
    /*
     * If annotation has two coordinates, get the difference of the two coordinates
     * that will be used if the user changes the coordinate space.  This results
     * in the annotation retaining its relative locations in the new space
     */
    float diffXyz[3]  = { 0.0, 0.0, 0.0 };
    bool diffXyzValid = false;
    AnnotationOneDimensionalShape* oneDimShape = dynamic_cast<AnnotationOneDimensionalShape*>(annotation);
    if ((oneDimShape != NULL)
        && (otherCoordinate != NULL)) {
        float xyz[3];
        coordinate->getXYZ(xyz);
        float otherXyz[3];
        otherCoordinate->getXYZ(otherXyz);
        
        switch (oldSpace) {
            case AnnotationCoordinateSpaceEnum::MODEL:
                break;
            case AnnotationCoordinateSpaceEnum::PIXELS:
                CaretAssert(0);
                break;
            case AnnotationCoordinateSpaceEnum::SURFACE:
                break;
            case AnnotationCoordinateSpaceEnum::TAB:
                diffXyzValid = true;
                break;
            case AnnotationCoordinateSpaceEnum::WINDOW:
                diffXyzValid = true;
                break;
        }
        
        if (diffXyzValid) {
            diffXyz[0] = otherXyz[0] - xyz[0];
            diffXyz[1] = otherXyz[1] - xyz[1];
            diffXyz[2] = otherXyz[2] - xyz[2];
        }
    }

    bool setOtherCoordinateFlag = false;
    switch (newSpace) {
        case AnnotationCoordinateSpaceEnum::MODEL:
            if (m_coordInfo.m_modelXYZValid) {
                coordinate->setXYZ(m_coordInfo.m_modelXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::MODEL);
            }
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            CaretAssert(0);
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            if (m_coordInfo.m_surfaceNodeValid) {
                coordinate->setSurfaceSpace(m_coordInfo.m_surfaceStructure,
                                            m_coordInfo.m_surfaceNumberOfNodes,
                                            m_coordInfo.m_surfaceNodeIndex,
                                            m_coordInfo.m_surfaceNodeOffset);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::SURFACE);
            }
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
            if (m_coordInfo.m_tabIndex >= 0) {
                const int32_t oldTabIndex = annotation->getTabIndex();
                const int32_t newTabIndex = m_coordInfo.m_tabIndex;
                coordinate->setXYZ(m_coordInfo.m_tabXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::TAB);
                annotation->setTabIndex(newTabIndex);
                
                /*
                 * Will need to move 'other' coordinate since it may not
                 * be within the new tab's region.
                 */
                if (otherCoordinate != NULL) {
                    if ((newSpace != oldSpace)
                        || (newTabIndex != oldTabIndex)) {
                        setOtherCoordinateFlag = true;
                    }
                }
            }
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
            if (m_coordInfo.m_windowIndex >= 0) {
                const int32_t oldWindowIndex = annotation->getWindowIndex();
                const int32_t newWindowIndex = m_coordInfo.m_windowIndex;
                coordinate->setXYZ(m_coordInfo.m_windowXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::WINDOW);
                annotation->setWindowIndex(newWindowIndex);
                
                /*
                 * VERIFY
                 * Will need to move 'other' coordinate since it may not
                 * be within the new windows's region.
                 */
                if (otherCoordinate != NULL) {
                    if ((newSpace != oldSpace)
                        || (newWindowIndex != oldWindowIndex)) {
                        setOtherCoordinateFlag = true;
                    }
                }
            }
            break;
    }
    
    /*
     * If the space changes, the 'other' coordinate will need to be moved.
     * For example, if the     */
        if (setOtherCoordinateFlag) {
            float xyz[3];
            coordinate->getXYZ(xyz);
            if (diffXyzValid) {
                xyz[0] += diffXyz[0];
                xyz[1] += diffXyz[1];
                xyz[2] += diffXyz[2];
                xyz[0] = MathFunctions::clamp(xyz[0], 0.01, 0.99);
                xyz[1] = MathFunctions::clamp(xyz[1], 0.01, 0.99);
                xyz[2] = MathFunctions::clamp(xyz[2], 0.01, 0.99);
            }
            else {
                if (xyz[1] > 0.5) {
                    xyz[1] -= 0.25;
                }
                else {
                    xyz[1] += 0.25;
                }
            }
            otherCoordinate->setXYZ(xyz);
        }
    
    updateAnnotationDisplayProperties(annotation);
    
    return true;
}

/**
 * Set the coordinate with the current space selection.
 *
 * @param coordinate
 *     Coordinate that is set.
 * @param errorMessageOut
 *     Contains error information.
 * @return
 *     True if successful, else false.
 */
bool
AnnotationCoordinateSelectionWidget::setCoordinateForNewAnnotation(Annotation* annotation,
                                                                   QString& errorMessageOut)
{
    errorMessageOut.clear();
    
    bool valid = false;
    const AnnotationCoordinateSpaceEnum::Enum buttSpace = getSelectedCoordinateSpace(valid);
    if ( ! valid) {
        errorMessageOut = ("A coordinate space has not been selected.");
        return false;
    }
    
    AnnotationOneDimensionalShape* oneDimAnn = dynamic_cast<AnnotationOneDimensionalShape*>(annotation);
    AnnotationTwoDimensionalShape* twoDimAnn = dynamic_cast<AnnotationTwoDimensionalShape*>(annotation);
    
    AnnotationCoordinate* coordinate = NULL;
    AnnotationCoordinate* otherCoordinate = NULL;
    if (oneDimAnn != NULL) {
        coordinate      = oneDimAnn->getStartCoordinate();
        otherCoordinate = oneDimAnn->getEndCoordinate();
    }
    else if (twoDimAnn != NULL) {
        coordinate = twoDimAnn->getCoordinate();
    }
    else {
        const QString msg("PROGRAM ERROR: Annotation is neither one nor two dimensional");
        CaretAssertMessage(0, msg);
        CaretLogSevere(msg);
        errorMessageOut = msg;
        return false;
    }
    
    switch (buttSpace) {
        case AnnotationCoordinateSpaceEnum::MODEL:
            if (m_coordInfo.m_modelXYZValid) {
                coordinate->setXYZ(m_coordInfo.m_modelXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::MODEL);
            }
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            CaretAssert(0);
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            if (m_coordInfo.m_surfaceNodeValid) {
                coordinate->setSurfaceSpace(m_coordInfo.m_surfaceStructure,
                                            m_coordInfo.m_surfaceNumberOfNodes,
                                            m_coordInfo.m_surfaceNodeIndex,
                                            m_coordInfo.m_surfaceNodeOffset);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::SURFACE);
            }
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
            if (m_coordInfo.m_tabIndex >= 0) {
                coordinate->setXYZ(m_coordInfo.m_tabXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::TAB);
                annotation->setTabIndex(m_coordInfo.m_tabIndex);
                
                if (otherCoordinate != NULL) {
                    double xyz[3] = {
                        m_coordInfo.m_tabXYZ[0],
                        m_coordInfo.m_tabXYZ[1],
                        m_coordInfo.m_tabXYZ[2]
                    };
                    if (xyz[1] > 0.5) {
                        xyz[1] -= 0.25;
                    }
                    else {
                        xyz[1] += 0.25;
                    }
                    otherCoordinate->setXYZ(xyz);
                }
            }
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
            if (m_coordInfo.m_windowIndex >= 0) {
                coordinate->setXYZ(m_coordInfo.m_windowXYZ);
                annotation->setCoordinateSpace(AnnotationCoordinateSpaceEnum::WINDOW);
                annotation->setWindowIndex(m_coordInfo.m_windowIndex);
                
                if (otherCoordinate != NULL) {
                    double xyz[3] = {
                        m_coordInfo.m_windowXYZ[0],
                        m_coordInfo.m_windowXYZ[1],
                        m_coordInfo.m_windowXYZ[2]
                    };
                    if (xyz[1] > 0.5) {
                        xyz[1] -= 0.25;
                    }
                    else {
                        xyz[1] += 0.25;
                    }
                    otherCoordinate->setXYZ(xyz);
                }
            }
            break;
    }
    
    updateAnnotationDisplayProperties(annotation);
    
    return true;
}

/**
 * Update the annotation display properties after creating/updating an annotation.
 * It is possible that the user will create an annotation in a space (tab 4) and tab 4
 * anotations are not enabled for display.  So, ensure the display property is enabled
 * for the annotation.  Otherwise, the user could create an annotation and not see
 * the annotation if the display property is disabled.
 *
 * @param annotation
 *     Annotation that has been created or updated.
 */
void
AnnotationCoordinateSelectionWidget::updateAnnotationDisplayProperties(const Annotation* annotation)
{
    DisplayPropertiesAnnotation* dpa = GuiManager::get()->getBrain()->getDisplayPropertiesAnnotation();
    
    CaretAssert(annotation);
    
    switch (annotation->getCoordinateSpace()) {
        case AnnotationCoordinateSpaceEnum::MODEL:
            dpa->setDisplayModelAnnotations(m_coordInfo.m_tabIndex,
                                            true);
            break;
        case AnnotationCoordinateSpaceEnum::PIXELS:
            CaretAssert(0);
            break;
        case AnnotationCoordinateSpaceEnum::SURFACE:
            dpa->setDisplaySurfaceAnnotations(m_coordInfo.m_tabIndex,
                                              true);
            break;
        case AnnotationCoordinateSpaceEnum::TAB:
            dpa->setDisplayTabAnnotations(annotation->getTabIndex(),
                                          true);
            break;
        case AnnotationCoordinateSpaceEnum::WINDOW:
            dpa->setDisplayWindowAnnotations(annotation->getWindowIndex(),
                                             true);
            break;
    }
}


