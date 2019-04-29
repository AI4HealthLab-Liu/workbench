
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

#define __MOVIE_RECORDER_DECLARE__
#include "MovieRecorder.h"
#undef __MOVIE_RECORDER_DECLARE__

#include <QDir>
#include <QFile>
#include <QImage>
#include <QProcess>

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "DataFileException.h"
#include "FileInformation.h"
#include "MovieRecorderVideoFormatTypeEnum.h"
#include "ImageFile.h"
#include "TextFile.h"

using namespace caret;


    
/**
 * \class caret::MovieRecorder 
 * \brief Records images and creates movie file from images
 * \ingroup Brain
 */

/**
 * Constructor.
 */
MovieRecorder::MovieRecorder()
: CaretObject()
{
    m_temporaryImagesDirectory = SystemUtilities::getTempDirectory();
    m_tempImageFileNamePrefix  = "movie";
    m_tempImageFileNameSuffix = ".png";
    //QDir::tempPath();
    reset();
}

/**
 * Destructor.
 */
MovieRecorder::~MovieRecorder()
{
    reset();
}

/**
 * Add an image to the movie, typically used during automatic mode recording
 *
 * @param image
 *     Image that is added
 */
void
MovieRecorder::addImageToMovie(const QImage* image)
{
    if (image == NULL) {
        CaretLogSevere("Attempting to add NULL image to movie");
        return;
    }
    
    if (getNumberOfFrames() <= 0) {
        std::cout << "Temporary Directory for movie images: "
        << std::endl
        << "   " << m_temporaryImagesDirectory << std::endl << std::flush;
    }
    
    CaretAssert(m_tempImageSequenceNumberOfDigits > 0);
    
    /*
     * First image starts at 1 and is padded with zeros on the left
     */
    const int32_t imageIndexInt = getNumberOfFrames() + 1;
    const QString imageIndex = QString::number(imageIndexInt).rightJustified(m_tempImageSequenceNumberOfDigits, '0');
    const QString imageFileName(m_temporaryImagesDirectory
                                + "/"
                                + m_tempImageFileNamePrefix
                                + imageIndex
                                + m_tempImageFileNameSuffix);
    
    if (image->save(imageFileName)) {
        if (m_imageFileNames.empty()) {
            m_firstImageWidth  = image->width();
            m_firstImageHeight = image->height();
        }
        
        if ((image->width()     == m_firstImageWidth)
            && (image->height() == m_firstImageHeight)) {
            m_imageFileNames.push_back(imageFileName);
        }
        else {
            CaretLogSevere("Attempting to create movie with images that are different sizes.  "
                           "First image width=" + QString::number(m_firstImageWidth)
                           + ", height=" + QString::number(m_firstImageHeight)
                           + "  Image number=" + QString::number(imageIndexInt)
                           + ", width=" + QString::number(image->width())
                           + ", height=" + QString::number(image->height()));
        }
    }
    else {
        CaretLogSevere("Saving temporary image failed: "
                       + imageFileName);
    }
//    std::cout << "Adding image " << image->width() << ", " << image->height() << " " << imageFileName << std::endl;
    
    
}

/**
 * Add an image to the movie, typically used during automatic mode recording.
 * After image is added, manual recording is disabled
 *
 * @param image
 *     Image that is added
 */
void
MovieRecorder::addImageToMovieWithManualDuration(const QImage* image)
{
    const int32_t recordingCount = m_frameRate * m_manualRecordingDurationSeconds;
    for (int32_t i = 0; i < recordingCount; i++) {
        addImageToMovie(image);
    }
    setManualRecordingOfImageRequested(false);
}

/**
 * Reset by removing all images and starting a new movie
 */
void
MovieRecorder::reset()
{
    const QString nameFilter(m_tempImageFileNamePrefix
                             + "*"
                             + m_tempImageFileNameSuffix);
    QStringList allNameFilters;
    allNameFilters.append(nameFilter);
    QDir dir(m_temporaryImagesDirectory);
    QFileInfoList fileInfoList = dir.entryInfoList(allNameFilters,
                                                   QDir::Files,
                                                   QDir::Name);
    QListIterator<QFileInfo> iter(fileInfoList);
    while (iter.hasNext()) {
        QFile file(iter.next().absoluteFilePath());
        if (file.exists()) {
            file.remove();
        }
    }
    
    m_imageFileNames.clear();
    m_firstImageWidth  = -1;
    m_firstImageHeight = -1;
}


