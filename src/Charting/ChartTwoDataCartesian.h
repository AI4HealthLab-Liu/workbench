#ifndef __CHART_TWO_DATA_CARTESIAN_H__
#define __CHART_TWO_DATA_CARTESIAN_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
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

#include "CaretColorEnum.h"
#include "ChartAxisUnitsEnum.h"
#include "ChartTwoData.h"
#include "GraphicsPrimitive.h"


namespace caret {

    class ChartPoint;
    class GraphicsPrimitiveV3f;
    
    class ChartTwoDataCartesian : public ChartTwoData {
        
    public:
        ChartTwoDataCartesian(const ChartTwoDataTypeEnum::Enum chartDataType,
                              const ChartAxisUnitsEnum::Enum dataAxisUnitsX,
                              const ChartAxisUnitsEnum::Enum dataAxisUnitsY,
                              const GraphicsPrimitive::PrimitiveType graphicsPrimitiveType);
        
        virtual ~ChartTwoDataCartesian();
        
        virtual ChartTwoData* clone() const;
        
        void addPoint(const float x,
                      const float y);
        
        int32_t getNumberOfPoints() const;
        
        const ChartPoint* getPointAtIndex(const int32_t pointIndex) const;
        
        void getBounds(float& xMinimumOut,
                       float& xMaximumOut,
                       float& yMinimumOut,
                       float& yMaximumOut) const;
        
        GraphicsPrimitiveV3f* getGraphicsPrimitive() const;
        
        ChartAxisUnitsEnum::Enum getDataAxisUnitsX();
        
        ChartAxisUnitsEnum::Enum getDataAxisUnitsY();
        
        CaretColorEnum::Enum getColor() const;
        
        void setColor(const CaretColorEnum::Enum color);
        
        float getTimeStartInSecondsAxisX() const;
        
        void setTimeStartInSecondsAxisX(const float timeStart);
        
        float getTimeStepInSecondsAxisX() const;
        
        void setTimeStepInSecondsAxisX(const float timeStep);

        // ADD_NEW_METHODS_HERE

    protected:
        virtual void saveSubClassDataToScene(const SceneAttributes* sceneAttributes,
                                             SceneClass* sceneClass);
        
        virtual void restoreSubClassDataFromScene(const SceneAttributes* sceneAttributes,
                                                  const SceneClass* sceneClass);
        
        
    private:
        ChartTwoDataCartesian(const ChartTwoDataCartesian& obj);
        
        ChartTwoDataCartesian& operator=(const ChartTwoDataCartesian& obj);
        
        void copyHelperChartTwoDataCartesian(const ChartTwoDataCartesian& obj);

        void initializeMembersChartTwoDataCartesian();
        
        void removeAllPoints();
        
        std::unique_ptr<GraphicsPrimitiveV3f> createGraphicsPrimitive();
        
        std::vector<ChartPoint*> m_points;
        
        std::unique_ptr<GraphicsPrimitiveV3f> m_graphicsPrimitive;
        
        mutable float m_bounds[6];
        
        mutable bool m_boundsValid;
        
        ChartAxisUnitsEnum::Enum m_dataAxisUnitsX;
        
        ChartAxisUnitsEnum::Enum m_dataAxisUnitsY;
        
        const GraphicsPrimitive::PrimitiveType m_graphicsPrimitiveType;
        
        CaretColorEnum::Enum m_color;
        
        float m_timeStartInSecondsAxisX;
        
        float m_timeStepInSecondsAxisX;
        
        static int32_t caretColorIndex;
        
        SceneClassAssistant* m_sceneAssistant;
        

        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __CHART_TWO_DATA_CARTESIAN_DECLARE__
    int32_t ChartTwoDataCartesian::caretColorIndex = 0;
#endif // __CHART_TWO_DATA_CARTESIAN_DECLARE__

} // namespace
#endif  //__CHART_TWO_DATA_CARTESIAN_H__
