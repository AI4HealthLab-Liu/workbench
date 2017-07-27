#ifndef __CHART_OVERLAY_H__
#define __CHART_OVERLAY_H__

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


#include "CaretObject.h"

#include "ChartTwoCompoundDataType.h"
#include "ChartTwoMatrixTriangularViewingModeEnum.h"
#include "EventListenerInterface.h"
#include "MapYokingGroupEnum.h"
#include "SceneableInterface.h"


namespace caret {
    class AnnotationColorBar;
    class CaretMappableDataFile;
    class ChartOverlaySet;
    class PlainTextStringBuilder;
    class SceneClassAssistant;

    class ChartOverlay : public CaretObject, public EventListenerInterface, public SceneableInterface {
        
    public:
        ChartOverlay(ChartOverlaySet* parentChartOverlaySet,
                     const ChartTwoDataTypeEnum::Enum chartDataType,
                     const int32_t overlayIndex);
        
        virtual ~ChartOverlay();
        
        ChartTwoDataTypeEnum::Enum getChartDataType() const;
        
        ChartTwoCompoundDataType getChartTwoCompoundDataType() const;
        
        void setChartTwoCompoundDataType(const ChartTwoCompoundDataType& chartCompoundDataType);
        
        AString getName() const;
        
        virtual AString toString() const;
        
        virtual void getDescriptionOfContent(PlainTextStringBuilder& descriptionOut) const;
        
        bool isEnabled() const;
        
        void setEnabled(const bool enabled);
        
        void copyData(const ChartOverlay* overlay);
        
        void swapData(ChartOverlay* overlay);
        
        bool isHistorySupported() const;
        
        bool isMapYokingSupported() const;
        
        MapYokingGroupEnum::Enum getMapYokingGroup() const;
        
        void setMapYokingGroup(const MapYokingGroupEnum::Enum mapYokingGroup);
        
        AnnotationColorBar* getColorBar();
        
        const AnnotationColorBar* getColorBar() const;
        
        ChartTwoMatrixTriangularViewingModeEnum::Enum getMatrixTriangularViewingMode() const;
        
        void setMatrixTriangularViewingMode(const ChartTwoMatrixTriangularViewingModeEnum::Enum mode);
        
        bool isMatrixTriangularViewingModeSupported() const;
        
        void getSelectionData(std::vector<CaretMappableDataFile*>& mapFilesOut,
                              CaretMappableDataFile* &selectedMapFileOut,
                              std::vector<AString>& selectedFileMapNamesOut,
                              int32_t& selectedMapIndexOut) const;
        
        void getSelectionData(std::vector<CaretMappableDataFile*>& mapFilesOut,
                              CaretMappableDataFile* &selectedMapFileOut,
                              int32_t& selectedMapIndexOut) const;
        
        void getSelectionData(CaretMappableDataFile* &selectedMapFileOut,
                              int32_t& selectedMapIndexOut) const;
        
        void setSelectionData(CaretMappableDataFile* selectedMapFile,
                              const int32_t selectedMapIndex);
        
        bool isAllMapsSupported() const;
        
        bool isAllMapsSelected() const;
        
        void setAllMapsSelected(const bool status);
        
        virtual void receiveEvent(Event* event);

        // ADD_NEW_METHODS_HERE
        
        virtual SceneClass* saveToScene(const SceneAttributes* sceneAttributes,
                                        const AString& instanceName);

        virtual void restoreFromScene(const SceneAttributes* sceneAttributes,
                                      const SceneClass* sceneClass);
        
// If there will be sub-classes of this class that need to save
// and restore data from scenes, these pure virtual methods can
// be uncommented to force their implementation by sub-classes.
//    protected: 
//        virtual void saveSubClassDataToScene(const SceneAttributes* sceneAttributes,
//                                             SceneClass* sceneClass) = 0;
//
//        virtual void restoreSubClassDataFromScene(const SceneAttributes* sceneAttributes,
//                                                  const SceneClass* sceneClass) = 0;

    private:
        ChartOverlay(const ChartOverlay&);

        ChartOverlay& operator=(const ChartOverlay&);
        
        void getSelectionDataPrivate(std::vector<CaretMappableDataFile*>& mapFilesOut,
                                     CaretMappableDataFile* &selectedMapFileOut,
                                     std::vector<AString>* selectedFileMapNamesOut,
                                     int32_t& selectedMapIndexOut) const;
        
        /** Parent chart overlay set (only used by first overlay in the set */
        ChartOverlaySet* m_parentChartOverlaySet;
        
        /** Enumerated Type of charts allowed in this overlay */
        const ChartTwoDataTypeEnum::Enum m_chartDataType;
        
        /** Index of this overlay (DO NOT COPY)*/
        const int32_t m_overlayIndex;
        
        std::unique_ptr<SceneClassAssistant> m_sceneAssistant;

        /** Current 'compound chart type' of charts allowed in this overlay */
        mutable ChartTwoCompoundDataType m_chartCompoundDataType;
        
        /** Name of overlay (DO NOT COPY)*/
        AString m_name;
        
        /** enabled status */
        mutable bool m_enabled = true;
        
        /** map yoking group */
        mutable MapYokingGroupEnum::Enum m_mapYokingGroup;

        /** The color bar displayed in the graphics window */
        std::unique_ptr<AnnotationColorBar> m_colorBar;
        
        /** selected mappable file */
        mutable CaretMappableDataFile* m_selectedMapFile = NULL;
        
        /** histogram selected map index */
        mutable int32_t m_selectedHistogramMapIndex = -1;
        
        bool m_allHistogramMapsSelectedFlag = false;
        
        ChartTwoMatrixTriangularViewingModeEnum::Enum m_matrixTriangularViewingMode;
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __CHART_OVERLAY_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __CHART_OVERLAY_DECLARE__

} // namespace
#endif  //__CHART_OVERLAY_H__