/**
 * @return The recording mode
 */
MovieRecorderModeEnum::Enum
MovieRecorder::getRecordingMode() const
{
    return m_recordingMode;
}

/**
 * Set the recording mode
 *
 * @param recordingMode
 *     New recording mode
 */
void
MovieRecorder::setRecordingMode(const MovieRecorderModeEnum::Enum recordingMode)
{
    m_recordingMode = recordingMode;
}

/**
 * @return Index of window that is recorded
 */
int32_t
MovieRecorder::getRecordingWindowIndex() const
{
    return m_windowIndex;
}

/**
 * Set index of window that is recorded
 *
 * @param windowIndex
 *     Index of window
 */
void
MovieRecorder::setRecordingWindowIndex(const int32_t windowIndex)
{
    m_windowIndex = windowIndex;
}

/**
 * @return Video resolution type
 */
MovieRecorderVideoResolutionTypeEnum::Enum
MovieRecorder::getVideoResolutionType() const
{
    return m_resolutionType;
}

/**
 * Set the video resolution type
 *
 * @param resolutionType
 *     New resolution type
 */
void
MovieRecorder::setVideoResolutionType(const MovieRecorderVideoResolutionTypeEnum::Enum resolutionType)
{
    m_resolutionType = resolutionType;
}

/**
 * Get the video width and height
 *
 * @param widthOut
 *     Output width
 * @param heightOut
 *     Output height
 */
void
MovieRecorder::getVideoWidthAndHeight(int32_t& widthOut,
                                      int32_t& heightOut) const
{
    widthOut  = 100;
    heightOut = 100;
    
    const MovieRecorderVideoResolutionTypeEnum::Enum dimType = getVideoResolutionType();
    if (dimType == MovieRecorderVideoResolutionTypeEnum::CUSTOM) {
        getCustomWidthAndHeight(widthOut,
                                heightOut);
    }
    else {
        MovieRecorderVideoResolutionTypeEnum::getWidthAndHeight(dimType,
                                                                widthOut,
                                                                heightOut);
    }
}

/**
 * Get the custom width and height
 *
 * @param widthOut
 *     Output width
 * @param heightOut
 *     Output height
 */
void
MovieRecorder::getCustomWidthAndHeight(int32_t& widthOut,
                                       int32_t& heightOut) const
{
    widthOut  = m_customWidth;
    heightOut = m_customHeight;
}

/**
 * Set the custom width and height
 *
 * @param width
 *     New width
 * @param height
 *     New height
 */
void
MovieRecorder::setCustomWidthAndHeight(const int32_t width,
                                       const int32_t height)
{
    m_customWidth  = width;
    m_customHeight = height;
}

/**
 * @return The capture region type
 */
MovieRecorderCaptureRegionTypeEnum::Enum
MovieRecorder::getCaptureRegionType() const
{
    return m_captureRegionType;
}

/**
 * Set the capture region type
 *
 * @param captureRegionType
 *     New capture region type
 */
void
MovieRecorder::setCaptureRegionType(const MovieRecorderCaptureRegionTypeEnum::Enum captureRegionType)
{
    m_captureRegionType = captureRegionType;
}

/**
 * @return Name of movie file
 */
AString
MovieRecorder::getMovieFileName() const
{
    return m_movieFileName;
}

/**
 * Set name of movie file
 *
 * @param filename
 *     New name for movie file
 */
void
MovieRecorder::setMovieFileName(const AString& filename)
{
    m_movieFileName = filename;
}

/**
 * If the movie file name is empty,
 * initialize the movie file to be in the given path.
 *
 * @param path
 *     Path of where to place the movie file
 */
void
MovieRecorder::initializeMovieFileName(const AString& path)
{
    if (m_movieFileName.isEmpty()) {
        const QString extension = MovieRecorderVideoFormatTypeEnum::toFileNameExtensionNoDot(MovieRecorderVideoFormatTypeEnum::MPEG_4);
        
        m_movieFileName = FileInformation::assembleFileComponents(path,
                                                                  "Movie",
                                                                  extension);
    }
}

/**
 * Get a description of this object's content.
 * @return String describing this object's content.
 */
AString 
MovieRecorder::toString() const
{
    return "MovieRecorder";
}

/**
 * @return The frame rate (number of frames per second)
 */
