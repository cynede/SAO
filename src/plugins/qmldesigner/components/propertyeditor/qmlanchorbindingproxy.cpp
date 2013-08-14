/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "qmlanchorbindingproxy.h"
#include "abstractview.h"
#include <qmlanchors.h>
#include <nodeabstractproperty.h>
#include <variantproperty.h>
#include <QDebug>

namespace QmlDesigner {

class ModelNode;
class NodeState;

const PropertyName auxDataString("anchors_");

static inline void backupPropertyAndRemove(ModelNode node, const PropertyName &propertyName)
{
    if (node.hasVariantProperty(propertyName)) {
        node.setAuxiliaryData(auxDataString + propertyName, node.variantProperty(propertyName).value());
        node.removeProperty(propertyName);

    }
    if (node.hasBindingProperty(propertyName)) {
        node.setAuxiliaryData(auxDataString + propertyName, QmlItemNode(node).instanceValue(propertyName));
        node.removeProperty(propertyName);
    }
}


static inline void restoreProperty(ModelNode node, const PropertyName &propertyName)
{
    if (node.hasAuxiliaryData(auxDataString + propertyName))
        node.variantProperty(propertyName).setValue(node.auxiliaryData(auxDataString + propertyName));
}

namespace Internal {

QmlAnchorBindingProxy::QmlAnchorBindingProxy(QObject *parent) :
        QObject(parent), m_locked(false)
{
}

QmlAnchorBindingProxy::~QmlAnchorBindingProxy()
{
}

void QmlAnchorBindingProxy::setup(const QmlItemNode &fxItemNode)
{
    m_qmlItemNode = fxItemNode;

    if (m_qmlItemNode.modelNode().hasParentProperty()) {
        setDefaultAnchorTarget(m_qmlItemNode.modelNode().parentProperty().parentModelNode());
    } else {
        setDefaultAnchorTarget(ModelNode());
    }

    if (topAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Top).qmlItemNode();
        if (targetNode.isValid())
            m_topTarget = targetNode;
    }

    if (bottomAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Bottom).qmlItemNode();
        if (targetNode.isValid())
            m_bottomTarget = targetNode;
    }

    if (leftAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Left).qmlItemNode();
        if (targetNode.isValid())
            m_leftTarget = targetNode;
    }

    if (rightAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Right).qmlItemNode();
        if (targetNode.isValid())
            m_rightTarget = targetNode;
    }

    if (verticalCentered()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::VerticalCenter).qmlItemNode();
        if (targetNode.isValid())
            m_verticalTarget = targetNode;
    }

    if (horizontalCentered()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::HorizontalCenter).qmlItemNode();
        if (targetNode.isValid())
            m_horizontalTarget = targetNode;
    }

    emit itemNodeChanged();
    emit parentChanged();
    emit topAnchorChanged();
    emit bottomAnchorChanged();
    emit leftAnchorChanged();
    emit rightAnchorChanged();
    emit centeredHChanged();
    emit centeredVChanged();
    emit anchorsChanged();

    if (m_qmlItemNode.hasNodeParent()) {
        emit itemNodeChanged();
        emit topTargetChanged();
        emit bottomTargetChanged();
        emit leftTargetChanged();
        emit rightTargetChanged();
        emit verticalTargetChanged();
        emit horizontalTargetChanged();
    }
}

void QmlAnchorBindingProxy::invalidate(const QmlItemNode &fxItemNode)
{
    if (m_locked)
        return;

    m_qmlItemNode = fxItemNode;

    m_verticalTarget = m_horizontalTarget = m_topTarget = m_bottomTarget = m_leftTarget = m_rightTarget = m_qmlItemNode.modelNode().parentProperty().parentModelNode();

    if (topAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Top).qmlItemNode();
        if (targetNode.isValid())
            m_topTarget = targetNode;
    }

    if (bottomAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Bottom).qmlItemNode();
        if (targetNode.isValid())
            m_bottomTarget = targetNode;
    }

    if (leftAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Left).qmlItemNode();
        if (targetNode.isValid())
            m_leftTarget = targetNode;
    }

    if (rightAnchored()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::Right).qmlItemNode();
        if (targetNode.isValid())
            m_rightTarget = targetNode;
    }

    if (verticalCentered()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::VerticalCenter).qmlItemNode();
        if (targetNode.isValid())
            m_verticalTarget = targetNode;
    }

    if (horizontalCentered()) {
        ModelNode targetNode = m_qmlItemNode.anchors().instanceAnchor(AnchorLine::HorizontalCenter).qmlItemNode();
        if (targetNode.isValid())
            m_horizontalTarget = targetNode;
    }

    emit topAnchorChanged();
    emit bottomAnchorChanged();
    emit leftAnchorChanged();
    emit rightAnchorChanged();
    emit centeredHChanged();
    emit centeredVChanged();
    emit anchorsChanged();

    if (m_qmlItemNode.hasNodeParent()) {
        emit itemNodeChanged();
        emit topTargetChanged();
        emit bottomTargetChanged();
        emit leftTargetChanged();
        emit rightTargetChanged();
        emit verticalTargetChanged();
        emit horizontalTargetChanged();
    }
}

