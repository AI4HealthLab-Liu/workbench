#ifndef __WU_Q_MACRO_HELPER_INTERFACE_H__
#define __WU_Q_MACRO_HELPER_INTERFACE_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2018 Washington University School of Medicine
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

/**
 * \class caret::WuQMacroHelperInterface
 * \brief Interface that provides Macro Groups and is used by WuQMacroManager
 * \ingroup GuiQt
 */

#include <QObject>
#include <vector>

namespace caret {
    class WuQMacro;
    class WuQMacroGroup;

    class WuQMacroHelperInterface : public QObject {
        Q_OBJECT
        
    public:
        WuQMacroHelperInterface(QObject* parent) : QObject(parent) { }
        
        virtual ~WuQMacroHelperInterface() { }
        
        WuQMacroHelperInterface(const WuQMacroHelperInterface&) = delete;

        WuQMacroHelperInterface& operator=(const WuQMacroHelperInterface&) = delete;
        
        /**
         * @return All available macro groups
         */
        virtual std::vector<WuQMacroGroup*> getMacroGroups() = 0;

        /**
         * Is called when the given macro is modified
         *
         * @param macro
         *     Macro that is modified
         */
        virtual void macroWasModified(WuQMacro* macro) = 0;
        
        /**
         * Is called when the given macro group is modified
         *
         * @param macroGroup
         *     Macro Group that is modified
         */
        virtual void macroGroupWasModified(WuQMacroGroup* macroGroup) = 0;
        
        // ADD_NEW_METHODS_HERE

    signals:
        void requestDialogsUpdate();
        
    private:
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __WU_Q_MACRO_HELPER_INTERFACE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __WU_Q_MACRO_HELPER_INTERFACE_DECLARE__

} // namespace
#endif  //__WU_Q_MACRO_HELPER_INTERFACE_H__
