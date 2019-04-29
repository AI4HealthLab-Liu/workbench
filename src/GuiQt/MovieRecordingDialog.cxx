
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

#define __MOVIE_RECORDING_DIALOG_DECLARE__
#include "MovieRecordingDialog.h"
#undef __MOVIE_RECORDING_DIALOG_DECLARE__

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "BrainBrowserWindowComboBox.h"
#include "CaretAssert.h"
#include "CaretFileDialog.h"
#include "CursorDisplayScoped.h"
#include "Event.h"
#include "EventManager.h"
#include "EventGraphicsUpdateOneWindow.h"
#include "EnumComboBoxTemplate.h"
#include "FileInformation.h"
#include "MovieRecorder.h"
#include "SessionManager.h"
#include "WuQMessageBox.h"

using namespace caret;


    
/**
 * \class caret::MovieRecordingDialog 
 * \brief Dialog for control of movie recording and creation
 * \ingroup GuiQt
 */

/**
 * Constructor.
 */
MovieRecordingDialog::MovieRecordingDialog(QWidget* parent)
: WuQDialogNonModal("Movie Recording",
                    parent)
{
    QTabWidget* tabWidget = new QTabWidget();
    tabWidget->addTab(createMainWidget(), "Main");
    tabWidget->addTab(createSettingsWidget(), "Settings");
    
    setCentralWidget(tabWidget,
                     SCROLL_AREA_NEVER);

    EventManager::get()->addEventListener(this,
                                          EventTypeEnum::EVENT_MOVIE_RECORDING_DIALOG_UPDATE);
    disableAutoDefaultForAllPushButtons();
}

/**
 * Destructor.
 */
MovieRecordingDialog::~MovieRecordingDialog()
{
    EventManager::get()->removeAllEventsFromListener(this);
}

/**
 * Receive an event.
 *
 * @param event
 *    An event for which this instance is listening.
 */
void
MovieRecordingDialog::receiveEvent(Event* event)
{
    if (event->getEventType() == EventTypeEnum::EVENT_MOVIE_RECORDING_DIALOG_UPDATE) {
        updateDialog();
        event->setEventProcessed();
    }
}

/**
 * May be called to update the dialog's content.
 */
void
MovieRecordingDialog::updateDialog()
{
    const MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);
    
    m_windowComboBox->updateComboBox();
    m_windowComboBox->setBrowserWindowByIndex(movieRecorder->getRecordingWindowIndex());
    
    bool manualRecordingEnabledFlag(false);
    switch (movieRecorder->getRecordingMode()) {
        case MovieRecorderModeEnum::MANUAL:
            m_recordingManualRadioButton->setChecked(true);
            manualRecordingEnabledFlag = true;
            break;
        case MovieRecorderModeEnum::AUTOMATIC:
            m_recordingAutomaticRadioButton->setChecked(true);
            break;
    }
    
    const MovieRecorderVideoFormatTypeEnum::Enum formatType = movieRecorder->getVideoFormatType();
    m_movieRecorderVideoFormatTypeEnumComboBox->setSelectedItem<MovieRecorderVideoFormatTypeEnum,MovieRecorderVideoFormatTypeEnum::Enum>(formatType);

    const MovieRecorderVideoDimensionsTypeEnum::Enum dimType = movieRecorder->getVideoDimensionsType();
    m_movieRecorderVideoDimensionsTypeEnumComboBox->setSelectedItem<MovieRecorderVideoDimensionsTypeEnum,MovieRecorderVideoDimensionsTypeEnum::Enum>(dimType);

    updateManualRecordingOptions();
    updateCustomWidthHeightSpinBoxes();
    updateFileNameLabel();
    updateFrameCountLabel();
}

/**
 * Update the manual recording options
 */
