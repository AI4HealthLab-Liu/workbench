
/*LICENSE_START*/
/*
 *  Copyright (C) 2017 Washington University School of Medicine
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

#define __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__
#include "BrainOpenGLChartTwoDrawingFixedPipeline.h"
#undef __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__

#include <algorithm>

#include "AnnotationCoordinate.h"
#include "AnnotationColorBar.h"
#include "AnnotationPercentSizeText.h"
#include "AnnotationPointSizeText.h"
#include "Brain.h"
#include "BrainOpenGLAnnotationDrawingFixedPipeline.h"
#include "BrainOpenGLFixedPipeline.h"
#include "BrainOpenGLTextRenderInterface.h"
#include "BrainOpenGLViewportContent.h"
#include "BrowserTabContent.h"
#include "CaretAssert.h"
#include "CaretLogger.h"
#include "CaretPreferences.h"
#include "ChartTwoCartesianAxis.h"
#include "ChartTwoDataCartesian.h"
#include "ChartTwoLineSeriesHistory.h"
#include "ChartTwoMatrixDisplayProperties.h"
#include "ChartTwoOverlay.h"
#include "ChartTwoOverlaySet.h"
#include "ChartableTwoFileDelegate.h"
#include "ChartableTwoFileHistogramChart.h"
#include "ChartableTwoFileMatrixChart.h"
#include "ChartableTwoFileLineSeriesChart.h"
#include "CiftiMappableConnectivityMatrixDataFile.h"
#include "FastStatistics.h"
#include "GraphicsEngineDataOpenGL.h"
#include "GraphicsPrimitiveV3f.h"
#include "GraphicsPrimitiveV3fC4f.h"
#include "GraphicsPrimitiveV3fC4ub.h"
#include "IdentificationWithColor.h"
#include "MathFunctions.h"
#include "ModelChartTwo.h"
#include "NodeAndVoxelColoring.h"
#include "PaletteColorMapping.h"
#include "PaletteFile.h"
#include "SessionManager.h"
#include "SelectionItemAnnotation.h"
#include "SelectionItemChartTwoHistogram.h"
#include "SelectionItemChartTwoLineSeries.h"
#include "SelectionItemChartTwoMatrix.h"
#include "SelectionManager.h"

using namespace caret;

/**
 * \class caret::BrainOpenGLChartTwoDrawingFixedPipeline 
 * \brief Drawing of version two charts.
 * \ingroup Brain
 */

/**
 * Constructor.
 */
BrainOpenGLChartTwoDrawingFixedPipeline::BrainOpenGLChartTwoDrawingFixedPipeline()
: BrainOpenGLChartTwoDrawingInterface()
{
    m_preferences = SessionManager::get()->getCaretPreferences();
}

/**
 * Destructor.
 */
BrainOpenGLChartTwoDrawingFixedPipeline::~BrainOpenGLChartTwoDrawingFixedPipeline()
{
}

/**
 * Draw charts from a chart overlay.
 *
 * @param brain
 *     Brain.
 * @param viewportContent
 *     Content of the viewport
 * @param chartTwoModel
 *     The chart two model.
 * @param fixedPipelineDrawing
 *     The fixed pipeline OpenGL drawing.
 * @param selectionItemDataType
 *     Selected data type.
 * @param viewport
 *     Viewport for the chart.
 * @param viewportSpaceAnnotationsOut
 *     Output containing annotation chart axis labels that will be drawn 
 *     by the Annotation OpenGL Drawing in viewport space.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawChartOverlaySet(Brain* brain,
                                                             BrainOpenGLViewportContent* viewportContent,
                                                             ModelChartTwo* chartTwoModel,
                                                             BrainOpenGLFixedPipeline* fixedPipelineDrawing,
                                                             const SelectionItemDataTypeEnum::Enum selectionItemDataType,
                                                             const int32_t viewport[4],
                                                             std::vector<Annotation*>& viewportSpaceAnnotationsOut)
{
    viewportSpaceAnnotationsOut.clear();
    m_viewportSpaceAnnotationsOut.clear();
    
    BrowserTabContent* browserTabContent = viewportContent->getBrowserTabContent();
    CaretAssert(browserTabContent);
    CaretAssert(brain);
    CaretAssert(browserTabContent);
    CaretAssert(chartTwoModel);
    CaretAssert(fixedPipelineDrawing);
    
    m_brain = brain;
    m_viewportContent = viewportContent;
    m_chartTwoModel = chartTwoModel;
    m_fixedPipelineDrawing = fixedPipelineDrawing;
    m_textRenderer = fixedPipelineDrawing->getTextRenderer();
    m_browserTabContent = browserTabContent;
    m_translation[0] = 0.0;
    m_translation[1] = 0.0;
    m_translation[2] = 0.0;
    m_browserTabContent->getTranslation(m_translation);
    m_tabIndex = m_browserTabContent->getTabNumber();
    m_zooming        = m_browserTabContent->getScaling();
    m_chartOverlaySet = m_chartTwoModel->getChartTwoOverlaySet(m_tabIndex);
    m_selectionItemDataType = selectionItemDataType;
    m_viewport[0] = viewport[0];
    m_viewport[1] = viewport[1];
    m_viewport[2] = viewport[2];
    m_viewport[3] = viewport[3];

    m_selectionItemAnnotation = m_brain->getSelectionManager()->getAnnotationIdentification();
    m_selectionItemHistogram  = m_brain->getSelectionManager()->getChartTwoHistogramIdentification();
    m_selectionItemLineSeries = m_brain->getSelectionManager()->getChartTwoLineSeriesIdentification();
    m_selectionItemMatrix     = m_brain->getSelectionManager()->getChartTwoMatrixIdentification();

    m_fixedPipelineDrawing->disableLighting();
    
    /*
     * Find color bars for this tab and decrease height of 
     * viewport so chart is above color bars.
     */
    std::vector<AnnotationColorBar*> colorBars;
    m_browserTabContent->getAnnotationColorBars(colorBars);
    if ( ! colorBars.empty()) {
        int heightOfAllColorBars = 0;
        
        for (auto cb : colorBars) {
            bool useItFlag = false;
            switch (cb->getCoordinateSpace()) {
                case AnnotationCoordinateSpaceEnum::CHART:
                    break;
                case AnnotationCoordinateSpaceEnum::STEREOTAXIC:
                    break;
                case AnnotationCoordinateSpaceEnum::SURFACE:
                    break;
                case AnnotationCoordinateSpaceEnum::TAB:
                    useItFlag = true;
                    break;
                case AnnotationCoordinateSpaceEnum::VIEWPORT:
                    break;
                case AnnotationCoordinateSpaceEnum::WINDOW:
                    break;
            }
            
            if (useItFlag) {
                float bottomLeft[3], bottomRight[3], topRight[3], topLeft[3];
                float viewportXYZ[3] = { 5.0, 5.0, 0.0 };
                const bool boundsValid = cb->getShapeBounds(m_viewport[2], m_viewport[3],
                                                            viewportXYZ,
                                                            bottomLeft, bottomRight, topRight, topLeft);
                if (boundsValid) {
                    heightOfAllColorBars += (topRight[1] - bottomRight[1]);
                }
            }
        }
        
        if (heightOfAllColorBars > 0) {
            const int32_t extraSpace = 5;
            heightOfAllColorBars += extraSpace;
            if (heightOfAllColorBars < viewport[3]) {
                m_viewport[1] += heightOfAllColorBars;
                m_viewport[3] -= heightOfAllColorBars;
            }
        }
    }
    
    bool drawHistogramFlag  = true;
    bool drawLineSeriesFlag = true;
    bool drawMatrixFlag     = true;
    
    /*
     * Check for a 'selection' type mode
     */
    m_identificationModeFlag = false;
    switch (m_fixedPipelineDrawing->mode) {
        case BrainOpenGLFixedPipeline::MODE_DRAWING:
            break;
        case BrainOpenGLFixedPipeline::MODE_IDENTIFICATION:
            if ( (! m_selectionItemHistogram->isEnabledForSelection())
                && (! m_selectionItemAnnotation->isEnabledForSelection()) ){
                drawHistogramFlag = false;
            }
            if ( (! m_selectionItemLineSeries->isEnabledForSelection())
                  && (! m_selectionItemAnnotation->isEnabledForSelection()) ) {
                drawLineSeriesFlag = false;
            }
            if ( (! m_selectionItemMatrix->isEnabledForSelection())
                  && (! m_selectionItemAnnotation->isEnabledForSelection()) ) {
                drawMatrixFlag = false;
            }
            if (drawHistogramFlag
                || drawLineSeriesFlag
                || drawMatrixFlag) {
                m_identificationModeFlag = true;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            else {
                return;
            }
            break;
        case BrainOpenGLFixedPipeline::MODE_PROJECTION:
            return;
            break;
    }
    
    saveStateOfOpenGL();
    
    if (m_chartOverlaySet != NULL) {
        const int32_t numberOfOverlays = m_chartOverlaySet->getNumberOfDisplayedOverlays();
        if (numberOfOverlays > 0) {
            ChartTwoOverlay* topOverlay = m_chartOverlaySet->getOverlay(0);
            CaretAssert(topOverlay);
            CaretMappableDataFile* mapFile = NULL;
            ChartTwoOverlay::SelectedIndexType selectedIndexType = ChartTwoOverlay::SelectedIndexType::INVALID;
            int32_t selectedIndex = -1;
            topOverlay->getSelectionData(mapFile,
                                      selectedIndexType,
                                      selectedIndex);
            if (mapFile != NULL) {
                const ChartTwoDataTypeEnum::Enum chartDataType = topOverlay->getChartTwoDataType();
                switch (chartDataType) {
                    case ChartTwoDataTypeEnum::CHART_DATA_TYPE_INVALID:
                        break;
                    case ChartTwoDataTypeEnum::CHART_DATA_TYPE_HISTOGRAM:
                        if (drawHistogramFlag) {
                            drawHistogramOrLineSeriesChart(ChartTwoDataTypeEnum::CHART_DATA_TYPE_HISTOGRAM);
                        }
                        break;
                    case ChartTwoDataTypeEnum::CHART_DATA_TYPE_LINE_SERIES:
                        if (drawLineSeriesFlag) {
                            drawHistogramOrLineSeriesChart(ChartTwoDataTypeEnum::CHART_DATA_TYPE_LINE_SERIES);
                        }
                        break;
                    case ChartTwoDataTypeEnum::CHART_DATA_TYPE_MATRIX:
                        if (drawMatrixFlag) {
                            drawMatrixChart();
                        }
                        break;
                }
                
                /*
                 * Draw annotations for the chart.
                 */
                BrainOpenGLAnnotationDrawingFixedPipeline::Inputs inputs(m_fixedPipelineDrawing->m_brain,
                                                                         m_fixedPipelineDrawing->mode,
                                                                         BrainOpenGLFixedPipeline::s_gluLookAtCenterFromEyeOffsetDistance,
                                                                         m_fixedPipelineDrawing->m_windowIndex,
                                                                         m_fixedPipelineDrawing->windowTabIndex,
                                                                         BrainOpenGLAnnotationDrawingFixedPipeline::Inputs::WINDOW_DRAWING_NO);
                std::vector<AnnotationColorBar*> emptyColorBars;
                std::vector<Annotation*> emptyViewportAnnotations;
                m_fixedPipelineDrawing->m_annotationDrawing->drawAnnotations(&inputs,
                                                         AnnotationCoordinateSpaceEnum::STEREOTAXIC,
                                                         emptyColorBars,
                                                         emptyViewportAnnotations,
                                                         NULL);
            }
        }
    }
    
    restoreStateOfOpenGL();
    
    viewportSpaceAnnotationsOut = m_viewportSpaceAnnotationsOut;
}


