#ifndef __WB_MACRO_CUSTOM_OPERATION_MANAGER_H__
#define __WB_MACRO_CUSTOM_OPERATION_MANAGER_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2019 Washington University School of Medicine
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

#include "CaretObject.h"
#include "WbMacroCustomOperationTypeEnum.h"
#include "WuQMacroCustomOperationManagerInterface.h"

class QWidget;

namespace caret {
    class BrowserTabContent;
    class CaretMappableDataFile;
    class WbMacroCustomOperationBase;
    class WuQMacroCommand;
    class WuQMacroCommandParameter;

    class WbMacroCustomOperationManager : public CaretObject, public WuQMacroCustomOperationManagerInterface {
        
    public:
        WbMacroCustomOperationManager();
        
        virtual ~WbMacroCustomOperationManager();
        
        WbMacroCustomOperationManager(const WbMacroCustomOperationManager&) = delete;

        WbMacroCustomOperationManager& operator=(const WbMacroCustomOperationManager&) = delete;

        virtual bool editCustomDataValueParameter(QWidget* parent,
                                                  WuQMacroCommand* macroCommand,
                                                  WuQMacroCommandParameter* parameter);
        
        virtual bool executeCustomOperationMacroCommand(QWidget* parent,
                                                        const WuQMacroExecutorMonitor* executorMonitor,
                                                        const WuQMacroExecutorOptions* executorOptions,
                                                        const WuQMacroCommand* macroCommand,
                                                        QString& errorMessageOut) override;
        
        virtual std::vector<QString> getNamesOfCustomOperationMacroCommands() override;
        
        virtual std::vector<WuQMacroCommand*> getAllCustomOperationMacroCommands() override;        

        virtual WuQMacroCommand* newInstanceOfCustomOperationMacroCommand(const QString& customMacroCommandName,
                                                                          QString& errorMessageOut) override;
        

        // ADD_NEW_METHODS_HERE

        virtual AString toString() const;
        
    private:

        WbMacroCustomOperationBase* createCommand(const WbMacroCustomOperationTypeEnum::Enum operationType);
        
        bool editOverlayIndex(QWidget* parentWidget,
                              WuQMacroCommandParameter* parameter);
        
        bool editOverlayFile(QWidget* parentWidget,
                              WuQMacroCommand* macroCommand,
                              WuQMacroCommandParameter* parameter,
                              QString& errorMessageOut);
        
        bool editOverlayMap(QWidget* parentWidget,
                            WuQMacroCommand* macroCommand,
                            WuQMacroCommandParameter* parameter,
                            QString& errorMessageOut);
        
        bool editScreenAxis(QWidget* parentWidget,
                            WuQMacroCommandParameter* parameter);
        
        bool editSurface(QWidget* parentWidget,
                         WuQMacroCommandParameter* parameter,
                         QString& errorMessageOut);

        bool getMapFilesInOverlay(QWidget* parentWidget,
                                  WuQMacroCommand* macroCommand,
                                  std::vector<CaretMappableDataFile*>& mapFilesOut,
                                  CaretMappableDataFile* &selectedMapFileOut,
                                  QString& errorMessageOut);

        BrowserTabContent* getTabContent(QWidget* parentWidget,
                                         const QString& promptMessage,
                                         QString& errorMessageOut);
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __WB_MACRO_CUSTOM_OPERATION_MANAGER_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __WB_MACRO_CUSTOM_OPERATION_MANAGER_DECLARE__

} // namespace
#endif  //__WB_MACRO_CUSTOM_OPERATION_MANAGER_H__
