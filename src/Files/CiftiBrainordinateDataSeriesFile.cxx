
/*LICENSE_START*/
/*
 * Copyright 2013 Washington University,
 * All rights reserved.
 *
 * Connectome DB and Connectome Workbench are part of the integrated Connectome 
 * Informatics Platform.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of Washington University nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*LICENSE_END*/

#define __CIFTI_BRAINORDINATE_DATA_SERIES_FILE_DECLARE__
#include "CiftiBrainordinateDataSeriesFile.h"
#undef __CIFTI_BRAINORDINATE_DATA_SERIES_FILE_DECLARE__

#include "CaretLogger.h"
#include "ChartDataCartesian.h"
#include "SceneClass.h"
#include "TimeLine.h"

using namespace caret;


    
/**
 * \class caret::CiftiBrainordinateDataSeriesFile 
 * \brief CIFTI Brainordinate by Data-Series File.
 * \ingroup Files
 */

/**
 * Constructor.
 */
CiftiBrainordinateDataSeriesFile::CiftiBrainordinateDataSeriesFile()
: CiftiMappableDataFile(DataFileTypeEnum::CONNECTIVITY_DENSE_TIME_SERIES,
                        CiftiMappableDataFile::FILE_READ_DATA_ALL,
                        CIFTI_INDEX_TYPE_TIME_POINTS,
                        CIFTI_INDEX_TYPE_BRAIN_MODELS,
                        CiftiMappableDataFile::DATA_ACCESS_WITH_COLUMN_METHODS,
                        CiftiMappableDataFile::DATA_ACCESS_WITH_ROW_METHODS)
{
    m_chartingEnabled = false;
}

/**
 * Destructor.
 */
CiftiBrainordinateDataSeriesFile::~CiftiBrainordinateDataSeriesFile()
{
}

/**
 * Update coloring for all maps.
 *
 * Note: Overridden since Data-Series files have one palette that is
 * applied to ALL maps.  For data-series, just invalidate the coloring
 * for all maps (data points).
 *
 * @param paletteFile
 *    Palette file containing palettes.
 */
void
CiftiBrainordinateDataSeriesFile::updateScalarColoringForAllMaps(const PaletteFile* /*paletteFile*/)
{
    /*
     * Just need to invalidate coloring.
     * Updating coloring for all maps would take time.
     * Coloring update is triggered by code that colors nodes/voxels
     * when drawing.
     */
    const int64_t numMaps = static_cast<int64_t>(getNumberOfMaps());
    for (int64_t i = 0; i < numMaps; i++) {
        CaretAssertVectorIndex(m_mapContent, i);
        m_mapContent[i]->m_rgbaValid = false;
    }
}

/**
 * @return Is charting enabled for this file?
 */
bool
CiftiBrainordinateDataSeriesFile::isChartingEnabled() const
{
    return m_chartingEnabled;
}

/**
 * @return Return true if the file's current state supports
 * charting data, else false.  Typically a brainordinate file
 * is chartable if it contains more than one map.
 */
bool
CiftiBrainordinateDataSeriesFile::isChartingSupported() const
{
    if (getNumberOfMaps() > 1) {
        return true;
    }
    
    return false;
}

/**
 * Set charting enabled for this file.
 *
 * @param enabled
 *    New status for charting enabled.
 */
void
CiftiBrainordinateDataSeriesFile::setChartingEnabled(const bool enabled)
{
    m_chartingEnabled = enabled;
}

ChartTypeEnum::Enum CiftiBrainordinateDataSeriesFile::getDefaultChartType() const
{
    return ChartTypeEnum::PLOT2D;
}

void CiftiBrainordinateDataSeriesFile::getSupportedChartTypes(std::vector<ChartTypeEnum::Enum> &list) const
{
    list.clear();
    list.push_back(ChartTypeEnum::PLOT2D);    
}

