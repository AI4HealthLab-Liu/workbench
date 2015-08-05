#ifndef __QGLWIDGET_TEXT_RENDERER_H__
#define __QGLWIDGET_TEXT_RENDERER_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2015 Washington University School of Medicine
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

#include <map>
#include <set>

#include "BrainOpenGLTextRenderInterface.h"

class QFont;
class QGLWidget;

namespace caret {

    class QGLWidgetTextRenderer : public BrainOpenGLTextRenderInterface {
        
    public:
        QGLWidgetTextRenderer(QGLWidget* glWidget);
        
        virtual ~QGLWidgetTextRenderer();
        
        virtual void drawTextAtViewportCoords(const double viewportX,
                                              const double viewportY,
                                              const double viewportZ,
                                              const AnnotationText& annotationText);
        
        virtual void drawTextAtViewportCoords(const double viewportX,
                                              const double viewportY,
                                              const AnnotationText& annotationText);
        
        virtual void drawTextAtModelCoords(const double modelX,
                                           const double modelY,
                                           const double modelZ,
                                           const AnnotationText& annotationText);
        
        virtual void getTextWidthHeightInPixels(const AnnotationText& annotationText,
                                                double& widthOut,
                                                double& heightOut);
        
        virtual void getBoundsForTextAtViewportCoords(const AnnotationText& annotationText,
                                                      const double viewportX,
                                                      const double viewportY,
                                                      const double viewportZ,
                                                      double bottomLeftOut[3],
                                                      double bottomRightOut[3],
                                                      double topRightOut[3],
                                                      double topLeftOut[3]);
        virtual bool isValid() const;

        virtual AString getName() const;


        // ADD_NEW_METHODS_HERE

    private:
        class FontData {
        public:
            FontData();
            
            FontData(const AnnotationText&  annotationText);
            
            ~FontData();
            
            void initialize(const AString& fontFileName);
            
            QFont* m_font;
            
            bool m_valid;
        };
        
        struct CharInfo {
            CharInfo(const QString& theChar,
                     double x,
                     double y) : m_char(theChar), m_x(x), m_y(y) { }
            
             QString m_char;
             double m_x;
             double m_y;
        };
        
        QGLWidgetTextRenderer(const QGLWidgetTextRenderer&);

        QGLWidgetTextRenderer& operator=(const QGLWidgetTextRenderer&);
        
        QFont* findFont(const AnnotationText& annotationText,
                        const bool creatingDefaultFontFlag);
        
        void getVerticalTextCharInfo(const AnnotationText& annotationText,
                                     double& xMinOut,
                                     double& xMaxOut,
                                     double& heightOut,
                                     std::vector<CharInfo>& charInfoOut);
        
        void drawHorizontalTextAtWindowCoords(const double windowX,
                                              const double windowY,
                                              const AnnotationText& annotationText);
        
        void drawVerticalTextAtWindowCoords(const double windowX,
                                            const double windowY,
                                            const AnnotationText& annotationText);
        
        void applyForegroundColoring(const AnnotationText& annotationText);
        
        void applyBackgroundColoring(const AnnotationText& annotationText,
                                     const double textBoundsBox[4]);
        
        QGLWidget* m_glWidget;
        
        /**
         * The default font.  DO NOT delete it since it points to
         * a font in "m_fontNameToFontMap".
         */
        QFont* m_defaultFont;
        
        /**
         * Map for caching font
         */
        typedef std::map<AString, FontData*> FONT_MAP;
        
        /**
         * Iterator for cached fonts.
         */
        typedef FONT_MAP::iterator FONT_MAP_ITERATOR;
        
        /**
         * Caches fonts as they are created
         */
        FONT_MAP m_fontNameToFontMap;
        
        /**
         * Tracks fonts that failed creation to avoid
         * printing an error message more than once.
         */
        std::set<AString> m_failedFontNames;
        
        // ADD_NEW_MEMBERS_HERE

        static const double s_textMarginSize;
    };
    
#ifdef __QT_OPEN_G_L_TEXT_RENDERER_DECLARE__
    const double QGLWidgetTextRenderer::s_textMarginSize = 2.0;
#endif // __QT_OPEN_G_L_TEXT_RENDERER_DECLARE__

} // namespace
#endif  //__QT_OPEN_G_L_TEXT_RENDERER_H__