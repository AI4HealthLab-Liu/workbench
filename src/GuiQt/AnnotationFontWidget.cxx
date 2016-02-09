
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

#include <QAction>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#define __ANNOTATION_FONT_WIDGET_DECLARE__
#include "AnnotationFontWidget.h"
#undef __ANNOTATION_FONT_WIDGET_DECLARE__

#include "AnnotationColorBar.h"
#include "AnnotationFontAttributesInterface.h"
#include "AnnotationManager.h"
#include "AnnotationRedoUndoCommand.h"
#include "AnnotationTextFontNameEnum.h"
#include "AnnotationTextFontPointSizeEnum.h"
#include "AnnotationPercentSizeText.h"
#include "Brain.h"
#include "CaretAssert.h"
#include "CaretColorEnumMenu.h"
#include "EnumComboBoxTemplate.h"
#include "EventGraphicsUpdateAllWindows.h"
#include "EventManager.h"
#include "GuiManager.h"
#include "WuQSpecialIncrementDoubleSpinBox.h"
#include "WuQtUtilities.h"

using namespace caret;



/**
 * \class caret::AnnotationFontWidget
 * \brief Widget for annotation font selection
 * \ingroup GuiQt
 */

/**
 * Processes increment and decrements for double spin box.
 */
class FontSizeFunctionObject : public WuQSpecialIncrementDoubleSpinBox::StepFunctionObject {
public:
    double getNewValue(const double currentValue,
                       const int steps) const {
        const double stepAmount = 0.1;
        //const double stepAmount = currentValue * 0.10;
        const double outputValue = currentValue + (stepAmount * steps);
        return outputValue;
    }
};

/**
 * Constructor.
 *
 * @param browserWindowIndex
 *     Index of window in which this instance is displayed
 * @param parent
 *     Parent for this widget.
 */
AnnotationFontWidget::AnnotationFontWidget(const AnnotationWidgetParentEnum::Enum parentWidgetType,
                                           const int32_t browserWindowIndex,
                                           QWidget* parent)