void
MovieRecordingDialog::updateManualRecordingOptions()
{
    const MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);
    
    bool manualRecordingEnabledFlag(false);
    switch (movieRecorder->getRecordingMode()) {
        case MovieRecorderModeEnum::MANUAL:
            manualRecordingEnabledFlag = true;
            break;
        case MovieRecorderModeEnum::AUTOMATIC:
            break;
    }
    
    m_manualCaptureSecondsSpinBox->setValue(movieRecorder->getManualRecordingDurationSeconds());
    m_manualCaptureToolButton->setEnabled(manualRecordingEnabledFlag);
}

/**
 * Update the custom width/height spin boxes
 */
void
MovieRecordingDialog::updateCustomWidthHeightSpinBoxes()
{
    const MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);

    int32_t customWidth(0);
    int32_t customHeight(0);
    movieRecorder->getCustomDimensions(customWidth,
                                       customHeight);
    QSignalBlocker widthBlocker(m_customWidthSpinBox);
    m_customWidthSpinBox->setValue(customWidth);
    QSignalBlocker heightBlocker(m_customHeightSpinBox);
    m_customHeightSpinBox->setValue(customHeight);
    QSignalBlocker frameRateBlocker(m_frameRateSpinBox);
    m_frameRateSpinBox->setValue(movieRecorder->getFramesRate());
    
    bool customSpinBoxesEnabled(false);
    switch (movieRecorder->getVideoDimensionsType()) {
        case MovieRecorderVideoDimensionsTypeEnum::CUSTOM:
            customSpinBoxesEnabled = true;
            break;
        case MovieRecorderVideoDimensionsTypeEnum::HD_1280_720:
            break;
        case MovieRecorderVideoDimensionsTypeEnum::SD_640_480:
            break;
    }
    m_customWidthSpinBox->setEnabled(customSpinBoxesEnabled);
    m_customHeightSpinBox->setEnabled(customSpinBoxesEnabled);
}

/**
 * Update the filename label
 */
void
MovieRecordingDialog::updateFileNameLabel()
{
    QString name = SessionManager::get()->getMovieRecorder()->getMovieFileName();
    if ( ! name.isEmpty()) {
        FileInformation fileInfo(name);
        name = fileInfo.getFileName();
    }
    
    m_filenameLabel->setText(name);
}

/**
 * Update the frame count label
 */
void
MovieRecordingDialog::updateFrameCountLabel()
{
    m_frameCountNumberLabel->setNum(SessionManager::get()->getMovieRecorder()->getNumberOfFrames());
}

/**
 * Called when window index is changed
 *
 * @param windowIndex
 *     Index of window for recording
 */
void
MovieRecordingDialog::windowIndexSelected(const int32_t windowIndex)
{
    SessionManager::get()->getMovieRecorder()->setRecordingWindowIndex(windowIndex);
}

/**
 * Called when video dimensions type is changed
 */
void
MovieRecordingDialog::movieRecorderVideoDimensionsTypeEnumComboBoxItemActivated()
{
    const MovieRecorderVideoDimensionsTypeEnum::Enum dimType = m_movieRecorderVideoDimensionsTypeEnumComboBox->getSelectedItem<MovieRecorderVideoDimensionsTypeEnum,MovieRecorderVideoDimensionsTypeEnum::Enum>();
    SessionManager::get()->getMovieRecorder()->setVideoDimensionsType(dimType);
    updateCustomWidthHeightSpinBoxes();
}

/**
 * Called when recording format is changed
 */
void
MovieRecordingDialog::movieRecorderVideoFormatTypeEnumComboBoxItemActivated()
{
    const MovieRecorderVideoFormatTypeEnum::Enum formatType = m_movieRecorderVideoFormatTypeEnumComboBox->getSelectedItem<MovieRecorderVideoFormatTypeEnum,MovieRecorderVideoFormatTypeEnum::Enum>();
    SessionManager::get()->getMovieRecorder()->setVideoFormatType(formatType);
    updateFileNameLabel();
}

