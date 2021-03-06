/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "formeditoritem.h"
#include "formeditorscene.h"

#include <bindingproperty.h>

#include <modelnode.h>
#include <nodehints.h>
#include <nodemetainfo.h>

#include <utils/theme/theme.h>

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimeLine>

#include <cmath>

namespace QmlDesigner {

FormEditorScene *FormEditorItem::scene() const {
    return qobject_cast<FormEditorScene*>(QGraphicsItem::scene());
}

FormEditorItem::FormEditorItem(const QmlItemNode &qmlItemNode, FormEditorScene* scene)
    : QGraphicsItem(scene->formLayerItem()),
    m_snappingLineCreator(this),
    m_qmlItemNode(qmlItemNode),
    m_borderWidth(1.0),
    m_highlightBoundingRect(false),
    m_blurContent(false),
    m_isContentVisible(true),
    m_isFormEditorVisible(true)
{
    setCacheMode(QGraphicsItem::NoCache);
    setup();
}

void FormEditorItem::setup()
{
    setAcceptedMouseButtons(Qt::NoButton);
    if (qmlItemNode().hasInstanceParent()) {
        setParentItem(scene()->itemForQmlItemNode(qmlItemNode().instanceParent().toQmlItemNode()));
        setOpacity(qmlItemNode().instanceValue("opacity").toDouble());
    }

    setFlag(QGraphicsItem::ItemClipsChildrenToShape, qmlItemNode().instanceValue("clip").toBool());

    if (NodeHints::fromModelNode(qmlItemNode()).forceClip())
        setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    if (QGraphicsItem::parentItem() == scene()->formLayerItem())
        m_borderWidth = 0.0;

    setContentVisible(qmlItemNode().instanceValue("visible").toBool());

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true);
    updateGeometry();
    updateVisibilty();
}

QRectF FormEditorItem::boundingRect() const
{
    return m_boundingRect.adjusted(-2, -2, 2, 2);
}

QPainterPath FormEditorItem::shape() const
{
    QPainterPath painterPath;
    painterPath.addRect(m_selectionBoundingRect);

    return painterPath;
}

bool FormEditorItem::contains(const QPointF &point) const
{
    return m_selectionBoundingRect.contains(point);
}

void FormEditorItem::updateGeometry()
{
    prepareGeometryChange();
    m_selectionBoundingRect = qmlItemNode().instanceBoundingRect().adjusted(0, 0, 1., 1.);
    m_paintedBoundingRect = qmlItemNode().instancePaintedBoundingRect();
    m_boundingRect = m_paintedBoundingRect.united(m_selectionBoundingRect);
    setTransform(qmlItemNode().instanceTransformWithContentTransform());
    //the property for zValue is called z in QGraphicsObject
    if (qmlItemNode().instanceValue("z").isValid() && !qmlItemNode().isRootModelNode())
        setZValue(qmlItemNode().instanceValue("z").toDouble());
}

void FormEditorItem::updateVisibilty()
{
}

FormEditorView *FormEditorItem::formEditorView() const
{
    return scene()->editorView();
}

void FormEditorItem::setHighlightBoundingRect(bool highlight)
{
    if (m_highlightBoundingRect != highlight) {
        m_highlightBoundingRect = highlight;
        update();
    }
}

void FormEditorItem::blurContent(bool blurContent)
{
    if (!scene())
        return;

    if (m_blurContent != blurContent) {
        m_blurContent = blurContent;
        update();
    }
}

void FormEditorItem::setContentVisible(bool visible)
{
    if (visible == m_isContentVisible)
        return;

    m_isContentVisible = visible;
    update();
}

bool FormEditorItem::isContentVisible() const
{
    if (parentItem())
        return parentItem()->isContentVisible() && m_isContentVisible;

    return m_isContentVisible;
}


bool FormEditorItem::isFormEditorVisible() const
{
    return m_isFormEditorVisible;
}
void FormEditorItem::setFormEditorVisible(bool isVisible)
{
    m_isFormEditorVisible = isVisible;
    setVisible(isVisible);
}

