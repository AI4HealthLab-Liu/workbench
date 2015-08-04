
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

#define __ANNOTATION_COORDINATE_WIDGET_DECLARE__
#include "AnnotationCoordinateWidget.h"
#undef __ANNOTATION_COORDINATE_WIDGET_DECLARE__

#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpinBox>

#include "AnnotationCoordinate.h"
#include "CaretAssert.h"
#include "EventGraphicsUpdateOneWindow.h"
#include "EventManager.h"
#include "StructureEnumComboBox.h"
#include "WuQFactory.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::AnnotationCoordinateWidget 
 * \brief Widget for editing annotation coordinate, size, and rotation.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 */
AnnotationCoordinateWidget::AnnotationCoordinateWidget(const int32_t browserWindowIndex,
                                                       QWidget* parent)
: QWidget(parent),
m_browserWindowIndex(browserWindowIndex)
{
    m_coordinate = NULL;
    
    QLabel* surfaceVertexLabel = new QLabel("Vertex:");
    std::vector<StructureEnum::Enum> validStructures;
    validStructures.push_back(StructureEnum::CORTEX_LEFT);
    validStructures.push_back(StructureEnum::CORTEX_RIGHT);
    validStructures.push_back(StructureEnum::CEREBELLUM);
    m_surfaceStructureComboBox = new StructureEnumComboBox(this);
    m_surfaceStructureComboBox->listOnlyTheseStructures(validStructures);
    
    m_surfaceNodeIndexSpinBox = new QSpinBox();
    m_surfaceNodeIndexSpinBox->setRange(0, 1000000);
    m_surfaceNodeIndexSpinBox->setSingleStep(1);
    
    QLabel* xCoordLabel = new QLabel(" X:");
    m_xCoordSpinBox = WuQFactory::newDoubleSpinBoxWithMinMaxStepDecimalsSignalDouble(0.0, 1.0, 0.01, 2,
                                                                                     this, SLOT(coordinateValueChanged()));
    WuQtUtilities::setWordWrappedToolTip(m_xCoordSpinBox,
                                         "X-coordinate of annotation\n"
                                         "   MODEL: Stereotaxic Coordinate\n"
                                         "   TAB and WINDOW X-Range: [0.0, 1.0]\n"
                                         "      0.0 => Left side of tab/window\n"
                                         "      1.0 => Right side of tab/window\n");
    
    QLabel* yCoordLabel = new QLabel(" Y:");
    m_yCoordSpinBox = WuQFactory::newDoubleSpinBoxWithMinMaxStepDecimalsSignalDouble(0.0, 1.0, 0.01, 2,
                                                                                     this, SLOT(coordinateValueChanged()));
    WuQtUtilities::setWordWrappedToolTip(m_yCoordSpinBox,
                                         "Y-coordinate of annotation\n"
                                         "   MODEL: Stereotaxic Coordinate\n"
                                         "   TAB and WINDOW Y-Range: [0.0, 1.0]\n"
                                         "      0.0 => Bottom of tab/window\n"
                                         "      1.0 => Top of tab/window\n");
    
    QLabel* zCoordLabel = new QLabel(" Z:");
    m_zCoordSpinBox = WuQFactory::newDoubleSpinBoxWithMinMaxStepDecimalsSignalDouble(-1.0, 1.0, 0.01, 2,
                                                                                     this, SLOT(coordinateValueChanged()));
    WuQtUtilities::setWordWrappedToolTip(m_zCoordSpinBox,
                                         "Z-coordinate of annotation\n"
                                         "   MODEL: Stereotaxic Coordinate\n"
                                         "   TAB and WINDOW DEPTH Range: [-1.0, 1.0]\n"
                                         "      -1.0 => Toward's viewer\n"
                                         "       1.0 => Away from viewer\n");
    

    m_surfaceWidget = new QWidget();
    QHBoxLayout* surfaceLayout = new QHBoxLayout(m_surfaceWidget);
    WuQtUtilities::setLayoutSpacingAndMargins(surfaceLayout, 2, 0);
    surfaceLayout->addWidget(surfaceVertexLabel);
    surfaceLayout->addWidget(m_surfaceStructureComboBox->getWidget());
    surfaceLayout->addWidget(m_surfaceNodeIndexSpinBox);
    
    m_coordinateWidget = new QWidget();
    QHBoxLayout* coordinateLayout = new QHBoxLayout(m_coordinateWidget);
    WuQtUtilities::setLayoutSpacingAndMargins(coordinateLayout, 2, 0);
    coordinateLayout->addWidget(xCoordLabel);
    coordinateLayout->addWidget(m_xCoordSpinBox);
    coordinateLayout->addWidget(yCoordLabel);
    coordinateLayout->addWidget(m_yCoordSpinBox);
    coordinateLayout->addWidget(zCoordLabel);
    coordinateLayout->addWidget(m_zCoordSpinBox);
    
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    WuQtUtilities::setLayoutSpacingAndMargins(layout, 2, 2);
    layout->addWidget(m_surfaceWidget);
    layout->addWidget(m_coordinateWidget);
    
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
//    EventManager::get()->addEventListener(this, EventTypeEnum::);
}