bool QmlAnchorBindingProxy::hasParent()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.hasNodeParent();
}

bool QmlAnchorBindingProxy::isFilled()
{
    return m_qmlItemNode.isValid() && hasAnchors() && topAnchored() && bottomAnchored() && leftAnchored() && rightAnchored()
            && (m_qmlItemNode.instanceValue("anchors.topMargin").toInt() == 0)
            && (m_qmlItemNode.instanceValue("anchors.bottomMargin").toInt() == 0)
            && (m_qmlItemNode.instanceValue("anchors.leftMargin").toInt() == 0)
            && (m_qmlItemNode.instanceValue("anchors.rightMargin").toInt() == 0);
}

bool QmlAnchorBindingProxy::topAnchored()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::Top);
}

bool QmlAnchorBindingProxy::bottomAnchored()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::Bottom);
}

bool QmlAnchorBindingProxy::leftAnchored()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::Left);
}

bool QmlAnchorBindingProxy::rightAnchored()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::Right);
}

bool QmlAnchorBindingProxy::hasAnchors()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchors();
}


void QmlAnchorBindingProxy::setTopTarget(const QVariant &target)
{
    QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_topTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_topTarget = newTarget;
    calcTopMargin();

    emit topTargetChanged();
}


void QmlAnchorBindingProxy::setBottomTarget(const QVariant &target)
{
    QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_bottomTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_bottomTarget = newTarget;
    calcBottomMargin();

    emit bottomTargetChanged();
}

void QmlAnchorBindingProxy::setLeftTarget(const QVariant &target)
{
    QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_leftTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_leftTarget = newTarget;
    calcLeftMargin();

    emit leftTargetChanged();
}

void QmlAnchorBindingProxy::setRightTarget(const QVariant &target)
{
    QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_rightTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_rightTarget = newTarget;
    calcRightMargin();

    emit rightTargetChanged();
}

void QmlAnchorBindingProxy::setVerticalTarget(const QVariant &target)
{
     QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_verticalTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_verticalTarget = newTarget;
    m_qmlItemNode.anchors().setAnchor(AnchorLine::VerticalCenter, m_verticalTarget, AnchorLine::VerticalCenter);

    emit verticalTargetChanged();
}

void QmlAnchorBindingProxy::setHorizontalTarget(const QVariant &target)
{
    QmlItemNode newTarget(target.value<ModelNode>());

    if (newTarget == m_horizontalTarget)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_horizontalTarget = newTarget;
    m_qmlItemNode.anchors().setAnchor(AnchorLine::HorizontalCenter, m_horizontalTarget, AnchorLine::HorizontalCenter);

    emit horizontalTargetChanged();
}

void QmlAnchorBindingProxy::resetLayout() {
    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

        m_qmlItemNode.anchors().removeAnchors();
        m_qmlItemNode.anchors().removeMargins();

        restoreProperty(modelNode(), "x");
        restoreProperty(modelNode(), "y");
        restoreProperty(modelNode(), "width");
        restoreProperty(modelNode(), "height");

        emit topAnchorChanged();
        emit bottomAnchorChanged();
        emit leftAnchorChanged();
        emit rightAnchorChanged();
        emit anchorsChanged();
    }

void QmlAnchorBindingProxy::setBottomAnchor(bool anchor)
{
    if (!m_qmlItemNode.hasNodeParent())
        return;

    if (bottomAnchored() == anchor)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!anchor) {
        removeBottomAnchor();
    } else {
        calcBottomMargin();
        if (topAnchored())
            backupPropertyAndRemove(modelNode(), "height");
    }
    emit bottomAnchorChanged();

    if (hasAnchors() != anchor)
        emit anchorsChanged();
}