QPointF FormEditorItem::center() const
{
    return mapToScene(qmlItemNode().instanceBoundingRect().center());
}

qreal FormEditorItem::selectionWeigth(const QPointF &point, int iteration)
{
    if (!qmlItemNode().isValid())
        return 100000;

    QRectF boundingRect = mapRectToScene(qmlItemNode().instanceBoundingRect());

    float weight = point.x()- boundingRect.left()
            + point.y() - boundingRect.top()
            + boundingRect.right()- point.x()
            + boundingRect.bottom() - point.y()
            + (center() - point).manhattanLength()
            + sqrt(boundingRect.width() * boundingRect.height()) / 2 * iteration;

    return weight;
}

void FormEditorItem::synchronizeOtherProperty(const QByteArray &propertyName)
{
    if (propertyName == "opacity")
        setOpacity(qmlItemNode().instanceValue("opacity").toDouble());

    if (propertyName == "clip")
        setFlag(QGraphicsItem::ItemClipsChildrenToShape, qmlItemNode().instanceValue("clip").toBool());

    if (NodeHints::fromModelNode(qmlItemNode()).forceClip())
        setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    if (propertyName == "z")
        setZValue(qmlItemNode().instanceValue("z").toDouble());

    if (propertyName == "visible")
        setContentVisible(qmlItemNode().instanceValue("visible").toBool());
}

void FormEditorItem::setDataModelPosition(const QPointF &position)
{
    qmlItemNode().setPosition(position);
}

void FormEditorItem::setDataModelPositionInBaseState(const QPointF &position)
{
    qmlItemNode().setPostionInBaseState(position);
}

QPointF FormEditorItem::instancePosition() const
{
    return qmlItemNode().instancePosition();
}

QTransform FormEditorItem::instanceSceneTransform() const
{
    return qmlItemNode().instanceSceneTransform();
}

QTransform FormEditorItem::instanceSceneContentItemTransform() const
{
    return qmlItemNode().instanceSceneContentItemTransform();
}

bool FormEditorItem::flowHitTest(const QPointF & ) const
{
    return false;
}

FormEditorItem::~FormEditorItem()
{
    scene()->removeItemFromHash(this);
}

/* \brief returns the parent item skipping all proxyItem*/
FormEditorItem *FormEditorItem::parentItem() const
{
    return qgraphicsitem_cast<FormEditorItem*> (QGraphicsItem::parentItem());
}

FormEditorItem* FormEditorItem::fromQGraphicsItem(QGraphicsItem *graphicsItem)
{
    return qgraphicsitem_cast<FormEditorItem*>(graphicsItem);
}

void FormEditorItem::paintBoundingRect(QPainter *painter) const
{
    if (!m_boundingRect.isValid()
        || (QGraphicsItem::parentItem() == scene()->formLayerItem() && qFuzzyIsNull(m_borderWidth)))
          return;

     if (m_boundingRect.width() < 8 || m_boundingRect.height() < 8)
         return;

    QPen pen;
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::MiterJoin);

    const QColor frameColor(0xaa, 0xaa, 0xaa);
    static const QColor selectionColor = Utils::creatorTheme()->color(Utils::Theme::QmlDesigner_FormEditorSelectionColor);

    if (scene()->showBoundingRects()) {
        pen.setColor(frameColor.darker(150));
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(m_boundingRect.adjusted(0., 0., -1., -1.));

    }

    if (m_highlightBoundingRect) {
        pen.setColor(selectionColor);
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawRect(m_selectionBoundingRect.adjusted(0., 0., -1., -1.));
    }
}

