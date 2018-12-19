
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

#define __WU_Q_MACRO_MOUSE_EVENT_INFO_DECLARE__
#include "WuQMacroMouseEventInfo.h"
#undef __WU_Q_MACRO_MOUSE_EVENT_INFO_DECLARE__

#include "CaretAssert.h"
using namespace caret;


    
/**
 * \class caret::WuQMacroMouseEventInfo 
 * \brief Information about and related to a QMouseEvent
 * \ingroup Common
 */

/**
 * Constructor.
 * @param mouseEventType
 *     Type of mouse event
 * @param localX
 *     Position of mouse relative to widget
 * @param  localY
 *     Position of mouse relative to widget
 * @param  windowX
 *     Position of mouse relative to window
 * @param  windowY
 *     Position of mouse relative to window
 * @param  screenX
 *     Position of mouse relative to screen
 * @param  screenY
 *     Position of mouse relative to screen
 * @param  mouseButton
 *     Button that caused the event
 * @param  mouseButtonsMask
 *     Mask with buttons down during mouse event
 * @param  keyboardModifiersMask
 *     Mask with any keys down during mouse event
 * @param  widgetWidth
 *     Width of widget where mouse event occurred
 * @param  widgetHeight
 *    Width of widget where mouse event occurred
 */
WuQMacroMouseEventInfo::WuQMacroMouseEventInfo(const WuQMacroMouseEventTypeEnum::Enum mouseEventType,
                                               const int32_t localX,
                                               const int32_t localY,
                                               const int32_t windowX,
                                               const int32_t windowY,
                                               const int32_t screenX,
                                               const int32_t screenY,
                                               const uint32_t mouseButton,
                                               const uint32_t mouseButtonsMask,
                                               const uint32_t keyboardModifiersMask,
                                               const int32_t widgetWidth,
                                               const int32_t widgetHeight)
: CaretObject(),
m_mouseEventType(mouseEventType),
m_localX(localX),
m_localY(localY),
m_windowX(windowX),
m_windowY(windowY),
m_screenX(screenX),
m_screenY(screenY),
m_mouseButton(mouseButton),
m_mouseButtonsMask(mouseButtonsMask),
m_keyboardModifiersMask(keyboardModifiersMask),
m_widgetWidth(widgetWidth),
m_widgetHeight(widgetHeight)
{
    
}

/**
 * Destructor.
 */
WuQMacroMouseEventInfo::~WuQMacroMouseEventInfo()
{
}

/**
 * Widget size may change so scale the x and y position to fit new widget size
 *
 * @param newWidth
 *    Current width of widget
 * @param newHeight
 *    Current height of widget
 * @param xLocalOut
 *    Output with adjusted local-X position
 * @param yLocalOut
 *    Output with adjusted local-Y position
 */
void
WuQMacroMouseEventInfo::getLocalPositionRescaledToWidgetSize(const int32_t newWidth,
                                                             const int32_t newHeight,
                                                             int32_t& xLocalOut,
                                                             int32_t& yLocalOut) const
{
    xLocalOut = m_localX;
    yLocalOut = m_localY;
    
    if ((newWidth != m_widgetWidth)
        || (newHeight != m_widgetHeight)) {
        const float normalizedWidth = (static_cast<float>(m_localX)
                                       / static_cast<float>(m_widgetWidth));
        const float normalizedHeight = (static_cast<float>(m_localY)
                                        / static_cast<float>(m_widgetHeight));
        
        xLocalOut = (newWidth * normalizedWidth);
        yLocalOut = (newHeight * normalizedHeight);
    }
}

/**
 * @return Type of the mouse event
 */
WuQMacroMouseEventTypeEnum::Enum
WuQMacroMouseEventInfo::getMouseEventType() const
{
    return m_mouseEventType;
}

/**
 * @return Position of mouse relative to widget
 */
int32_t
WuQMacroMouseEventInfo::getLocalX() const
{
    return m_localX;
}

/** 
 * @return Position of mouse relative to widget
 */
int32_t
WuQMacroMouseEventInfo::getLocalY() const
{
    return m_localY;
}

/** 
 * @return Position of mouse relative to window
 */
int32_t
WuQMacroMouseEventInfo::getWindowX() const
{
    return m_windowX;
}

/** 
 * @return Position of mouse relative to window 
 */
int32_t
WuQMacroMouseEventInfo::getWindowY() const
{
    return m_windowY;
}

/** 
 * @return Position of mouse relative to screen 
 */
int32_t
WuQMacroMouseEventInfo::getScreenX() const
{
    return m_screenX;
}

/** 
 * @return Position of mouse relative to screen
 */
int32_t
WuQMacroMouseEventInfo::getScreenY() const
{
    return m_screenY;
}

/** 
 * @return Button that caused the event 
 */
uint32_t
WuQMacroMouseEventInfo::getMouseButton() const
{
    return m_mouseButton;
}

/** 
 * @return Mask with buttons down during mouse event
 */
uint32_t WuQMacroMouseEventInfo::getMouseButtonsMask() const
{
    return m_mouseButtonsMask;
}

/** 
 * @@return Mask with any keys down during mouse event 
 */
uint32_t
WuQMacroMouseEventInfo::getKeyboardModifiersMask() const
{
    return m_keyboardModifiersMask;
}

/** 
 * @return Width of widget where mouse event occurred 
 */
int32_t
WuQMacroMouseEventInfo::getWidgetWidth() const
{
    return m_widgetWidth;
}

/** 
 * @return Width of widget where mouse event occurred 
 */
int32_t
WuQMacroMouseEventInfo::getWidgetHeight() const
{
    return m_widgetHeight;
}

/**
 * Get a description of this object's content.
 * @return String describing this object's content.
 */
AString
WuQMacroMouseEventInfo::toString() const
{
    return "WuQMacroMouseEventInfo";
}