/**
 * Load the average of chart data for a group of surface nodes.
 * Note: This method will return a chart even if charting for
 * this file is disabled.
 *
 * @param structure
 *     The surface's structure
 * @param nodeIndices
 *     Indices of nodes whose chart data is averaged
 * @param timeLineOut
 *     Output charting data.
 * @return
 *     True if chart data is valid, else false.
 */
bool
CiftiBrainordinateDataSeriesFile::loadAverageChartForSurfaceNodes(const StructureEnum::Enum structure,
                                                                  const std::vector<int32_t>& nodeIndices,
                                                                  TimeLine& timeLineOut) throw (DataFileException)
{
    std::vector<double> dataAverage;
    double numValidNodes = 0.0;
    
    std::vector<float> data;
    
    const int64_t numNodeIndices = static_cast<int64_t>(nodeIndices.size());
    for (int64_t iNode = 0; iNode < numNodeIndices; iNode++) {
        if (getSeriesDataForSurfaceNode(structure,
                                        nodeIndices[iNode],
                                        data)) {
            const int64_t numData = static_cast<int64_t>(data.size());
            if (numData > 0) {
                if (dataAverage.empty()) {
                    dataAverage.resize(numData,
                                       0.0);
                }
                
                for (int64_t iData = 0; iData < numData; iData++) {
                    dataAverage[iData] += data[iData];
                }
                numValidNodes += 1.0;
            }
        }
    }
    
    if (numValidNodes > 0.0) {
        const int64_t numData = static_cast<int64_t>(dataAverage.size());
        for (int64_t i = 0; i < numData; i++) {
            dataAverage[i] /= numValidNodes;
        }
        
        timeLineOut.x.resize(numData);
        timeLineOut.y.resize(numData);
        for (int64_t i = 0; i < numData; i++) {
            timeLineOut.x[i] = i;
            timeLineOut.y[i] = dataAverage[i];
        }
        timeLineOut.nodeid = 0;
        timeLineOut.type = AVERAGE;
        timeLineOut.id = this;
        timeLineOut.filename = getFileName();
        
        float start, step;
        getMapIntervalStartAndStep(start, step);
        timeLineOut.timeStep = step;
        
        return true;
    }
    
    return false;
}

/**
 * Load charting data for the surface with the given structure and node index.
 *
 * @param structure
 *     The surface's structure.
 * @param nodeIndex
 *     Index of the node.
 * @return
 *     Pointer to the chart data.  If the data FAILED to load,
 *     the returned pointer will return true.  Caller takes ownership
 *     of the pointer and must delete it when no longer needed.
 */