: QWidget(parent),
m_parentWidgetType(parentWidgetType),
m_browserWindowIndex(browserWindowIndex)
{
    m_annotationColorBar = NULL;
    
    /*
     * "Font" label
     */
    QLabel* fontLabel = new QLabel("Font ");
    
    /*
     * Combo box for font name selection
     */
    m_fontNameComboBox = new EnumComboBoxTemplate(this);
    m_fontNameComboBox->setup<AnnotationTextFontNameEnum,AnnotationTextFontNameEnum::Enum>();
    QObject::connect(m_fontNameComboBox, SIGNAL(itemActivated()),
                     this, SLOT(fontNameChanged()));
    WuQtUtilities::setToolTipAndStatusTip(m_fontNameComboBox->getWidget(),
                                          "Change font");
    
    /*
     * Combo box for font size
     */
    m_fontSizeSpinBox = new WuQSpecialIncrementDoubleSpinBox(new FontSizeFunctionObject);
    m_fontSizeSpinBox->setRange(0.0, 100.0);
    m_fontSizeSpinBox->setDecimals(1);
    m_fontSizeSpinBox->setSingleStep(0.1);
    m_fontSizeSpinBox->setSuffix("%");
    QObject::connect(m_fontSizeSpinBox, SIGNAL(valueChanged(double)),
                     this, SLOT(fontSizeChanged()));
    WuQtUtilities::setToolTipAndStatusTip(m_fontSizeSpinBox,
                                          "Change font size (height) as percentage, zero to one-hundred, of viewport height");
    

    /*
     * Text color menu
     */
    m_textColorMenu = new CaretColorEnumMenu((CaretColorEnum::OPTION_INCLUDE_CUSTOM_COLOR
                                                    | CaretColorEnum::OPTION_INCLUDE_NONE_COLOR));
    QObject::connect(m_textColorMenu, SIGNAL(colorSelected(const CaretColorEnum::Enum)),
                     this, SLOT(textColorSelected(const CaretColorEnum::Enum)));
    
    /*
     * Text color action and toolbutton
     */
    QLabel* textColorLabel = new QLabel("Color");
    const QSize toolButtonSize(16, 16);
    m_textColorAction = new QAction("C",
                                    this);
    m_textColorAction->setToolTip("Adjust the text color");
    m_textColorAction->setMenu(m_textColorMenu);
    m_textColorToolButton = new QToolButton();
    m_textColorToolButton->setDefaultAction(m_textColorAction);
    m_textColorToolButton->setIconSize(toolButtonSize);
    
    QToolButton* boldFontToolButton      = NULL;
    QToolButton* italicFontToolButton    = NULL;
    QToolButton* underlineFontToolButton = NULL;
    m_boldFontAction      = NULL;
    m_italicFontAction    = NULL;
    m_underlineFontAction = NULL;
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            /*
             * Bold Font
             */
            m_boldFontAction = WuQtUtilities::createAction("B", //boldFontText.toStringWithHtmlBody(),
                                                           "Enable/disable bold styling",
                                                           this, this, SLOT(fontBoldChanged()));
            m_boldFontAction->setCheckable(true);
            boldFontToolButton = new QToolButton();
            boldFontToolButton->setDefaultAction(m_boldFontAction);
            
            /*
             * Change the bold toolbutton's font to bold.
             */
            QFont boldFont = boldFontToolButton->font();
            boldFont.setBold(true);
            boldFontToolButton->setFont(boldFont);
            
            /*
             * Italic font toolbutton
             */
            m_italicFontAction = WuQtUtilities::createAction("i", "Enable/disable italic styling",
                                                             this, this, SLOT(fontItalicChanged()));
            m_italicFontAction->setCheckable(true);
            italicFontToolButton = new QToolButton();
            italicFontToolButton->setDefaultAction(m_italicFontAction);
            
            /*
             * Change the italic toolbutton's font to italic.
             */
            QFont italicFont = italicFontToolButton->font();
            italicFont.setItalic(true);
            italicFontToolButton->setFont(italicFont);
            
            /*
             * Underline font toolbutton
             */
            m_underlineFontAction =  WuQtUtilities::createAction("U", "Enable/disable font underlining",
                                                                 this, this, SLOT(fontUnderlineChanged()));
            m_underlineFontAction->setCheckable(true);
            underlineFontToolButton = new QToolButton();
            underlineFontToolButton->setDefaultAction(m_underlineFontAction);
            
            /*
             * Change the underline toolbutton's font to underline.
             */
            QFont underlineFont = underlineFontToolButton->font();
            underlineFont.setUnderline(true);
            underlineFontToolButton->setFont(underlineFont);
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            break;
    }
    
    
    /*
     * Layout the widgets
     */
    
    const bool newLayoutFlag = true;
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
            if (newLayoutFlag) {
                QLabel* sizeLabel  = new QLabel("Size");
                QLabel* styleLabel = new QLabel("Style");
                
                QHBoxLayout* stylesLayout = new QHBoxLayout();
                WuQtUtilities::setLayoutSpacingAndMargins(stylesLayout, 0, 0);
                //stylesLayout->addStretch();
                stylesLayout->addWidget(boldFontToolButton);
                stylesLayout->addWidget(italicFontToolButton);
                stylesLayout->addWidget(underlineFontToolButton);
                stylesLayout->addStretch();
                
                QGridLayout* fontNameSizeLayout = new QGridLayout(this);
                WuQtUtilities::setLayoutSpacingAndMargins(fontNameSizeLayout, 2, 0);
                fontNameSizeLayout->setColumnStretch(0, 0);
                fontNameSizeLayout->setColumnStretch(1, 0);
                fontNameSizeLayout->setColumnStretch(2, 0);
                fontNameSizeLayout->setColumnStretch(3, 100);
                fontNameSizeLayout->addWidget(fontLabel, 0, 0);
                fontNameSizeLayout->addWidget(m_fontNameComboBox->getWidget(),
                                              0, 1, 1, 3);
                fontNameSizeLayout->addWidget(sizeLabel, 1, 0);
                fontNameSizeLayout->addWidget(m_fontSizeSpinBox,
                                              1, 1);
                fontNameSizeLayout->addWidget(styleLabel, 2, 0);
                fontNameSizeLayout->addLayout(stylesLayout, 2, 1);
                fontNameSizeLayout->addWidget(textColorLabel, 1, 2);
                fontNameSizeLayout->addWidget(m_textColorToolButton, 2, 2);
                
                
//                QVBoxLayout* layout = new QVBoxLayout(this);
//                WuQtUtilities::setLayoutSpacingAndMargins(layout, 0, 0);
//                layout->addLayout(fontNameSizeLayout);
//                layout->addLayout(stylesLayout);
            }
            else {
                QHBoxLayout* topRowLayout = new QHBoxLayout();
                WuQtUtilities::setLayoutSpacingAndMargins(topRowLayout, 2, 0);
                topRowLayout->addWidget(fontLabel, 0);
                topRowLayout->addWidget(m_fontNameComboBox->getWidget(), 100);
                QHBoxLayout* bottomRowLayout = new QHBoxLayout();
                WuQtUtilities::setLayoutSpacingAndMargins(bottomRowLayout, 2, 0);
                
                bottomRowLayout->addWidget(boldFontToolButton);
                bottomRowLayout->addWidget(italicFontToolButton);
                bottomRowLayout->addWidget(underlineFontToolButton);
                bottomRowLayout->addStretch();
                bottomRowLayout->addWidget(m_fontSizeSpinBox);
                
                QVBoxLayout* layout = new QVBoxLayout(this);
                WuQtUtilities::setLayoutSpacingAndMargins(layout, 0, 0);
                layout->addLayout(topRowLayout);
                layout->addLayout(bottomRowLayout);
                //            layout->addStretch();
            }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
        {
            QLabel* sizeLabel = new QLabel("Size");
            
            QGridLayout* gridLayout = new QGridLayout(this);
            WuQtUtilities::setLayoutSpacingAndMargins(gridLayout, 2, 2);
            gridLayout->addWidget(fontLabel, 0, 0);
            gridLayout->addWidget(m_fontNameComboBox->getWidget(), 0, 1);
            gridLayout->addWidget(sizeLabel, 1, 0);
            gridLayout->addWidget(m_fontSizeSpinBox, 1, 1);
        }
            break;
    }
    
    
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
}