void QmlAnchorBindingProxy::setLeftAnchor(bool anchor)
{
    if (!m_qmlItemNode.hasNodeParent())
        return;

    if (leftAnchored() == anchor)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!anchor) {
        removeLeftAnchor();
    } else {
        calcLeftMargin();
        backupPropertyAndRemove(modelNode(), "x");
        if (rightAnchored())
            backupPropertyAndRemove(modelNode(), "width");
    }

    emit leftAnchorChanged();
    if (hasAnchors() != anchor)
        emit anchorsChanged();
}

void QmlAnchorBindingProxy::setRightAnchor(bool anchor)
{
    if (!m_qmlItemNode.hasNodeParent())
        return;

    if (rightAnchored() == anchor)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!anchor) {
        removeRightAnchor();
    } else {
        calcRightMargin();
        if (leftAnchored())
            backupPropertyAndRemove(modelNode(), "width");
    }
    emit rightAnchorChanged();
    if (hasAnchors() != anchor)
        emit anchorsChanged();
}
 QRectF QmlAnchorBindingProxy::parentBoundingBox()
{
    if (m_qmlItemNode.hasInstanceParent()) {
        if (m_qmlItemNode.instanceParentItem().instanceContentItemBoundingRect().isValid())
            return m_qmlItemNode.instanceParentItem().instanceContentItemBoundingRect();
        return m_qmlItemNode.instanceParentItem().instanceBoundingRect();
    }

    return QRect();
}

QRectF QmlAnchorBindingProxy::boundingBox(QmlItemNode node)
{
    if (node.isValid())
        return node.instanceTransform().mapRect(node.instanceBoundingRect());

    return QRect();
}

QRectF QmlAnchorBindingProxy::transformedBoundingBox()
{
    return m_qmlItemNode.instanceTransformWithContentTransform().mapRect(m_qmlItemNode.instanceBoundingRect());
}

void QmlAnchorBindingProxy::calcTopMargin()
{
    m_locked = true;

    if (m_topTarget.modelNode() == m_qmlItemNode.modelNode().parentProperty().parentModelNode()) {
        qreal topMargin = transformedBoundingBox().top() - parentBoundingBox().top();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Top, topMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Top, m_topTarget, AnchorLine::Top);
    } else {
        qreal topMargin = boundingBox(m_qmlItemNode).top() - boundingBox(m_topTarget).bottom();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Top, topMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Top, m_topTarget, AnchorLine::Bottom);
    }

    m_locked = false;
}

void QmlAnchorBindingProxy::calcBottomMargin()
{
    m_locked = true;

    if (m_bottomTarget.modelNode() == m_qmlItemNode.modelNode().parentProperty().parentModelNode()) {
        qreal bottomMargin =  parentBoundingBox().bottom() - transformedBoundingBox().bottom();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Bottom, bottomMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Bottom, m_bottomTarget, AnchorLine::Bottom);
    } else {
        qreal bottomMargin = boundingBox(m_bottomTarget).top()- boundingBox(m_qmlItemNode).bottom();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Bottom, bottomMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Bottom, m_bottomTarget, AnchorLine::Top);
    }

    m_locked = false;
}

void QmlAnchorBindingProxy::calcLeftMargin()
{
    m_locked = true;

    if (m_leftTarget.modelNode() == m_qmlItemNode.modelNode().parentProperty().parentModelNode()) {
        qreal leftMargin = transformedBoundingBox().left() - parentBoundingBox().left();
        m_qmlItemNode.anchors().setMargin(AnchorLine::Left, leftMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Left, m_leftTarget, AnchorLine::Left);
    } else {
        qreal leftMargin = boundingBox(m_qmlItemNode).left() - boundingBox(m_leftTarget).right();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Left, leftMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Left, m_leftTarget, AnchorLine::Right);
    }

    m_locked = false;
}

void QmlAnchorBindingProxy::calcRightMargin()
{
    m_locked = true;

    if (m_rightTarget.modelNode() == m_qmlItemNode.modelNode().parentProperty().parentModelNode()) {
        qreal rightMargin = parentBoundingBox().right() - transformedBoundingBox().right();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Right, rightMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Right, m_rightTarget, AnchorLine::Right);
    } else {
        qreal rightMargin = boundingBox(m_rightTarget).left() - boundingBox(m_qmlItemNode).right();
        m_qmlItemNode.anchors().setMargin( AnchorLine::Right, rightMargin);
        m_qmlItemNode.anchors().setAnchor(AnchorLine::Right, m_rightTarget, AnchorLine::Left);
    }

    m_locked = false;
}

ModelNode QmlAnchorBindingProxy::modelNode() const
{
    return m_qmlItemNode.modelNode();
}