/**
 * Destructor.
 */
AnnotationCoordinateWidget::~AnnotationCoordinateWidget()
{
    EventManager::get()->removeAllEventsFromListener(this);
}

/**
 * Receive an event.
 *
 * @param event
 *    An event for which this instance is listening.
 */
void
AnnotationCoordinateWidget::receiveEvent(Event* event)
{
//    if (event->getEventType() == EventTypeEnum::) {
//        <EVENT_CLASS_NAME*> eventName = dynamic_cast<EVENT_CLASS_NAME*>(event);
//        CaretAssert(eventName);
//
//        event->setEventProcessed();
//    }
}

/**
 * Update with the given annotation coordinate.
 *
 * @param coordinate.
 */
void
AnnotationCoordinateWidget::updateContent(const AnnotationCoordinateSpaceEnum::Enum coordinateSpace,
                                          AnnotationCoordinate* coordinate)
{
    m_coordinate = coordinate;
    
    bool surfaceFlag    = false;
    
    if (m_coordinate != NULL) {
        double xyMin =  0.0;
        double xyMax =  1.0;
        double zMin  = -1.0;
        double zMax  =  1.0;
        double xyzStep = 0.01;
        switch (coordinateSpace) {
            case AnnotationCoordinateSpaceEnum::MODEL:
                xyMax = 10000.0;
                xyMin = -xyMax;
                zMin = xyMin;
                zMax = xyMax;
                xyzStep = 1.0;
                break;
            case AnnotationCoordinateSpaceEnum::PIXELS:
                break;
            case AnnotationCoordinateSpaceEnum::SURFACE:
                surfaceFlag = true;
                break;
            case AnnotationCoordinateSpaceEnum::TAB:
                break;
            case AnnotationCoordinateSpaceEnum::WINDOW:
                break;
        }
        
        m_xCoordSpinBox->setRange(xyMin,
                                  xyMax);
        m_xCoordSpinBox->setSingleStep(xyzStep);
        m_yCoordSpinBox->setRange(xyMin,
                                  xyMax);
        m_yCoordSpinBox->setSingleStep(xyzStep);
        m_zCoordSpinBox->setRange(xyMin,
                                  xyMax);
        m_zCoordSpinBox->setSingleStep(xyzStep);
        
        float xyz[3];
        m_coordinate->getXYZ(xyz);
        
        m_xCoordSpinBox->setValue(xyz[0]);
        m_yCoordSpinBox->setValue(xyz[1]);
        m_zCoordSpinBox->setValue(xyz[2]);
        
        if (surfaceFlag) {
            StructureEnum::Enum structure = StructureEnum::INVALID;
            int32_t surfaceNumberOfNodes  = -1;
            int32_t surfaceNodeIndex      = -1;
            m_coordinate->getSurfaceSpace(structure,
                                          surfaceNumberOfNodes,
                                          surfaceNodeIndex);
        }
        
        setEnabled(true);
    }
    else {
        setEnabled(false);
    }
    
    m_surfaceWidget->setVisible(surfaceFlag);
    m_coordinateWidget->setVisible( ! surfaceFlag);
}

/**
 * Gets called when an X, Y, or Z-coordinate value is changed.
 */
void
AnnotationCoordinateWidget::coordinateValueChanged()
{
    if (m_coordinate != NULL) {
        float xyz[3] = {
            m_xCoordSpinBox->value(),
            m_yCoordSpinBox->value(),
            m_zCoordSpinBox->value()
        };
        m_coordinate->setXYZ(xyz);
    }
    
    EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(m_browserWindowIndex).getPointer());
}