static void paintTextInPlaceHolderForInvisbleItem(QPainter *painter,
                                                  const QString &id,
                                                  const QString &typeName,
                                                  const QRectF &boundingRect)
{
    QString displayText = id.isEmpty() ? typeName : id;

    QTextOption textOption;
    textOption.setAlignment(Qt::AlignTop);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    if (boundingRect.height() > 60) {
        QFont font;
        font.setStyleHint(QFont::SansSerif);
        font.setBold(true);
        font.setPixelSize(12);
        painter->setFont(font);

        QFontMetrics fm(font);
        painter->rotate(90);
        if (fm.horizontalAdvance(displayText) > (boundingRect.height() - 32) && displayText.length() > 4) {

            displayText = fm.elidedText(displayText, Qt::ElideRight, boundingRect.height() - 32, Qt::TextShowMnemonic);
        }

        QRectF rotatedBoundingBox;
        rotatedBoundingBox.setWidth(boundingRect.height());
        rotatedBoundingBox.setHeight(12);
        rotatedBoundingBox.setY(-boundingRect.width() + 12);
        rotatedBoundingBox.setX(20);

        painter->setFont(font);
        painter->setPen(QColor(48, 48, 96, 255));
        painter->setClipping(false);
        painter->drawText(rotatedBoundingBox, displayText, textOption);
    }
}

void paintDecorationInPlaceHolderForInvisbleItem(QPainter *painter, const QRectF &boundingRect)
{
    qreal stripesWidth = 8;

    QRegion innerRegion = QRegion(boundingRect.adjusted(stripesWidth, stripesWidth, -stripesWidth, -stripesWidth).toRect());
    QRegion outerRegion  = QRegion(boundingRect.toRect()) - innerRegion;

    painter->setClipRegion(outerRegion);
    painter->setClipping(true);
    painter->fillRect(boundingRect.adjusted(1, 1, -1, -1), Qt::BDiagPattern);
}

void FormEditorItem::paintPlaceHolderForInvisbleItem(QPainter *painter) const
{
    painter->save();
    paintDecorationInPlaceHolderForInvisbleItem(painter, m_boundingRect);
    paintTextInPlaceHolderForInvisbleItem(painter, qmlItemNode().id(), qmlItemNode().simplifiedTypeName(), m_boundingRect);
    painter->restore();
}

void FormEditorItem::paintComponentContentVisualisation(QPainter *painter, const QRectF &clippinRectangle) const
{
    painter->setBrush(QColor(0, 0, 0, 150));
    painter->fillRect(clippinRectangle, Qt::BDiagPattern);
}

QList<FormEditorItem *> FormEditorItem::offspringFormEditorItemsRecursive(const FormEditorItem *formEditorItem) const
{
    QList<FormEditorItem*> formEditorItemList;

    foreach (QGraphicsItem *item, formEditorItem->childItems()) {
        FormEditorItem *formEditorItem = fromQGraphicsItem(item);
        if (formEditorItem) {
            formEditorItemList.append(formEditorItem);
        }
    }

    return formEditorItemList;
}

void FormEditorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!painter->isActive())
        return;

    if (!qmlItemNode().isValid())
        return;

    painter->save();

    bool showPlaceHolder = qmlItemNode().instanceIsRenderPixmapNull() || !isContentVisible();

    const bool isInStackedContainer = qmlItemNode().isInStackedContainer();

    /* If already the parent is invisible then show nothing */
    const bool hideCompletely = !isContentVisible() && (parentItem() && !parentItem()->isContentVisible());

    if (isInStackedContainer)
        showPlaceHolder = qmlItemNode().instanceIsRenderPixmapNull() && isContentVisible();

    QRegion clipRegion = painter->clipRegion();
    if (clipRegion.contains(m_selectionBoundingRect.toRect().topLeft())
            && clipRegion.contains(m_selectionBoundingRect.toRect().bottomRight()))
        painter->setClipRegion(boundingRect().toRect());
    painter->setClipping(true);

    if (!hideCompletely) {
        if (showPlaceHolder) {
            if (scene()->showBoundingRects() && m_boundingRect.width() > 15 && m_boundingRect.height() > 15)
                paintPlaceHolderForInvisbleItem(painter);
        } else if (!isInStackedContainer || isContentVisible() ) {
            painter->save();
            const QTransform &painterTransform = painter->transform();
            if (painterTransform.m11() < 1.0 // horizontally scaled down?
                    || painterTransform.m22() < 1.0 // vertically scaled down?
                    || painterTransform.isRotating())
                painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

            if (m_blurContent)
                painter->drawPixmap(m_paintedBoundingRect.topLeft(), qmlItemNode().instanceBlurredRenderPixmap());
            else
                painter->drawPixmap(m_paintedBoundingRect.topLeft(), qmlItemNode().instanceRenderPixmap());

            painter->restore();
        }
    }

    if (!qmlItemNode().isRootModelNode())
        paintBoundingRect(painter);

    painter->restore();
}