/**
 * Draw a histogram or line series chart.
 *
 * @param chartDataType
 *     Type of chart to draw.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawHistogramOrLineSeriesChart(const ChartTwoDataTypeEnum::Enum chartDataType)
{
    bool drawHistogramFlag  = false;
    bool drawLineSeriesFlag = false;
    switch (chartDataType) {
        case ChartTwoDataTypeEnum::CHART_DATA_TYPE_INVALID:
            CaretAssert(0);
            return;
            break;
        case ChartTwoDataTypeEnum::CHART_DATA_TYPE_HISTOGRAM:
            drawHistogramFlag = true;
            break;
        case ChartTwoDataTypeEnum::CHART_DATA_TYPE_LINE_SERIES:
            drawLineSeriesFlag = true;
            break;
        case ChartTwoDataTypeEnum::CHART_DATA_TYPE_MATRIX:
            CaretAssert(0);
            return;
            break;
    }
    
    const int32_t tabViewportX = m_viewport[0];
    const int32_t tabViewportY = m_viewport[1];
    const int32_t tabViewportWidth  = m_viewport[2];
    const int32_t tabViewportHeight = m_viewport[3];
    
    int32_t chartGraphicsDrawingViewport[4] = {
        tabViewportX,
        tabViewportY,
        tabViewportWidth,
        tabViewportHeight
    };
    
    const int32_t numberOfOverlays = m_chartOverlaySet->getNumberOfDisplayedOverlays();
    CaretAssert(numberOfOverlays > 0);
    const ChartTwoOverlay* topOverlay = m_chartOverlaySet->getOverlay(0);
    const ChartTwoCompoundDataType cdt = topOverlay->getChartTwoCompoundDataType();
    if (drawHistogramFlag) {
        CaretAssert(cdt.getChartTwoDataType() == ChartTwoDataTypeEnum::CHART_DATA_TYPE_HISTOGRAM);
    }
    else if (drawLineSeriesFlag) {
        CaretAssert(cdt.getChartTwoDataType() == ChartTwoDataTypeEnum::CHART_DATA_TYPE_LINE_SERIES);
    }
    else {
        CaretAssert(0);
    }
    
    std::vector<std::unique_ptr<HistogramChartDrawingInfo>> histogramDrawingInfo;
    std::deque<LineSeriesChartDrawingInfo> lineSeriesChartsToDraw;
    
    /*
     * Get the histogram drawing information and overall extent
     */
    float xMin = std::numeric_limits<float>::max();
    float xMax = -std::numeric_limits<float>::max();
    float yMinLeft  = std::numeric_limits<float>::max();
    float yMaxLeft  = -std::numeric_limits<float>::max();
    float yMinRight = std::numeric_limits<float>::max();
    float yMaxRight = -std::numeric_limits<float>::max();
    for (int32_t iOverlay = (numberOfOverlays - 1); iOverlay >= 0; iOverlay--) {
        ChartTwoOverlay* chartOverlay = m_chartOverlaySet->getOverlay(iOverlay);
        if ( ! chartOverlay->isEnabled()) {
            continue;
        }
        
        CaretMappableDataFile* mapFile = NULL;
        ChartTwoOverlay::SelectedIndexType selectedIndexType = ChartTwoOverlay::SelectedIndexType::INVALID;
        int32_t selectedIndex = -1;
        chartOverlay->getSelectionData(mapFile,
                                       selectedIndexType,
                                       selectedIndex);
        if (mapFile == NULL) {
            continue;
        }
        
        ChartableTwoFileDelegate* chartDelegate = mapFile->getChartingDelegate();
        
        if (drawHistogramFlag) {
            ChartableTwoFileHistogramChart* histogramChart = chartDelegate->getHistogramCharting();
            
            if (histogramChart->isValid()) {
                CaretAssert(selectedIndexType == ChartTwoOverlay::SelectedIndexType::MAP);
                AString errorMessage;
                histogramDrawingInfo.push_back(std::unique_ptr<HistogramChartDrawingInfo>(new HistogramChartDrawingInfo(histogramChart,
                                                                                                               selectedIndex,
                                                                                                               chartOverlay->getCartesianVerticalAxisLocation(),
                                                                                                               (chartOverlay->isAllMapsSupported()
                                                                                                                && chartOverlay->isAllMapsSelected()))));
                const Histogram* histogram = histogramChart->getHistogramForChartDrawing(selectedIndex,
                                                                                         (chartOverlay->isAllMapsSupported()
                                                                                          && chartOverlay->isAllMapsSelected()));
                CaretAssert(histogram);
                float histogramMinX = 0.0, histogramMaxX = 0.0, histogramMaxY = 0.0;
                histogram->getRangeAndMaxDisplayHeight(histogramMinX, histogramMaxX, histogramMaxY);
                if (histogramMaxX > histogramMinX) {
                    xMin = std::min(xMin, histogramMinX);
                    xMax = std::max(xMax, histogramMaxX);
                    
                    switch (chartOverlay->getCartesianVerticalAxisLocation()) {
                        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                            CaretAssertMessage(0, "TOP axis not allowed for vertical axis");
                            break;
                        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                            yMinRight = std::min(yMinRight, 0.0f);
                            yMaxRight = std::max(yMaxRight, histogramMaxY);
                            break;
                        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                            yMinLeft = std::min(yMinLeft, 0.0f);
                            yMaxLeft = std::max(yMaxLeft, histogramMaxY);
                            break;
                        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                            CaretAssertMessage(0, "BOTTOM axis not allowed for vertical axis");
                            break;
                    }
                }
            }
        }
        
        if (drawLineSeriesFlag) {
            const ChartableTwoFileLineSeriesChart* lineSeriesChart = chartDelegate->getLineSeriesCharting();
            const ChartTwoLineSeriesHistory* lineSeriesHistory = lineSeriesChart->getHistory();
            const int32_t numHistory = lineSeriesHistory->getHistoryCount();
            for (int32_t iHistory = (numHistory - 1); iHistory >= 0; iHistory--) {
                const ChartTwoDataCartesian* data = lineSeriesHistory->getHistoryItem(iHistory);
                CaretAssert(data);
                if (data->isSelected()) {
                    BoundingBox boundingBox;
                    if (data->getBounds(boundingBox)) {
                        xMin = std::min(xMin, boundingBox.getMinX());
                        xMax = std::max(xMax, boundingBox.getMaxX());
                        
                        switch (chartOverlay->getCartesianVerticalAxisLocation()) {
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                                CaretAssertMessage(0, "TOP axis not allowed for vertical axis");
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                                yMinRight = std::min(yMinRight, boundingBox.getMinY());
                                yMaxRight = std::max(yMaxRight, boundingBox.getMaxY());
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                                yMinLeft = std::min(yMinLeft, boundingBox.getMinY());
                                yMaxLeft = std::max(yMaxLeft, boundingBox.getMaxY());
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                                CaretAssertMessage(0, "BOTTOM axis not allowed for vertical axis");
                                break;
                        }
                        
                        lineSeriesChartsToDraw.push_back(LineSeriesChartDrawingInfo(lineSeriesChart,
                                                                                    data,
                                                                                    chartOverlay->getCartesianVerticalAxisLocation()));
                    }
                }
            }
        }
    }
    
    /*
     * Bounds valid?
     */
    if ((xMin < xMax)
        && ((yMinLeft < yMaxLeft) || (yMinRight < yMaxRight))) {
        /*
         * Margin is region around the chart in which
         * the axes legends, values, and ticks are drawn.
         */
        const double marginSize = 10;
        Margins margins(marginSize);
        
        ChartTwoCartesianAxis* leftAxis   = NULL;
        ChartTwoCartesianAxis* rightAxis  = NULL;
        ChartTwoCartesianAxis* bottomAxis = NULL;
        ChartTwoCartesianAxis* topAxis    = NULL;
        
        std::vector<ChartTwoCartesianAxis*> displayedAxes;
        AnnotationPercentSizeText* leftAxisLabel   = NULL;
        AnnotationPercentSizeText* rightAxisLabel  = NULL;
        AnnotationPercentSizeText* bottomAxisLabel = NULL;
        m_chartOverlaySet->getDisplayedChartAxes(displayedAxes,
                                                 leftAxisLabel,
                                                 rightAxisLabel,
                                                 bottomAxisLabel);
        for (auto axis : displayedAxes) {
            CaretAssert(axis);
            switch (axis->getAxisLocation()) {
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                    bottomAxis = axis;
                    break;
                case  ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                    leftAxis = axis;
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                    rightAxis = axis;
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                    topAxis = axis;
                    break;
            }
        }
        
        double width = 0.0, height = 0.0;
        
        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        
        glViewport(tabViewportX,
                   tabViewportY,
                   tabViewportWidth,
                   tabViewportHeight);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, tabViewportWidth, 0, tabViewportHeight, -1.0, 1.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        estimateCartesianChartAxisLegendsWidthHeight(yMinLeft, yMaxLeft, tabViewportWidth, tabViewportHeight, leftAxis, leftAxisLabel, width, height);
        margins.m_left = std::max(margins.m_left, width);
        estimateCartesianChartAxisLegendsWidthHeight(yMinRight, yMaxRight, tabViewportWidth, tabViewportHeight, rightAxis, rightAxisLabel, width, height);
        margins.m_right = std::max(margins.m_right, width);
        estimateCartesianChartAxisLegendsWidthHeight(xMin, xMax, tabViewportWidth, tabViewportHeight, topAxis, NULL, width, height);
        margins.m_top = std::max(margins.m_top, height);
        estimateCartesianChartAxisLegendsWidthHeight(xMin, xMax, tabViewportWidth, tabViewportHeight, bottomAxis, bottomAxisLabel, width, height);
        margins.m_bottom = std::max(margins.m_bottom, height);
        
        /*
         * Ensure that there is sufficient space for the axes data display.
         */
        if ((tabViewportWidth > (marginSize * 3))
            && (tabViewportHeight > (marginSize * 3))) {
            
            float axisMinimumValue = 0.0;
            float axisMaximumValue = 0.0;
            
            /* Draw legends and grids */
            if (drawChartAxisCartesian(yMinLeft,
                                       yMaxLeft,
                                       tabViewportX,
                                       tabViewportY,
                                       tabViewportWidth,
                                       tabViewportHeight,
                                       margins,
                                       leftAxis,
                                       leftAxisLabel,
                                       axisMinimumValue,
                                       axisMaximumValue)) {
                yMinLeft = axisMinimumValue;
                yMaxLeft = axisMaximumValue;
            }
            
            if (drawChartAxisCartesian(yMinRight,
                                       yMaxRight,
                                       tabViewportX,
                                       tabViewportY,
                                       tabViewportWidth,
                                       tabViewportHeight,
                                       margins,
                                       rightAxis,
                                       rightAxisLabel,
                                       axisMinimumValue,
                                       axisMaximumValue)) {
                yMinRight = axisMinimumValue;
                yMaxRight = axisMaximumValue;
            }
            if (drawChartAxisCartesian(xMin,
                                       xMax,
                                       tabViewportX,
                                       tabViewportY,
                                       tabViewportWidth,
                                       tabViewportHeight,
                                       margins,
                                       bottomAxis,
                                       bottomAxisLabel,
                                       axisMinimumValue,
                                       axisMaximumValue)) {
                xMin = axisMinimumValue;
                xMax = axisMaximumValue;
            }
            
            drawChartAxisCartesian(xMin,
                                   xMax,
                                   tabViewportX,
                                   tabViewportY,
                                   tabViewportWidth,
                                   tabViewportHeight,
                                   margins,
                                   topAxis,
                                   NULL,
                                   axisMinimumValue,
                                   axisMaximumValue);
            
            drawChartGraphicsBoxAndSetViewport(tabViewportX,
                                               tabViewportY,
                                               tabViewportWidth,
                                               tabViewportHeight,
                                               margins,
                                               chartGraphicsDrawingViewport);
        }
        
        glViewport(chartGraphicsDrawingViewport[0],
                   chartGraphicsDrawingViewport[1],
                   chartGraphicsDrawingViewport[2],
                   chartGraphicsDrawingViewport[3]);
        
        
        if (drawHistogramFlag) {
            /*
             * Draw the bars for all histogram and then draw the envelopes
             * so the envelopes are not obscured by the bars
             */
            for (auto& drawInfo : histogramDrawingInfo) {
                if (drawInfo->m_histogramChart->isValid()) {
                    const CaretMappableDataFile* cmdf = drawInfo->m_histogramChart->getCaretMappableDataFile();
                    CaretAssert(cmdf);
                    const PaletteColorMapping* paletteColorMapping = cmdf->getMapPaletteColorMapping(drawInfo->m_mapIndex);
                    const bool drawBarsFlag     = paletteColorMapping->isHistogramBarsVisible();
                    const bool drawEnvelopeFlag = paletteColorMapping->isHistogramEnvelopeVisible();
                    
                    if (drawBarsFlag
                        || drawEnvelopeFlag) {
                        bool leftVerticalAxisFlag = true;
                        switch (drawInfo->m_verticalAxisLocation) {
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                                leftVerticalAxisFlag = false;
                                break;
                            case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                                break;
                        }
                        
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();
                        if (leftVerticalAxisFlag) {
                            glOrtho(xMin, xMax,
                                    yMinLeft, yMaxLeft,
                                    -10.0, 10.0);
                        }
                        else {
                            glOrtho(xMin, xMax,
                                    yMinRight, yMaxRight,
                                    -10.0, 10.0);
                        }
                        
                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();
                        
                        bool applyTransformationsFlag = false;
                        if (applyTransformationsFlag) {
                            glTranslatef(m_translation[0],
                                         m_translation[1],
                                         0.0);
                            
                            const float chartWidth  = chartGraphicsDrawingViewport[2];
                            const float chartHeight = chartGraphicsDrawingViewport[3];
                            const float halfWidth   = chartWidth  / 2.0;
                            const float halfHeight  = chartHeight / 2.0;
                            glTranslatef(halfWidth,
                                         halfHeight,
                                         0.0);
                            glScalef(m_zooming,
                                     m_zooming,
                                     1.0);
                            glTranslatef(-halfWidth,
                                         -halfHeight,
                                         0.0);
                        }
                        
                        /*
                         * Save the transformation matrices and the viewport
                         * If there is more than one line chart, this code will be executed
                         * several times but since the top overlay is drawn last, the contents
                         * of the top overlay will be used.
                         */
                        updateViewportContentForCharting(chartGraphicsDrawingViewport);
                        
                        ChartableTwoFileHistogramChart::HistogramPrimitives* histogramPrimitives =
                        drawInfo->m_histogramChart->getMapHistogramDrawingPrimitives(drawInfo->m_mapIndex,
                                                                                     drawInfo->m_allMapsSelected);
                        
                        if (histogramPrimitives != NULL) {
                            const float ENVELOPE_LINE_WIDTH = 1.0;
                            
                            if (m_identificationModeFlag) {
                                int32_t primitiveIndex = -1;
                                float   primitiveDepth = 0.0;
                                
                                if (drawBarsFlag) {
                                    /*
                                     * Scale histogram bars in Y-axis so that identification
                                     * will function when mouse is above a bar.  This is helpful
                                     * for identification of bars with a very small height.
                                     */
                                    glPushMatrix();
                                    glScalef(1.0, 1000.0, 1.0);
                                    GraphicsEngineDataOpenGL::drawWithSelection(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                                                                histogramPrimitives->getBarsPrimitive(),
                                                                                m_fixedPipelineDrawing->mouseX,
                                                                                m_fixedPipelineDrawing->mouseY,
                                                                                primitiveIndex,
                                                                                primitiveDepth);
                                    glPopMatrix();
                                }
                                else if (drawEnvelopeFlag) {
                                    /*
                                     * Increase line width for identification
                                     */
                                    glLineWidth(ENVELOPE_LINE_WIDTH * 3.0f);
                                    GraphicsEngineDataOpenGL::drawWithSelection(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                                                                histogramPrimitives->getEnvelopePrimitive(),
                                                                                m_fixedPipelineDrawing->mouseX,
                                                                                m_fixedPipelineDrawing->mouseY,
                                                                                primitiveIndex,
                                                                                primitiveDepth);
                                }
                                
                                if (primitiveIndex >= 0) {
                                    if (m_selectionItemHistogram->isOtherScreenDepthCloserToViewer(primitiveDepth)) {
                                        m_selectionItemHistogram->setHistogramChart(const_cast<ChartableTwoFileHistogramChart*>(drawInfo->m_histogramChart),
                                                                                    drawInfo->m_mapIndex,
                                                                                    primitiveIndex,
                                                                                    drawInfo->m_allMapsSelected);
                                    }
                                }
                            }
                            else {
                                drawPrimitivePrivate(histogramPrimitives->getThresholdPrimitive());
                                if (drawBarsFlag) {
                                    drawPrimitivePrivate(histogramPrimitives->getBarsPrimitive());
                                }
                                if (drawEnvelopeFlag) {
                                    glLineWidth(ENVELOPE_LINE_WIDTH);
                                    m_fixedPipelineDrawing->enableLineAntiAliasing();
                                    drawPrimitivePrivate(histogramPrimitives->getEnvelopePrimitive());
                                    m_fixedPipelineDrawing->disableLineAntiAliasing();
                                }
                            }
                        }
                    }
                }
            }
        }
        if (drawLineSeriesFlag) {
            for (const auto lineChart : lineSeriesChartsToDraw) {
                bool leftVerticalAxisFlag = true;
                switch (lineChart.m_verticalAxisLocation) {
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                        leftVerticalAxisFlag = false;
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                        break;
                }
                
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                if (leftVerticalAxisFlag) {
                    glOrtho(xMin, xMax,
                            yMinLeft, yMaxLeft,
                            -10.0, 10.0);
                }
                else {
                    glOrtho(xMin, xMax,
                            yMinRight, yMaxRight,
                            -10.0, 10.0);
                }
                
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                bool applyTransformationsFlag = false;
                if (applyTransformationsFlag) {
                    glTranslatef(m_translation[0],
                                 m_translation[1],
                                 0.0);
                    
                    const float chartWidth  = chartGraphicsDrawingViewport[2];
                    const float chartHeight = chartGraphicsDrawingViewport[3];
                    const float halfWidth   = chartWidth  / 2.0;
                    const float halfHeight  = chartHeight / 2.0;
                    glTranslatef(halfWidth,
                                 halfHeight,
                                 0.0);
                    glScalef(m_zooming,
                             m_zooming,
                             1.0);
                    glTranslatef(-halfWidth,
                                 -halfHeight,
                                 0.0);
                }
                
                const float LINE_SERIES_LINE_WIDTH = 1.0f;
                if (m_identificationModeFlag) {
                    int32_t primitiveIndex = -1;
                    float   primitiveDepth = 0.0;
                    
                    glLineWidth(LINE_SERIES_LINE_WIDTH * 5.0f);
                    GraphicsEngineDataOpenGL::drawWithSelection(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                                                lineChart.m_chartTwoCartesianData->getGraphicsPrimitive(),
                                                                m_fixedPipelineDrawing->mouseX,
                                                                m_fixedPipelineDrawing->mouseY,
                                                                primitiveIndex,
                                                                primitiveDepth);
                    
                    if (primitiveIndex >= 0) {
                        if (m_selectionItemLineSeries->isOtherScreenDepthCloserToViewer(primitiveDepth)) {
                            m_selectionItemLineSeries->setLineSeriesChart(const_cast<ChartableTwoFileLineSeriesChart*>(lineChart.m_lineSeriesChart),
                                                                          const_cast<ChartTwoDataCartesian*>(lineChart.m_chartTwoCartesianData),
                                                                          primitiveIndex);
                        }
                    }
                }
                else {
                    /*
                     * Note: Anti-aliasing is only enabled when drawing the line.
                     * Anti-aliasing cannot be enabled during identification since
                     * identification encodes information in the colors (anti-aliasing
                     * will alter the colors during drawing).
                     */
                    m_fixedPipelineDrawing->enableLineAntiAliasing();
                    glLineWidth(lineChart.m_chartTwoCartesianData->getLineWidth());
                    GraphicsEngineDataOpenGL::draw(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                               lineChart.m_chartTwoCartesianData->getGraphicsPrimitive());
                    m_fixedPipelineDrawing->disableLineAntiAliasing();
                }
                
                /*
                 * Save the transformation matrices and the viewport
                 * If there is more than one line chart, this code will be executed
                 * several times but since the top overlay is drawn last, the contents
                 * of the top overlay will be used.
                 */
                updateViewportContentForCharting(chartGraphicsDrawingViewport);
            }
        }
    }
}


