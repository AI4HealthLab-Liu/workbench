
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

#define __WU_Q_MACRO_GROUP_XML_STREAM_WRITER_DECLARE__
#include "WuQMacroGroupXmlStreamWriter.h"
#undef __WU_Q_MACRO_GROUP_XML_STREAM_WRITER_DECLARE__

#include <QTextStream>
#include <QXmlStreamWriter>

#include "CaretAssert.h"
#include "WuQMacro.h"
#include "WuQMacroCommand.h"
#include "WuQMacroCommandParameter.h"
#include "WuQMacroGroup.h"
#include "WuQMacroMouseEventInfo.h"

using namespace caret;


    
/**
 * \class caret::WuQMacroGroupXmlStreamWriter
 * \brief Writes contents of macro group to XML format
 * \ingroup Common
 */

/**
 * Constructor
 */
WuQMacroGroupXmlStreamWriter::WuQMacroGroupXmlStreamWriter()
: WuQMacroGroupXmlStreamBase()
{
}

/**
 * Destructor.
 */
WuQMacroGroupXmlStreamWriter::~WuQMacroGroupXmlStreamWriter()
{
}

/**
 * Wite the given macro group to the given xml writer
 *
 * @param xmlWriter
 *     The XML stream writer
 * @param macroGroup
 *     Macro group that is written to XML
 */
void
WuQMacroGroupXmlStreamWriter::writeXml(QXmlStreamWriter* xmlWriter,
                                       const WuQMacroGroup* macroGroup)
{
    CaretAssert(xmlWriter);
    CaretAssert(macroGroup);
    
    if (macroGroup->getNumberOfMacros() <= 0) {
//        xmlWriter->writeEmptyElement(ELEMENT_MACRO_GROUP);
        return;
    }
    
    m_xmlStreamWriter = xmlWriter;
    
    m_xmlStreamWriter->writeStartElement(ELEMENT_MACRO_GROUP);
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_NAME, macroGroup->getName());
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_VERSION, "1");
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_UNIQUE_IDENTIFIER,
                                      macroGroup->getUniqueIdentifier());
    
    const int32_t numMacros = macroGroup->getNumberOfMacros();
    for (int32_t i = 0; i < numMacros; i++) {
        const WuQMacro* macro = macroGroup->getMacroAtIndex(i);
        CaretAssert(macro);
        writeMacroToXML(macro);
    }
    m_xmlStreamWriter->writeEndElement();
    
    m_xmlStreamWriter = NULL;
}

/**
 * Write the macro group to a text string
 *
 * @param macroGroup
 *     Macro group that is written to XML
 * @param contentTextString
 *     Pointer to string to which XML is written
 */
void
WuQMacroGroupXmlStreamWriter::writeToString(const WuQMacroGroup* macroGroup,
                                           QString& contentTextString)
{
    CaretAssert(macroGroup);
    contentTextString.clear();
    
    std::unique_ptr<QXmlStreamWriter> xmlWriter(new QXmlStreamWriter(&contentTextString));
    xmlWriter->setAutoFormatting(true);
    writeXml(xmlWriter.get(),
             macroGroup);
}


/**
 * Write a macro to XML format
 * 
 * @param macro
 *     The macro
 */
void
WuQMacroGroupXmlStreamWriter::writeMacroToXML(const WuQMacro* macro)
{
    CaretAssert(macro);
    
    m_xmlStreamWriter->writeStartElement(ELEMENT_MACRO);
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_NAME,
                                      macro->getName());
    const QString shortCutText = WuQMacroShortCutKeyEnum::toName(macro->getShortCutKey());
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_SHORT_CUT_KEY,
                                      shortCutText);
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_UNIQUE_IDENTIFIER,
                                      macro->getUniqueIdentifier());
    m_xmlStreamWriter->writeTextElement(ELEMENT_DESCRIPTION,
                                        macro->getDescription());
    
    const int32_t numCommands = macro->getNumberOfMacroCommands();
    for (int32_t i = 0; i < numCommands; i++) {
        const WuQMacroCommand* macroCommand = macro->getMacroCommandAtIndex(i);
        CaretAssert(macroCommand);
        writeMacroCommandToXML(macroCommand);
    }
    
    m_xmlStreamWriter->writeEndElement();
}

/**
 * Write a macro command to XML format
 *
 * @param macroCommand
 *     The macro command
 */