void QmlAnchorBindingProxy::setTopAnchor(bool anchor)
{
    if (!m_qmlItemNode.hasNodeParent())
        return ;

    if (topAnchored() == anchor)
        return;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!anchor) {
        removeTopAnchor();
    } else {
        calcTopMargin();
        backupPropertyAndRemove(modelNode(), "y");
        if (bottomAnchored())
            backupPropertyAndRemove(modelNode(), "height");
    }
    emit topAnchorChanged();
    if (hasAnchors() != anchor)
        emit anchorsChanged();
}

void QmlAnchorBindingProxy::removeTopAnchor() {
    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_qmlItemNode.anchors().removeAnchor(AnchorLine::Top);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Top);

    restoreProperty(modelNode(), "y");
    restoreProperty(modelNode(), "height");

}

void QmlAnchorBindingProxy::removeBottomAnchor() {
    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_qmlItemNode.anchors().removeAnchor(AnchorLine::Bottom);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Bottom);


    restoreProperty(modelNode(), "height");
}

void QmlAnchorBindingProxy::removeLeftAnchor() {
    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_qmlItemNode.anchors().removeAnchor(AnchorLine::Left);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Left);

    restoreProperty(modelNode(), "x");
    restoreProperty(modelNode(), "width");
}

void QmlAnchorBindingProxy::removeRightAnchor() {
    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    m_qmlItemNode.anchors().removeAnchor(AnchorLine::Right);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Right);

    restoreProperty(modelNode(), "width");
}

void QmlAnchorBindingProxy::setVerticalCentered(bool centered)
{
    if (!m_qmlItemNode.hasNodeParent())
        return ;

    if (verticalCentered() == centered)
        return;

    m_locked = true;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!centered) {
        m_qmlItemNode.anchors().removeAnchor(AnchorLine::VerticalCenter);
        m_qmlItemNode.anchors().removeMargin(AnchorLine::VerticalCenter);
    } else {
        m_qmlItemNode.anchors().setAnchor(AnchorLine::VerticalCenter, m_qmlItemNode.modelNode().parentProperty().parentModelNode(), AnchorLine::VerticalCenter);
    }

    m_locked = false;

    emit centeredVChanged();
}

void QmlAnchorBindingProxy::setHorizontalCentered(bool centered)
{
    if (!m_qmlItemNode.hasNodeParent())
        return ;

    if (horizontalCentered() == centered)
        return;

    m_locked = true;

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();

    if (!centered) {
        m_qmlItemNode.anchors().removeAnchor(AnchorLine::HorizontalCenter);
        m_qmlItemNode.anchors().removeMargin(AnchorLine::HorizontalCenter);
    } else {
        m_qmlItemNode.anchors().setAnchor(AnchorLine::HorizontalCenter, m_qmlItemNode.modelNode().parentProperty().parentModelNode(), AnchorLine::HorizontalCenter);
    }

    m_locked = false;

    emit centeredHChanged();
}

bool QmlAnchorBindingProxy::verticalCentered()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::VerticalCenter);
}

bool QmlAnchorBindingProxy::horizontalCentered()
{
    return m_qmlItemNode.isValid() && m_qmlItemNode.anchors().instanceHasAnchor(AnchorLine::HorizontalCenter);
}

void QmlAnchorBindingProxy::fill()
{

    RewriterTransaction transaction = m_qmlItemNode.modelNode().view()->beginRewriterTransaction();


    backupPropertyAndRemove(modelNode(), "x");
    backupPropertyAndRemove(modelNode(), "y");
    backupPropertyAndRemove(modelNode(), "width");
    backupPropertyAndRemove(modelNode(), "height");

    m_qmlItemNode.anchors().fill();

    setHorizontalCentered(false);
    setVerticalCentered(false);

    m_qmlItemNode.anchors().removeMargin(AnchorLine::Right);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Left);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Top);
    m_qmlItemNode.anchors().removeMargin(AnchorLine::Bottom);

    emit topAnchorChanged();
    emit bottomAnchorChanged();
    emit leftAnchorChanged();
    emit rightAnchorChanged();
    emit anchorsChanged();
}

void QmlAnchorBindingProxy::setDefaultAnchorTarget(const ModelNode &modelNode)
{
    m_verticalTarget = modelNode;
    m_horizontalTarget = modelNode;
    m_topTarget = modelNode;
    m_bottomTarget = modelNode;
    m_leftTarget = modelNode;
    m_rightTarget = modelNode;
}

} // namespace Internal
} // namespace QmlDesigner

