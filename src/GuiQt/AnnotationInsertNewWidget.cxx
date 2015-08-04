
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

#define __ANNOTATION_INSERT_NEW_WIDGET_DECLARE__
#include "AnnotationInsertNewWidget.h"
#undef __ANNOTATION_INSERT_NEW_WIDGET_DECLARE__

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

#include "Annotation.h"
#include "AnnotationManager.h"
#include "CaretAssert.h"
#include "Brain.h"
#include "EventAnnotation.h"
#include "EventGraphicsUpdateAllWindows.h"
#include "EventUserInterfaceUpdate.h"
#include "EventManager.h"
#include "GuiManager.h"
#include "WuQMessageBox.h"
#include "WuQtUtilities.h"

using namespace caret;



/**
 * \class caret::AnnotationInsertNewWidget
 * \brief Widget for creating new annotations.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param browserWindowIndex
 *     Index of the browser window.
 * @param parent
 *     Parent of this widget.
 */
AnnotationInsertNewWidget::AnnotationInsertNewWidget(const int32_t browserWindowIndex,
                                                     QWidget* parent)
: QWidget(parent),
m_browserWindowIndex(browserWindowIndex)
{
    m_annotation = NULL;
    
    QLabel* insertLabel = new QLabel("Insert");
    QLabel* deleteLabel = new QLabel("Delete");
    QWidget* textToolButton = NULL; //createTextToolButton();
    QWidget* shapeToolButton = createShapeToolButton();
    m_deleteToolButton = createDeleteToolButton();
    
    WuQtUtilities::matchWidgetHeights(shapeToolButton,
                                      m_deleteToolButton,
                                      textToolButton);
    
    QGridLayout* gridLayout = new QGridLayout(this);
    WuQtUtilities::setLayoutSpacingAndMargins(gridLayout, 2, 2);
    gridLayout->addWidget(insertLabel,
                          0, 0, 1, 2, Qt::AlignHCenter);
    if (textToolButton != NULL) {
        gridLayout->addWidget(textToolButton,
                              1, 0);
        gridLayout->addWidget(shapeToolButton,
                              1, 1);
    }
    else {
        gridLayout->addWidget(shapeToolButton,
                              1, 0,
                              1, 2, Qt::AlignHCenter);
    }
    gridLayout->addWidget(deleteLabel,
                          0, 2, Qt::AlignHCenter);
    gridLayout->addWidget(m_deleteToolButton,
                          1, 2, Qt::AlignHCenter);
    
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
}

/**
 * Destructor.
 */
AnnotationInsertNewWidget::~AnnotationInsertNewWidget()
{
}

/**
 * Update with the given annotation.
 *
 * @param annotation.
 */
void
AnnotationInsertNewWidget::updateContent(Annotation* annotation)
{
    m_annotation = annotation;
    
    m_deleteToolButtonAction->setEnabled(m_annotation != NULL);
}

/**
 * @return Create the text tool button.
 */
QWidget*
AnnotationInsertNewWidget::createTextToolButton()
{
    QAction* textToolButtonAction;
    textToolButtonAction = WuQtUtilities::createAction("A",
                                                         "Create a text annotation",
                                                         this,
                                                         this,
                                                         SLOT(textActionTriggered()));
    QFont font = textToolButtonAction->font();
    font.setPixelSize(20);
    textToolButtonAction->setFont(font);
    
    QToolButton* toolButton = new QToolButton();
    toolButton->setDefaultAction(textToolButtonAction);
    return toolButton;
}

/**
 * @return Create the shape tool button.
 */
