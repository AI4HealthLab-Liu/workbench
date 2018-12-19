#ifndef __WU_Q_MACRO_H__
#define __WU_Q_MACRO_H__

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



#include <memory>

#include "CaretObjectTracksModification.h"

namespace caret {

    class WuQMacroCommand;
    
    class WuQMacro : public CaretObjectTracksModification {
        
    public:
        WuQMacro();
        
        virtual ~WuQMacro();
        
        WuQMacro(const WuQMacro& obj);
        
        WuQMacro& operator=(const WuQMacro& obj);
        
        void addMacroCommand(WuQMacroCommand* macroCommand);
        
        int32_t getNumberOfMacroCommands() const;
        
        const WuQMacroCommand* getMacroCommandAtIndex(const int32_t index) const;
        
        WuQMacroCommand* getMacroCommandAtIndex(const int32_t index);
        
        QString getName() const;
        
        void setName(const QString& name);
        
        QString getDescription() const;
        
        void setDescription(const QString& description);
        
        QString getFunctionKey() const;
        
        void setFunctionKey(const QString& functionKey);
        
        // ADD_NEW_METHODS_HERE

        virtual AString toString() const;

        virtual bool isModified() const override;
        
        virtual void clearModified() override;
        
    private:
        void copyHelperWuQMacro(const WuQMacro& obj);
        
        void clearCommands();
        
        std::vector<WuQMacroCommand*> m_macroCommands;
        
        QString m_name;
        
        QString m_description;
        
        QString m_functionKey;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __WU_Q_MACRO_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __WU_Q_MACRO_DECLARE__

} // namespace
#endif  //__WU_Q_MACRO_H__