/**
* Set the selected browser window to the browser window with the
* given index.
* @param browserWindowIndex
*    Index of browser window.
*/
void
MovieRecordingDialog::setBrowserWindowIndex(const int32_t browserWindowIndex)
{
    m_windowComboBox->setBrowserWindowByIndex(browserWindowIndex);
    windowIndexSelected(browserWindowIndex);
}

/**
 * @param Called when custom width spin box value changed
 *
 * @param width
 *     New custom width
 */
void
MovieRecordingDialog::customWidthSpinBoxValueChanged(int width)
{
    SessionManager::get()->getMovieRecorder()->setCustomDimensions(width,
                                                                   m_customHeightSpinBox->value());
}

/**
 * @param Called when custom height spin box value changed
 *
 * @param height
 *     New custom height
 */
void
MovieRecordingDialog::customHeightSpinBoxValueChanged(int height)
{
    SessionManager::get()->getMovieRecorder()->setCustomDimensions(m_customWidthSpinBox->value(),
                                                                   height);
}

/**
 * @param Called when frame rate spin box value changed
 *
 * @param frameRate
 *     New frame rate
 */
void
MovieRecordingDialog::frameRateSpinBoxValueChanged(int frameRate)
{
    SessionManager::get()->getMovieRecorder()->setFramesRate(frameRate);
}

/**
 * Called when recording mode button is clicked
 *
 * @param button
 *     Button that was clicked
 */
void
MovieRecordingDialog::recordingModeRadioButtonClicked(QAbstractButton* button)
{
    MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);
    
    if (button == m_recordingAutomaticRadioButton) {
        movieRecorder->setRecordingMode(MovieRecorderModeEnum::AUTOMATIC);
    }
    else if (button == m_recordingManualRadioButton) {
        movieRecorder->setRecordingMode(MovieRecorderModeEnum::MANUAL);
    }
    else {
        CaretAssert(0);
    }
    
    updateManualRecordingOptions();
}

/**
 * Called when manual capture tool button is clicked
 */
void
MovieRecordingDialog::manualCaptureToolButtonClicked()
{
    CursorDisplayScoped cursor;
    cursor.showWaitCursor();
    
    MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);
    movieRecorder->setManualRecordingOfImageRequested(true);
    EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(m_windowComboBox->getSelectedBrowserWindowIndex()).getPointer());
    movieRecorder->setManualRecordingOfImageRequested(false);
    updateFrameCountLabel();
}

/**
 * Called when manual capture seconds spin box value changed
 */
void
MovieRecordingDialog::manualCaptureSecondsSpinBoxValueChanged(int seconds)
{
    MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    CaretAssert(movieRecorder);
    movieRecorder->setManualRecordingDurationSeconds(seconds);
}

/**
 * Called when movie file name button is clicked
 */
void
MovieRecordingDialog::fileNameButtonClicked()
{
    const MovieRecorderVideoFormatTypeEnum::Enum formatType = m_movieRecorderVideoFormatTypeEnumComboBox->getSelectedItem<MovieRecorderVideoFormatTypeEnum,MovieRecorderVideoFormatTypeEnum::Enum>();
    QString filename = CaretFileDialog::getSaveFileNameDialog(m_filenamePushButton,
                                                              "Choose Movie File",
                                                              "",
                                                              MovieRecorderVideoFormatTypeEnum::toFileNameExtensionNoDot(formatType));
    if ( ! filename.isEmpty()) {
        SessionManager::get()->getMovieRecorder()->setMovieFileName(filename);
        updateFileNameLabel();
    }
}

/**
 * Called when create movie push button is clicked
 */
