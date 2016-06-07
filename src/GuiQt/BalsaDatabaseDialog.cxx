
/*LICENSE_START*/
/*
 *  Copyright (C) 2016 Washington University School of Medicine
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

#define __BALSA_DATABASE_DIALOG_DECLARE__
#include "BalsaDatabaseDialog.h"
#undef __BALSA_DATABASE_DIALOG_DECLARE__

#include <QComboBox>
#include <QDesktopServices>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include "BalsaDatabaseManager.h"
#include "CaretAssert.h"
#include "CaretHttpManager.h"
#include "CaretLogger.h"
#include "CursorDisplayScoped.h"
#include "EventManager.h"
#include "FileInformation.h"
#include "OperationException.h"
#include "OperationZipSceneFile.h"
#include "ProgressReportingBar.h"
#include "SceneFile.h"
#include "SystemUtilities.h"
#include "WuQMessageBox.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::BalsaDatabaseDialog 
 * \brief Wizard dialog for uploading to BALSA Database.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param sceneFileName
 *     Name of Scene File that will be uploaded to BALSA.
 * @param parent
 *     Parent widget of dialog.
 */
BalsaDatabaseDialog::BalsaDatabaseDialog(const SceneFile* sceneFile,
                                         QWidget* parent)
: QWizard(parent)
{
    CaretAssert(sceneFile);
    
    m_dialogData.grabNew(new BalsaDatabaseDialogSharedData(sceneFile));
    
    setWindowTitle("BALSA Database");
    
    m_pageLogin = new BalsaDatabaseLoginPage(m_dialogData);
    
    m_pageCreateZipFile = new BalsaDatabaseCreateZipFilePage(m_dialogData);

    m_pageUpload = new BalsaDatabaseUploadPage(m_dialogData);
    
    m_afterUploadPage = new BalsaDatabaseAfterUploadPage(m_dialogData);
    
    addPage(m_pageLogin);
    addPage(m_pageCreateZipFile);
    addPage(m_pageUpload);
    addPage(m_afterUploadPage);

    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::NoDefaultButton, false);
    setWizardStyle(QWizard::ModernStyle);
}

/**
 * Destructor.
 */
BalsaDatabaseDialog::~BalsaDatabaseDialog()
{
}

/* =============================================================================
 *
 * Login Page
 */

/**
 * Contruct Login page.
 *
 * @param dialogData
 *     Data shared by dialog and its pages
 */