/**
 * Draw a matrix chart.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawMatrixChart()
{
    glViewport(m_viewport[0],
               m_viewport[1],
               m_viewport[2],
               m_viewport[3]);
    
    bool applyTransformationsFlag = true;
    float cellWidth = 1.0;
    float cellHeight = 1.0;
    
    /*
     * Setup width/height of area in which matrix is drawn with a
     * small margin along all of the edges
     */
    float margin = 10.0;
    if ((m_viewport[2] < (margin * 3.0))
        || (m_viewport[3] < (margin * 3.0))) {
        margin = 0.0;
    }
    const float graphicsWidth  = m_viewport[2] - (margin * 2.0);
    const float graphicsHeight = m_viewport[3] - (margin * 2.0);

    /*
     * First overlay is ALWAYS ON and since all matrices must have 
     * same number of rows/columns, use first matrix for rows/columns
     */
    const int32_t numberOfOverlays = m_chartOverlaySet->getNumberOfDisplayedOverlays();
    CaretAssert(numberOfOverlays > 0);
    const ChartTwoOverlay* topOverlay = m_chartOverlaySet->getOverlay(0);
    const ChartTwoCompoundDataType cdt = topOverlay->getChartTwoCompoundDataType();
    CaretAssert(cdt.getChartTwoDataType() == ChartTwoDataTypeEnum::CHART_DATA_TYPE_MATRIX);
    const int32_t numberOfRows = cdt.getMatrixNumberOfRows();
    const int32_t numberOfCols = cdt.getMatrixNumberOfColumns();
    if ((numberOfRows > 0)
        && (numberOfCols > 0)) {
        cellWidth  = graphicsWidth  / numberOfCols;
        cellHeight = graphicsHeight / numberOfRows;
    }
    else {
        return;
    }
    
    /*
     * Set the width and neight of each matrix cell.
     */
    const ChartTwoMatrixDisplayProperties* matrixProperties = m_browserTabContent->getChartTwoMatrixDisplayProperties();
    CaretAssert(matrixProperties);
    const float cellWidthZoom  = matrixProperties->getCellPercentageZoomWidth()  / 100.0;
    const float cellHeightZoom = matrixProperties->getCellPercentageZoomHeight() / 100.0;
    if ((cellWidthZoom > 0.0)
        && (cellHeightZoom > 0.0)) {
        cellWidth  *= cellWidthZoom;
        cellHeight *= cellHeightZoom;
    }
    
    /*
     * Set the coordinates for the area in which the matrix is drawn.
     */
    const float xMin = -margin;
    const float xMax = graphicsWidth + margin;
    const float yMin = -margin;
    const float yMax = graphicsHeight + margin;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(xMin, xMax,
            yMin, yMax,
            -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (applyTransformationsFlag) {
        glTranslatef(m_translation[0],
                     m_translation[1],
                     0.0);
        
        const float chartWidth  = cellWidth  * numberOfCols;
        const float chartHeight = cellHeight * numberOfRows;
        const float halfWidth   = chartWidth  / 2.0;
        const float halfHeight  = chartHeight / 2.0;
        glTranslatef(halfWidth,
                     halfHeight,
                     0.0);
        glScalef(m_zooming,
                 m_zooming,
                 1.0);
        glTranslatef(-halfWidth,
                     -halfHeight,
                     0.0);
    }

    /*
     * Save the transformation matrices and the viewport
     * If there is more than one line chart, this code will be executed
     * several times but since the top overlay is drawn last, the contents
     * of the top overlay will be used.
     */
    updateViewportContentForCharting(m_viewport);
    
    for (int32_t iOverlay = (numberOfOverlays - 1); iOverlay >= 0; iOverlay--) {
        ChartTwoOverlay* chartOverlay = m_chartOverlaySet->getOverlay(iOverlay);
        if ( ! chartOverlay->isEnabled()) {
            continue;
        }
        
        CaretMappableDataFile* mapFile = NULL;
        ChartTwoOverlay::SelectedIndexType selectedIndexType = ChartTwoOverlay::SelectedIndexType::INVALID;
        int32_t selectedIndex = -1;
        chartOverlay->getSelectionData(mapFile,
                                       selectedIndexType,
                                       selectedIndex);
        if (mapFile == NULL) {
            continue;
        }
        
        const ChartableTwoFileMatrixChart* matrixChart = mapFile->getChartingDelegate()->getMatrixCharting();
        int32_t overlayRows = 0;
        int32_t overlayColumns = 0;
        matrixChart->getMatrixDimensions(overlayRows,
                                         overlayColumns);
        
        /*
         * All matrices must have same rows/columns
         */
        if ((overlayRows == numberOfRows)
            && (overlayColumns == numberOfCols)) {
            drawMatrixChartContent(matrixChart,
                                   chartOverlay->getMatrixTriangularViewingMode(),
                                cellWidth,
                                cellHeight,
                                m_zooming);
        }
    }
}

