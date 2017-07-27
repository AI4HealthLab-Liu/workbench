#ifndef __CHART_OVERLAY_SET_VIEW_CONTROLLER_H__
#define __CHART_OVERLAY_SET_VIEW_CONTROLLER_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2016 Washington University School of Medicine
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

#include "EventListenerInterface.h"

class QScrollArea;

namespace caret {
    class ChartOverlaySet;
    class ChartOverlayViewController;
    class WuQGridLayoutGroup;
    
    class ChartOverlaySetViewController : public QWidget, public EventListenerInterface {
        
        Q_OBJECT

    public:
        ChartOverlaySetViewController(const Qt::Orientation orientation,
                                      const int32_t browserWindowIndex,
                                      QWidget* parent = 0);
        
        virtual ~ChartOverlaySetViewController();
        

        // ADD_NEW_METHODS_HERE

        virtual void receiveEvent(Event* event);

    private slots:
        void processAddOverlayAbove(const int32_t overlayIndex);
        
        void processAddOverlayBelow(const int32_t overlayIndex);
        
        void processRemoveOverlay(const int32_t overlayIndex);
        
        void processMoveOverlayDown(const int32_t overlayIndex);
        
        void processMoveOverlayUp(const int32_t overlayIndex);
        
    private:
        ChartOverlaySetViewController(const ChartOverlaySetViewController&);

        ChartOverlaySetViewController& operator=(const ChartOverlaySetViewController&);
        
        ChartOverlaySet* getChartOverlaySet();
        
        void updateViewController();
        
        void updateColoringAndGraphics();
        
        const int32_t m_browserWindowIndex;
        
        std::vector<ChartOverlayViewController*> m_chartOverlayViewControllers;

        std::vector<WuQGridLayoutGroup*> m_chartOverlayGridLayoutGroups;

        QScrollArea* m_scrollArea;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __CHART_OVERLAY_SET_VIEW_CONTROLLER_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __CHART_OVERLAY_SET_VIEW_CONTROLLER_DECLARE__

} // namespace
#endif  //__CHART_OVERLAY_SET_VIEW_CONTROLLER_H__