BalsaDatabaseLoginPage::BalsaDatabaseLoginPage(BalsaDatabaseDialogSharedData* dialogData)
: QWizardPage(0),
m_dialogData(dialogData)
{
    const QString defaultUserName = "balsaTest";
    const QString defaultPassword = "@2password";
    
    setTitle("Login to the BALSA Database");
    setSubTitle("Enter username and password to login to the BALSA Database");
    
//    QLabel* loginLabel = new QLabel("<html>"
//                                     "Login for "
//                                     "<bold><a href=\"/\">BALSA Database</a></bold>"
//                                     "</html>");
//    QObject::connect(loginLabel, SIGNAL(linkActivated(const QString&)),
//                     this, SLOT(labelHtmlLinkClicked(const QString&)));
    
    
    QLabel* databaseNameLabel = new QLabel("DataBase: ");
    m_databaseComboBox = new QComboBox();
    m_databaseComboBox->setEditable(true);
    m_databaseComboBox->addItem("http://johnsdev.wustl.edu:8080");
    m_databaseComboBox->addItem("https://johnsdev.wustl.edu:8080");
    m_databaseComboBox->addItem("https://balsa.wustl.edu");
    
    const int minimumLineEditWidth = 250;
    
    QLabel* usernameLabel = new QLabel("Username: ");
    m_usernameLineEdit = new QLineEdit();
    m_usernameLineEdit->setMinimumWidth(minimumLineEditWidth);
    m_usernameLineEdit->setText(defaultUserName);
    
    QLabel* passwordLabel = new QLabel("Password: ");
    m_passwordLineEdit = new QLineEdit();
    m_passwordLineEdit->setMinimumWidth(minimumLineEditWidth);
    m_passwordLineEdit->setEchoMode(QLineEdit::Password);
    m_passwordLineEdit->setText(defaultPassword);
    
    QLabel* forgotUsernameLabel = new QLabel("<html>"
                                             "<bold><a href=\"register/forgotUsername\">Forgot Username</a></bold>"
                                             "</html>");
    QObject::connect(forgotUsernameLabel, SIGNAL(linkActivated(const QString&)),
                     this, SLOT(labelHtmlLinkClicked(const QString&)));
    
    QLabel* forgotPasswordLabel = new QLabel("<html>"
                                             "<bold><a href=\"register/forgotPassword\">Forgot Password</a></bold>"
                                             "</html>");
    QObject::connect(forgotPasswordLabel, SIGNAL(linkActivated(const QString&)),
                     this, SLOT(labelHtmlLinkClicked(const QString&)));
    
    QLabel* registerLabel = new QLabel("<html>"
                                       "<bold><a href=\"register/register\">Register</a></bold>"
                                       "</html>");
    QObject::connect(registerLabel, SIGNAL(linkActivated(const QString&)),
                     this, SLOT(labelHtmlLinkClicked(const QString&)));
    
    /*
     * Setup progress bar for manual updates
     */
    m_progressReportingBar = new ProgressReportingBar(this);
    m_progressReportingBar->setRange(PROGRESS_NONE,
                                     PROGRESS_DONE);
    m_progressReportingBar->setEnabledForUpdates(false);
    
    QHBoxLayout* linkLabelsLayout = new QHBoxLayout();
    linkLabelsLayout->addSpacing(5);
    linkLabelsLayout->addWidget(forgotUsernameLabel);
    linkLabelsLayout->addStretch();
    linkLabelsLayout->addWidget(forgotPasswordLabel);
    linkLabelsLayout->addStretch();
    linkLabelsLayout->addWidget(registerLabel);
    linkLabelsLayout->addSpacing(5);
    
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 100);
    int row = 0;
//    gridLayout->addWidget(loginLabel, row, 0, 1, 2);
//    row++;
    gridLayout->addWidget(databaseNameLabel, row, 0);
    gridLayout->addWidget(m_databaseComboBox, row, 1);
    row++;
    gridLayout->addWidget(usernameLabel, row, 0);
    gridLayout->addWidget(m_usernameLineEdit, row, 1);
    row++;
    gridLayout->addWidget(passwordLabel, row, 0);
    gridLayout->addWidget(m_passwordLineEdit, row, 1);
    row++;
    gridLayout->addLayout(linkLabelsLayout, row, 0, 1, 2);
    row++;
    gridLayout->addWidget(m_progressReportingBar, row, 0, 1, 2);
}


/**
 * Destructor.
 */
BalsaDatabaseLoginPage::~BalsaDatabaseLoginPage()
{
    
}

/**
 * @return Name of selected database.
 */
AString
BalsaDatabaseLoginPage::getDataBaseURL() const
{
    return m_databaseComboBox->currentText().trimmed();
}


/**
 * Gets called when the user clicks a link in the forgot username
 * or password label.
 *
 * @param linkPath
 *     Path relative to database.
 */
void
BalsaDatabaseLoginPage::labelHtmlLinkClicked(const QString& linkPath)
{
    if (linkPath.isEmpty() == false) {
        const QString dbURL = getDataBaseURL();
        const AString linkURL = (getDataBaseURL() +
                                 (linkPath.startsWith("/") ? "" : "/")
                                 + linkPath);
        QDesktopServices::openUrl(QUrl(linkURL));
    }
}

/**
 * Called just before page is shown.
 */
void
BalsaDatabaseLoginPage::initializePage()
{
    m_progressReportingBar->setValue(PROGRESS_NONE);
    m_progressReportingBar->setMessage("");
}

/**
 * Returns true if Next/Finish page should be enabled
 * implying that the page's content is valid.
 */
bool
BalsaDatabaseLoginPage::isComplete() const
{
    return true;
}

/**
 * Called when Next/Finish button is clicked on page.
 *
 * @return True if next page is shown (or wizard is closed).
 */