/*
 * Draw a matrix chart.
 *
 * @param matrixChart
 *     Matrix chart that is drawn.
 * @param chartViewingType
 *     Type of chart viewing.
 * @param cellWidth
 *     Width of cell.
 * @param cellHeight
 *     Height of cell.
 * @param zooming
 *     Current zooming.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawMatrixChartContent(const ChartableTwoFileMatrixChart* matrixChart,
                                                             const ChartTwoMatrixTriangularViewingModeEnum::Enum chartViewingType,
                                                             const float cellWidth,
                                                             const float cellHeight,
                                                             const float zooming)
{
    GraphicsPrimitiveV3fC4f* matrixPrimitive = matrixChart->getMatrixChartingGraphicsPrimitive(chartViewingType);
    if (matrixPrimitive == NULL) {
        return;
    }
    
    if (m_identificationModeFlag) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    glPushMatrix();
    glScalef(cellWidth, cellHeight, 1.0);
    /*
     * Enable alpha blending so voxels that are not drawn from higher layers
     * allow voxels from lower layers to be seen.
     */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (m_identificationModeFlag) {
        int32_t primitiveIndex = -1;
        float   primitiveDepth = 0.0;
        GraphicsEngineDataOpenGL::drawWithSelection(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                                    matrixPrimitive,
                                                    m_fixedPipelineDrawing->mouseX,
                                                    m_fixedPipelineDrawing->mouseY,
                                                    primitiveIndex,
                                                    primitiveDepth);
        if (primitiveIndex >= 0) {
            int32_t numberOfRows = 0;
            int32_t numberOfColumns = 0;
            matrixChart->getMatrixDimensions(numberOfRows,
                                             numberOfColumns);
            
            const int32_t rowIndex = primitiveIndex / numberOfColumns;
            const int32_t colIndex = primitiveIndex % numberOfColumns;
            
            if (m_selectionItemMatrix->isOtherScreenDepthCloserToViewer(primitiveDepth)) {
                m_selectionItemMatrix->setMatrixChart(const_cast<ChartableTwoFileMatrixChart*>(matrixChart),
                                                      rowIndex,
                                                      colIndex);
            }
        }
    }
    else {
        drawPrimitivePrivate(matrixPrimitive);
        
        const ChartTwoMatrixDisplayProperties* matrixProperties = m_browserTabContent->getChartTwoMatrixDisplayProperties();
        CaretAssert(matrixProperties);
        
        if (matrixProperties->isGridLinesDisplayed()) {
            glPolygonMode(GL_FRONT,
                          GL_LINE);
            
            GraphicsEngineDataOpenGL::drawWithAlternativeColor(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                                               matrixPrimitive,
                                                               matrixChart->getMatrixChartGraphicsPrimitiveGridColorIdentifier());
            glPolygonMode(GL_FRONT,
                          GL_FILL);
        }
        
        int32_t numberOfRows = 0;
        int32_t numberOfColumns = 0;
        matrixChart->getMatrixDimensions(numberOfRows,
                                         numberOfColumns);
        
        if (matrixProperties->isSelectedRowColumnHighlighted()) {
            uint8_t highlightRGBByte[3];
            m_preferences->getBackgroundAndForegroundColors()->getColorForegroundChartView(highlightRGBByte);
            const float highlightRGBA[4] = {
                highlightRGBByte[0] / 255.0f,
                highlightRGBByte[1] / 255.0f,
                highlightRGBByte[2] / 255.0f,
                1.0f
            };
            
            std::vector<int32_t> selectedColumnIndices;
            std::vector<int32_t> selectedRowIndices;
            ChartTwoMatrixLoadingDimensionEnum::Enum selectedRowColumnDimension;
            matrixChart->getSelectedRowColumnIndices(m_tabIndex,
                                                     selectedRowColumnDimension,
                                                     selectedRowIndices,
                                                     selectedColumnIndices);
            for (auto rowIndex : selectedRowIndices) {
                std::unique_ptr<GraphicsPrimitiveV3f> rowOutlineData4f
                = std::unique_ptr<GraphicsPrimitiveV3f>(GraphicsPrimitive::newPrimitiveV3f(GraphicsPrimitive::PrimitiveType::LINE_LOOP,
                                                                                           highlightRGBA));
                rowOutlineData4f->reserveForNumberOfVertices(4);
                
                float minX = 0;
                float maxX = numberOfColumns;
                const float minY = numberOfRows - rowIndex - 1;
                const float maxY = minY + 1.0f;
                
                switch (chartViewingType) {
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_FULL:
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_FULL_NO_DIAGONAL:
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_LOWER_NO_DIAGONAL:
                        maxX = rowIndex;  /* matrix is symmetric */
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_UPPER_NO_DIAGONAL:
                        minX = rowIndex + 1;  /* matrix is symmetric */
                        break;
                }
                
                rowOutlineData4f->addVertex(minX, minY);
                rowOutlineData4f->addVertex(maxX, minY);
                rowOutlineData4f->addVertex(maxX, maxY);
                rowOutlineData4f->addVertex(minX, maxY);
                const float highlightLineWidth = std::max(((zooming) * 0.20), 3.0);
                glLineWidth(highlightLineWidth);
                drawPrimitivePrivate(rowOutlineData4f.get());
            }
            
            for (auto columnIndex : selectedColumnIndices) {
                std::unique_ptr<GraphicsPrimitiveV3f> columnOutlineData4f
                = std::unique_ptr<GraphicsPrimitiveV3f>(GraphicsPrimitive::newPrimitiveV3f(GraphicsPrimitive::PrimitiveType::LINE_LOOP,
                                                                                           highlightRGBA));
                columnOutlineData4f->reserveForNumberOfVertices(4);
                
                const float minX = columnIndex;
                const float maxX = columnIndex + 1;
                float minY = 0;
                float maxY = numberOfRows;
                
                switch (chartViewingType) {
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_FULL:
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_FULL_NO_DIAGONAL:
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_LOWER_NO_DIAGONAL:
                        maxY = columnIndex;  /* matrix is symmetric */
                        break;
                    case ChartTwoMatrixTriangularViewingModeEnum::MATRIX_VIEW_UPPER_NO_DIAGONAL:
                        minY = columnIndex + 1;  /* matrix is symmetric */
                        break;
                }
                columnOutlineData4f->addVertex(minX, minY);
                columnOutlineData4f->addVertex(maxX, minY);
                columnOutlineData4f->addVertex(maxX, maxY);
                columnOutlineData4f->addVertex(minX, maxY);
                const float highlightLineWidth = std::max(((zooming) * 0.20), 3.0);
                glLineWidth(highlightLineWidth);
                drawPrimitivePrivate(columnOutlineData4f.get());
            }
        }
    }
    
    glDisable(GL_BLEND);
    glPopMatrix();
}