AbstractFormEditorTool* FormEditorItem::tool() const
{
    return static_cast<FormEditorScene*>(scene())->currentTool();
}

SnapLineMap FormEditorItem::topSnappingLines() const
{
    return m_snappingLineCreator.topLines();
}

SnapLineMap FormEditorItem::bottomSnappingLines() const
{
    return m_snappingLineCreator.bottomLines();
}

SnapLineMap FormEditorItem::leftSnappingLines() const
{
    return m_snappingLineCreator.leftLines();
}

SnapLineMap FormEditorItem::rightSnappingLines() const
{
    return m_snappingLineCreator.rightLines();
}

SnapLineMap FormEditorItem::horizontalCenterSnappingLines() const
{
    return m_snappingLineCreator.horizontalCenterLines();
}

SnapLineMap FormEditorItem::verticalCenterSnappingLines() const
{
    return m_snappingLineCreator.verticalCenterLines();
}

SnapLineMap FormEditorItem::topSnappingOffsets() const
{
    return m_snappingLineCreator.topOffsets();
}

SnapLineMap FormEditorItem::bottomSnappingOffsets() const
{
    return m_snappingLineCreator.bottomOffsets();
}

SnapLineMap FormEditorItem::leftSnappingOffsets() const
{
    return m_snappingLineCreator.leftOffsets();
}

SnapLineMap FormEditorItem::rightSnappingOffsets() const
{
    return m_snappingLineCreator.rightOffsets();
}


void FormEditorItem::updateSnappingLines(const QList<FormEditorItem*> &exceptionList,
                                         FormEditorItem *transformationSpaceItem)
{
    m_snappingLineCreator.update(exceptionList, transformationSpaceItem, this);
}

QList<FormEditorItem*> FormEditorItem::childFormEditorItems() const
{
    QList<FormEditorItem*> formEditorItemList;

    foreach (QGraphicsItem *item, childItems()) {
        FormEditorItem *formEditorItem = fromQGraphicsItem(item);
        if (formEditorItem)
            formEditorItemList.append(formEditorItem);
    }

    return formEditorItemList;
}

QList<FormEditorItem *> FormEditorItem::offspringFormEditorItems() const
{
    return offspringFormEditorItemsRecursive(this);
}

bool FormEditorItem::isContainer() const
{
    NodeMetaInfo nodeMetaInfo = qmlItemNode().modelNode().metaInfo();

    if (nodeMetaInfo.isValid())
        return !nodeMetaInfo.defaultPropertyIsComponent() && !nodeMetaInfo.isLayoutable();

    return true;
}

QmlItemNode FormEditorItem::qmlItemNode() const
{
    return m_qmlItemNode;
}

void FormEditorFlowItem::synchronizeOtherProperty(const QByteArray &)
{
    setContentVisible(true);
}

void FormEditorFlowItem::setDataModelPosition(const QPointF &position)
{
    qmlItemNode().setFlowItemPosition(position);
    updateGeometry();
    for (QGraphicsItem *item : scene()->items()) {
        if (item == this)
            continue;

        auto formEditorItem = qgraphicsitem_cast<FormEditorItem*>(item);
        if (formEditorItem)
            formEditorItem->updateGeometry();
    }

}

void FormEditorFlowItem::setDataModelPositionInBaseState(const QPointF &position)
{
    setDataModelPosition(position);
}

void FormEditorFlowItem::updateGeometry()
{
    FormEditorItem::updateGeometry();
    const QPointF pos = qmlItemNode().flowPosition();
    setTransform(QTransform::fromTranslate(pos.x(), pos.y()));

    QmlFlowItemNode flowItem(qmlItemNode());

    if (flowItem.isValid() && flowItem.flowView().isValid()) {
        const auto nodes = flowItem.flowView().transitions();
        for (const ModelNode &node : nodes) {
            FormEditorItem *item = scene()->itemForQmlItemNode(node);
            if (item)
                item->updateGeometry();
        }
    }

}