void
MovieRecordingDialog::createMoviePushButtonClicked()
{
    MovieRecorder* movieRecorder = SessionManager::get()->getMovieRecorder();
    FileInformation fileInfo(movieRecorder->getMovieFileName());
    if (fileInfo.exists()) {
        AString msg("Movie file \""
                    + fileInfo.getFileName()
                    + "\" exists, overwrite?");
        if ( ! WuQMessageBox::warningOkCancel(m_createMoviePushButton,
                                              msg)) {
            return;
        }
        if ( ! fileInfo.remove()) {
            AString msg("Unable to remove movie file \""
                        + fileInfo.getFileName()
                        + "\"");
            WuQMessageBox::errorOk(m_createMoviePushButton,
                                   msg);
        }
    }
    
    AString errorMessage;
    const bool successFlag = movieRecorder->createMovie(errorMessage);
    if (successFlag) {
        const QString text("Movie has been created.  Reset (delete) recorded images for new movie?  ");
        const QString infoText("If no is selected, creation of a new movie will include all images from "
                               "this movie and any addition recorded images.  Selecting yes is "
                               "equivalent to clicking the Reset button.");
        if (WuQMessageBox::warningYesNo(m_createMoviePushButton,
                                        text,
                                        infoText)) {
            SessionManager::get()->getMovieRecorder()->reset();
        }
    }
    else {
        WuQMessageBox::errorOk(m_createMoviePushButton,
                               errorMessage);
    }
    
    updateDialog();
}

/**
 * Called when reset push button is clicked
 */
void
MovieRecordingDialog::resetPushButtonClicked()
{
    if (WuQMessageBox::warningOkCancel(m_resetPushButton,
                                       "Reset (delete) recorded images for new movie")) {
        SessionManager::get()->getMovieRecorder()->reset();
        updateDialog();
    }
}

/**
 * @return New instance of main widget
 */