ChartData*
CiftiBrainordinateDataSeriesFile::loadChartDataForSurfaceNode(const StructureEnum::Enum structure,
                                                               const int32_t nodeIndex) throw (DataFileException)
{
    ChartData* chartData = helpLoadChartDataForSurfaceNode(structure,
                                                           nodeIndex);
    return chartData;
    
//    ChartDataCartesian* chartData = NULL;
//    
//    try {
//        std::vector<float> data;
//        if (getSeriesDataForSurfaceNode(structure,
//                                        nodeIndex,
//                                        data)) {
//            const int64_t numData = static_cast<int64_t>(data.size());
//            
//            bool timeSeriesFlag = false;
//            bool dataSeriesFlag = false;
//            float convertTimeToSeconds = 1.0;
//            switch (getMapIntervalUnits()) {
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_HZ:
//                    break;
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_MSEC:
//                    timeSeriesFlag = true;
//                    convertTimeToSeconds = 1000.0;
//                    break;
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_PPM:
//                    break;
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_SEC:
//                    convertTimeToSeconds = 1.0;
//                    timeSeriesFlag = true;
//                    break;
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_UNKNOWN:
//                    dataSeriesFlag = true;
//                    break;
//                case NiftiTimeUnitsEnum::NIFTI_UNITS_USEC:
//                    convertTimeToSeconds = 1000000.0;
//                    timeSeriesFlag = true;
//                    break;
//            }
//            
//            if (dataSeriesFlag) {
//                chartData = new ChartDataCartesian(ChartDataTypeEnum::CHART_DATA_TYPE_DATA_SERIES,
//                                                   ChartAxisUnitsEnum::CHART_AXIS_UNITS_NONE,
//                                                   ChartAxisUnitsEnum::CHART_AXIS_UNITS_NONE);
//            }
//            else if (timeSeriesFlag) {
//                chartData = new ChartDataCartesian(ChartDataTypeEnum::CHART_DATA_TYPE_TIME_SERIES,
//                                                   ChartAxisUnitsEnum::CHART_AXIS_UNITS_TIME,
//                                                   ChartAxisUnitsEnum::CHART_AXIS_UNITS_NONE);
//            }
//            
//            if (chartData != NULL) {
//                float timeStart = 0.0;
//                float timeStep  = 1.0;
//                if (timeSeriesFlag) {
//                    getMapIntervalStartAndStep(timeStart,
//                                               timeStep);
//                    timeStart *= convertTimeToSeconds;
//                    timeStep  *= convertTimeToSeconds;
//                    chartData->setTimeStartInSecondsAxisX(timeStart);
//                    chartData->setTimeStepInSecondsAxisX(timeStep);
//                }
//                
//                for (int64_t i = 0; i < numData; i++) {
//                    float xValue = i;
//                    
//                    if (timeSeriesFlag) {
//                        xValue = timeStart + (i * timeStep);
//                    }
//                    
//                    chartData->addPoint(xValue,
//                                        data[i]);
//                }
//                
//                const AString description = (getFileNameNoPath()
//                                             + " node "
//                                             + AString::number(nodeIndex));
//                chartData->setDescription(description);
//            }
//            else {
//                const AString msg = "New type of units for data series flag, needs updating for charting";
//                CaretAssertMessage(0, msg);
//                throw DataFileException(msg);
//            }
//        }
//    }
//    catch (const DataFileException& dfe) {
//        if (chartData != NULL) {
//            delete chartData;
//            chartData = NULL;
//        }
//        
//        throw dfe;
//    }
//    
//    return chartData;
}

/**
 * Load average charting data for the surface with the given structure and node indices.
 *
 * @param structure
 *     The surface's structure.
 * @param nodeIndices
 *     Indices of the node.
 * @return
 *     Pointer to the chart data.  If the data FAILED to load,
 *     the returned pointer will be NULL.  Caller takes ownership
 *     of the pointer and must delete it when no longer needed.
 */
ChartData*
CiftiBrainordinateDataSeriesFile::loadAverageChartDataForSurfaceNodes(const StructureEnum::Enum structure,
                                                                      const std::vector<int32_t>& nodeIndices) throw (DataFileException)
{
    ChartData* chartData = helpLoadChartDataForSurfaceNodeAverage(structure,
                                                                  nodeIndices);
    return chartData;
}

/**
 * Load charting data for the voxel enclosing the given coordinate.
 *
 * @param xyz
 *     Coordinate of voxel.
 * @return
 *     Pointer to the chart data.  If the data FAILED to load,
 *     the returned pointer will be NULL.  Caller takes ownership
 *     of the pointer and must delete it when no longer needed.
 */
ChartData*
CiftiBrainordinateDataSeriesFile::loadChartDataForVoxelAtCoordinate(const float xyz[3]) throw (DataFileException)
{
    ChartData* chartData = helpLoadChartDataForVoxelAtCoordinate(xyz);
    return chartData;
}


/**
 * Load chart data for a surface node.
 * Note: This method will return a chart even if charting for
 * this file is disabled.
 *
 * @param structure
 *     The surface's structure
 * @param nodeIndex
 *     Index of node
 * @param timeLineOut
 *     Output charting data.
 * @return
 *     True if chart data is valid, else false.
 */