float
MovieRecorder::getFramesRate() const
{
    return m_frameRate;
}

/**
 * Set the frame rate (number of frames per second)
 *
 * @param frameRate
 *     New frame rate
 */
void
MovieRecorder::setFramesRate(const float frameRate)
{
    m_frameRate = frameRate;
}

/**
 * @return Duration of manual record duration in seconds
 */
float
MovieRecorder::getManualRecordingDurationSeconds() const
{
    return m_manualRecordingDurationSeconds;
}

/**
 * Set the duration of manual recording
 *
 * @param seconds
 *     The duration
 */
void
MovieRecorder::setManualRecordingDurationSeconds(const float seconds)
{
    m_manualRecordingDurationSeconds = seconds;
}

/**
 * @return Is request for manual recording of an image when graphics updated set?
 * After image is recorded, manual recording request is reset to false
 */
bool
MovieRecorder::isManualRecordingOfImageRequested() const
{
    return m_manualRecordingOfImageRequested;
}

/**
 * Set request for manual recording of an image when graphics updated
 *
 * @param requestFlag
 *     New value for request
 */
void
MovieRecorder::setManualRecordingOfImageRequested(const bool requestFlag)
{
    m_manualRecordingOfImageRequested = requestFlag;
}

/**
 * @return Number of frames (images) that have been recorded
 */
int32_t
MovieRecorder::getNumberOfFrames() const
{
    return m_imageFileNames.size();
}

/**
 * Create the movie using images captured thus far
 *
 * @param errorMessageOut
 *     Contains information if movie creation failed
 * @return
 *     True if successful, else false
 */
bool
MovieRecorder::createMovie(AString& errorMessageOut)
{
    errorMessageOut.clear();
    
    if (m_movieFileName.isEmpty()) {
        errorMessageOut = "Movie file name is invalid or empty";
        return false;
    }
    
    FileInformation fileInfo(m_movieFileName);
    if (fileInfo.exists()) {
        errorMessageOut = ("Movie file exists, delete or change name: "
                           + m_movieFileName);
        return false;
    }
    
    if (m_imageFileNames.empty()) {
        errorMessageOut.appendWithNewLine("No images have been recorded for the movie.");
    }
    if (m_movieFileName.isEmpty()) {
        errorMessageOut.appendWithNewLine("Movie file name is empty.");
    }
    
    if ( ! errorMessageOut.isEmpty()) {
        return false;
    }
    
    const AString sequenceDigitsPattern("%0"
                                        + AString::number(m_tempImageSequenceNumberOfDigits)
                                        + "d");
    
    const AString imagesRegularExpressionMatch(m_temporaryImagesDirectory
                                               + "/"
                                               + m_tempImageFileNamePrefix
                                               + sequenceDigitsPattern
                                               + m_tempImageFileNameSuffix);
    QString workbenchHomeDir = SystemUtilities::getWorkbenchHome();

    /* Qt after 5.? const QString ffmpegDir = qEnvironmentVariable("WORKBENCH_FFMPEG_DIR"); */
    const QString ffmpegDir = qgetenv("WORKBENCH_FFMPEG_DIR").constData();
    if ( ! ffmpegDir.isEmpty()) {
        workbenchHomeDir = ffmpegDir;
    }

    const bool qProcessPipeFlag(false);
    const QString textFileName(m_temporaryImagesDirectory
                               + "/"
                               + "images.txt");
    
    const QString programName(workbenchHomeDir
                              + "/ffmpeg");
    FileInformation ffmpegInfo(programName);
    if ( ! ffmpegInfo.exists()) {
        errorMessageOut = ("Invalid path for ffmpeg: "
                           + programName
                           + "\n  WORKBENCH_FFMPEG_DIR can be set to directory containing ffmpeg.");
        return false;
    }
    
    QStringList arguments;
    arguments.append("-threads");
    arguments.append("4");
    arguments.append("-framerate");
    arguments.append(AString::number(m_frameRate));
    if (qProcessPipeFlag) {
        /* list of images in file */
        arguments.append("-f");
        arguments.append("concat");
//        arguments.append("-safe");
//        arguments.append("0");
        arguments.append("-i");
        arguments.append(textFileName);
    }
    else {
        arguments.append("-i");
        arguments.append(imagesRegularExpressionMatch);
    }
    arguments.append("-q:v");
    arguments.append("1");
    arguments.append(m_movieFileName);
    
    bool successFlag(false);
    if (qProcessPipeFlag) {
        successFlag = createMovieWithQProcessPipe(programName,
                                                  arguments,
                                                  textFileName,
                                                  errorMessageOut);
    }
    else {
        const bool useQProcessFlag(true);
        if (useQProcessFlag) {
            successFlag = createMovieWithQProcess(programName,
                                                  arguments,
                                                  errorMessageOut);
        }
        else {
            successFlag = createMovieWithSystemCommand(programName,
                                                       arguments,
                                                       errorMessageOut);
        }
    }
    
    return successFlag;
}

