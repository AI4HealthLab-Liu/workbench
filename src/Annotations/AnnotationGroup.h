#ifndef __ANNOTATION_GROUP_H__
#define __ANNOTATION_GROUP_H__

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

#include <QSharedPointer>

#include "AnnotationCoordinateSpaceEnum.h"
#include "AnnotationGroupTypeEnum.h"
#include "CaretObjectTracksModification.h"

#include "SceneableInterface.h"


namespace caret {
    class Annotation;
    class AnnotationFile;
    class SceneClassAssistant;

    class AnnotationGroup : public CaretObjectTracksModification, public SceneableInterface {
        
    public:
        AnnotationGroup(AnnotationFile* annotationFile,
                        const AnnotationGroupTypeEnum::Enum groupType,
                        const AnnotationCoordinateSpaceEnum::Enum coordinateSpace,
                        const int32_t tabOrWindowIndex);
        
        virtual ~AnnotationGroup();

        bool isEmpty() const;
        
        int32_t getUniqueKey() const;
        
        AString getName() const;
        
        AnnotationFile* getAnnotationFile() const;
        
        AnnotationGroupTypeEnum::Enum getGroupType() const;
        
        AnnotationCoordinateSpaceEnum::Enum getCoordinateSpace() const;
        
        int32_t getTabOrWindowIndex() const;
        
        int32_t getNumberOfAnnotations() const;
        
        Annotation* getAnnotation(const int32_t index);
        
        const Annotation* getAnnotation(const int32_t index) const;
        
        void getAllAnnotations(std::vector<Annotation*>& annotationsOut) const;

        bool isModified() const;
        
        void clearModified();
        
        // ADD_NEW_METHODS_HERE

        virtual AString toString() const;
        
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
        AnnotationGroup(const AnnotationGroup& obj);
        
        AnnotationGroup& operator=(const AnnotationGroup& obj);
        
        void copyHelperAnnotationGroup(const AnnotationGroup& obj);

        void setUniqueKey(const int32_t uniqueKey);
        
        void addAnnotationPrivate(Annotation* annotation);
        
        bool removeAnnotation(Annotation* annotation);
        
        void initializeInstance();
        
        SceneClassAssistant* m_sceneAssistant;

        AnnotationFile* m_annotationFile;
        
        AnnotationGroupTypeEnum::Enum m_groupType;
        
        AnnotationCoordinateSpaceEnum::Enum m_coordinateSpace;
        
        int32_t m_tabOrWindowIndex;
        
        AString m_name;
        
        int32_t m_uniqueKey;
        
        std::vector<QSharedPointer<Annotation> > m_annotations;
        
        typedef std::vector<QSharedPointer<Annotation> >::iterator AnnotationIterator;
        
        typedef std::vector<QSharedPointer<Annotation> >::const_iterator AnnotationConstIterator;
        
        // ADD_NEW_MEMBERS_HERE

        friend class AnnotationFile;
        friend class AnnotationFileXmlReader;
        friend class AnnotationFileXmlWriter;
    };
    
#ifdef __ANNOTATION_GROUP_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __ANNOTATION_GROUP_DECLARE__

} // namespace
#endif  //__ANNOTATION_GROUP_H__