QWidget*
AnnotationInsertNewWidget::createShapeToolButton()
{
    QMenu* shapeMenu = new QMenu();
    QObject::connect(shapeMenu, SIGNAL(triggered(QAction*)),
                     this, SLOT(shapeMenuActionTriggered(QAction*)));
    
    std::vector<AnnotationTypeEnum::Enum> allTypes;
    AnnotationTypeEnum::getAllEnums(allTypes);
    
    QAction* boxAction = NULL;
    QAction* textAction = NULL;
    for (std::vector<AnnotationTypeEnum::Enum>::iterator iter = allTypes.begin();
         iter != allTypes.end();
         iter++) {
        const AnnotationTypeEnum::Enum annType = *iter;
        
        bool useTypeFlag = false;
        switch (annType) {
            case AnnotationTypeEnum::BOX:
                useTypeFlag = true;
                break;
            case AnnotationTypeEnum::IMAGE:
                break;
            case AnnotationTypeEnum::LINE:
                useTypeFlag = true;
                break;
            case AnnotationTypeEnum::OVAL:
                useTypeFlag = true;
                break;
            case AnnotationTypeEnum::TEXT:
                useTypeFlag = true;
                break;
        }
        
        if (useTypeFlag) {
            QAction* typeAction = shapeMenu->addAction(AnnotationTypeEnum::toGuiName(annType));
            typeAction->setIcon(createShapePixmap(shapeMenu,
                                                  annType));
            typeAction->setData((int)AnnotationTypeEnum::toIntegerCode(annType));
            
            if (annType == AnnotationTypeEnum::BOX) {
                boxAction = typeAction;
            }
            else if (annType == AnnotationTypeEnum::TEXT) {
                textAction = typeAction;
            }
        }
    }
    
    QAction* selectedAction = ((textAction != NULL)
                               ? textAction
                               : boxAction);
    
    m_shapeToolButtonAction = WuQtUtilities::createAction("Shape",
                                                          "Create the selected shape annotation\n"
                                                          "Click right side of button to change shape.",
                                                          this,
                                                          this,
                                                          SLOT(shapeActionTriggered()));
    m_shapeToolButtonAction->setData((int)-1);
    m_shapeToolButtonAction->setMenu(shapeMenu);
    
    QToolButton* toolButton = new QToolButton();
    toolButton->setDefaultAction(m_shapeToolButtonAction);

    /*
     * Initialize the shape tool button action to the BOX action.
     */
    if (selectedAction != NULL) {
        m_shapeToolButtonAction->blockSignals(true);
        m_shapeToolButtonAction->setIcon(selectedAction->icon());
        m_shapeToolButtonAction->setData(selectedAction->data());
        m_shapeToolButtonAction->setText("");
        m_shapeToolButtonAction->blockSignals(false);
    }
    
    return toolButton;
}

/**
 * @return The delete tool button.
 */
QToolButton*
AnnotationInsertNewWidget::createDeleteToolButton()
{
    m_deleteToolButtonAction = WuQtUtilities::createAction("",
                                                         ("Delete the selected annotation\n"
                                                          "\n"
                                                          "Pressing the Delete key while an annotation\n"
                                                          "is selected will also delete an annotation"),
                                                         this,
                                                         this,
                                                         SLOT(deleteActionTriggered()));
    QToolButton* toolButton = new QToolButton();

    const float width  = 24.0;
    const float height = 24.0;
    QPixmap pixmap(static_cast<int>(width),
                   static_cast<int>(height));
    QSharedPointer<QPainter> painter = WuQtUtilities::createPixmapWidgetPainter(toolButton,
                                                                                pixmap);
    
    
    /* trash can */
    painter->drawLine(4, 6, 4, 22);
    painter->drawLine(4, 22, 20, 22);
    painter->drawLine(20, 22, 20, 6);
    
    /* trash can lines */
    painter->drawLine(12, 8, 12, 20);
    painter->drawLine(8,  8,  8, 20);
    painter->drawLine(16, 8, 16, 20);
    
    /* trash can lid and handle */
    painter->drawLine(2, 6, 22, 6);
    painter->drawLine(8, 6, 8, 2);
    painter->drawLine(8, 2, 16, 2);
    painter->drawLine(16, 2, 16, 6);
    
    
    m_deleteToolButtonAction->setIcon(QIcon(pixmap));

    toolButton->setIconSize(pixmap.size());
    toolButton->setDefaultAction(m_deleteToolButtonAction);
    
    return toolButton;
}

/**
 * Gets called when the delete action is triggered.
 */
void
AnnotationInsertNewWidget::deleteActionTriggered()
{
    AnnotationManager* annotationManager = GuiManager::get()->getBrain()->getAnnotationManager();
    std::vector<Annotation*> selectedAnnotations = annotationManager->getSelectedAnnotations();
    if ( ! selectedAnnotations.empty()) {
        if (WuQMessageBox::warningOkCancel(this,
                                           "Delete selected annotation(s)?")) {
            /*
             * Delete all selected annotations and update graphics and UI.
             */
            AnnotationManager* annotationManager = GuiManager::get()->getBrain()->getAnnotationManager();
            annotationManager->deleteSelectedAnnotations();
            EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
            EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
        }
    }
//    if (m_annotation != NULL) {
//        if (WuQMessageBox::warningOkCancel(m_deleteToolButton, "Delete selected annotation(s)?")) {
//            /*
//             * Delete all selected annotations and update graphics and UI.
//             */
//            AnnotationManager* annotationManager = GuiManager::get()->getBrain()->getAnnotationManager();
//            annotationManager->deleteSelectedAnnotations();
//            EventManager::get()->sendEvent(EventGraphicsUpdateAllWindows().getPointer());
//            EventManager::get()->sendEvent(EventUserInterfaceUpdate().getPointer());
//        }
//    }
}


