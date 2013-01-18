
/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#include <QBoxLayout>
#include <QCheckBox>
#include <QFocusEvent>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>

#define __MAP_SETTINGS_EDITOR_DIALOG_DECLARE__
#include "MapSettingsEditorDialog.h"
#undef __MAP_SETTINGS_EDITOR_DIALOG_DECLARE__

#include "CaretMappableDataFile.h"
#include "EventManager.h"
#include "EventOverlayValidate.h"
#include "MapSettingsOverlayWidget.h"
#include "MapSettingsPaletteColorMappingWidget.h"
#include "Overlay.h"
#include "WuQtUtilities.h"

using namespace caret;

/**
 * \class caret::MapSettingsScalarDataEditorDialog 
 * \brief Dialog for editing scalar data map settings
 *
 * Presents controls for setting palettes, and thresholding used to color
 * scalar data.
 */

/**
 * Constructor for editing a palette selection.
 *
 * @param parent
 *    Parent widget on which this dialog is displayed.
 */
MapSettingsEditorDialog::MapSettingsEditorDialog(QWidget* parent)
: WuQDialogNonModal("Overlay and Map Settings",
                    parent)
{
    /*
     * No context menu, it screws things up
     */
    this->setContextMenuPolicy(Qt::NoContextMenu);
    
    this->setDeleteWhenClosed(false);

    m_caretMappableDataFile = NULL;
    m_mapIndex = -1;
    
    /*
     * No apply button
     */
    this->setApplyButtonText("");
    
    QWidget* mapNameWidget = createMapFileAndNameSection();
    
    m_overlayWidget = new MapSettingsOverlayWidget();
    
    m_paletteColorMappingWidget = new MapSettingsPaletteColorMappingWidget();
    
    QWidget* windowOptionsWidget = this->createWindowOptionsSection();
    
    QTabWidget* tabWidget = new QTabWidget();
    tabWidget->addTab(new QWidget(),
                      "Labels");
    tabWidget->setTabEnabled(tabWidget->count() - 1, false);
    tabWidget->addTab(new QWidget(),
                      "Metadata");
    tabWidget->setTabEnabled(tabWidget->count() - 1, false);
    tabWidget->addTab(m_overlayWidget,
                      "Overlay");
    tabWidget->addTab(m_paletteColorMappingWidget,
                      "Palette");
    tabWidget->setCurrentIndex(tabWidget->count() - 1);
    
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(w);
    this->setLayoutMargins(layout);
    layout->addWidget(mapNameWidget);
    layout->addWidget(tabWidget);
    //layout->addWidget(windowOptionsWidget);

    this->setCentralWidget(w);
    this->addWidgetToLeftOfButtons(windowOptionsWidget);
}

/**
 * Destructor.
 */
MapSettingsEditorDialog::~MapSettingsEditorDialog()
{
}

QWidget*
MapSettingsEditorDialog::createMapFileAndNameSection()
{
    QLabel* mapFileNameLabel = new QLabel("Map File: ");
    m_selectedMapFileNameLabel = new QLabel("");
    QLabel* mapNameLabel = new QLabel("Map Name: ");
    m_selectedMapNameLabel = new QLabel("");
    
    QGroupBox* groupBox = new QGroupBox();
    QGridLayout* gridLayout = new QGridLayout(groupBox);
    WuQtUtilities::setLayoutMargins(gridLayout, 2, 2);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 100);
    gridLayout->addWidget(mapFileNameLabel, 0, 0);
    gridLayout->addWidget(m_selectedMapFileNameLabel, 0, 1, Qt::AlignLeft);
    gridLayout->addWidget(mapNameLabel, 1, 0);
    gridLayout->addWidget(m_selectedMapNameLabel, 1, 1, Qt::AlignLeft);
    
    return groupBox;
}

/**
 * Called for focus events.  Since this dialog stores a pointer
 * to the overlay, we need to be aware that the overlay's parameters
 * may change or the overlay may even be deleted.  So, when
 * this dialog gains focus, validate the overlay and then update
 * the dialog.
 *
 * @param event
 *     The focus event.
 */
void
MapSettingsEditorDialog::focusInEvent(QFocusEvent* event)
{
    updateDialog();
}

/**
 * May be called to update the dialog's content.
 *
 * @param overlay
 *    Overlay for the dialog.
 */
