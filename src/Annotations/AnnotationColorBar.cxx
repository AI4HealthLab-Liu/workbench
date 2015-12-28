
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

#include <limits>

#define __ANNOTATION_COLOR_BAR_DECLARE__
#include "AnnotationColorBar.h"
#undef __ANNOTATION_COLOR_BAR_DECLARE__

#include "AnnotationColorBarNumericText.h"
#include "AnnotationColorBarSection.h"
#include "CaretAssert.h"
#include "SceneClassAssistant.h"

using namespace caret;


    
/**
 * \class caret::AnnotationColorBar 
 * \brief Annotation used for drawing a color bar.
 * \ingroup Annotations
 */

/**
 * Constructor.
 *
 * @param attributeDefaultType
 *    Type for attribute defaults
 */
AnnotationColorBar::AnnotationColorBar(const AnnotationAttributesDefaultTypeEnum::Enum attributeDefaultType)
: AnnotationTwoDimensionalShape(AnnotationTypeEnum::COLOR_BAR,
                                attributeDefaultType)
{
    reset();
    
    m_sceneAssistant.grabNew(new SceneClassAssistant());
    m_sceneAssistant->add<AnnotationTextFontNameEnum, AnnotationTextFontNameEnum::Enum>("m_fontName",
                                                                                        &m_fontName);
    m_sceneAssistant->add("m_fontPercentViewportHeight",
                          &m_fontPercentViewportHeight);
    m_sceneAssistant->add<AnnotationColorBarPositionModeEnum, AnnotationColorBarPositionModeEnum::Enum>("m_positionMode",
                                                                                                        &m_positionMode);
    m_sceneAssistant->add("m_displayedFlag",
                          &m_displayedFlag);
    
    m_sceneAssistant->add("m_showTickMarksSelected",
                          &m_showTickMarksSelected);
}

/**
 * Destructor.
 */
AnnotationColorBar::~AnnotationColorBar()
{
    clearSections();
    
    clearNumericText();
}

/**
 * Copy constructor.
 * @param obj
 *    Object that is copied.
 */
AnnotationColorBar::AnnotationColorBar(const AnnotationColorBar& obj)
: AnnotationTwoDimensionalShape(obj)
{
    this->copyHelperAnnotationColorBar(obj);
}

/**
 * Assignment operator.
 * @param obj
 *    Data copied from obj to this.
 * @return 
 *    Reference to this object.
 */
AnnotationColorBar&
AnnotationColorBar::operator=(const AnnotationColorBar& obj)
{
    if (this != &obj) {
        AnnotationTwoDimensionalShape::operator=(obj);
        this->copyHelperAnnotationColorBar(obj);
    }
    return *this;    
}

/**
 * Helps with copying an object of this type.
 *
 * @param obj
 *    Object that is copied.
 */
void 
AnnotationColorBar::copyHelperAnnotationColorBar(const AnnotationColorBar& obj)
{
    /*
     * NOTE: sections and numeric text are not copied
     */
    clearSections();
    clearNumericText();
    
    m_positionMode              = obj.m_positionMode;
    m_fontName                  = obj.m_fontName;
    m_fontPercentViewportHeight = obj.m_fontPercentViewportHeight;
    m_positionMode              = obj.m_positionMode;
    m_displayedFlag             = obj.m_displayedFlag;
    m_showTickMarksSelected     = obj.m_showTickMarksSelected;
}

/**
 * Reset the annotation colorbar.
 *
 * DO NOT make this method virtual is it is called from constructor.
 */
void
AnnotationColorBar::reset()
{
    resetSizeAttributes();
    m_fontName      = AnnotationTextFontNameEnum::getDefaultFontName();
    m_positionMode  = AnnotationColorBarPositionModeEnum::AUTOMATIC;
    m_displayedFlag = false;
    m_showTickMarksSelected = false;
    
    setForegroundColor(CaretColorEnum::WHITE);
    setBackgroundColor(CaretColorEnum::BLACK);
    
    clearSections();
    clearNumericText();
}

/**
 * Reset the size attributes of the color bar.
 */
void
AnnotationColorBar::resetSizeAttributes()
{
    setWidth(25.0);
    setHeight(7.0);
    setRotationAngle(0.0);
    m_fontPercentViewportHeight = 3.33;
}


/**
 * @return The font.
 */
AnnotationTextFontNameEnum::Enum
AnnotationColorBar::getFont() const
{
    return m_fontName;
}

/**
 * Set the font.
 *
 * @param font
 *     New value for font.
 */
void
AnnotationColorBar::setFont(const AnnotationTextFontNameEnum::Enum font)
{
    if (font != m_fontName) {
        m_fontName = font;
        setModified();
    }
}

/**
 * @return THe percent viewport height for the font.
 */
float
AnnotationColorBar::getFontPercentViewportSize() const
{
    return m_fontPercentViewportHeight;
}