bool
BalsaDatabaseLoginPage::validatePage()
{
    m_progressReportingBar->setValue(PROGRESS_LOGGING_IN);
    m_progressReportingBar->setMessage("Trying to Login...");
    
    CursorDisplayScoped cursor;
    cursor.showWaitCursor();
    
    const AString loginURL(getDataBaseURL()
                           + "/j_spring_security_check");
    AString errorMessage;
    if ( ! m_dialogData->m_balsaDatabaseManager->login(loginURL,
                                                       m_usernameLineEdit->text().trimmed(),
                                                       m_passwordLineEdit->text().trimmed(),
                                                       errorMessage)) {
        m_progressReportingBar->setValue(PROGRESS_NONE);
        m_progressReportingBar->setMessage("Login failed.");
        
        cursor.restoreCursor();
        WuQMessageBox::errorOk(this,
                               errorMessage);
        
        return false;
    }
    
    m_dialogData->m_databaseURL = getDataBaseURL();
    
    CaretLogInfo("SESSION ID from BALSA Login: "
                 + m_dialogData->m_balsaDatabaseManager->getJSessionIdCookie());

    m_progressReportingBar->setValue(PROGRESS_DONE);
    m_progressReportingBar->setMessage("Login successful with Session ID: "
                                       + m_dialogData->m_balsaDatabaseManager->getJSessionIdCookie());
    return true;
}



/* =============================================================================
 *
 * Create ZIP File Page
 */

/**
 * Contruct Login page.
 *
 * @param dialogData
 *     Data shared by dialog and its pages
 */
BalsaDatabaseCreateZipFilePage::BalsaDatabaseCreateZipFilePage(BalsaDatabaseDialogSharedData* dialogData)
: QWizardPage(0),
m_dialogData(dialogData)
{
    setTitle("Create Zip File");
    setSubTitle("Creates a ZIP file containing scene and data files for uploading to the BALSA Database.");
    
    QLabel* zipFileNameLabel = new QLabel("Zip File Name");
    m_zipFileNameLineEdit = new QLineEdit;
    m_zipFileNameLineEdit->setText("Scene.zip");
    
    QLabel* extractDirectoryLabel = new QLabel("Extract to Directory");
    m_extractDirectoryNameLineEdit = new QLineEdit();
    m_extractDirectoryNameLineEdit->setText("ExtDir");
    
    QLabel* extractSuggestionLabelOne = new QLabel("Suggested Extract Directory Format: <Surname>_<OptionalDescriptor>_<StudyID>");
    QLabel* extractSuggestionLabelTwo = new QLabel("Example: Trump_JNeuroscience2016_W336  [OR just  Trump_W336  OR  Trump_2016_W336]");
    
    const AString defaultDirName(SystemUtilities::getUserName()
                                 + "_"
                                 + m_dialogData->m_sceneFile->getBalsaStudyID());
    m_extractDirectoryNameLineEdit->setText(defaultDirName);
    
//    QPushButton* zipScenePushButton = new QPushButton("Zip Scene File");
//    QObject::connect(zipScenePushButton, SIGNAL(clicked()),
//                     this, SLOT(runZipSceneFile()));
    
    m_progressReportingBar = new ProgressReportingBar(this);
    
    QGridLayout* layout = new QGridLayout(this);
    int32_t row = 0;
    layout->addWidget(zipFileNameLabel, row, 0);
    layout->addWidget(m_zipFileNameLineEdit, row, 1);
    row++;
    layout->addWidget(extractDirectoryLabel, row, 0);
    layout->addWidget(m_extractDirectoryNameLineEdit, row, 1);
    row++;
    layout->addWidget(extractSuggestionLabelOne, row, 1);
    row++;
    layout->addWidget(extractSuggestionLabelTwo, row, 1);
    row++;
//    layout->addWidget(zipScenePushButton, row, 0, 1, 2, Qt::AlignHCenter);
//    row++;
    layout->addWidget(m_progressReportingBar, row, 0, 1, 2);
    row++;
    
    
}

BalsaDatabaseCreateZipFilePage::~BalsaDatabaseCreateZipFilePage()
{
    
}