void
WuQMacroGroupXmlStreamWriter::writeMacroCommandToXML(const WuQMacroCommand* macroCommand)
{
    CaretAssert(macroCommand);
    
    m_xmlStreamWriter->writeStartElement(ELEMENT_MACRO_COMMAND);
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_NAME,
                                      macroCommand->getObjectName());
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_OBJECT_CLASS,
                                      WuQMacroClassTypeEnum::toName(macroCommand->getClassType()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_OBJECT_DESCRIPTIVE_NAME,
                                      macroCommand->getDescriptiveName());
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_DELAY,
                                      QString::number(macroCommand->getDelayInSeconds(), 'f', 2));
    
    switch (macroCommand->getClassType()) {
        case WuQMacroClassTypeEnum::ACTION:
            break;
        case WuQMacroClassTypeEnum::ACTION_CHECKABLE:
            break;
        case WuQMacroClassTypeEnum::ACTION_GROUP:
            break;
        case WuQMacroClassTypeEnum::BUTTON_GROUP:
            break;
        case WuQMacroClassTypeEnum::CHECK_BOX:
            break;
        case WuQMacroClassTypeEnum::COMBO_BOX:
            break;
        case WuQMacroClassTypeEnum::DOUBLE_SPIN_BOX:
            break;
        case WuQMacroClassTypeEnum::INVALID:
            break;
        case WuQMacroClassTypeEnum::LINE_EDIT:
            break;
        case WuQMacroClassTypeEnum::LIST_WIDGET:
            break;
        case WuQMacroClassTypeEnum::MENU:
            break;
        case WuQMacroClassTypeEnum::MOUSE_USER_EVENT:
            writeMacroMouseEventInfo(macroCommand->getMouseEventInfo());
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON:
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON_CHECKABLE:
            break;
        case WuQMacroClassTypeEnum::RADIO_BUTTON:
            break;
        case WuQMacroClassTypeEnum::SLIDER:
            break;
        case WuQMacroClassTypeEnum::SPIN_BOX:
            break;
        case WuQMacroClassTypeEnum::TAB_BAR:
            break;
        case WuQMacroClassTypeEnum::TAB_WIDGET:
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON:
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON_CHECKABLE:
            break;
    }
    
    const QString toolTip = macroCommand->getObjectToolTip();
    if (toolTip != NULL) {
        m_xmlStreamWriter->writeTextElement(ELEMENT_MACRO_COMMAND_TOOL_TIP,
                                            toolTip);
    }
    
    const int32_t numberOfParameters = macroCommand->getNumberOfParameters();
    for (int32_t i = 0; i < numberOfParameters; i++) {
        const WuQMacroCommandParameter* parameter = macroCommand->getParameterAtIndex(i);
        CaretAssert(parameter);
        
        m_xmlStreamWriter->writeStartElement(ELEMENT_MACRO_COMMAND_PARAMETER);
        
        m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MACRO_COMMAND_PARAMETER_DATA_TYPE,
                                          WuQMacroDataValueTypeEnum::toName(parameter->getDataType()));
        
        const QVariant value(parameter->getValue());
        QString stringValue;
        switch (parameter->getDataType()) {
            case WuQMacroDataValueTypeEnum::INVALID:
                break;
            case WuQMacroDataValueTypeEnum::BOOLEAN:
                stringValue = (value.toBool() ? VALUE_BOOL_TRUE : VALUE_BOOL_FALSE);
                break;
            case WuQMacroDataValueTypeEnum::FLOAT:
                stringValue = QString::number(value.toFloat());
                break;
            case WuQMacroDataValueTypeEnum::INTEGER:
                stringValue = QString::number(value.toInt());
                break;
            case WuQMacroDataValueTypeEnum::MOUSE:
                stringValue = "MouseEvent";
                break;
            case WuQMacroDataValueTypeEnum::NONE:
                stringValue = "";
                break;
            case WuQMacroDataValueTypeEnum::STRING:
                stringValue = value.toString();
                break;
        }
        m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MACRO_COMMAND_PARAMETER_VALUE,
                                          stringValue);
        m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MACRO_COMMAND_PARAMETER_NAME,
                                          parameter->getName());
        m_xmlStreamWriter->writeEndElement();
    }
    
    m_xmlStreamWriter->writeEndElement();
}

/**
 * Write a macro mouse event info to XML format
 *
 * @param mouseEventInfo
 *     The macro mouse event info
 */
void
WuQMacroGroupXmlStreamWriter::writeMacroMouseEventInfo(const WuQMacroMouseEventInfo* mouseEventInfo)
{
    CaretAssert(mouseEventInfo);
    
    m_xmlStreamWriter->writeStartElement(ELEMENT_MACRO_COMMAND_MOUSE_EVENT_INFO);
    
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_BUTTON, QString::number(mouseEventInfo->getMouseButton()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_BUTTONS_MASK, QString::number(mouseEventInfo->getMouseButtonsMask()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_EVENT_TYPE, WuQMacroMouseEventTypeEnum::toName(mouseEventInfo->getMouseEventType()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_KEYBOARD_MODIFIERS_MASK, QString::number(mouseEventInfo->getKeyboardModifiersMask()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_WIDGET_WIDTH, QString::number(mouseEventInfo->getWidgetWidth()));
    m_xmlStreamWriter->writeAttribute(ATTRIBUTE_MOUSE_WIDGET_HEIGHT, QString::number(mouseEventInfo->getWidgetHeight()));
    
    QString xyString;
    QTextStream textStream(&xyString);
    const int32_t numXY = mouseEventInfo->getNumberOfLocalXY();
    for (int32_t i = 0; i < numXY; i++) {
        textStream << mouseEventInfo->getLocalX(i) << " "
        << mouseEventInfo->getLocalY(i) << " ";
    }
    
    m_xmlStreamWriter->writeCharacters(xyString);
    
    m_xmlStreamWriter->writeEndElement();
}

