#ifndef __ANNOTATION_COORDINATE_WIDGET_H__
#define __ANNOTATION_COORDINATE_WIDGET_H__

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


#include <QWidget>

#include "AnnotationCoordinateSpaceEnum.h"
#include "EventListenerInterface.h"

class QDoubleSpinBox;
class QSpinBox;

namespace caret {

    class AnnotationCoordinate;
    class StructureEnumComboBox;
    
    class AnnotationCoordinateWidget : public QWidget, public EventListenerInterface {
        
        Q_OBJECT

    public:
        AnnotationCoordinateWidget(const int32_t browserWindowIndex,
                                   QWidget* parent = 0);
        
        virtual ~AnnotationCoordinateWidget();
        

        // ADD_NEW_METHODS_HERE

        void updateContent(const AnnotationCoordinateSpaceEnum::Enum coordinateSpace,
                           AnnotationCoordinate* coordinate);
        
        virtual void receiveEvent(Event* event);

    private slots:
        void coordinateValueChanged();
        
    private:
        AnnotationCoordinateWidget(const AnnotationCoordinateWidget&);

        AnnotationCoordinateWidget& operator=(const AnnotationCoordinateWidget&);
        
        // ADD_NEW_MEMBERS_HERE

        const int32_t m_browserWindowIndex;
        
        QWidget* m_surfaceWidget;
        
        QWidget* m_coordinateWidget;
        
        StructureEnumComboBox* m_surfaceStructureComboBox;
        
        QSpinBox* m_surfaceNodeIndexSpinBox;
        
        QDoubleSpinBox* m_xCoordSpinBox;
        
        QDoubleSpinBox* m_yCoordSpinBox;
        
        QDoubleSpinBox* m_zCoordSpinBox;

        AnnotationCoordinateSpaceEnum::Enum m_coordinateSpace;
        
        AnnotationCoordinate* m_coordinate;
        
    };
    
#ifdef __ANNOTATION_COORDINATE_WIDGET_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __ANNOTATION_COORDINATE_WIDGET_DECLARE__

} // namespace
#endif  //__ANNOTATION_COORDINATE_WIDGET_H__
