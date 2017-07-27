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
#include "ChartTwoMatrixTriangularViewingModeEnum.h"

namespace caret {

    class CaretPreferences;
    class ChartableTwoFileHistogramChart;
    class ChartableTwoFileLineSeriesChart;
    class ChartableTwoFileMatrixChart;
    class SelectionItemChartTwoHistogram;
    class SelectionItemChartTwoLineSeries;
    class SelectionItemChartTwoMatrix;
    
    class BrainOpenGLChartTwoDrawingFixedPipeline : public BrainOpenGLChartTwoDrawingInterface {
        
    public:
        BrainOpenGLChartTwoDrawingFixedPipeline();
        
        virtual ~BrainOpenGLChartTwoDrawingFixedPipeline();
        
        virtual void drawChartOverlaySet(Brain* brain,
                                         ModelChartTwo* chartTwoModel,
                                         BrainOpenGLFixedPipeline* fixedPipelineDrawing,
                                         BrainOpenGLTextRenderInterface* textRenderer,
                                         const float translation[3],
                                         const float zooming,
                                         ChartTwoOverlaySet* chartOverlaySet,
                                         const SelectionItemDataTypeEnum::Enum selectionItemDataType,
                                         const int32_t viewport[4],
                                         const int32_t tabIndex) override;

        // ADD_NEW_METHODS_HERE

    private:
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
        
        void resetIdentification();
        
        void processHistogramIdentification(const ChartableTwoFileHistogramChart* histogramChart);
        
        void processLineSeriesIdentification(const ChartableTwoFileLineSeriesChart* lineSeriesChart);
        
        void processMatrixIdentification(const ChartableTwoFileMatrixChart* matrixChart);
        
        void addToChartMatrixIdentification(const int32_t matrixRowIndex,
                                            const int32_t matrixColumnIndex,
                                            uint8_t rgbaForColorIdentification[4]);
        void drawMatrixChart();
        
        void drawMatrixChartContent(const ChartableTwoFileMatrixChart* matrixChart,
                                 const ChartTwoMatrixTriangularViewingModeEnum::Enum chartViewingType,
                                 const float cellWidth,
                                 const float cellHeight,
                                 const float zooming);
        
        void drawHistogramChart();
        
        void drawHistogramChartContent(const ChartableTwoFileHistogramChart* histogramChart);

//        void getMatrixHighlighting(const ChartableTwoFileMatrixChart* matrixChart,
//                                   std::set<int32_t>& rowIndicesOut,
//                                   std::set<int32_t>& columnIndicesOut) const;
        
        Brain* m_brain;
        
        ModelChartTwo* m_chartTwoModel;
        
        BrainOpenGLFixedPipeline* m_fixedPipelineDrawing;
        
        BrainOpenGLTextRenderInterface* m_textRenderer;
        
        ChartTwoOverlaySet* m_chartOverlaySet;
        
        SelectionItemDataTypeEnum::Enum m_selectionItemDataType;
        
        int32_t m_viewport[4];
        
        int32_t m_tabIndex;
        
        float m_translation[3];
        
        float m_zooming;
        
        std::vector<int32_t> m_identificationIndices;
        
        CaretPreferences* m_preferences;
        
        SelectionItemChartTwoHistogram* m_selectionItemHistogram;
        
        SelectionItemChartTwoLineSeries* m_selectionItemLineSeries;

        SelectionItemChartTwoMatrix* m_selectionItemMatrix;

        bool m_identificationModeFlag;
        
        static const int32_t IDENTIFICATION_INDICES_PER_CHART_LINE     = 2;
        static const int32_t IDENTIFICATION_INDICES_PER_MATRIX_ELEMENT = 2;
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_DECLARE__

} // namespace
#endif  //__BRAIN_OPEN_G_L_CHART_TWO_DRAWING_FIXED_PIPELINE_H__