/**
 * Save the state of OpenGL.
 * Copied from Qt's qgl.cpp, qt_save_gl_state().
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::saveStateOfOpenGL()
{
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

/**
 * Restore the state of OpenGL.
 * Copied from Qt's qgl.cpp, qt_restore_gl_state().
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::restoreStateOfOpenGL()
{
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    glPopClientAttrib();
    
}

/**
 * Estimate the size of the axis' text.
 *
 * @param minimumDataValue
 *     Minimum data value
 * @param maximumDataValue
 *     Maximum data value
 * @param viewportWidth
 *     Widgth of viewport.
 * @param viewportHeight
 *     Height of viewport.
 * @param cartesianAxis
 *    The axis.
 * @param chartAxisLabel
 *    Label for the axis.
 * @param widthOut
 *    Width of text out.
 * @param heightOut
 *    Height of text out.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::estimateCartesianChartAxisLegendsWidthHeight(const float minimumDataValue,
                                                                                      const float maximumDataValue,
                                                                                      const float viewportWidth,
                                                                                      const float viewportHeight,
                                                                                      ChartTwoCartesianAxis* cartesianAxis,
                                                                                      AnnotationPercentSizeText* chartAxisLabel,
                                                                                      double& widthOut,
                                                                                      double& heightOut)
{
    widthOut  = 0;
    heightOut = 0;
    
    if (cartesianAxis == NULL) {
        return;
    }
    if ( ! cartesianAxis->isEnabledByChart()) {
        return;
    }
    if ( ! cartesianAxis->isDisplayedByUser()) {
        return;
    }
    
    /*
     * The percentage height from the label is also used
     * by the axis numeric values.
     */
    CaretAssert(chartAxisLabel);
    chartAxisLabel->setTabIndex(m_tabIndex);
    
    /*
     * The length of the axis does not matter here.
     * We are just estimating the width and height of the axes text labels.
     */
    const float axisLength = 1000.0;
    float minimumValue = 0.0;
    float maximumValue = 0.0;
    std::vector<float> scaleValueOffsetInPixels;
    std::vector<AString> scaleValuesText;
    cartesianAxis->getScaleValuesAndOffsets(minimumDataValue,
                                            maximumDataValue,
                                            axisLength,
                                            minimumValue,
                                            maximumValue,
                                            scaleValueOffsetInPixels,
                                            scaleValuesText);
    
    /*
     * For sizing text for numeric values, copy the label annotation 
     * (so font size, style) are the same.  We are only finding 
     * height and width of all numeric values.
     */
    AnnotationPercentSizeText numericValueAnnotation(AnnotationAttributesDefaultTypeEnum::NORMAL,
                                                     AnnotationTextFontSizeTypeEnum::PERCENTAGE_OF_VIEWPORT_HEIGHT);
    numericValueAnnotation = *chartAxisLabel;
    for (std::vector<AString>::iterator iter = scaleValuesText.begin();
         iter != scaleValuesText.end();
         iter++) {
        const AString text = *iter;
        if ( ! text.isEmpty()) {
            numericValueAnnotation.setText(text);
            double textWidth = 0.0;
            double textHeight = 0.0;
            m_textRenderer->getTextWidthHeightInPixels(numericValueAnnotation, viewportWidth, viewportHeight, textWidth, textHeight);
            
            widthOut  = std::max(widthOut,  textWidth);
            heightOut = std::max(heightOut, textHeight);
        }
    }
    
    /*
     * Add space for the numeric value tick marks
     */
    switch (cartesianAxis->getAxisLocation()) {
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
            if (heightOut > 0) {
                heightOut += s_tickLength;
            }
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
            if (widthOut > 0) {
                widthOut += s_tickLength;
            }
            break;
    }
    
    double labelWidth  = 0.0;
    double labelHeight = 0.0;
    m_textRenderer->getTextWidthHeightInPixels(*chartAxisLabel, viewportWidth, viewportHeight, labelWidth, labelHeight);
    
    switch (cartesianAxis->getAxisLocation()) {
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
            heightOut += labelHeight;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
            /** Text is rotated 90 degrees so use HEIGHT */
            widthOut += labelHeight;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
            /** Text is rotated 90 degrees so use HEIGHT */
            widthOut += labelHeight;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
            heightOut += labelHeight;
            break;
    }
}