/**
 * Called just before page is shown.
 */
void
BalsaDatabaseCreateZipFilePage::initializePage()
{
    m_progressReportingBar->reset();
    m_dialogData->m_zipFileName = "";
}


bool
BalsaDatabaseCreateZipFilePage::isComplete() const
{
    return true;
}

/**
 * Called when Next/Finish button is clicked on page.
 *
 * @return True if next page is shown (or wizard is closed).
 */
bool
BalsaDatabaseCreateZipFilePage::validatePage()
{
    m_progressReportingBar->setEnabledForUpdates(true);
    const bool successFlag = runZipSceneFile();
    m_progressReportingBar->setEnabledForUpdates(false);
    if ( ! successFlag) {
        m_progressReportingBar->reset();
    }
    
    return successFlag;
}


/**
 * @return True if Zip file was created, else false.
 */
bool
BalsaDatabaseCreateZipFilePage::runZipSceneFile()
{
    m_dialogData->m_zipFileName = "";
    
    const SceneFile* sceneFile = m_dialogData->m_sceneFile;
    
    if (sceneFile == NULL) {
        WuQMessageBox::errorOk(this,
                               "Scene file is invalid.");
        return false;
    }
    const QString sceneFileName = sceneFile->getFileName();
    if (sceneFileName.isEmpty()) {
        WuQMessageBox::errorOk(this, "Scene File does not have a name.");
        return false;
    }
    
    const AString extractToDirectoryName = m_extractDirectoryNameLineEdit->text().trimmed();
    if (extractToDirectoryName.isEmpty()) {
        WuQMessageBox::errorOk(this, "Extract to directory is empty.");
        return false;
    }
    
    const AString zipFileName = m_zipFileNameLineEdit->text().trimmed();
    if (zipFileName.isEmpty()) {
        WuQMessageBox::errorOk(this, "Zip File name is empty");
        return false;
    }
    
    bool successFlag = false;
    
    CursorDisplayScoped cursor;
    cursor.showWaitCursor();
    AString errorMessage;
    if ( ! m_dialogData->m_balsaDatabaseManager->zipSceneAndDataFiles(sceneFile,
                                                                      extractToDirectoryName,
                                                                      zipFileName,
                                                                      errorMessage)) {
        cursor.restoreCursor();
        m_progressReportingBar->reset();
        WuQMessageBox::errorOk(this, errorMessage);
    }
    else {
        m_dialogData->m_zipFileName = zipFileName;
        successFlag = true;
    }

    return successFlag;
    
//    AString baseDirectoryName;
//    if ( ! sceneFile->getBaseDirectory().isEmpty()) {
//        /* validate ? */
//        baseDirectoryName = sceneFile->getBaseDirectory();
//    }
//    /*
//     * Create parameters for running zip scene file command.
//     * Need to use strdup() since QString::toAscii() returns
//     * QByteArray instance that will go out of scope.  Use
//     * strdup() for all parameters since "free" is later
//     * used to free the memory allocated by strdup().
//     */
//    std::vector<char*> argvVector;
//    argvVector.push_back(strdup("wb_command_in_wb_view"));
//    argvVector.push_back(strdup(OperationZipSceneFile::getCommandSwitch().toAscii().constData()));
//    argvVector.push_back(strdup(sceneFileName.toAscii().constData()));
//    argvVector.push_back(strdup(extractToDirectoryName.toAscii().constData()));
//    argvVector.push_back(strdup(zipFileName.toAscii().constData()));
//    if ( ! baseDirectoryName.isEmpty()) {
//        argvVector.push_back(strdup("-base-dir"));
//        argvVector.push_back(strdup(baseDirectoryName.toAscii().constData()));
//    }
//    
//    //    for (uint32_t i = 0; i < argvVector.size(); i++) {
//    //        std::cout << "Zip Scene File Param " << i << ": " << argvVector[i] << std::endl;
//    //    }
//    
//    
//    try {
//        CursorDisplayScoped cursor;
//        cursor.showWaitCursor();
//        
//        CommandOperationManager* cmdMgr = CommandOperationManager::getCommandOperationManager();
//        ProgramParameters progParams(argvVector.size(),
//                                     &argvVector[0]);
//        cmdMgr->runCommand(progParams);
//        
//        cursor.restoreCursor();
//        
//        WuQMessageBox::informationOk(this, "Zip file successfully created.");
//    }
//    catch (const CaretException& e) {
//        WuQMessageBox::errorOk(this, e.whatString());
//    }
//    
//    /*
//     * Free memory from use of strdup().
//     */
//    for (std::vector<char*>::iterator charIter = argvVector.begin();
//         charIter != argvVector.end();
//         charIter++) {
//        std::free(*charIter);
//    }
    
}