QWidget*
MovieRecordingDialog::createMainWidget()
{
    QLabel* windowLabel = new QLabel("Record from Window:");
    m_windowComboBox = new BrainBrowserWindowComboBox(BrainBrowserWindowComboBox::STYLE_NAME_AND_NUMBER,
                                                      this);
    m_windowComboBox->setToolTip("Sets window that is recorded");
    QObject::connect(m_windowComboBox, &BrainBrowserWindowComboBox::browserWindowIndexSelected,
                     this, &MovieRecordingDialog::windowIndexSelected);
    QHBoxLayout* windowLayout = new QHBoxLayout();
    windowLayout->addWidget(windowLabel);
    windowLayout->addWidget(m_windowComboBox->getWidget());
    windowLayout->addStretch();
    
    m_recordingAutomaticRadioButton  = new QRadioButton(MovieRecorderModeEnum::toGuiName(MovieRecorderModeEnum::AUTOMATIC));
    m_recordingAutomaticRadioButton->setToolTip("When selected, images are recorded as graphics updated");
    
    const QString recordButtonText("Record");
    m_recordingManualRadioButton = new QRadioButton(MovieRecorderModeEnum::toGuiName(MovieRecorderModeEnum::MANUAL));
    m_recordingManualRadioButton->setToolTip("When selected, images recorded when "
                                             + recordButtonText
                                             + " is clicked");
    QButtonGroup* recordingButtonGroup = new QButtonGroup(this);
    recordingButtonGroup->addButton(m_recordingAutomaticRadioButton);
    recordingButtonGroup->addButton(m_recordingManualRadioButton);
    QObject::connect(recordingButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                     this, &MovieRecordingDialog::recordingModeRadioButtonClicked);

    m_manualCaptureToolButton = new QToolButton();
    m_manualCaptureToolButton->setText(recordButtonText);
    m_manualCaptureToolButton->setToolTip("Duration of image displayed in movie");
    QObject::connect(m_manualCaptureToolButton, &QToolButton::clicked,
                     this, &MovieRecordingDialog::manualCaptureToolButtonClicked);

    m_manualCaptureSecondsSpinBox = new QSpinBox();
    m_manualCaptureSecondsSpinBox->setMinimum(1);
    m_manualCaptureSecondsSpinBox->setMaximum(100);
    m_manualCaptureSecondsSpinBox->setSingleStep(1);
    m_manualCaptureSecondsSpinBox->setSizePolicy(QSizePolicy::Fixed,
                                                 m_manualCaptureSecondsSpinBox->sizePolicy().verticalPolicy());
    
    QLabel* captureSecondsLabel = new QLabel("seconds");
    
    QGroupBox* modeGroupBox = new QGroupBox("Recording Mode");
    QGridLayout* modeLayout = new QGridLayout(modeGroupBox);
    modeLayout->setColumnStretch(0, 0);
    modeLayout->setColumnStretch(1, 0);
    modeLayout->setColumnStretch(2, 0);
    modeLayout->setColumnStretch(3, 0);
    modeLayout->setColumnStretch(4, 100);
    int32_t modeRow(0);
    modeLayout->addWidget(m_recordingAutomaticRadioButton, modeRow, 0);
    modeRow++;
    modeLayout->addWidget(m_recordingManualRadioButton, modeRow, 0);
    modeLayout->addWidget(m_manualCaptureToolButton, modeRow, 1);
    modeLayout->addWidget(m_manualCaptureSecondsSpinBox, modeRow, 2);
    modeLayout->addWidget(captureSecondsLabel, modeRow, 3);
    modeRow++;
    
    m_filenamePushButton = new QPushButton("Movie File...");
    m_filenamePushButton->setToolTip("Select movie file using file dialog");
    QObject::connect(m_filenamePushButton, &QPushButton::clicked,
                     this, &MovieRecordingDialog::fileNameButtonClicked);
    m_filenameLabel      = new QLabel("");
    m_filenameLabel->setMinimumWidth(150);

    m_createMoviePushButton = new QPushButton("Create Movie");
    m_createMoviePushButton->setToolTip("Create a movie file using images that have been recorded");
    QObject::connect(m_createMoviePushButton, &QPushButton::clicked, this,
                     &MovieRecordingDialog::createMoviePushButtonClicked);
    
    m_resetPushButton = new QPushButton("Reset");
    m_resetPushButton->setToolTip("Remove all recorded images to start a new movie");
    QObject::connect(m_resetPushButton, &QPushButton::clicked, this,
                     &MovieRecordingDialog::resetPushButtonClicked);
    
    QLabel* frameCountLabel = new QLabel("Frames: ");
    m_frameCountNumberLabel = new QLabel("0");
    
    QGroupBox* movieFileGroupBox = new QGroupBox("Output Movie");
    QGridLayout* movieLayout = new QGridLayout(movieFileGroupBox);
    movieLayout->setColumnStretch(0, 0);
    movieLayout->setColumnStretch(1, 0);
    movieLayout->setColumnStretch(2, 0);
    int32_t movieRow(0);
    movieLayout->addWidget(m_filenamePushButton, movieRow, 0);
    movieLayout->addWidget(m_filenameLabel, movieRow, 1, 1, 2, Qt::AlignLeft);
    movieRow++;
    movieLayout->addWidget(m_createMoviePushButton, movieRow, 0);
    movieRow++;
    movieLayout->addWidget(m_resetPushButton, movieRow, 0);
    movieLayout->addWidget(frameCountLabel, movieRow, 1);
    movieLayout->addWidget(m_frameCountNumberLabel, movieRow, 2);
    movieRow++;
    

    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->addLayout(windowLayout);
    layout->addWidget(modeGroupBox);
    layout->addWidget(movieFileGroupBox);
    layout->addStretch();
    
    return widget;
}

/**
 * @return New instance of settings widget
 */