/**
 * Destructor.
 */
AnnotationFontWidget::~AnnotationFontWidget()
{
}

/**
 * Update the content of this widget with the given annotations.
 *
 * @param annotations
 *     The selected annotations.
 */
void
AnnotationFontWidget::updateContent(std::vector<AnnotationFontAttributesInterface*>& annotations)
{
    m_annotations = annotations;
    
    if ( ! m_annotations.empty()) {
        bool boldOnFlag        = true;
        bool italicOnFlag      = true;
        bool underlineOnFlag   = true;
        int32_t stylesEnabledCount = 0;
        
        AnnotationTextFontNameEnum::Enum fontName = AnnotationTextFontNameEnum::VERA;
        bool fontNameValid = true;
        float fontSizeValue = 5.0;
        bool haveMultipleFontSizeValues = false;
        
        const int32_t numAnn = static_cast<int32_t>(m_annotations.size());
        for (int32_t i = 0; i < numAnn; i++) {
            CaretAssertVectorIndex(m_annotations, i);
            const AnnotationFontAttributesInterface* annText = m_annotations[i];
            CaretAssert(annText);
            
            const float sizeValue = annText->getFontPercentViewportSize();
            if (i == 0) {
                fontName = annText->getFont();
                fontSizeValue = sizeValue;
            }
            else {
                if (annText->getFont() != fontName) {
                    fontNameValid = false;
                }
                if (fontSizeValue != sizeValue) {
                    haveMultipleFontSizeValues = true;
                    fontSizeValue = std::min(fontSizeValue,
                                             sizeValue);
                }
            }
            
            if (annText->isStylesSupported()) {
                if ( ! annText->isBoldStyleEnabled()) {
                    boldOnFlag = false;
                }
                if ( ! annText->isItalicStyleEnabled()) {
                    italicOnFlag = false;
                }
                if ( ! annText->isUnderlineStyleEnabled()) {
                    underlineOnFlag = false;
                }
                
                ++stylesEnabledCount;
            }
        }
        
        m_fontNameComboBox->setSelectedItem<AnnotationTextFontNameEnum,AnnotationTextFontNameEnum::Enum>(fontName);
        
        updateFontSizeSpinBox(fontSizeValue,
                              haveMultipleFontSizeValues);
        
        /*
         * Font styles are ON only if all selected
         * text annotations have the style enabled
         */
        const bool stylesEnabledFlag = (stylesEnabledCount > 0);
        
        m_boldFontAction->setEnabled(stylesEnabledFlag);
        m_boldFontAction->setChecked(boldOnFlag && stylesEnabledFlag);

        m_italicFontAction->setEnabled(stylesEnabledFlag);
        m_italicFontAction->setChecked(italicOnFlag && stylesEnabledFlag);
        
        m_underlineFontAction->setEnabled(stylesEnabledFlag);
        m_underlineFontAction->setChecked(underlineOnFlag && stylesEnabledFlag);
        
        AnnotationText::setUserDefaultFont(fontName);
        AnnotationText::setUserDefaultFontPercentViewportSize(fontSizeValue);
        if (stylesEnabledFlag) {
            AnnotationText::setUserDefaultBoldEnabled(boldOnFlag);
            AnnotationText::setUserDefaultItalicEnabled(italicOnFlag);
            AnnotationText::setUserDefaultUnderlineEnabled(underlineOnFlag);
        }
    }
    
    updateTextColorButton();
    
    setEnabled( ! m_annotations.empty());
}