void 
MapSettingsEditorDialog::updateDialogContent(Overlay* overlay)
{
    m_overlay = overlay;
//    m_caretMappableDataFile = caretMappableDataFile;
//    m_mapFileIndex = mapFileIndex;
    
    m_caretMappableDataFile = NULL;
    m_mapIndex = -1;
    if (m_overlay != NULL) {
        overlay->getSelectionData(m_caretMappableDataFile, m_mapIndex);
    }
    
    bool isPaletteValid = false;
    
    if (m_caretMappableDataFile != NULL) {
        if (m_caretMappableDataFile->isMappedWithPalette()) {
            isPaletteValid = true;
            m_paletteColorMappingWidget->updateEditor(m_caretMappableDataFile,
                                                      m_mapIndex);
        }
    }
  
    QString mapFileName = "";
    QString mapName = "";
    
    bool isOverlayValid = false;
    if (m_overlay != NULL) {
        m_overlayWidget->updateContent(m_overlay);
        isOverlayValid = true;
        
        if (m_caretMappableDataFile != NULL) {
            mapFileName = m_caretMappableDataFile->getFileNameNoPath();
            if (m_mapIndex >= 0) {
                mapName = m_caretMappableDataFile->getMapName(m_mapIndex);
            }
        }
    }
    
    m_overlayWidget->setEnabled(isOverlayValid);
    m_paletteColorMappingWidget->setEnabled(isPaletteValid);
    
    m_selectedMapFileNameLabel->setText(mapFileName);
    m_selectedMapNameLabel->setText(mapName);
}

/**
 * May be called to update the dialog.
 */
void
MapSettingsEditorDialog::updateDialog()
{
    /*
     * Validate overlay to prevent crash
     */
    EventOverlayValidate validateOverlayEvent(m_overlay);
    EventManager::get()->sendEvent(validateOverlayEvent.getPointer());
    if (validateOverlayEvent.isValidOverlay() == false) {
        m_overlay = NULL;
    }
    
    updateDialogContent(m_overlay);
    
    if (m_overlay == NULL) {
        close();
    }
}


/**
 * Called when close button pressed.
 */ 
void
MapSettingsEditorDialog::closeButtonPressed()
{
    /*
     * Allow this dialog to be reused (checked means DO NOT reuse)
     */
    m_doNotReplaceCheckBox->setCheckState(Qt::Unchecked);
    
    WuQDialogNonModal::closeButtonPressed();
}

/**
 * Set the layout margins.
 * @param layout
 *   Layout for which margins are set.
 */
void 
MapSettingsEditorDialog::setLayoutMargins(QLayout* layout)
{
    WuQtUtilities::setLayoutMargins(layout, 5, 3);
}

/**
 * @return Is the Do Not Replace selected.  If so, this instance of the
 * dialog should not be replaced.
 */
bool 
MapSettingsEditorDialog::isDoNotReplaceSelected() const
{
    const bool checked = (m_doNotReplaceCheckBox->checkState() == Qt::Checked);
    return checked;
}

/**
 * Called when the state of the do not reply checkbox is changed.
 * @param state
 *    New state of checkbox.
 */
void 
MapSettingsEditorDialog::doNotReplaceCheckBoxStateChanged(int /*state*/)
{
//    const bool checked = (state == Qt::Checked);
}

/**
 * @return A widget containing the window options.
 */
QWidget*
MapSettingsEditorDialog::createWindowOptionsSection()
{
    m_doNotReplaceCheckBox = new QCheckBox("Do Not Replace");
    m_doNotReplaceCheckBox->setToolTip("If checked: \n"
                                           "   (1) this window remains displayed until it is\n"
                                           "       closed.\n"
                                           "   (2) if the user selects editing of another map's\n"
                                           "       palette, it will not replace the content of\n"
                                           "       this window.\n"
                                           "If NOT checked:\n"
                                           "   If the user selects editing of another map's \n"
                                           "   palette, it will replace the content of this\n"
                                           "   window.");
    QWidget* optionsWidget = new QWidget();
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsWidget);
    this->setLayoutMargins(optionsLayout);
    optionsLayout->addWidget(m_doNotReplaceCheckBox);
    optionsWidget->setSizePolicy(QSizePolicy::Fixed,
                                 QSizePolicy::Fixed);
    
    return optionsWidget;
}