/**
 * Set the percent viewport size for the font.
 *
 * @param fontPercentViewportHeight
 *     New value for percent viewport height.
 */
void
AnnotationColorBar::setFontPercentViewportSize(const float fontPercentViewportHeight)
{
    if (fontPercentViewportHeight != m_fontPercentViewportHeight) {
        m_fontPercentViewportHeight = fontPercentViewportHeight;
        setModified();
    }
}

/**
 * Are font styles (Bold, Italic, Underline) supported?
 */
bool
AnnotationColorBar::isStylesSupported() const
{
    return false;
}

/**
 * @return
 *    Is bold enabled ?
 */
bool
AnnotationColorBar::isBoldStyleEnabled() const
{
    return false;
}

/**
 * Set bold enabled.
 *
 * @param enabled
 *     New status for bold enabled.
 */
void
AnnotationColorBar::setBoldStyleEnabled(const bool /*enabled*/)
{
}

/**
 * @return
 *    Is italic enabled ?
 */
bool
AnnotationColorBar::isItalicStyleEnabled() const
{
    return false;
}

/**
 * Set italic enabled.
 *
 * @param enabled
 *     New status for italic enabled.
 */
void
AnnotationColorBar::setItalicStyleEnabled(const bool /*enabled*/)
{
}

/**
 * @return
 *    Is underline enabled ?
 */
bool
AnnotationColorBar::isUnderlineStyleEnabled() const
{
    return false;
}

/**
 * Set underline enabled.
 *
 * @param enabled
 *     New status for underline enabled.
 */
void
AnnotationColorBar::setUnderlineStyleEnabled(const bool /*enabled*/)
{
}

/**
 * @return
 *    Is outline enabled ?
 */
bool
AnnotationColorBar::isOutlineStyleEnabled() const
{
    return false;
}

/**
 * Set outline enabled.
 *
 * @param enabled
 *     New status for outline enabled.
 */
void
AnnotationColorBar::setOutlineStyleEnabled(const bool /*enabled*/)
{
}

/**
 * @return Is foreground line width supported?
 * Most annotations support a foreground line width.
 * Annotations that do not support a foreground line width
 * must override this method and return a value of false.
 */
bool
AnnotationColorBar::isForegroundLineWidthSupported() const
{
    return false;
}


/**
 * @return The position mode for the colorbar annotation.
 */
AnnotationColorBarPositionModeEnum::Enum
AnnotationColorBar::getPositionMode() const
{
    return m_positionMode;
}

/**
 * Set the position mode for the colorbar.
 *
 * @param positionMode
 *     New position mode for the colorbar.
 */
void
AnnotationColorBar::setPositionMode(const AnnotationColorBarPositionModeEnum::Enum positionMode)
{
    if (positionMode != m_positionMode) {
        m_positionMode = positionMode;
        setModified();
    }
}

/**
 * @return Display status of colorbar.
 */
bool
AnnotationColorBar::isDisplayed() const
{
    return m_displayedFlag;
}

/**
 * Set the color bar annotation displayed.
 *
 * Note that this also sets the annotation's selection
 * status to off so that if the user turns off display
 * of the annotation while the annotation is selected
 * the annotation does not show up as selected when
 * the color bar is later displayed by the user.
 *
 * @param displayed
 *     New status for display of colorbar.
 */
void
AnnotationColorBar::setDisplayed(const bool displayed)
{
    if (displayed != m_displayedFlag) {
        m_displayedFlag = displayed;
        setDeselected();
        setModified();
    }
}

/**
 * Save subclass data to the scene.
 *
 * @param sceneAttributes
 *    Attributes for the scene.  Scenes may be of different types
 *    (full, generic, etc) and the attributes should be checked when
 *    restoring the scene.
 *
 * @param sceneClass
 *     sceneClass to which data members should be added.  Will always
 *     be valid (non-NULL).
 */
void
AnnotationColorBar::saveSubClassDataToScene(const SceneAttributes* sceneAttributes,
                                            SceneClass* sceneClass)
{
    AnnotationTwoDimensionalShape::saveSubClassDataToScene(sceneAttributes,
                                                           sceneClass);
    m_sceneAssistant->saveMembers(sceneAttributes,
                                  sceneClass);
}

/**
 * Restore file data from the scene.
 *
 * @param sceneAttributes
 *    Attributes for the scene.  Scenes may be of different types
 *    (full, generic, etc) and the attributes should be checked when
 *    restoring the scene.
 *
 * @param sceneClass
 *     sceneClass for the instance of a class that implements
 *     this interface.  Will NEVER be NULL.
 */
void
AnnotationColorBar::restoreSubClassDataFromScene(const SceneAttributes* sceneAttributes,
                                                 const SceneClass* sceneClass)
{
    AnnotationTwoDimensionalShape::restoreSubClassDataFromScene(sceneAttributes,
                                                                sceneClass);
    m_sceneAssistant->restoreMembers(sceneAttributes,
                                     sceneClass);    
}