///**
// * Update the content of this widget with the given text annotation.
// *
// * @param annotationColorBar
// *     Color bar for display (may be NULL).
// */
//void
//AnnotationFontWidget::updateAnnotationColorBarContent(AnnotationColorBar* annotationColorBar)
//{
//    m_annotationColorBar = annotationColorBar;
//    
//    if (m_annotationColorBar != NULL) {
//        m_fontNameComboBox->setSelectedItem<AnnotationTextFontNameEnum,AnnotationTextFontNameEnum::Enum>(m_annotationColorBar->getFont());
//        updateFontSizeSpinBox(m_annotationColorBar->getFontPercentViewportSize(),
//                              false);
//    }
//    
//    updateTextColorButton();
//}

/**
 * Update the font size spin box.
 *
 * @param value
 *     New value for font size spin box.
 * @param haveMultipleValuesFlag
 *     If true, there are multiple font size values so indicate
 *     this with a '+' sign as a suffix
 */
void
AnnotationFontWidget::updateFontSizeSpinBox(const float value,
                                            const bool haveMultipleValuesFlag)
{
    m_fontSizeSpinBox->blockSignals(true);
    m_fontSizeSpinBox->setValue(value);
    m_fontSizeSpinBox->blockSignals(false);
    QString fontSizeSuffix("%");
    if (haveMultipleValuesFlag) {
        fontSizeSuffix = "%+";
    }
    m_fontSizeSpinBox->setSuffix(fontSizeSuffix);
}

/**
 * Gets called when the text color is changed.
 *
 * @param caretColor
 *     Color that was selected.
 */