/* =============================================================================
 *
 * Upload Page
 */

/**
 * Contruct Login page.
 *
 * @param dialogData
 *     Data shared by dialog and its pages
 */
BalsaDatabaseUploadPage::BalsaDatabaseUploadPage(BalsaDatabaseDialogSharedData* dialogData)
: QWizardPage(0),
m_dialogData(dialogData)
{
    setTitle("Upload Zip File to BALSA");
    setSubTitle("Upload the ZIP file to the BALSA DataBase.");
    
    QLabel* zipFileNameLabel = new QLabel("Zip File Name");
    m_zipFileNameLineEdit = new QLineEdit;
    m_zipFileNameLineEdit->setReadOnly(true);
    
    /*
     * Setup progress bar for manual updates
     */
    m_progressReportingBar = new ProgressReportingBar(this);
    m_progressReportingBar->setRange(PROGRESS_NONE,
                                     PROGRESS_DONE);
    m_progressReportingBar->setEnabledForUpdates(false);
    
    QGridLayout* layout = new QGridLayout(this);
    int32_t row = 0;
    layout->addWidget(zipFileNameLabel, row, 0);
    layout->addWidget(m_zipFileNameLineEdit, row, 1);
    row++;
    layout->addWidget(m_progressReportingBar, row, 0, 1, 2);
    row++;
    
    
}

BalsaDatabaseUploadPage::~BalsaDatabaseUploadPage()
{
    
}

/**
 * Called just before page is shown.
 */
void
BalsaDatabaseUploadPage::initializePage()
{
    m_zipFileNameLineEdit->setText(m_dialogData->m_zipFileName);
    m_progressReportingBar->setValue(PROGRESS_NONE);
    m_progressReportingBar->setMessage("");
    m_dialogData->m_uploadResultText.clear();
}


bool
BalsaDatabaseUploadPage::isComplete() const
{
    return true;
}

/**
 * Called when Next/Finish button is clicked on page.
 *
 * @return True if next page is shown (or wizard is closed).
 */
bool
BalsaDatabaseUploadPage::validatePage()
{
    m_progressReportingBar->setEnabledForUpdates(true);
    const bool successFlag = uploadZipFile();
    m_progressReportingBar->setEnabledForUpdates(false);
    
    m_progressReportingBar->setRange(PROGRESS_NONE, PROGRESS_DONE);
    if (successFlag) {
        m_progressReportingBar->setValue(PROGRESS_DONE);
        m_progressReportingBar->setMessage("Uploading Successful: "
                                           + m_dialogData->m_zipFileName);
    }
    else {
        m_progressReportingBar->setValue(PROGRESS_NONE);
        m_progressReportingBar->setMessage("Uploading Failed: "
                                           + m_dialogData->m_zipFileName);
    }
    
    
    
//    m_progressReportingBar->setValue(PROGRESS_UPLOADING);
//    m_progressReportingBar->setMessage("Uploading: "
//                                       + m_dialogData->m_zipFileName);
//    const bool successFlag = uploadZipFile();
//    if (successFlag) {
//        m_progressReportingBar->setValue(PROGRESS_DONE);
//        m_progressReportingBar->setMessage("Uploading Successful: "
//                                           + m_dialogData->m_zipFileName);
//    }
//    else {
//        m_progressReportingBar->setValue(PROGRESS_NONE);
//        m_progressReportingBar->setMessage("Uploading Failed: "
//                                           + m_dialogData->m_zipFileName);
//    }
    
    return successFlag;
}

/**
 * @return True if Zip file was uploaded, else false.
 */