QPointF FormEditorFlowItem::instancePosition() const
{
    return qmlItemNode().flowPosition();
}

void FormEditorFlowActionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!painter->isActive())
        return;

    if (!qmlItemNode().isValid())
        return;

    painter->save();

    QPen pen;
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::MiterJoin);

    QColor flowColor = "#e71919";

    if (qmlItemNode().modelNode().hasAuxiliaryData("color"))
        flowColor = qmlItemNode().modelNode().auxiliaryData("color").value<QColor>();

    int width = 4;


    if (qmlItemNode().modelNode().hasAuxiliaryData("width"))
        width = qmlItemNode().modelNode().auxiliaryData("width").toInt();

    bool dash = false;


    if (qmlItemNode().modelNode().hasAuxiliaryData("dash"))
        dash = qmlItemNode().modelNode().auxiliaryData("dash").toBool();


    pen.setColor(flowColor);
    if (dash)
        pen.setStyle(Qt::DashLine);
    else
        pen.setStyle(Qt::SolidLine);

    pen.setWidth(width);
    pen.setCosmetic(true);
    painter->setPen(pen);

    if (qmlItemNode().modelNode().hasAuxiliaryData("fillColor")) {

       const QColor fillColor = qmlItemNode().modelNode().auxiliaryData("fillColor").value<QColor>();
       painter->fillRect(boundingRect(), fillColor);
    }

    painter->drawRect(boundingRect());


    painter->restore();
}

QTransform FormEditorFlowActionItem::instanceSceneTransform() const
{
    return sceneTransform();
}

QTransform FormEditorFlowActionItem::instanceSceneContentItemTransform() const
{
    return sceneTransform();
}

void FormEditorTransitionItem::synchronizeOtherProperty(const QByteArray &)
{
    setContentVisible(true);
}

void FormEditorTransitionItem::setDataModelPosition(const QPointF &)
{

}

void FormEditorTransitionItem::setDataModelPositionInBaseState(const QPointF &)
{

}

void FormEditorTransitionItem::updateGeometry()
{
    FormEditorItem::updateGeometry();

    const ModelNode from = qmlItemNode().modelNode().bindingProperty("from").resolveToModelNode();
    const ModelNode to = qmlItemNode().modelNode().bindingProperty("to").resolveToModelNode();

    QPointF fromP = QmlItemNode(from).flowPosition();
    QRectF sizeTo = QmlItemNode(to).instanceBoundingRect();

    QPointF toP = QmlItemNode(to).flowPosition();

    qreal x1 = fromP.x();
    qreal x2 = toP.x();

    if (x2 < x1) {
        qreal s = x1;
        x1 = x2;
        x2 = s;
    }

    qreal y1 = fromP.y();
    qreal y2 = toP.y();

    if (y2 < y1) {
        qreal s = y1;
        y1 = y2;
        y2 = s;
    }

    x2 += sizeTo.width();
    y2 += sizeTo.height();

    setX(x1);
    setY(y1);
    m_selectionBoundingRect = QRectF(0,0,x2-x1,y2-y1);
    m_paintedBoundingRect = m_selectionBoundingRect;
    m_boundingRect = m_selectionBoundingRect;
    setZValue(10);
}

QPointF FormEditorTransitionItem::instancePosition() const
{
    return qmlItemNode().flowPosition();
}

static bool verticalOverlap(const QRectF &from, const QRectF &to)
{
    if (from.top()  < to.bottom() && (from.top() + from.height()) > to.top())
        return true;

    if (to.top()  < from.bottom() && (to.top() + to.height()) > from.top())
        return true;

    return false;
}


static bool horizontalOverlap(const QRectF &from, const QRectF &to)
{
    if (from.left()  < to.right() && (from.left() + from.width()) > to.left())
        return true;

    if (to.left()  < from.right() && (to.left() + to.width()) > from.left())
        return true;

    return false;
}