void
AnnotationFontWidget::textColorSelected(const CaretColorEnum::Enum caretColor)
{
    if ( ! m_annotations.empty()) {
        float rgba[4];
        m_annotations[0]->getTextColorRGBA(rgba);
        
        if (caretColor == CaretColorEnum::CUSTOM) {
            QColor color;
            color.setRgbF(rgba[0], rgba[1], rgba[2]);
            
            QColor newColor = QColorDialog::getColor(color,
                                                     m_textColorToolButton,
                                                     "Text Color");
            if (newColor.isValid()) {
                rgba[0] = newColor.redF();
                rgba[1] = newColor.greenF();
                rgba[2] = newColor.blueF();
                
                
                switch (m_parentWidgetType) {
                    case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                        AnnotationText::setUserDefaultCustomTextColor(rgba);
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
                undoCommand->setModeTextColor(caretColor,
                                              rgba,
                                              annMan->getSelectedAnnotations(m_browserWindowIndex));
                annMan->applyCommand(undoCommand);
                
                AnnotationText::setUserDefaultTextColor(caretColor);
            }
                break;
            case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                if (m_annotationColorBar != NULL) {
                    m_annotationColorBar->setTextColor(caretColor);
                    m_annotationColorBar->setCustomTextColor(rgba);
                }
                break;
        }
    }
    
    updateTextColorButton();
    
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
 * Update the text color button.
 */
void
AnnotationFontWidget::updateTextColorButton()
{
    CaretColorEnum::Enum colorEnum = CaretColorEnum::NONE;
    float rgba[4];
    CaretColorEnum::toRGBFloat(colorEnum, rgba);
    rgba[3] = 1.0;
    
    bool colorButtonValidFlag = false;
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            const int32_t numAnnotations = static_cast<int32_t>(m_annotations.size());
            if (numAnnotations > 0) {
                bool firstColorSupportFlag = true;
                bool allSameColorFlag = true;
                
                for (int32_t i = 0; i < numAnnotations; i++) {
                    if (firstColorSupportFlag) {
                        m_annotations[i]->getTextColorRGBA(rgba);
                        firstColorSupportFlag = false;
                        colorButtonValidFlag = true;
                    }
                    else {
                        float colorRGBA[4];
                        m_annotations[i]->getTextColorRGBA(colorRGBA);
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
                    colorEnum = m_annotations[0]->getTextColor();
                    m_annotations[0]->getTextColorRGBA(rgba);
                    
                    float customRGBA[4];
                    m_annotations[0]->getCustomTextColor(customRGBA);
                    m_textColorMenu->setCustomIconColor(customRGBA);
                    
                    switch (m_parentWidgetType) {
                        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
                            AnnotationText::setUserDefaultTextColor(colorEnum);
                            AnnotationText::setUserDefaultCustomTextColor(customRGBA);
                            break;
                        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
                            break;
                    }
                    
                }
            }
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            if (m_annotationColorBar != NULL) {
                colorEnum = m_annotationColorBar->getTextColor();
                m_annotationColorBar->getCustomTextColor(rgba);
                colorButtonValidFlag = true;
            }
            break;
    }
    
    
    QPixmap pm = WuQtUtilities::createCaretColorEnumPixmap(m_textColorToolButton, 24, 24, colorEnum, rgba, true);
    m_textColorAction->setIcon(QIcon(pm));
    m_textColorMenu->setSelectedColor(colorEnum);
    
    if (colorButtonValidFlag) {
        
    }
}

/**
 * Gets called when font bold changed.
 */
void
AnnotationFontWidget::fontBoldChanged()
{
    AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
    AnnotationRedoUndoCommand* command = new AnnotationRedoUndoCommand();
    command->setModeTextFontBold(m_boldFontAction->isChecked(),
                                 annMan->getSelectedAnnotations(m_browserWindowIndex));
    annMan->applyCommand(command);
    
    EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    
    AnnotationText::setUserDefaultBoldEnabled(m_boldFontAction->isChecked());
}

/**
 * Gets called when font italic changed.
 */
void
AnnotationFontWidget::fontItalicChanged()
{
    AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
    AnnotationRedoUndoCommand* command = new AnnotationRedoUndoCommand();
    command->setModeTextFontItalic(m_italicFontAction->isChecked(),
                                   annMan->getSelectedAnnotations(m_browserWindowIndex));
    annMan->applyCommand(command);
    
    EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    
    AnnotationText::setUserDefaultItalicEnabled(m_italicFontAction->isChecked());
}

/**
 * Gets called when font name changed.
 */
void
AnnotationFontWidget::fontNameChanged()
{
    const AnnotationTextFontNameEnum::Enum fontName = m_fontNameComboBox->getSelectedItem<AnnotationTextFontNameEnum,AnnotationTextFontNameEnum::Enum>();
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
            AnnotationRedoUndoCommand* command = new AnnotationRedoUndoCommand();
            command->setModeTextFontName(fontName,
                                         annMan->getSelectedAnnotations(m_browserWindowIndex));
            annMan->applyCommand(command);
            
            EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
            AnnotationText::setUserDefaultFont(fontName);
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            if (m_annotationColorBar != NULL) {
                m_annotationColorBar->setFont(fontName);
            }
            break;
    }
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    
}

/**
 * Gets called when font size changed.
 */
void
AnnotationFontWidget::fontSizeChanged()
{
    const float fontPercentSize = m_fontSizeSpinBox->value();
    
    switch (m_parentWidgetType) {
        case AnnotationWidgetParentEnum::ANNOTATION_TOOL_BAR_WIDGET:
        {
            AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
            AnnotationRedoUndoCommand* command = new AnnotationRedoUndoCommand();
            command->setModeTextFontPercentSize(fontPercentSize,
                                                annMan->getSelectedAnnotations(m_browserWindowIndex));
            annMan->applyCommand(command);
            
            EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
            AnnotationText::setUserDefaultFontPercentViewportSize(fontPercentSize);
        }
            break;
        case AnnotationWidgetParentEnum::COLOR_BAR_EDITOR_WIDGET:
            if (m_annotationColorBar != NULL) {
                m_annotationColorBar->setFontPercentViewportSize(fontPercentSize);
            }
            break;
    }
    
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    
}

/**
 * Gets called when font underline changed.
 */
void
AnnotationFontWidget::fontUnderlineChanged()
{
    AnnotationManager* annMan = GuiManager::get()->getBrain()->getAnnotationManager();
    AnnotationRedoUndoCommand* command = new AnnotationRedoUndoCommand();
    command->setModeTextFontUnderline(m_underlineFontAction->isChecked(),
                                      annMan->getSelectedAnnotations(m_browserWindowIndex));
    annMan->applyCommand(command);
    
    EventManager::get()->sendSimpleEvent(EventTypeEnum::EVENT_ANNOTATION_TOOLBAR_UPDATE);
    EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
    
    AnnotationText::setUserDefaultUnderlineEnabled(m_underlineFontAction->isChecked());
}

/**
 * Create a horizontal alignment pixmap.
 *
 * @param widget
 *    To color the pixmap with backround and foreground,
 *    the palette from the given widget is used.
 * @return
 *    Pixmap with icon for the given horizontal alignment.
 */
QPixmap
AnnotationFontWidget::createOutlineButtonPixmap(const QWidget* widget)
{
    CaretAssert(widget);
    
    /*
     * Create a small, square pixmap that will contain
     * the foreground color around the pixmap's perimeter.
     */
    float width  = 24;
    float height = 24;
//
    QPixmap pixmap(static_cast<int>(width),
                   static_cast<int>(height));
    QSharedPointer<QPainter> painter = WuQtUtilities::createPixmapWidgetPainter(widget,
                                                                                pixmap);
    QPen pen = painter->pen();
    pen.setWidthF(pen.widthF() * 2.0);
    painter->setPen(pen);
    QFont font = painter->font();
    const int fontHeight = 16;
    font.setPointSize(fontHeight);
    painter->setFont(font);
    
    const int boxMargin = 2;
    const int boxWidth  = width - (boxMargin * 2);
    const int boxHeight = height - (boxMargin * 2);
    const int boxX = boxMargin;
    const int boxY = boxMargin;
    painter->drawRect(boxX, boxY, boxWidth, boxHeight);
    
    painter->drawText(boxX, boxY + 2,
                      boxWidth, boxHeight - 2,
                      (Qt::AlignCenter),
                      "O");
    
    return pixmap;
}

