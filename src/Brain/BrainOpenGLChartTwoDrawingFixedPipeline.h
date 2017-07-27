#ifndef __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_H__
#define __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_H__

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

#include <set>
#include "BrainOpenGLChartTwoDrawingInterface.h"
#include "CaretColorEnum.h"
#include "ChartAxisLocationEnum.h"
#include "ChartTwoDataTypeEnum.h"
#include "ChartTwoMatrixTriangularViewingModeEnum.h"

namespace caret {

    class Annotation;
    class AnnotationPercentSizeText;
    class BrowserTabContent;
    class CaretPreferences;
    class ChartTwoCartesianAxis;
    class ChartTwoDataCartesian;
    class ChartTwoOverlaySet;
    class ChartableTwoFileHistogramChart;
    class ChartableTwoFileLineSeriesChart;
    class ChartableTwoFileMatrixChart;
    class GraphicsPrimitive;
    class SelectionItemAnnotation;
    class SelectionItemChartTwoHistogram;
    class SelectionItemChartTwoLineSeries;
    class SelectionItemChartTwoMatrix;
    
    class BrainOpenGLChartTwoDrawingFixedPipeline : public BrainOpenGLChartTwoDrawingInterface {
        
    public:
        BrainOpenGLChartTwoDrawingFixedPipeline();
        
        virtual ~BrainOpenGLChartTwoDrawingFixedPipeline();
        
        virtual void drawChartOverlaySet(Brain* brain,
                                         BrowserTabContent* browserTabContent,
                                         ModelChartTwo* chartTwoModel,
                                         BrainOpenGLFixedPipeline* fixedPipelineDrawing,
                                         const SelectionItemDataTypeEnum::Enum selectionItemDataType,
                                         const int32_t viewport[4],
                                         std::vector<Annotation*>& viewportSpaceAnnotationsOut) override;

        // ADD_NEW_METHODS_HERE

    private:
        class HistogramChartDrawingInfo {
        public:
            HistogramChartDrawingInfo(ChartableTwoFileHistogramChart* histogramChart,
                                      int32_t mapIndex,
                                      ChartAxisLocationEnum::Enum verticalAxisLocation,
                                      const bool allMapsSelected);
            
            ChartableTwoFileHistogramChart* m_histogramChart;
            int32_t m_mapIndex;
            ChartAxisLocationEnum::Enum m_verticalAxisLocation;
            const bool m_allMapsSelected;
            
            ~HistogramChartDrawingInfo();
        };
        
        class LineSeriesChartDrawingInfo {
        public:
            LineSeriesChartDrawingInfo(const ChartableTwoFileLineSeriesChart* lineSeriesChart,
                                       const ChartTwoDataCartesian* chartTwoCartesianData,
                                       const ChartAxisLocationEnum::Enum verticalAxisLocation)
            : m_lineSeriesChart(lineSeriesChart),
            m_chartTwoCartesianData(chartTwoCartesianData),
            m_verticalAxisLocation(verticalAxisLocation) { }
            
            const ChartableTwoFileLineSeriesChart* m_lineSeriesChart;
            const ChartTwoDataCartesian* m_chartTwoCartesianData;
            const ChartAxisLocationEnum::Enum m_verticalAxisLocation;
        };
        
        class Margins {
        public:
            Margins(const double defaultSize) {
                m_bottom = defaultSize;
                m_left   = defaultSize;
                m_right  = defaultSize;
                m_top    = defaultSize;
            }
            
            double m_bottom;
            double m_left;
            double m_right;
            double m_top;
        };
        
        class RowColumnMinMax {
        public:
            RowColumnMinMax() { }

            int32_t m_min =  9999999;
            int32_t m_max = -9999999;
        };

        BrainOpenGLChartTwoDrawingFixedPipeline(const BrainOpenGLChartTwoDrawingFixedPipeline&);

        BrainOpenGLChartTwoDrawingFixedPipeline& operator=(const BrainOpenGLChartTwoDrawingFixedPipeline&);
        
        void restoreStateOfOpenGL();
        
        void saveStateOfOpenGL();
        
        void drawMatrixChart();
        
        void drawMatrixChartContent(const ChartableTwoFileMatrixChart* matrixChart,
                                 const ChartTwoMatrixTriangularViewingModeEnum::Enum chartViewingType,
                                 const float cellWidth,
                                 const float cellHeight,
                                 const float zooming);
        
        void drawHistogramOrLineSeriesChart(const ChartTwoDataTypeEnum::Enum chartDataType);
        
        void drawChartGraphicsBoxAndSetViewport(const float vpX,
                                                const float vpY,
                                                const float vpWidth,
                                                const float vpHeight,
                                                const Margins& margins,
                                                int32_t chartGraphicsDrawingViewportOut[4]);
        
        bool drawChartAxisCartesian(const float minimumDataValue,
                                    const float maximumDataValue,
                                    const float tabViewportX,
                                    const float tabViewportY,
                                    const float tabViewportWidth,
                                    const float tabViewportHeight,
                                    const Margins& margins,
                                    ChartTwoCartesianAxis* axis,
                                    AnnotationPercentSizeText* chartAxisLabel,
                                    float& axisMinimumOut,
                                    float& axisMaximumOut);
        
        void estimateCartesianChartAxisLegendsWidthHeight(const float minimumDataValue,
                                                          const float maximumDataValue,
                                                          const float viewportWidth,
                                                          const float viewportHeight,
                                                          ChartTwoCartesianAxis* cartesianAxis,
                                                          AnnotationPercentSizeText* chartAxisLabel,
                                                          double& widthOut,
                                                          double& heightOut);
        
        void drawPrimitivePrivate(GraphicsPrimitive* primitive);
        
        Brain* m_brain;
        
        BrowserTabContent* m_browserTabContent;
        
        ModelChartTwo* m_chartTwoModel;
        
        BrainOpenGLFixedPipeline* m_fixedPipelineDrawing;
        
        BrainOpenGLTextRenderInterface* m_textRenderer;
        
        ChartTwoOverlaySet* m_chartOverlaySet;
        
        SelectionItemDataTypeEnum::Enum m_selectionItemDataType;
        
        int32_t m_viewport[4];
        
        int32_t m_tabIndex;
        
        float m_translation[3];
        
        float m_zooming;
        
        CaretPreferences* m_preferences;
        
        SelectionItemAnnotation* m_selectionItemAnnotation;
        
        SelectionItemChartTwoHistogram* m_selectionItemHistogram;
        
        SelectionItemChartTwoLineSeries* m_selectionItemLineSeries;

        SelectionItemChartTwoMatrix* m_selectionItemMatrix;

        const float GRID_LINE_WIDTH = 2.0;
        
        bool m_identificationModeFlag;
        
        std::vector<Annotation*> m_viewportSpaceAnnotationsOut;
        
        static const int32_t IDENTIFICATION_INDICES_PER_HISTOGRAM      = 2;
        static const int32_t IDENTIFICATION_INDICES_PER_CHART_LINE     = 2;
        static const int32_t IDENTIFICATION_INDICES_PER_MATRIX_ELEMENT = 2;
        
        static constexpr float s_tickLength = 5.0;

        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__

} // namespace
#endif  //__BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_H__
