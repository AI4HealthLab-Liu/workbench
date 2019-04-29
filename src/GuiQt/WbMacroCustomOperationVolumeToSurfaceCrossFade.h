#ifndef __WB_MACRO_CUSTOM_OPERATION_VOLUME_TO_SURFACE_CROSS_FADE_H__
#define __WB_MACRO_CUSTOM_OPERATION_VOLUME_TO_SURFACE_CROSS_FADE_H__

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



#include <memory>

#include "WbMacroCustomOperationBase.h"

namespace caret {

    class Overlay;
    
    class WbMacroCustomOperationVolumeToSurfaceCrossFade : public WbMacroCustomOperationBase {
        
    public:
        WbMacroCustomOperationVolumeToSurfaceCrossFade();
        
        virtual ~WbMacroCustomOperationVolumeToSurfaceCrossFade();
        
        WbMacroCustomOperationVolumeToSurfaceCrossFade(const WbMacroCustomOperationVolumeToSurfaceCrossFade&) = delete;
        
        WbMacroCustomOperationVolumeToSurfaceCrossFade& operator=(const WbMacroCustomOperationVolumeToSurfaceCrossFade&) = delete;
        
        virtual bool executeCommand(QWidget* parent,
                                   const WuQMacroCommand* macroCommand) override;

        virtual WuQMacroCommand* createCommand() override;
        
        
        
        // ADD_NEW_METHODS_HERE

    private:
        bool performCrossFade(Overlay* volumeOverlay,
                              const float durationSeconds);
        
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __WB_MACRO_CUSTOM_OPERATION_VOLUME_TO_SURFACE_CROSS_FADE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __WB_MACRO_CUSTOM_OPERATION_OVERLAY_CROSS_FADE_DECLARE__

} // namespace
#endif  //__WB_MACRO_CUSTOM_OPERATION_VOLUME_TO_SURFACE_CROSS_FADE_H__