/**
 * Draw the chart graphics surrounding box and set the graphics viewport.
 *  drawChartGraphicsBoxAndSetViewport
 * @param vpX
 *     Viewport X
 * @param vpY
 *     Viewport Y
 * @param vpWidth
 *     Viewport width
 * @param vpHeight
 *     Viewport height
 * @param marginSize
 *     Margin around grid/box
 * @param chartGraphicsDrawingViewportOut
 *     Output containing viewport for drawing chart graphics within
 *     the box/grid that is adjusted for the box's line thickness.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawChartGraphicsBoxAndSetViewport(const float vpX,
                                                                         const float vpY,
                                                                         const float vpWidth,
                                                                         const float vpHeight,
                                                                         const Margins& margins,
                                                                         int32_t chartGraphicsDrawingViewportOut[4])
{
    const float halfGridLineWidth = GRID_LINE_WIDTH / 2.0;
    
    const float gridLeft   = vpX + margins.m_left;
    const float gridRight  = vpX + vpWidth - margins.m_right;
    const float gridBottom = vpY + margins.m_bottom;
    const float gridTop    = vpY + vpHeight - margins.m_top;
    
    glViewport(vpX,
               vpY,
               vpWidth,
               vpHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(vpX, (vpX + vpWidth),
            vpY, (vpY + vpHeight),
            -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    std::unique_ptr<GraphicsPrimitiveV3f> gridData
    = std::unique_ptr<GraphicsPrimitiveV3f>(GraphicsPrimitive::newPrimitiveV3f(GraphicsPrimitive::PrimitiveType::LINES,
                                                                               m_fixedPipelineDrawing->m_foregroundColorFloat));
    gridData->reserveForNumberOfVertices(8);
    gridData->addVertex(gridLeft,  gridBottom + halfGridLineWidth);
    gridData->addVertex(gridRight, gridBottom + halfGridLineWidth);
    gridData->addVertex(gridRight - halfGridLineWidth, gridBottom);
    gridData->addVertex(gridRight - halfGridLineWidth, gridTop);
    gridData->addVertex(gridRight, gridTop - halfGridLineWidth);
    gridData->addVertex(gridLeft,  gridTop - halfGridLineWidth);
    gridData->addVertex(gridLeft + halfGridLineWidth, gridTop);
    gridData->addVertex(gridLeft + halfGridLineWidth, gridBottom);
    glLineWidth(GRID_LINE_WIDTH);
    drawPrimitivePrivate(gridData.get());
    
    /*
     * Region inside the grid's box
     */
    const int32_t graphicsLeft   = static_cast<int32_t>(gridLeft   + std::ceil(GRID_LINE_WIDTH  + 1.0));
    const int32_t graphicsRight  = static_cast<int32_t>(gridRight  - std::floor(GRID_LINE_WIDTH + 1.0));
    const int32_t graphicsBottom = static_cast<int32_t>(gridBottom + std::ceil(GRID_LINE_WIDTH  + 1.0));
    const int32_t graphicsTop    = static_cast<int32_t>(gridTop    - std::floor(GRID_LINE_WIDTH + 1.0));
    
    const int32_t graphicsWidth = graphicsRight - graphicsLeft;
    const int32_t graphicsHeight = graphicsTop  - graphicsBottom;
    chartGraphicsDrawingViewportOut[0] = graphicsLeft;
    chartGraphicsDrawingViewportOut[1] = graphicsBottom;
    chartGraphicsDrawingViewportOut[2] = graphicsWidth;
    chartGraphicsDrawingViewportOut[3] = graphicsHeight;
}

