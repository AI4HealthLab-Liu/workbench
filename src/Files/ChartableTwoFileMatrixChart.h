#ifndef __CHARTABLE_TWO_FILE_MATRIX_CHART_H__
#define __CHARTABLE_TWO_FILE_MATRIX_CHART_H__

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

#include "ChartTwoMatrixContentTypeEnum.h"
#include "ChartTwoMatrixLoadingDimensionEnum.h"
#include "ChartableTwoFileBaseChart.h"

namespace caret {

    class ChartableTwoFileMatrixChart : public ChartableTwoFileBaseChart {
        
    public:
        ChartableTwoFileMatrixChart(const ChartTwoMatrixContentTypeEnum::Enum matrixContentType,
                                    CaretMappableDataFile* parentCaretMappableDataFile,
                                    std::vector<ChartTwoMatrixLoadingDimensionEnum::Enum>& validRowColumnSelectionDimensions);
        
        virtual ~ChartableTwoFileMatrixChart();
        
        ChartTwoMatrixContentTypeEnum::Enum getMatrixContentType() const;
        
        virtual bool isValid() const override;
        
        virtual bool isEmpty() const override;
        
        void getMatrixDimensions(int32_t& numberOfRowsOut,
                                 int32_t& numberOfColumnsOut) const;
        
        bool getMatrixDataRGBA(int32_t& numberOfRowsOut,
                               int32_t& numberOfColumnsOut,
                               std::vector<float>& rgbaOut) const;
        
        // ADD_NEW_METHODS_HERE

        ChartTwoMatrixLoadingDimensionEnum::Enum getSelectedRowColumnDimension() const;
        
        void setSelectedRowColumnDimension(const ChartTwoMatrixLoadingDimensionEnum::Enum rowColumnDimension);
        
        void getValidRowColumnSelectionDimensions(std::vector<ChartTwoMatrixLoadingDimensionEnum::Enum>& validRowColumnSelectionDimensionOut) const;

        void getSelectedRowColumnIndices(const int32_t tabIndex,
                                         ChartTwoMatrixLoadingDimensionEnum::Enum& rowColumnDimensionOut,
                                         std::vector<int32_t>& rowIndicesOut,
                                         std::vector<int32_t>& columnIndicesOut) const;
        
        void setSelectedRowColumnIndex(const int32_t tabIndex,
                                       const int32_t rowColumnIndex);
        
    protected:
        virtual void saveSubClassDataToScene(const SceneAttributes* sceneAttributes,
                                             SceneClass* sceneClass) override;

        virtual void restoreSubClassDataFromScene(const SceneAttributes* sceneAttributes,
                                                  const SceneClass* sceneClass) override;

    private:
        ChartableTwoFileMatrixChart(const ChartableTwoFileMatrixChart&);

        ChartableTwoFileMatrixChart& operator=(const ChartableTwoFileMatrixChart&);
        
        SceneClassAssistant* m_sceneAssistant;

        ChartTwoMatrixContentTypeEnum::Enum m_matrixContentType;
       
        const std::vector<ChartTwoMatrixLoadingDimensionEnum::Enum> m_validRowColumnSelectionDimensions;
        
        ChartTwoMatrixLoadingDimensionEnum::Enum m_rowColumnDimension;

        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __CHARTABLE_TWO_FILE_MATRIX_CHART_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __CHARTABLE_TWO_FILE_MATRIX_CHART_DECLARE__

} // namespace
#endif  //__CHARTABLE_TWO_FILE_MATRIX_CHART_H__