/**
 * Create an annotation with the given type.
 *
 * @param annotationType
 *    Type of annotation.
 */
void
AnnotationInsertNewWidget::createAnnotationWithType(const AnnotationTypeEnum::Enum annotationType)
{
    EventManager::get()->sendEvent(EventAnnotation().setModeCreateNewAnnotationType(annotationType).getPointer());
}

/**
 * Gets called when the text tool button's action is triggered.
 *
 * @param action
 *     The selected action.
 */
void
AnnotationInsertNewWidget::textActionTriggered()
{
    createAnnotationWithType(AnnotationTypeEnum::TEXT);
}

/**
 * Gets called when the shape tool button's action is triggered.
 *
 * @param action
 *     The selected action.
 */
void
AnnotationInsertNewWidget::shapeActionTriggered()
{
    const int32_t shapeID = m_shapeToolButtonAction->data().toInt();
    if (shapeID >= 0) {
        const AnnotationTypeEnum::Enum annType = AnnotationTypeEnum::fromIntegerCode(shapeID, NULL);
        createAnnotationWithType(annType);
    }
}

/**
 * Gets called when an item is selected from the shape tool
 * button's menu.
 *
 * @param action
 *     The selected action.
 */
void
AnnotationInsertNewWidget::shapeMenuActionTriggered(QAction* action)
{
    CaretAssert(action);
    
    const int32_t typeID = action->data().toInt();
    const AnnotationTypeEnum::Enum annType = AnnotationTypeEnum::fromIntegerCode(typeID, NULL);
    
    m_shapeToolButtonAction->setIcon(action->icon());
    m_shapeToolButtonAction->setData(action->data());
    m_shapeToolButtonAction->setText("");
    
    createAnnotationWithType(annType);
}

/**
 * Create a pixmap for the given annotation shape type.
 *
 * @param widget
 *    To color the pixmap with backround and foreground,
 *    the palette from the given widget is used.
 * @param annotationType
 *    The annotation type.
 * @return
 *    Pixmap with icon for the given annotation type.
 */
QPixmap
AnnotationInsertNewWidget::createShapePixmap(const QWidget* widget,
                                             const AnnotationTypeEnum::Enum annotationType)
{
    CaretAssert(widget);
    
    /*
     * Get the widget's background and foreground color
     */
    const QPalette palette = widget->palette();
    const QPalette::ColorRole backgroundRole = widget->backgroundRole();
    const QBrush backgroundBrush = palette.brush(backgroundRole);
    const QColor backgroundColor = backgroundBrush.color();
    const QPalette::ColorRole foregroundRole = widget->foregroundRole();
    const QBrush foregroundBrush = palette.brush(foregroundRole);
    const QColor foregroundColor = foregroundBrush.color();
    
    /*
     * Create a small, square pixmap that will contain
     * the foreground color around the pixmap's perimeter.
     */
    const float width  = 24.0;
    const float height = 24.0;
    QPixmap pixmap(static_cast<int>(width),
                   static_cast<int>(height));
    
    /*
     * Create a painter and fill the pixmap with
     * the background color
     */
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing,
                          true);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.fillRect(pixmap.rect(), backgroundColor);
    
    /*
     * Draw lines (rectangle) around the perimeter of
     * the pixmap
     */
    painter.setPen(foregroundColor);
    
    /**
     * NOTE: ORIGIN is in TOP LEFT corner of pixmap.
     */
    switch (annotationType) {
        case AnnotationTypeEnum::BOX:
            painter.drawRect(1, 1, width - 2, height - 2);
            break;
        case AnnotationTypeEnum::IMAGE:
            CaretAssertMessage(0, "IMAGE is not a shape.");
            break;
        case AnnotationTypeEnum::LINE:
            painter.drawLine(1, height - 1, width - 1, 1);
            break;
        case AnnotationTypeEnum::OVAL:
            painter.drawEllipse(1, 1, width - 1, height - 1);
            break;
        case AnnotationTypeEnum::TEXT:
        {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(pixmap.rect(),
                             (Qt::AlignCenter),
                             "A");
        }
            break;
    }
    
    return pixmap;
}