bool
CiftiBrainordinateDataSeriesFile::loadChartForSurfaceNode(const StructureEnum::Enum structure,
                                                          const int32_t nodeIndex,
                                                          TimeLine& timeLineOut) throw (DataFileException)
{
    std::vector<float> data;
    if (getSeriesDataForSurfaceNode(structure,
                                    nodeIndex,
                                    data)) {
        const int64_t numData = static_cast<int64_t>(data.size());
        timeLineOut.nodeid = nodeIndex;
        timeLineOut.x.resize(numData);
        timeLineOut.y.resize(numData);
        for (int64_t i = 0; i < numData; i++) {
            timeLineOut.x[i] = i;
            timeLineOut.y[i] = data[i];
        }
        timeLineOut.id = this;
        timeLineOut.filename = getFileName();
        
        float start, step;
        getMapIntervalStartAndStep(start, step);
        timeLineOut.timeStep = step;
        
        return true;
    }
    
    return false;
}

/**
 * Load chart data for a voxel
 * Note: This method will return a chart even if charting for
 * this file is disabled.
 *
 * @param xyz
 *     Coordinate of voxel.
 * @param timeLineOut
 *     Output charting data.
 * @return
 *     True if chart data is valid, else false.
 */
bool
CiftiBrainordinateDataSeriesFile::loadChartForVoxelAtCoordinate(const float xyz[3],
                                                                TimeLine& timeLineOut) throw (DataFileException)
{
    std::vector<float> data;
    if (getSeriesDataForVoxelAtCoordinate(xyz, data)) {
        const int64_t numData = static_cast<int64_t>(data.size());
        timeLineOut.x.resize(numData);
        timeLineOut.y.resize(numData);
        for (int64_t i = 0; i < numData; i++) {
            timeLineOut.x[i] = i;
            timeLineOut.y[i] = data[i];
        }
        timeLineOut.id = this;
        timeLineOut.filename = getFileName();
        
        timeLineOut.label = "Voxel XYZ:[" + AString::fromNumbers(xyz,3,AString(", ")) + "]";
        
        float start, step;
        getMapIntervalStartAndStep(start, step);
        timeLineOut.timeStep = step;
        
        return true;
    }
    
    return false;
}

/**
 * @return The CaretMappableDataFile that implements this interface.
 */
CaretMappableDataFile*
CiftiBrainordinateDataSeriesFile::getCaretMappableDataFile()
{
    return dynamic_cast<CaretMappableDataFile*>(this);
}

/**
 * @return The CaretMappableDataFile that implements this interface.
 */
const CaretMappableDataFile*
CiftiBrainordinateDataSeriesFile::getCaretMappableDataFile() const
{
    return dynamic_cast<const CaretMappableDataFile*>(this);
}


/**
 * Save file data from the scene.  For subclasses that need to
 * save to a scene, this method should be overriden.  sceneClass
 * will be valid and any scene data should be added to it.
 *
 * @param sceneAttributes
 *    Attributes for the scene.  Scenes may be of different types
 *    (full, generic, etc) and the attributes should be checked when
 *    restoring the scene.
 *
 * @param sceneClass
 *     sceneClass to which data members should be added.
 */
void
CiftiBrainordinateDataSeriesFile::saveFileDataToScene(const SceneAttributes* sceneAttributes,
                                 SceneClass* sceneClass)
{
    CiftiMappableDataFile::saveFileDataToScene(sceneAttributes,
                                               sceneClass);
    
    sceneClass->addBoolean("m_chartingEnabled",
                           m_chartingEnabled);
}

/**
 * Restore file data from the scene.  For subclasses that need to
 * restore from a scene, this method should be overridden. The scene class
 * will be valid and any scene data may be obtained from it.
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
CiftiBrainordinateDataSeriesFile::restoreFileDataFromScene(const SceneAttributes* sceneAttributes,
                                      const SceneClass* sceneClass)
{
    CiftiMappableDataFile::restoreFileDataFromScene(sceneAttributes,
                                                    sceneClass);
    
    m_chartingEnabled = sceneClass->getBooleanValue("m_chartingEnabled",
                                                    false);
}