/**
 * Draw the chart axes grid/box
 *
 * @param minimumDataValue
 *     Minimum data value
 * @param maximumDataValue
 *     Maximum data value
 * @param tabViewportX
 *     Viewport X for all chart content
 * @param tabViewportY
 *     Viewport Y for all chart content
 * @param tabViewportWidth
 *     Viewport width for all chart content
 * @param tabViewportHeight
 *     Viewport height for all chart content
 * @param margins
 *     Margin around graphics region.  The margin corresponding to the
 *     axis may be changed so that all text in the axis is visible
 *     (and not cut off).
 * @param axis
 *     Axis that is drawn.
 * @param chartAxisLabel
 *     Axis label.
 * @param axisMinimumOut
 *     Minimum value along the axis.
 * @param axisMaximumOut
 *     Maximum value along the axis.
 * @return
 *     True if the axis is valid and was drawn, else false.
 */
bool
BrainOpenGLChartTwoDrawingFixedPipeline::drawChartAxisCartesian(const float minimumDataValue,
                                                                const float maximumDataValue,
                                                                const float tabViewportX,
                                                                const float tabViewportY,
                                                                const float tabViewportWidth,
                                                                const float tabViewportHeight,
                                                                const Margins& margins,
                                                                ChartTwoCartesianAxis* axis,
                                                                AnnotationPercentSizeText* chartAxisLabel,
                                                                float& axisMinimumOut,
                                                                float& axisMaximumOut)
{
    if (axis == NULL) {
        return false;
    }
    if ( ! axis->isEnabledByChart()) {
        return false;
    }
    
    int32_t axisVpX      = 0;
    int32_t axisVpY      = 0;
    int32_t axisVpWidth  = 0;
    int32_t axisVpHeight = 0;
    float axisLength = 0.0;
    switch (axis->getAxisLocation()) {
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
            axisVpX      = tabViewportX + margins.m_left;
            axisVpY      = tabViewportY;
            axisVpWidth  = tabViewportWidth - (margins.m_left + margins.m_right);
            axisVpHeight = margins.m_bottom;
            axisLength   = axisVpWidth;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
            axisVpX      = tabViewportX + margins.m_left;
            axisVpY      = tabViewportY + tabViewportHeight - margins.m_top;
            axisVpWidth  = tabViewportWidth - (margins.m_left + margins.m_right);
            axisVpHeight = margins.m_top;
            axisLength   = axisVpWidth;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
            axisVpX      = tabViewportX;
            axisVpY      = tabViewportY + margins.m_bottom;
            axisVpWidth  = margins.m_left + 1; /* so ticks go to box */
            axisVpHeight = tabViewportHeight - (margins.m_bottom + margins.m_top);
            axisLength   = axisVpHeight;
            break;
        case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
            axisVpX      = tabViewportX + tabViewportWidth - margins.m_right;
            axisVpY      = tabViewportY + margins.m_bottom;
            axisVpWidth  = margins.m_right;
            axisVpHeight = tabViewportHeight - (margins.m_bottom + margins.m_top);
            axisLength   = axisVpHeight;
            break;
    }
    
    std::vector<float> scaleValueOffsetInPixels;
    std::vector<AString> scaleValuesText;
    const bool validAxisFlag = axis->getScaleValuesAndOffsets(minimumDataValue,
                                                              maximumDataValue,
                                                              axisLength,
                                                              axisMinimumOut,
                                                              axisMaximumOut,
                                                              scaleValueOffsetInPixels,
                                                              scaleValuesText);
    if ( ! validAxisFlag) {
        return false;
    }
    
    /*
     * If user has disabled display of axis no further action is needed
     * as the axisMinimumOut and axisMaximumOut have been calculated
     */
    if ( ! axis->isDisplayedByUser()) {
        /*
         * Even though axis is not displayed, declare the axis as valid
         * so that axisMinimumOut and axisMaximumOut are used by caller
         * of this method.  Otherwise, height of chart will 'jump' as
         * axis is turned on/off
         */
        return true;
    }
    
    const float rgba[4] = {
        m_fixedPipelineDrawing->m_foregroundColorFloat[0],
        m_fixedPipelineDrawing->m_foregroundColorFloat[1],
        m_fixedPipelineDrawing->m_foregroundColorFloat[2],
        1.0
    };
    
    const int32_t numScaleValuesToDraw = static_cast<int32_t>(scaleValuesText.size());
    
    /*
     * For scale text, make a copy of the axis label so font properties match
     */
    AnnotationPercentSizeText annotationText(AnnotationAttributesDefaultTypeEnum::NORMAL,
                                                     AnnotationTextFontSizeTypeEnum::PERCENTAGE_OF_VIEWPORT_HEIGHT);
    annotationText = *chartAxisLabel;
    annotationText.setCustomTextColor(rgba);
    annotationText.setTextColor(CaretColorEnum::CUSTOM);
    annotationText.setCoordinateSpace(AnnotationCoordinateSpaceEnum::TAB);
    annotationText.setTabIndex(m_tabIndex);
    
    if (numScaleValuesToDraw > 0) {
        const bool showTicksEnabledFlag = axis->isShowTickmarks();

        std::unique_ptr<GraphicsPrimitiveV3f> ticksData
        = std::unique_ptr<GraphicsPrimitiveV3f>(GraphicsPrimitive::newPrimitiveV3f(GraphicsPrimitive::PrimitiveType::LINES,
                                                                                   m_fixedPipelineDrawing->m_foregroundColorFloat));
        ticksData->reserveForNumberOfVertices(numScaleValuesToDraw * 2);
        
        const int32_t firstTickIndex = 0;
        const int32_t lastTickIndex = numScaleValuesToDraw - 1;
        
        for (int32_t i = 0; i < numScaleValuesToDraw; i++) {
            float tickStartX = 0.0;
            float tickStartY = 0.0;
            float tickEndX   = 0.0;
            float tickEndY   = 0.0;
            const float halfGridLineWidth = GRID_LINE_WIDTH / 2.0;
            
            switch (axis->getAxisLocation()) {
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                    tickStartX = margins.m_left + scaleValueOffsetInPixels[i] + halfGridLineWidth;
                    tickStartY = margins.m_bottom;
                    tickEndX   = tickStartX;
                    tickEndY   = tickStartY - s_tickLength;
                    annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                    if (i == firstTickIndex) {
                        annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::LEFT);
                    }
                    else if (i == lastTickIndex) {
                        annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::RIGHT);
                    }
                    annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                    tickStartX = margins.m_left + scaleValueOffsetInPixels[i] + halfGridLineWidth;
                    tickStartY = tabViewportHeight - margins.m_top;
                    tickEndX   = tickStartX;
                    tickEndY   = tickStartY + s_tickLength;
                    annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                    if (i == firstTickIndex) {
                        annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::LEFT);
                    }
                    else if (i == lastTickIndex) {
                        annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::RIGHT);
                    }
                    annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::BOTTOM);
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                    tickStartX = margins.m_left;
                    tickStartY = margins.m_bottom + scaleValueOffsetInPixels[i] + halfGridLineWidth;
                    tickEndX   = tickStartX - s_tickLength;
                    tickEndY   = tickStartY;
                    annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::MIDDLE);
                    if (i == firstTickIndex) {
                        annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::BOTTOM);
                    }
                    else if (i == lastTickIndex) {
                        annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                    }
                    annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::RIGHT);
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                    tickStartX = tabViewportWidth - margins.m_right;
                    tickStartY = margins.m_bottom + scaleValueOffsetInPixels[i] + halfGridLineWidth;
                    tickEndX   = tickStartX + s_tickLength;
                    tickEndY   = tickStartY;
                    annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::MIDDLE);
                    if (i == firstTickIndex) {
                        annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::BOTTOM);
                    }
                    else if (i == lastTickIndex) {
                        annotationText.setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                    }
                    annotationText.setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::LEFT);
                    break;
            }
            
            if (showTicksEnabledFlag) {
                if ((i != firstTickIndex)
                    && (i != lastTickIndex)) {
                    ticksData->addVertex(tickStartX, tickStartY);
                    ticksData->addVertex(tickEndX,   tickEndY);
                }
            }
            
            float textX = tickEndX;
            float textY = tickEndY;
            const float textOffset = 3.0;
            switch (axis->getAxisLocation()) {
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                    textY -= textOffset;
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                    textY += textOffset;
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                    textX -= textOffset;
                    break;
                case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                    textX += textOffset;
                    break;
            }
            annotationText.setText(scaleValuesText[i]);
            
            m_textRenderer->drawTextAtViewportCoords(textX,
                                                     textY,
                                                     0.0,
                                                     annotationText);
        }
        
        glLineWidth(GRID_LINE_WIDTH);
        drawPrimitivePrivate(ticksData.get());
        
        
        if (chartAxisLabel != NULL) {
            const AString axisTitle = chartAxisLabel->getText();
            if ( ! axisTitle.isEmpty()) {
                chartAxisLabel->setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                chartAxisLabel->setVerticalAlignment(AnnotationTextAlignVerticalEnum::MIDDLE);
                
                float labelTabX = axisVpWidth / 2.0;
                float labelTabY = axisVpHeight / 2.0;
                const float textMarginOffset = 5.0;
                switch (axis->getAxisLocation()) {
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_BOTTOM:
                        labelTabX = (axisVpWidth / 2.0) + margins.m_left;
                        labelTabY = textMarginOffset;
                        chartAxisLabel->setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                        chartAxisLabel->setVerticalAlignment(AnnotationTextAlignVerticalEnum::BOTTOM);
                        chartAxisLabel->setRotationAngle(0.0);
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_TOP:
                        labelTabX = (axisVpWidth / 2.0) + margins.m_left;
                        labelTabY = tabViewportHeight - textMarginOffset;
                        chartAxisLabel->setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                        chartAxisLabel->setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                        chartAxisLabel->setRotationAngle(0.0);
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_LEFT:
                        labelTabX = textMarginOffset;
                        labelTabY = (axisVpHeight / 2.0) + margins.m_bottom;
                        chartAxisLabel->setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                        chartAxisLabel->setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                        chartAxisLabel->setRotationAngle(-90.0);
                        break;
                    case ChartAxisLocationEnum::CHART_AXIS_LOCATION_RIGHT:
                        labelTabX = tabViewportWidth - textMarginOffset;
                        labelTabY = (axisVpHeight / 2.0) + margins.m_bottom;
                        chartAxisLabel->setVerticalAlignment(AnnotationTextAlignVerticalEnum::TOP);
                        chartAxisLabel->setHorizontalAlignment(AnnotationTextAlignHorizontalEnum::CENTER);
                        chartAxisLabel->setRotationAngle(90.0);
                        break;
                }
                
                if ((axisVpWidth > 0.0)
                    && (axisVpHeight > 0.0)) {
                    chartAxisLabel->setCustomTextColor(rgba);
                    chartAxisLabel->setTextColor(CaretColorEnum::CUSTOM);
                    chartAxisLabel->setTabIndex(m_tabIndex);
                    chartAxisLabel->setCoordinateSpace(AnnotationCoordinateSpaceEnum::TAB);
                    const float tabPercentageX = (labelTabX / tabViewportWidth)  * 100.0f;
                    const float tabPercentageY = (labelTabY / tabViewportHeight) * 100.0f;
                    chartAxisLabel->getCoordinate()->setXYZ(tabPercentageX,
                                                            tabPercentageY,
                                                            0.0f);
                    
                    m_viewportSpaceAnnotationsOut.push_back(chartAxisLabel);
                }
                
            }
        }
        
        return true;
    }
    
    return false;
}