/**
 * Create the movie by using Qt's QProcess and using a
 * pipe to send the images to ffmpeg
 *
 * @param programName
 *     Name of program
 * @param arguments
 *     Arguments to program
 * @param errorMessageOut
 *     Output containing error message
 * @return
 *     True if movie was created or false if there was an error
 */
bool
MovieRecorder::createMovieWithQProcessPipe(const QString& programName,
                                           const QStringList& arguments,
                                           const QString& textFileName,
                                           QString& errorMessageOut)
{
    /*
     * https://trac.ffmpeg.org/wiki/Concatenate
     * https://trac.ffmpeg.org/wiki/Slideshow
     */
    bool successFlag(false);

    TextFile textFile;
    try {
        for (const auto name : m_imageFileNames) {
            textFile.addLine("file "
                             + name);
        }
        textFile.writeFile(textFileName);
    }
    catch (const DataFileException& dfe) {
        errorMessageOut = ("Error creating text file containing image names: "
                           + dfe.whatString());
        return false;
    }

    QProcess process;
    process.start(programName,
                  arguments);
    process.closeWriteChannel();
    
    const int noTimeout(-1);
    const bool finishedFlag = process.waitForFinished(noTimeout);
    if (finishedFlag) {
        if (process.exitStatus() == QProcess::NormalExit) {
            const int resultCode = process.exitCode();
            if (resultCode == 0) {
                successFlag = true;
            }
            else {
                QByteArray results = process.readAllStandardError();
                errorMessageOut = QString(results);
            }
        }
        else if (process.exitStatus() == QProcess::CrashExit) {
            errorMessageOut = "Running ffmpeg crashed";
        }
    }
    else {
        errorMessageOut = "Creating movie was terminated for unknown reason";
    }
    
    return successFlag;
}

/**
 * Create the movie by using Qt's QProcess
 *
 * @param programName
 *     Name of program
 * @param arguments
 *     Arguments to program
 * @param errorMessageOut
 *     Output containing error message
 * @return
 *     True if movie was created or false if there was an error
 */
bool
MovieRecorder::createMovieWithQProcess(const QString& programName,
                                       const QStringList& arguments,
                                       QString& errorMessageOut)
{
    bool successFlag(false);
    
    QProcess process;
    process.start(programName,
                  arguments);
    process.closeWriteChannel();
    
    const int noTimeout(-1);
    const bool finishedFlag = process.waitForFinished(noTimeout);
    if (finishedFlag) {
        if (process.exitStatus() == QProcess::NormalExit) {
            const int resultCode = process.exitCode();
            if (resultCode == 0) {
                successFlag = true;
            }
            else {
                QByteArray results = process.readAllStandardError();
                errorMessageOut = QString(results);
            }
        }
        else if (process.exitStatus() == QProcess::CrashExit) {
            errorMessageOut = "Running ffmpeg crashed";
        }
    }
    else {
        errorMessageOut = "Creating movie was terminated for unknown reason";
    }
    
    return successFlag;
}

/**
 * Create the movie by using the system command
 *
 * @param programName
 *     Name of program
 * @param arguments
 *     Arguments to program
 * @param errorMessageOut
 *     Output containing error message
 * @return
 *     True if movie was created or false if there was an error
 */
bool
MovieRecorder::createMovieWithSystemCommand(const QString& programName,
                                            const QStringList& arguments,
                                            QString& errorMessageOut)
{
    const AString commandString(programName
                                + " "
                                + arguments.join(" "));
    //std::cout << "Command: " << commandString << std::endl;
    const int result = system(commandString.toLatin1().constData());
    
    bool successFlag(false);
    if (result == 0) {
        successFlag = true;
    }
    else {
        errorMessageOut = ("Running ffmpeg failed with code="
                           + AString::number(result));
    }
    return successFlag;
}

