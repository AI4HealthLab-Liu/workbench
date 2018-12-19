#ifndef __WU_Q_MACRO_EXECUTOR_H__
#define __WU_Q_MACRO_EXECUTOR_H__

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

#include <QObject>

class QTabBar;
class QWidget;

namespace caret {

    class WuQMacro;
    class WuQMacroCommand;
    
    class WuQMacroExecutor : public QObject {
        
        Q_OBJECT

    public:
        class RunOptions {
        public:
            RunOptions()
            : m_secondsDelayBetweenCommands(1.0),
            m_showMouseMovementFlag(true),
            m_stopOnErrorFlag(true),
            m_loopFlag(false)
            {
                
            }
            RunOptions(const float secondsDelayBetweenCommands,
                       const bool showMouseMovementFlag,
                       const bool loopFlag)
            : m_secondsDelayBetweenCommands(secondsDelayBetweenCommands),
            m_showMouseMovementFlag(showMouseMovementFlag),
            m_stopOnErrorFlag(true),
            m_loopFlag(loopFlag)
            {
            }
            
            float m_secondsDelayBetweenCommands;
            
            bool m_showMouseMovementFlag;
            
            bool m_stopOnErrorFlag;
            
            bool m_loopFlag;
        };
        
        WuQMacroExecutor();
        
        virtual ~WuQMacroExecutor();
        
        WuQMacroExecutor(const WuQMacroExecutor&) = delete;

        WuQMacroExecutor& operator=(const WuQMacroExecutor&) = delete;
        
        bool runMacro(const WuQMacro* macro,
                      QObject* window,
                      std::vector<QObject*>& otherObjectParents,
                      const RunOptions& options,
                      QString& errorMessageOut) const;
        
        
        // ADD_NEW_METHODS_HERE
        
    private:
        void moveMouseToTabBarTab(QTabBar* tabBar,
                                  const int32_t tabIndex) const;

        void moveMouseToWidget(QObject* moveToObject,
                               const bool highlightFlag = true) const;

        void moveMouseToWidgetXY(QObject* moveToObject,
                                 const int x,
                                 const int y,
                                 const bool highlightFlag = true) const;
        
        void moveMouseToWidgetImplementation(QObject* moveToObject,
                                             const int x,
                                             const int y,
                                             const QRect* objectRect = NULL,
                                             const bool hightlightFlag = false) const;
        
        bool runMacroCommand(const WuQMacroCommand* macroCommand,
                             QObject* object,
                             QString& errorMessageOut) const;
        
        QObject* findObjectByName(const QString& objectName) const;
        
        mutable RunOptions m_runOptions;
        
        mutable std::vector<QObject*> m_parentObjects;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __WU_Q_MACRO_EXECUTOR_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __WU_Q_MACRO_EXECUTOR_DECLARE__

} // namespace
#endif  //__WU_Q_MACRO_EXECUTOR_H__