bool
BalsaDatabaseUploadPage::uploadZipFile()
{
    CursorDisplayScoped cursor;
    cursor.showWaitCursor();
    
    const AString uploadURL(m_dialogData->m_databaseURL
                           + "/study/handleUpload/"
                           + m_dialogData->m_sceneFile->getBalsaStudyID());
    AString errorMessage;
    if ( ! m_dialogData->m_balsaDatabaseManager->uploadFile(uploadURL,
                                                            m_dialogData->m_zipFileName,
                                                            "application/zip",
                                                            m_dialogData->m_uploadResultText,
                                                            errorMessage)) {
        cursor.restoreCursor();
        WuQMessageBox::errorOk(this,
                               errorMessage);
        
        return false;
    }

    return true;
    
//    const SceneFile* sceneFile = m_dialogData->m_sceneFile;
//    
//    if (sceneFile == NULL) {
//        WuQMessageBox::errorOk(this,
//                               "Scene file is invalid.");
//        return false;
//    }
//    const QString sceneFileName = sceneFile->getFileName();
//    if (sceneFileName.isEmpty()) {
//        WuQMessageBox::errorOk(this, "Scene File does not have a name.");
//        return false;
//    }
//    
//    const AString extractToDirectoryName = m_testingExtractDirectoryNameLineEdit->text().trimmed();
//    if (extractToDirectoryName.isEmpty()) {
//        WuQMessageBox::errorOk(this, "Extract to directory is empty.");
//        return false;
//    }
//    
//    const AString zipFileName = m_testingZipFileNameLineEdit->text().trimmed();
//    if (zipFileName.isEmpty()) {
//        WuQMessageBox::errorOk(this, "Zip File name is empty");
//        return false;
//    }
//    
//    bool successFlag = false;
//    
//    CursorDisplayScoped cursor;
//    cursor.showWaitCursor();
//    AString errorMessage;
//    if ( ! m_dialogData->m_balsaDatabaseManager->zipSceneAndDataFiles(sceneFile,
//                                                                      extractToDirectoryName,
//                                                                      zipFileName,
//                                                                      errorMessage)) {
//        cursor.restoreCursor();
//        m_progressReportingBar->reset();
//        WuQMessageBox::errorOk(this, errorMessage);
//    }
//    else {
//        //m_progressReportingBar->setMessage("Zip Complete");
//        successFlag = true;
//    }
//    
//    return successFlag;
}


/* =============================================================================
 *
 * After upload Page
 */

/**
 * Contruct Login page.
 *
 * @param dialogData
 *     Data shared by dialog and its pages
 */
BalsaDatabaseAfterUploadPage::BalsaDatabaseAfterUploadPage(BalsaDatabaseDialogSharedData* dialogData)
: QWizardPage(0),
m_dialogData(dialogData)
{
    setTitle("Upload Result from BALSA");
    setSubTitle("Information received from BALSA after successful file upload.");
    
    m_statusLabel = new QLabel("");
    
    QGridLayout* layout = new QGridLayout(this);
    int32_t row = 0;
    layout->addWidget(m_statusLabel, row, 0);
    row++;
    
}

BalsaDatabaseAfterUploadPage::~BalsaDatabaseAfterUploadPage()
{
    
}

/**
 * Called just before page is shown.
 */
void
BalsaDatabaseAfterUploadPage::initializePage()
{
    m_statusLabel->setText(m_dialogData->m_uploadResultText);
}


bool
BalsaDatabaseAfterUploadPage::isComplete() const
{
    return true;
}

/**
 * Called when Next/Finish button is clicked on page.
 *
 * @return True if next page is shown (or wizard is closed).
 */
bool
BalsaDatabaseAfterUploadPage::validatePage()
{
    return true;
}


/* =============================================================================
 *
 * Data shared with pages
 */
BalsaDatabaseDialogSharedData::BalsaDatabaseDialogSharedData(const SceneFile* sceneFile)
: m_sceneFile(sceneFile)
{
    CaretAssert(m_sceneFile);
    
    m_balsaDatabaseManager.grabNew(new BalsaDatabaseManager());
}




