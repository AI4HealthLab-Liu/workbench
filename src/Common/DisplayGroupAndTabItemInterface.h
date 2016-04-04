#ifndef __DISPLAY_GROUP_AND_TAB_ITEM_INTERFACE_H__
#define __DISPLAY_GROUP_AND_TAB_ITEM_INTERFACE_H__

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

#include "BrainConstants.h"
#include "DisplayGroupEnum.h"
#include "TriStateSelectionStatusEnum.h"

/**
 * \class caret::DisplayGroupAndTabItemInterface
 * \brief Interface for items in a display group tab selection hierarchy
 * \ingroup Common
 */

namespace caret {

    class DisplayGroupAndTabItemInterface {
        
    protected:
        /**
         * Constructor
         */
        DisplayGroupAndTabItemInterface() { }
        
    public:
        /**
         * Destructor
         */
        virtual ~DisplayGroupAndTabItemInterface() { }
        
        /**
         * @return Children of this item.
         */
        virtual std::vector<DisplayGroupAndTabItemInterface*> getItemChildren() const = 0;
        
        /**
         * @return Parent of this item.
         */
        virtual DisplayGroupAndTabItemInterface* getItemParent() const = 0;
        
        /**
         * Set the parent of this item.
         * 
         * @param itemParent
         *     Parent of this item.
         */
        virtual void setItemParent(DisplayGroupAndTabItemInterface* itemParent)  = 0;
        
        /**
         * @return Name of this item.
         */
        virtual AString getItemName() const = 0;
        
        /**
         * Get the icon color for this item.
         *
         * @param rgbaOut
         *     Red, green, blue, alpha components ranging [0, 1].
         */
        virtual void getItemIconColorRGBA(float rgbaOut[4]) const = 0;
        
        /**
         * @return This item can be expanded.
         */
        virtual bool isItemExpandable() const = 0;
        
        /**
         * @return Is this item expanded in the given display group/tab?
         *
         * @param displayGroup 
         *     The display group.
         * @param tabIndex
         *     Index of the tab.
         */
        virtual bool isItemExpanded(const DisplayGroupEnum::Enum displayGroup,
                                    const int32_t tabIndex) const = 0;
        
        /**
         * Set this item's expanded status in the given display group/tab.
         *
         * @param displayGroup
         *     The display group.
         * @param tabIndex
         *     Index of the tab.
         * @param status
         *     New expanded status.
         */
        virtual void setItemExpanded(const DisplayGroupEnum::Enum displayGroup,
                                     const int32_t tabIndex,
                                     const bool status) = 0;
        
        /**
         * Get selection status in the given display group/tab?
         *
         * @param displayGroup
         *     The display group.
         * @param tabIndex
         *     Index of the tab.
         */
        virtual TriStateSelectionStatusEnum::Enum getItemSelected(const DisplayGroupEnum::Enum displayGroup,
                                                                 const int32_t tabIndex) const = 0;
        
        /**
         * Set this item selected in the given display group/tab.
         *
         * @param displayGroup
         *     The display group.
         * @param tabIndex
         *     Index of the tab.
         * @param status
         *     New selection status.
         */
        virtual void setItemSelected(const DisplayGroupEnum::Enum displayGroup,
                                     const int32_t tabIndex,
                                     const TriStateSelectionStatusEnum::Enum status) = 0;
        

        // ADD_NEW_METHODS_HERE
        
    private:
        //DisplayGroupAndTabItemInterface(const DisplayGroupAndTabItemInterface& obj);

        //DisplayGroupAndTabItemInterface& operator=(const DisplayGroupAndTabItemInterface& obj);
        

        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __DISPLAY_GROUP_AND_TAB_ITEM_INTERFACE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __DISPLAY_GROUP_AND_TAB_ITEM_INTERFACE_DECLARE__

} // namespace
#endif  //__DISPLAY_GROUP_AND_TAB_ITEM_INTERFACE_H__