QWidget*
MovieRecordingDialog::createSettingsWidget()
{
    const int spinBoxWidth(100);
    
    QLabel* formatLabel = new QLabel("Format:");
    m_movieRecorderVideoFormatTypeEnumComboBox = new EnumComboBoxTemplate(this);
    m_movieRecorderVideoFormatTypeEnumComboBox->getWidget()->setToolTip("Choose video format (not all formats on all computers)");
    m_movieRecorderVideoFormatTypeEnumComboBox->setup<MovieRecorderVideoFormatTypeEnum,MovieRecorderVideoFormatTypeEnum::Enum>();
    QObject::connect(m_movieRecorderVideoFormatTypeEnumComboBox, SIGNAL(itemActivated()),
                     this, SLOT(movieRecorderVideoFormatTypeEnumComboBoxItemActivated()));
    
    QLabel* dimensionsLabel = new QLabel("Dimensions:");
    m_movieRecorderVideoDimensionsTypeEnumComboBox = new EnumComboBoxTemplate(this);
    m_movieRecorderVideoDimensionsTypeEnumComboBox->getWidget()->setToolTip("Choose width and height of movie");
    m_movieRecorderVideoDimensionsTypeEnumComboBox->setup<MovieRecorderVideoDimensionsTypeEnum,MovieRecorderVideoDimensionsTypeEnum::Enum>();
    QObject::connect(m_movieRecorderVideoDimensionsTypeEnumComboBox, SIGNAL(itemActivated()),
                     this, SLOT(movieRecorderVideoDimensionsTypeEnumComboBoxItemActivated()));
    
    QLabel* customLabel = new QLabel("Custom Dimensions:");
    m_customWidthSpinBox = new QSpinBox();
    m_customWidthSpinBox->setMinimum(1);
    m_customWidthSpinBox->setMaximum(500000);
    m_customWidthSpinBox->setSingleStep(1);
    QObject::connect(m_customWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &MovieRecordingDialog::customWidthSpinBoxValueChanged);
    m_customWidthSpinBox->setFixedWidth(spinBoxWidth);
    
    m_customHeightSpinBox = new QSpinBox();
    m_customHeightSpinBox->setMinimum(1);
    m_customHeightSpinBox->setMaximum(500000);
    m_customHeightSpinBox->setSingleStep(1);
    m_customHeightSpinBox->setFixedWidth(spinBoxWidth);
    QObject::connect(m_customHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &MovieRecordingDialog::customHeightSpinBoxValueChanged);

    QLabel* frameRateLabel = new QLabel("Frames Per Second:");
    m_frameRateSpinBox = new QSpinBox();
    m_frameRateSpinBox->setToolTip("20 or 30 recommended");
    m_frameRateSpinBox->setMinimum(1);
    m_frameRateSpinBox->setMaximum(1000);
    m_frameRateSpinBox->setSingleStep(1);
    m_frameRateSpinBox->setFixedWidth(spinBoxWidth);
    QObject::connect(m_frameRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &MovieRecordingDialog::frameRateSpinBoxValueChanged);

    QWidget* widget = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(widget);
    gridLayout->setRowStretch(100, 100);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 0);
    gridLayout->setColumnStretch(2, 0);
    gridLayout->setColumnStretch(3, 100);
    int32_t row(0);
    gridLayout->addWidget(formatLabel, row, 0);
    gridLayout->addWidget(m_movieRecorderVideoFormatTypeEnumComboBox->getWidget(),
                          row, 1, 1, 2, Qt::AlignLeft);
    row++;
    gridLayout->addWidget(dimensionsLabel, row, 0);
    gridLayout->addWidget(m_movieRecorderVideoDimensionsTypeEnumComboBox->getWidget(),
                          row, 1, 1, 2, Qt::AlignLeft);
    row++;
    gridLayout->addWidget(customLabel, row, 0);
    gridLayout->addWidget(m_customWidthSpinBox, row, 1);
    gridLayout->addWidget(m_customHeightSpinBox, row, 2);
    row++;
    gridLayout->addWidget(frameRateLabel, row, 0);
    gridLayout->addWidget(m_frameRateSpinBox,
                          row, 1, 1, 2, Qt::AlignLeft);
    row++;

    return widget;
}

