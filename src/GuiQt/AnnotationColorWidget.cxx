
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

#define __ANNOTATION_COLOR_WIDGET_DECLARE__
#include "AnnotationColorWidget.h"
#undef __ANNOTATION_COLOR_WIDGET_DECLARE__

#include <QAction>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "AnnotationManager.h"
#include "AnnotationOneDimensionalShape.h"
#include "AnnotationTwoDimensionalShape.h"
#include "AnnotationRedoUndoCommand.h"
#include "Brain.h"
#include "BrainOpenGL.h"
#include "CaretAssert.h"
#include "CaretColorEnumMenu.h"
#include "EventGraphicsUpdateAllWindows.h"
#include "EventManager.h"
#include "GuiManager.h"
#include "WuQFactory.h"
#include "WuQWidgetObjectGroup.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::AnnotationColorWidget 
 * \brief Widget for annotation color selection.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param parent
 *     Parent for this widget.
 */
AnnotationColorWidget::AnnotationColorWidget(const AnnotationWidgetParentEnum::Enum parentWidgetType,
                                             const int32_t browserWindowIndex,
                                             QWidget* parent)
: QWidget(parent),
m_parentWidgetType(parentWidgetType),
m_browserWindowIndex(browserWindowIndex)
{
    
    QLabel* backFillLabel      = new QLabel("Fill");
    QLabel* backFillColorLabel = new QLabel("Color");
    QLabel* foreLineLabel      = new QLabel("Line");
    QLabel* foreLineColorLabel = new QLabel("Color");
    
    QLabel* lineWidthLabel = NULL;
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
            lineWidthLabel = new QLabel("Width");
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            backFillLabel->setText("Background");
            foreLineLabel->setText("Text");
            break;
    }
    
    
    const QSize toolButtonSize(16, 16);
    
    /*
     * Background color menu
     */
    m_backgroundColorMenu = new CaretColorEnumMenu((CaretColorEnum::OPTION_INCLUDE_CUSTOM_COLOR
                                                    | CaretColorEnum::OPTION_INCLUDE_NONE_COLOR));
    QObject::connect(m_backgroundColorMenu, SIGNAL(colorSelected(const CaretColorEnum::Enum)),
                     this, SLOT(backgroundColorSelected(const CaretColorEnum::Enum)));
    
    /*
     * Background action and tool button
     */
    m_backgroundColorAction = new QAction("B",
                                          this);
    m_backgroundColorAction->setToolTip("Adjust the fill color");
    m_backgroundColorAction->setMenu(m_backgroundColorMenu);
    m_backgroundToolButton = new QToolButton();
    m_backgroundToolButton->setDefaultAction(m_backgroundColorAction);
    m_backgroundToolButton->setIconSize(toolButtonSize);
    
    /*
     * Widget/object group for background widgets
     */
    m_backgroundWidgetGroup = new WuQWidgetObjectGroup(this);
    m_backgroundWidgetGroup->add(backFillLabel);
    m_backgroundWidgetGroup->add(backFillColorLabel);
    m_backgroundWidgetGroup->add(m_backgroundToolButton);
    
    /*
     * Foreground color menu
     */
    m_foregroundColorMenu = new CaretColorEnumMenu((CaretColorEnum::OPTION_INCLUDE_CUSTOM_COLOR
                                                    | CaretColorEnum::OPTION_INCLUDE_NONE_COLOR));
    QObject::connect(m_foregroundColorMenu, SIGNAL(colorSelected(const CaretColorEnum::Enum)),
                     this, SLOT(foregroundColorSelected(const CaretColorEnum::Enum)));
    
    /*
     * Foreground color action and toolbutton
     */
    m_foregroundColorAction = new QAction("F",
                                          this);
    m_foregroundColorAction->setToolTip("Adjust the line/text color");
    m_foregroundColorAction->setMenu(m_foregroundColorMenu);
    m_foregroundToolButton = new QToolButton();
    m_foregroundToolButton->setDefaultAction(m_foregroundColorAction);
    m_foregroundToolButton->setIconSize(toolButtonSize);
    
    /*
     * Foreground thickness
     */
    float minimumLineWidth = 0.0;
    float maximumLineWidth = 1.0;
    
    m_foregroundThicknessSpinBox = NULL;
    if (lineWidthLabel != NULL) {
        BrainOpenGL::getMinMaxLineWidth(minimumLineWidth,
                                        maximumLineWidth);
        minimumLineWidth = std::max(minimumLineWidth, 1.0f);
        m_foregroundThicknessSpinBox = WuQFactory::newDoubleSpinBoxWithMinMaxStepDecimalsSignalDouble(minimumLineWidth,
                                                                                                      maximumLineWidth,
                                                                                                      1.0,
                                                                                                      0,
                                                                                                      this,
                                                                                                      SLOT(foregroundThicknessSpinBoxValueChanged(double)));
        WuQtUtilities::setWordWrappedToolTip(m_foregroundThicknessSpinBox,
                                             "Adjust the line thickness");
        m_foregroundThicknessSpinBox->setFixedWidth(45);
    }
    

    QGridLayout* gridLayout = new QGridLayout(this);
    WuQtUtilities::setLayoutSpacingAndMargins(gridLayout, 2, 0);
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            CaretAssert(lineWidthLabel);
            gridLayout->addWidget(foreLineLabel,
                                  0, 0,
                                  1, 2,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(lineWidthLabel,
                                  1, 0,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(foreLineColorLabel,
                                  1, 1,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(m_foregroundThicknessSpinBox,
                                  2, 0,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(m_foregroundToolButton,
                                  2, 1,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(backFillLabel,
                                  0, 2,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(backFillColorLabel,
                                  1, 2,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(m_backgroundToolButton,
                                  2, 2,
                                  Qt::AlignHCenter);
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
        {
            CaretAssert(lineWidthLabel == NULL);
            
            gridLayout->addWidget(foreLineLabel,
                                  0, 0,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(foreLineColorLabel,
                                  1, 0,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(m_foregroundToolButton,
                                  2, 0,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(backFillLabel,
                                  0, 1,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(backFillColorLabel,
                                  1, 1,
                                  Qt::AlignHCenter);
            gridLayout->addWidget(m_backgroundToolButton,
                                  2, 1,
                                  Qt::AlignHCenter);
        }
            break;
    }
    
    /*
     * Layout widgets
     */
    backgroundColorSelected(CaretColorEnum::WHITE);
    foregroundColorSelected(CaretColorEnum::BLACK);
    
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
}

/**
 * Destructor.
 */
AnnotationColorWidget::~AnnotationColorWidget()
{
}

/**
 * Update with the given annotation.
 *
 * @param annotations
 */
void
AnnotationColorWidget::updateContent(std::vector<Annotation*>& annotations)
{
    m_annotations = annotations;
    
    if ( ! m_annotations.empty()) {
        setEnabled(true);
    }
    else {
        setEnabled(false);
    }
    
    updateBackgroundColorButton();
    updateForegroundColorButton();
    updateForegroundThicknessSpinBox();
}

/**
 * Gets called when the background color is changed.
 *
 * @param caretColor
 *     Color that was selected.
 */
void
AnnotationColorWidget::backgroundColorSelected(const CaretColorEnum::Enum caretColor)
{
    if (! m_annotations.empty()) {
        float rgba[4];
        m_annotations[0]->getCustomBackgroundColor(rgba);
        
        if (caretColor == CaretColorEnum::CUSTOM) {
            QColor color;
            color.setRgbF(rgba[0], rgba[1], rgba[2]);
            
            QColor newColor = QColorDialog::getColor(color,
                                                     m_backgroundToolButton,
                                                     "Background Color");
            if (newColor.isValid()) {
                rgba[0] = newColor.redF();
                rgba[1] = newColor.greenF();
                rgba[2] = newColor.blueF();
                

                switch (m_parentWidgetType) {
                    case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                        Annotation::setUserDefaultCustomBackgroundColor(rgba);
                        break;
                    case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                        break;
                }
            }
        }
        
        switch (m_parentWidgetType) {
            case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
            {
                AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
                AnnotationRedoUndoCommand* undoCommand = new AnnotationRedoUndoCommand();
                undoCommand->setModeColorBackground(caretColor,
                                                    rgba,
                                                    annMan->getSelectedAnnotations());
                annMan->applyCommand(undoCommand);
                
                Annotation::setUserDefaultBackgroundColor(caretColor);
                EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
            }
                break;
            case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                for (std::vector<Annotation*>::iterator iter = m_annotations.begin();
                     iter != m_annotations.end();
                     iter++) {
                    Annotation* ann = *iter;
                    ann->setBackgroundColor(caretColor);
                    if (caretColor == CaretColorEnum::CUSTOM) {
                        ann->setCustomBackgroundColor(rgba);
                    }
                }
                break;
        }
    }

    updateBackgroundColorButton();
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
}

/**
 * Update the background color.
 */
void
AnnotationColorWidget::updateBackgroundColorButton()
{
    CaretColorEnum::Enum colorEnum = CaretColorEnum::NONE;
    float rgba[4];
    CaretColorEnum::toRGBFloat(colorEnum, rgba);
    rgba[3] = 1.0;
    
    const int32_t numAnnotations = static_cast<int32_t>(m_annotations.size());
    if (numAnnotations > 0) {
        bool firstColorSupportFlag = true;
        bool enableBackgroundFlag = false;
        bool allSameColorFlag = true;
        
        for (int32_t i = 0; i < numAnnotations; i++) {
            if (m_annotations[0]->isBackgroundColorSupported()) {
                if (firstColorSupportFlag) {
                    m_annotations[i]->getBackgroundColorRGBA(rgba);
                    firstColorSupportFlag = false;
                    enableBackgroundFlag = true;
                }
                else {
                    float colorRGBA[4];
                    m_annotations[i]->getBackgroundColorRGBA(colorRGBA);
                    for (int32_t iColor = 0; iColor < 4; iColor++) {
                        if (rgba[iColor] != colorRGBA[iColor]) {
                            allSameColorFlag = false;
                            break;
                        }
                    }
                    
                    if ( ! allSameColorFlag) {
                        break;
                    }
                }
            }
        }
        
        if (allSameColorFlag) {
            colorEnum = m_annotations[0]->getBackgroundColor();
            m_annotations[0]->getBackgroundColorRGBA(rgba);
            
            float customRGBA[4];
            m_annotations[0]->getCustomBackgroundColor(customRGBA);
            m_backgroundColorMenu->setCustomIconColor(customRGBA);
            
            switch (m_parentWidgetType) {
                case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                    break;
                case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                    break;
            }
            
            switch (m_parentWidgetType) {
                case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                    Annotation::setUserDefaultBackgroundColor(colorEnum);
                    Annotation::setUserDefaultCustomBackgroundColor(customRGBA);
                    break;
                case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                    break;
            }
        }
        
        
        m_backgroundWidgetGroup->setEnabled(enableBackgroundFlag);
        if ( ! enableBackgroundFlag) {
            colorEnum = CaretColorEnum::NONE;
        }
    }
    
    QPixmap pm = WuQtUtilities::createCaretColorEnumPixmap(m_backgroundToolButton,
                                                           24, 24,
                                                           colorEnum,
                                                           rgba,
                                                           false);
    QIcon icon(pm);
    
    m_backgroundColorAction->setIcon(icon);
    m_backgroundColorMenu->setSelectedColor(colorEnum);
}


/**
 * Update the foreground color.
 */
void
AnnotationColorWidget::updateForegroundColorButton()
{
//    CaretColorEnum::Enum colorEnum = CaretColorEnum::WHITE;
//    float rgba[4];
//    CaretColorEnum::toRGBFloat(colorEnum, rgba);
//    rgba[3] = 1.0;
//    
//    if ( ! m_annotations.empty()) {
//        colorEnum = m_annotations[0]->getForegroundColor();
//        m_annotations[0]->getForegroundColorRGBA(rgba);
//
//        float customRGBA[4];
//        m_annotations[0]->getCustomForegroundColor(customRGBA);
//        m_foregroundColorMenu->setCustomIconColor(customRGBA);
//    }
    CaretColorEnum::Enum colorEnum = CaretColorEnum::NONE;
    float rgba[4];
    CaretColorEnum::toRGBFloat(colorEnum, rgba);
    rgba[3] = 1.0;
    
    const int32_t numAnnotations = static_cast<int32_t>(m_annotations.size());
    if (numAnnotations > 0) {
        bool firstColorSupportFlag = true;
        bool enableForegroundFlag = false;
        bool allSameColorFlag = true;
        
        for (int32_t i = 0; i < numAnnotations; i++) {
                if (firstColorSupportFlag) {
                    m_annotations[i]->getForegroundColorRGBA(rgba);
                    firstColorSupportFlag = false;
                    enableForegroundFlag = true;
                }
                else {
                    float colorRGBA[4];
                    m_annotations[i]->getForegroundColorRGBA(colorRGBA);
                    for (int32_t iColor = 0; iColor < 4; iColor++) {
                        if (rgba[iColor] != colorRGBA[iColor]) {
                            allSameColorFlag = false;
                            break;
                        }
                    }
                    
                    if ( ! allSameColorFlag) {
                        break;
                    }
                }
        }
        
        if (allSameColorFlag) {
            colorEnum = m_annotations[0]->getForegroundColor();
            m_annotations[0]->getForegroundColorRGBA(rgba);
            
            float customRGBA[4];
            m_annotations[0]->getCustomForegroundColor(customRGBA);
            m_foregroundColorMenu->setCustomIconColor(customRGBA);

            switch (m_parentWidgetType) {
                case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                    Annotation::setUserDefaultForegroundColor(colorEnum);
                    Annotation::setUserDefaultCustomForegroundColor(customRGBA);
                    break;
                case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                    break;
            }
            
        }
        
        
        if ( ! enableForegroundFlag) {
            colorEnum = CaretColorEnum::NONE;
        }
    }
    
    QPixmap pm = WuQtUtilities::createCaretColorEnumPixmap(m_foregroundToolButton, 24, 24, colorEnum, rgba, true);
    m_foregroundColorAction->setIcon(QIcon(pm));
    m_foregroundColorMenu->setSelectedColor(colorEnum);
}

/**
 * Gets called when the foreground color is changed.
 *
 * @param caretColor
 *     Color that was selected.
 */
void
AnnotationColorWidget::foregroundColorSelected(const CaretColorEnum::Enum caretColor)
{
    if ( ! m_annotations.empty()) {
        float rgba[4];
        m_annotations[0]->getCustomForegroundColor(rgba);
        
        if (caretColor == CaretColorEnum::CUSTOM) {
            QColor color;
            color.setRgbF(rgba[0], rgba[1], rgba[2]);
            
            QColor newColor = QColorDialog::getColor(color,
                                                     m_backgroundToolButton,
                                                     "Foreground Color");
            if (newColor.isValid()) {
                rgba[0] = newColor.redF();
                rgba[1] = newColor.greenF();
                rgba[2] = newColor.blueF();
                
                
                switch (m_parentWidgetType) {
                    case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                        Annotation::setUserDefaultCustomForegroundColor(rgba);
                        break;
                    case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                        break;
                }
            }
        }
        
        
        switch (m_parentWidgetType) {
            case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
            {
                AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
                AnnotationRedoUndoCommand* undoCommand = new AnnotationRedoUndoCommand();
                undoCommand->setModeColorForeground(caretColor,
                                                    rgba,
                                                    annMan->getSelectedAnnotations());
                annMan->applyCommand(undoCommand);
                
                Annotation::setUserDefaultForegroundColor(caretColor);
            }
                break;
            case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                for (std::vector<Annotation*>::iterator iter = m_annotations.begin();
                     iter != m_annotations.end();
                     iter++) {
                    Annotation* ann = *iter;
                    ann->setForegroundColor(caretColor);
                    if (caretColor == CaretColorEnum::CUSTOM) {
                        ann->setCustomBackgroundColor(rgba);
                    }
                }
                break;
        }
    }
    
    updateForegroundColorButton();
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
            EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            break;
    }
    
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
}

/**
 * Gets called when the foreground thickness value changes.
 *
 * @param value
 *     New value for foreground thickness.
 */
void
AnnotationColorWidget::foregroundThicknessSpinBoxValueChanged(double value)
{
    if ( ! m_foregroundThicknessSpinBox->specialValueText().isEmpty()) {
        if (m_foregroundThicknessSpinBox->specialValueText()
            == m_foregroundThicknessSpinBox->text()) {
            /*
             * Ignore special text which is available when 
             * there are multiple annotations with different
             * foreground thicknesses.
             */
            std::cout << "Ignoring special text " << std::endl;
            return;
        }
    }
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
            AnnotationRedoUndoCommand* undoCommand = new AnnotationRedoUndoCommand();
            undoCommand->setModeLineWidthForeground(value, annMan->getSelectedAnnotations());
            annMan->applyCommand(undoCommand);
            
            EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
            Annotation::setUserDefaultForegroundLineWidth(value);
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            for (std::vector<Annotation*>::iterator iter = m_annotations.begin();
                 iter != m_annotations.end();
                 iter++) {
                Annotation* ann = *iter;
                ann->setForegroundLineWidth(value);
            }
            break;
    }
    
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
}

/**
 * Update the foreground thickness spin box.
 */
void
AnnotationColorWidget::updateForegroundThicknessSpinBox()
{
    if (m_foregroundThicknessSpinBox == NULL) {
        return;
    }
    
    float lineWidthValue = 1.0;
    bool  lineWidthValid = false;
    bool  haveMultipleLineWidthValues = false;
    
    const int32_t numAnnotations = static_cast<int32_t>(m_annotations.size());
    if (numAnnotations > 0) {
        for (int32_t i = 0; i < numAnnotations; i++) {
            if (m_annotations[i]->isForegroundLineWidthSupported()) {
                const float annLineWidth = m_annotations[i]->getForegroundLineWidth();
                if (lineWidthValid) {
                    if (annLineWidth != lineWidthValue) {
                        haveMultipleLineWidthValues = true;
                    }
                    lineWidthValue = std::min(lineWidthValue,
                                              annLineWidth);
                }
                else {
                    lineWidthValue = annLineWidth;
                    lineWidthValid = true;
                }
            }
        }
        
        if (lineWidthValid) {
            switch (m_parentWidgetType) {
                case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                    Annotation::setUserDefaultForegroundLineWidth(lineWidthValue);
                    break;
                case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                    break;
            }
        }
    }
    
    /*
     * When the selected annotations have different line
     * widths, the valid displayed is the minimum line
     * width with a suffix consisting of a plus symbol.
     */
    m_foregroundThicknessSpinBox->blockSignals(true);
    m_foregroundThicknessSpinBox->setValue(lineWidthValue);
    m_foregroundThicknessSpinBox->setEnabled(lineWidthValid);
    if (haveMultipleLineWidthValues) {
        m_foregroundThicknessSpinBox->setSuffix("+");
    }
    else {
        m_foregroundThicknessSpinBox->setSuffix("");
    }
    m_foregroundThicknessSpinBox->blockSignals(false);
}