static void paintConnection(QPainter *painter,
                            const QRectF &from,
                            const QRectF &to,
                            int width,
                            const QColor &color,
                            bool dash,
                            int startOffset,
                            int endOffset,
                            int breakOffset)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen;
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);

    pen.setColor(color);

    if (dash)
        pen.setStyle(Qt::DashLine);
    else
        pen.setStyle(Qt::SolidLine);
    pen.setWidth(width);
    painter->setPen(pen);

    //const bool forceVertical = false;
    //const bool forceHorizontal = false;

    const int padding = 16;

    const int arrowLength = 8;
    const int arrowWidth = 16;

    const bool boolExitRight = from.right() < to.center().x();
    const bool boolExitBottom = from.bottom() < to.center().y();

    bool horizontalFirst = horizontalOverlap(from, to) && !verticalOverlap(from, to);

    const qreal middleFactor = breakOffset / 100.0;

    QPointF startP;

    bool extraLine = false;

    if (horizontalFirst) {
        if (to.center().x() > from.left() && to.center().x() < from.right()) {
                horizontalFirst = false;
                extraLine = true;
            }

    } else {
        if (to.center().y() > from.top() && to.center().y() < from.bottom()) {
            horizontalFirst = true;
            extraLine = true;
        }
    }

    if (horizontalFirst) {
        const qreal startY = from.center().y() + startOffset;
        qreal startX = from.x() - padding;
        if (boolExitRight)
            startX = from.right() + padding;

        startP = QPointF(startX, startY);

        qreal endY = to.top() - padding;

        if (from.bottom() > to.y())
            endY = to.bottom() + padding;

        if (!extraLine) {


            const qreal endX = to.center().x() + endOffset;

            const QPointF midP(endX, startY);

            const QPointF endP(endX, endY);

            painter->drawLine(startP, midP);
            painter->drawLine(midP, endP);

            int flip = 1;

            if (midP.y() < endP.y())
                flip = -1;

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawLine(endP + flip * QPoint(arrowWidth / 2, arrowLength), endP);
            painter->drawLine(endP + flip * QPoint(-arrowWidth / 2, arrowLength), endP);
        } else {

            qreal endX = to.left() - padding;

            if (from.right() > to.x())
                endX = to.right() + padding;

            const qreal midX = startX * middleFactor + endX * (1-middleFactor);
            const QPointF midP(midX, startY);
            const QPointF midP2(midX, to.center().y() + endOffset);
            const QPointF endP(endX, to.center().y() + endOffset);
            painter->drawLine(startP, midP);
            painter->drawLine(midP, midP2);
            painter->drawLine(midP2, endP);

            int flip = 1;

            if (midP2.x() < endP.x())
                flip = -1;

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawLine(endP + flip * QPoint(arrowWidth / 2, arrowWidth / 2), endP);
            painter->drawLine(endP + flip * QPoint(arrowLength, -arrowWidth / 2), endP);
        }

    } else {
        const qreal startX = from.center().x() + startOffset;

        qreal startY = from.top() - padding;
        if (boolExitBottom)
            startY = from.bottom() + padding;

        startP = QPointF(startX, startY);
        qreal endX = to.left() - padding;

        if (from.right() > to.x())
            endX = to.right() + padding;

        if (!extraLine) {
            const qreal endY = to.center().y() + endOffset;

            const QPointF midP(startX, endY);

            const QPointF endP(endX, endY);

            painter->drawLine(startP, midP);
            painter->drawLine(midP, endP);

            int flip = 1;

            if (midP.x() < endP.x())
                flip = -1;

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawLine(endP + flip * QPoint(arrowWidth / 2, arrowWidth / 2), endP);
            painter->drawLine(endP + flip * QPoint(arrowLength, -arrowWidth / 2), endP);
        } else {

            qreal endY = to.top() - padding;

            if (from.bottom() > to.y())
                endY = to.bottom() + padding;

            const qreal midY = startY * middleFactor + endY * (1-middleFactor);
            const QPointF midP(startX, midY);
            const QPointF midP2(to.center().x() + endOffset, midY);
            const QPointF endP(to.center().x() + endOffset, endY);

            painter->drawLine(startP, midP);
            painter->drawLine(midP, midP2);
            painter->drawLine(midP2, endP);

            int flip = 1;

            if (midP2.y() < endP.y())
                flip = -1;

            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawLine(endP + flip * QPoint(arrowWidth / 2, arrowLength), endP);
            painter->drawLine(endP + flip * QPoint(-arrowWidth / 2, arrowLength), endP);
        }
    }

    pen.setWidth(4);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    painter->setBrush(Qt::white);
    painter->drawEllipse(startP, arrowLength - 2, arrowLength - 2);

    painter->restore();
}

void FormEditorTransitionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!painter->isActive())
        return;

    if (!qmlItemNode().modelNode().isValid())
        return;

    if (!(qmlItemNode().modelNode().hasBindingProperty("from")
          && qmlItemNode().modelNode().hasBindingProperty("from")))
        return;

    painter->save();

    const QmlFlowItemNode from = qmlItemNode().modelNode().bindingProperty("from").resolveToModelNode();
    const QmlFlowItemNode to = qmlItemNode().modelNode().bindingProperty("to").resolveToModelNode();

    QmlFlowActionAreaNode areaNode = ModelNode();

    if (from.isValid() && to.isValid())
        for (const QmlFlowActionAreaNode &area : from.flowActionAreas()) {
            if (area.targetTransition() ==  qmlItemNode().modelNode())
                areaNode = area;
        }

    QRectF fromRect = QmlItemNode(from).instanceBoundingRect();
    fromRect.translate(QmlItemNode(from).flowPosition());


    if (areaNode.isValid()) {
        fromRect = QmlItemNode(areaNode).instanceBoundingRect();
        fromRect.translate(QmlItemNode(from).flowPosition());
        fromRect.translate(areaNode.instancePosition());
    }

    QRectF toRect = QmlItemNode(to).instanceBoundingRect();
    toRect.translate(QmlItemNode(to).flowPosition());

    toRect.translate(-pos());
    fromRect.translate(-pos());


    int width = 4;

    if (qmlItemNode().modelNode().hasAuxiliaryData("width"))
        width = qmlItemNode().modelNode().auxiliaryData("width").toInt();

    if (qmlItemNode().modelNode().isSelected())
        width += 2;
    if (m_hitTest)
        width += 4;

    QColor color = "#e71919";

    bool dash = false;

    if (qmlItemNode().modelNode().hasAuxiliaryData("color"))
        color = qmlItemNode().modelNode().auxiliaryData("color").value<QColor>();

    if (qmlItemNode().modelNode().hasAuxiliaryData("dash"))
        dash = qmlItemNode().modelNode().auxiliaryData("dash").toBool();

    int outOffset = 0;
    int inOffset = 0;

    if (qmlItemNode().modelNode().hasAuxiliaryData("outOffset"))
        outOffset = qmlItemNode().modelNode().auxiliaryData("outOffset").toInt();

    if (qmlItemNode().modelNode().hasAuxiliaryData("inOffset"))
        inOffset = qmlItemNode().modelNode().auxiliaryData("inOffset").toInt();

    int breakOffset = 50;

    if (qmlItemNode().modelNode().hasAuxiliaryData("break"))
        breakOffset = qmlItemNode().modelNode().auxiliaryData("break").toInt();

    paintConnection(painter, fromRect, toRect, width ,color, dash, outOffset, inOffset, breakOffset);

    painter->restore();
}

bool FormEditorTransitionItem::flowHitTest(const QPointF &point) const
{
    QImage image(boundingRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(QColor("black"));

    QPainter p(&image);

    m_hitTest = true;
    const_cast<FormEditorTransitionItem *>(this)->paint(&p, nullptr, nullptr);
    m_hitTest = false;

    QPoint pos = mapFromScene(point).toPoint();
    return image.pixelColor(pos).value() > 0;
}

}
