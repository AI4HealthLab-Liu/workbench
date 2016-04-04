
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

#define __DISPLAY_GROUP_AND_TAB_ITEM_TREE_WIDGET_ITEM_DECLARE__
#include "DisplayGroupAndTabItemTreeWidgetItem.h"
#undef __DISPLAY_GROUP_AND_TAB_ITEM_TREE_WIDGET_ITEM_DECLARE__

#include <QPainter>
#include <QPen>
#include <QTreeWidget>

#include "CaretAssert.h"
#include "DisplayGroupAndTabItemInterface.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::DisplayGroupAndTabItemTreeWidgetItem 
 * \brief Item for display group and tab selection hierarchy
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param displayGroupAndTabItem
 *     Item for display in this instance.
 */
DisplayGroupAndTabItemTreeWidgetItem::DisplayGroupAndTabItemTreeWidgetItem(DisplayGroupAndTabItemInterface *displayGroupAndTabItem)
: QTreeWidgetItem(),
m_displayGroup(DisplayGroupEnum::DISPLAY_GROUP_A)
{
    CaretAssert(displayGroupAndTabItem);
    
    setText(NAME_COLUMN,
            displayGroupAndTabItem->getItemName());
    
    setData(NAME_COLUMN,
            Qt::UserRole,
            qVariantFromValue<void*>(displayGroupAndTabItem));
    
    std::vector<DisplayGroupAndTabItemInterface*> itemChildren = displayGroupAndTabItem->getItemChildren();
    for (std::vector<DisplayGroupAndTabItemInterface*>::iterator childIter = itemChildren.begin();
         childIter != itemChildren.end();
         childIter++) {
        DisplayGroupAndTabItemInterface* itemChild = *childIter;
        
        DisplayGroupAndTabItemTreeWidgetItem* childWidget = new DisplayGroupAndTabItemTreeWidgetItem(itemChild);
        addChild(childWidget);
    }
}

/**
 * Destructor.
 */
DisplayGroupAndTabItemTreeWidgetItem::~DisplayGroupAndTabItemTreeWidgetItem()
{
}

/**
 * Update the content of this widget.
 *
 * @param displayGroup
 *     The display group.
 * @param tabIndex
 *     The tab index.
 */
void
DisplayGroupAndTabItemTreeWidgetItem::updateContent(QTreeWidget* treeWidget,
                                                    const DisplayGroupEnum::Enum displayGroup,
                                                    const int32_t tabIndex)
{
    m_displayGroup = displayGroup;
    m_tabIndex     = tabIndex;
    
    void* myDataPtr = data(NAME_COLUMN, Qt::UserRole).value<void*>();
    DisplayGroupAndTabItemInterface* myData = (DisplayGroupAndTabItemInterface*)myDataPtr;
    CaretAssert(myData);
    
    
    Qt::CheckState qtCheckState = toQCheckState(myData->getItemSelected(m_displayGroup,
                                                                                          m_tabIndex));
    setCheckState(NAME_COLUMN,
                  qtCheckState);
    
    setItemIcon(treeWidget,
                myData);
    
    const int32_t numChildren = childCount();
    for (int32_t i = 0; i < numChildren; i++) {
        QTreeWidgetItem* ch = child(i);
        DisplayGroupAndTabItemTreeWidgetItem* item = dynamic_cast<DisplayGroupAndTabItemTreeWidgetItem*>(ch);
        item->updateContent(treeWidget, displayGroup, tabIndex);
    }

    const bool expandedFlag = myData->isItemExpanded(m_displayGroup,
                                                                       m_tabIndex);
    setExpanded(expandedFlag);
}

/**
 * Set the icon for this item.
 */
void
DisplayGroupAndTabItemTreeWidgetItem::setItemIcon(QTreeWidget* treeWidget,
                                                  DisplayGroupAndTabItemInterface* myDataItem)
{
    CaretAssert(myDataItem);
    
    float backgroundRGBA[4];
    float outlineRGBA[4];
    float textRGBA[4];
    myDataItem->getItemIconColorsRGBA(backgroundRGBA,
                                                    outlineRGBA,
                                                    textRGBA);
    if ((backgroundRGBA[3] > 0.0)
        || (outlineRGBA[3] > 0.0)
        || (textRGBA[3] > 0.0)) {
        
        const int pixmapSize = 24;
        QPixmap pixmap(pixmapSize,
                       pixmapSize);
        
//        QSharedPointer<QPainter> painter = WuQtUtilities::createPixmapPainterOriginBottomLeft(pixmap,
//                                                                                              backgroundRGBA);
        QSharedPointer<QPainter> painter = WuQtUtilities::createPixmapWidgetPainterOriginBottomLeft(treeWidget,
                                                                                              pixmap);
        if (backgroundRGBA[3] > 0.0) {
            painter->fillRect(pixmap.rect(),
                              QColor::fromRgbF(backgroundRGBA[0],
                                               backgroundRGBA[1],
                                               backgroundRGBA[2]));
        }
        
        if (outlineRGBA[3] > 0.0) {
            QPen pen = painter->pen();
            
            QColor outlineColor = QColor::fromRgbF(outlineRGBA[0],
                                                   outlineRGBA[1],
                                                   outlineRGBA[2]);
            painter->fillRect(0, 0, 3, pixmapSize, outlineColor);
            painter->fillRect(pixmapSize - 3, 0, 3, pixmapSize, outlineColor);
            painter->fillRect(0, 0, pixmapSize, 3, outlineColor);
            painter->fillRect(0, pixmapSize - 4, pixmapSize, 3, outlineColor);
        }
        
        if (textRGBA[3] > 0.0) {
            QColor textColor = QColor::fromRgbF(textRGBA[0],
                                                textRGBA[1],
                                                textRGBA[2]);
            const int rectSize = 8;
            const int cornerXY = (pixmapSize / 2) - (rectSize / 2);
            painter->fillRect(cornerXY, cornerXY, rectSize, rectSize, textColor);
        }
        
        setIcon(NAME_COLUMN,
                QIcon(pixmap));
    }
    else {
        setIcon(NAME_COLUMN,
                QIcon());
    }
}

/**
 * Convert the tri state selection status to Qt::CheckState
 *
 * @param triStateStatus
 *     The tri state selection status.
 * @return
 *     Qt::CheckState status.
 */
Qt::CheckState
DisplayGroupAndTabItemTreeWidgetItem::toQCheckState(const TriStateSelectionStatusEnum::Enum triStateStatus)
{
    switch (triStateStatus) {
        case TriStateSelectionStatusEnum::PARTIALLY_SELECTED:
            return Qt::PartiallyChecked;
            break;
        case TriStateSelectionStatusEnum::SELECTED:
            return Qt::Checked;
            break;
        case TriStateSelectionStatusEnum::UNSELECTED:
            return Qt::Unchecked;
            break;
    }

    return Qt::Unchecked;
}