/** 
 * Update the viewport content with charting viewport and matrices.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::updateViewportContentForCharting(const int32_t viewport[4])
{
    GLfloat modelviewArray[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelviewArray);
    GLfloat projectionArray[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projectionArray);
    Matrix4x4 modelviewMatrix;
    modelviewMatrix.setMatrixFromOpenGL(modelviewArray);
    Matrix4x4 projectionMatrix;
    projectionMatrix.setMatrixFromOpenGL(projectionArray);
    m_viewportContent->setChartDataMatricesAndViewport(projectionMatrix,
                                                       modelviewMatrix,
                                                       viewport);
}

/**
 * Draw the graphics primitive.
 *
 * @param primitive
 *     Primitive that will be drawn.
 */
void
BrainOpenGLChartTwoDrawingFixedPipeline::drawPrimitivePrivate(GraphicsPrimitive* primitive)
{
    if (primitive == NULL) {
        return;
    }
    
    GraphicsEngineDataOpenGL::draw(m_fixedPipelineDrawing->getContextSharingGroupPointer(),
                                   primitive);
}

/**
 * Constructor.
 *
 * @param histogramChart
 *    The file's histogram charting
 * @param mapIndex
 *    Index of the map for which histogram is displayed.
 * @param verticalAxisLocation
 *    Location of vertical axis for the histogram
 * @param allMapsSelected
 *    True if ALL MAPS selected for histogram, else false.
 */
BrainOpenGLChartTwoDrawingFixedPipeline::HistogramChartDrawingInfo::HistogramChartDrawingInfo(ChartableTwoFileHistogramChart* histogramChart,
                                                                                              int32_t mapIndex,
                                                                                              ChartAxisLocationEnum::Enum verticalAxisLocation,
                                                                                              const bool allMapsSelected)
:
m_histogramChart(histogramChart),
m_mapIndex(mapIndex),
m_verticalAxisLocation(verticalAxisLocation),
m_allMapsSelected(allMapsSelected)
{
}

/**
 * Destructor.
 */
BrainOpenGLChartTwoDrawingFixedPipeline::HistogramChartDrawingInfo::~HistogramChartDrawingInfo()
{
}