/**
 * Add a section.
 *
 * Note: sections are not saved to scene so this method DOES NOT change the modified status.
 *
 * @param startScalar
 *     Value of the starting scalar.
 * @param endScalar
 *     Value of the ending scalar.
 * @param startRGBA
 *     RGBA coloring at the starting scalar.
 * @param endRGBA
 *     RGBA coloring at the ending scalar.
 */
void
AnnotationColorBar::addSection(const float startScalar,
                               const float endScalar,
                               const float startRGBA[4],
                               const float endRGBA[4])
{
    m_sections.push_back(new AnnotationColorBarSection(startScalar,
                                                       endScalar,
                                                       startRGBA,
                                                       endRGBA));
}

/**
 * Clear the sections.
 *
 * Note: sections are not saved to scene so this method DOES NOT change the modified status.
 */
void
AnnotationColorBar::clearSections()
{
    for (std::vector<const AnnotationColorBarSection*>::iterator iter = m_sections.begin();
         iter != m_sections.end();
         iter++) {
        delete *iter;
    }
    
    m_sections.clear();
}

/**
 * @return Number of sections.
 */
int32_t
AnnotationColorBar::getNumberOfSections() const
{
    return m_sections.size();
}

/**
 * @return Section at the given index.
 *
 * @param index
 *     Index of the section.
 */
const AnnotationColorBarSection*
AnnotationColorBar::getSection(const int32_t index) const
{
    CaretAssertVectorIndex(m_sections, index);
    return m_sections[index];
}

/**
 * @param Is show tick marks selected?
 */
bool
AnnotationColorBar::isShowTickMarksSelected() const
{
    return m_showTickMarksSelected;
}

/**
 * Set show tick marks selected.
 *
 * @param selected
 *     New selection status.
 */
void
AnnotationColorBar::setShowTickMarksSelected(const bool selected)
{
    m_showTickMarksSelected = selected;
}

/**
 * Add numeric text.
 *
 * Note: numeric text is not saved to scene so this method DOES NOT change the modified status.
 *
 * @param scalar
 *     Scalar value for position of numeric text.
 * @param numericText
 *     The numeric text.
 */
void
AnnotationColorBar::addNumericText(const float scalar,
                                        const AString& numericText,
                                        const AnnotationTextAlignHorizontalEnum::Enum horizontalAlignment,
                                        const bool drawTickMarkAtScalar)
{
    m_numericText.push_back(new AnnotationColorBarNumericText(scalar,
                                                              numericText,
                                                              horizontalAlignment,
                                                              drawTickMarkAtScalar));
}

/**
 * Clear the numeric text.
 *
 * Note: numeric text is not saved to scene so this method DOES NOT change the modified status.
 */
void
AnnotationColorBar::clearNumericText()
{
    for (std::vector<const AnnotationColorBarNumericText*>::iterator iter = m_numericText.begin();
         iter != m_numericText.end();
         iter++) {
        delete *iter;
    }
    m_numericText.clear();
}

/**
 * @return Number of numeric text.
 */
int32_t
AnnotationColorBar::getNumberOfNumericText() const
{
    return m_numericText.size();
}

/**
 * @return Numeric text at the given index.
 *
 * @param index
 *     Inext of the numeric text.
 */
const AnnotationColorBarNumericText*
AnnotationColorBar::getNumericText(const int32_t index) const
{
    CaretAssertVectorIndex(m_numericText, index);
    return m_numericText[index];
}

/**
 * Get the minimum and maximum scalar values in the colorbar.
 *
 * @param minimumScalarOut
 *     Minimum scalar value upon exit.
 * @param maximumScalarOut
 *     Maximum scalar value upon exit.
 */
void
AnnotationColorBar::getScalarMinimumAndMaximumValues(float& minimumScalarOut,
                                                     float& maximumScalarOut) const
{
    minimumScalarOut =  std::numeric_limits<float>::max();
    maximumScalarOut = -std::numeric_limits<float>::max();
    
    const int32_t numSections = getNumberOfSections();
    if (numSections <= 0) {
        minimumScalarOut = 0.0;
        maximumScalarOut = 0.0;
        return;
    }
    
    for (int32_t i = 0; i < numSections; i++) {
        const AnnotationColorBarSection* section = getSection(i);
        minimumScalarOut = std::min(minimumScalarOut,
                                    section->getStartScalar());
        minimumScalarOut = std::min(minimumScalarOut,
                                    section->getEndScalar());
        maximumScalarOut = std::max(maximumScalarOut,
                                    section->getStartScalar());
        maximumScalarOut = std::max(maximumScalarOut,
                                    section->getEndScalar());
    }
}

