/*
 * Copyright (C) 2015-2016 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBGraphicsStroke.h"
#include "UBGraphicsScene.h"

#include "core/UBApplication.h"
#include "frameworks/UBGeometryUtils.h"

#include <QGraphicsOpacityEffect>

/**
 * @brief Constructs a new UBGraphicsStroke
 * @param smooth If true, the stroke will be interpolated as it is drawn, to have a smoother appearance.
 * @param parent Passed to the QGraphicsItem constructor.
 */
UBGraphicsStroke::UBGraphicsStroke(bool smooth, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , mSmoothDrawing(smooth)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                           | GF_RESPECT_RATIO
                                           | GF_REVOLVABLE
                                           | GF_FLIPPABLE_ALL_AXIS));

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    //setFlag(QGraphicsItem::ItemContainsChildrenInShape, true);

    mPath.setFillRule(Qt::WindingFill);
    mShouldPaintPath = false;
}

/**
 * @brief Constructs a new UBGraphicsStroke, from a list of polygons
 */
UBGraphicsStroke::UBGraphicsStroke(QList<QPolygonF> polygons, bool smooth, QGraphicsItem* parent)
    : UBGraphicsStroke(smooth, parent) // C++11
{
    mPolygons = polygons;
}

UBGraphicsStroke::~UBGraphicsStroke()
{
    if (QGraphicsItem::scene())
        QGraphicsItem::scene()->removeItem(this);
}

UBItem* UBGraphicsStroke::deepCopy() const
{
    UBGraphicsStroke* stroke = new UBGraphicsStroke(mSmoothDrawing, this->parentItem());
    copyItemParameters(stroke);
    return stroke;
}

void UBGraphicsStroke::copyItemParameters(UBItem *copy) const
{
    UBGraphicsStroke* other = dynamic_cast<UBGraphicsStroke*>(copy);
    if (!other)
        return;

    other->mPath = mPath;
    other->setColor(color(true), color(false));

    other->mDrawnPoints = mDrawnPoints;
    other->mReceivedPoints = mReceivedPoints;

    other->setPos(this->pos());
    other->setTransform(this->transform());
    other->setFlag(QGraphicsItem::ItemIsMovable, true);
    other->setFlag(QGraphicsItem::ItemIsSelectable, true);
    other->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    other->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    other->setData(UBGraphicsItemData::ItemEditable, data(UBGraphicsItemData::ItemEditable).toBool());
    other->setZValue(this->zValue());

    foreach(QGraphicsPolygonItem* poly, mPolygonItems) {
        QGraphicsPolygonItem* pi = new QGraphicsPolygonItem(poly->polygon(), other);
        other->initPolygonItem(pi);
    }
}

void UBGraphicsStroke::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

/**
 * @brief Returns true if the stroke is at least partially transparent.
 */
bool UBGraphicsStroke::hasAlpha() const
{
    return (qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect())
            && qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect())->opacity() != 1.0);
}

/**
 * @brief Set the stroke's color, on light and dark backgrounds. Transparency is supported.
 */
void UBGraphicsStroke::setColor(QColor lightBackground, QColor darkBackground)
{
    // Alpha channel is removed from the colors, as transparency is managed by a graphics effect.
    // To get the original QColors (including transparency), use UBGraphicsStroke::color

    if (lightBackground.alpha() != 255) {
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
        effect->setOpacity(lightBackground.alphaF());
        setGraphicsEffect(effect);
        lightBackground.setAlpha(255);
        darkBackground.setAlpha(255);
    }

    mColorOnDarkBackground = darkBackground;
    mColorOnLightBackground = lightBackground;
}


/**
 * @brief Return the stroke's color, including transparency
 * @param lightBackground If true, the stroke's color on light backgrounds is returned; otherwise, its color on dark backgrounds is returned.
 */
QColor UBGraphicsStroke::color(bool lightBackground) const
{
    QColor c = lightBackground ? mColorOnLightBackground : mColorOnDarkBackground;

    if (hasAlpha()) {
        c.setAlphaF(qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect())->opacity());
    }

    return c;
}

/**
 * @brief Simplify the stroke, removing unneeded points & polygons
 *
 * This can be called after drawing the stroke, to cut down on the number of objects on-screen.
 */
void UBGraphicsStroke::simplify()
{
    // TODO. Replaces itself with a simplified version

}

/**
 * @brief Return true if the width isn't constant across all points, or if there is only one point in the stroke.
 */
bool UBGraphicsStroke::hasPressure()
{
    if (mDrawnPoints.size() < 2)
        return true;

    qreal width = mDrawnPoints.first().second;

    foreach (strokePoint point, mDrawnPoints) {
        if (point.second != width)
            return true;
    }

    return false;
}

QPainterPath UBGraphicsStroke::shape () const
{
    return mPath;
}

QRectF UBGraphicsStroke::boundingRect() const
{
    return mPath.boundingRect();
}

/**
 * @brief Add a point to the end of the stroke, with a specified width
 *
 * The point is assumed to be a point received by the input device (mouse or pen). Internally, this may be used
 * to calculate a smoother curve to approximately join the last points of the stroke.
 *
 * In other words, strokes are built by adding one point (and width) at a time; the stroke handles the creation of
 * polygons that end up being displayed to the user.
 */
void UBGraphicsStroke::addPoint(const QPointF& point, qreal width)
{
    strokePoint newPoint(point, width);
    QPainterPath path;

    if (mReceivedPoints.empty())
        path.addEllipse(point, width/2., width/2.);

    else {
    //if (mSmoothDrawing) {
        // get the last drawn strokePoint
        strokePoint previousPoint = mReceivedPoints.last();

        // draw a straight line to scenePos, using UBGeometryUtils::curveToPath (curveToPath with two points will just return a line)
        path = UBGeometryUtils::curveToPath(QList<strokePoint>() << previousPoint << newPoint, true, true);
    }

        // Problem: just drawing the path (after mPath.addPath()) requires a call to update() for the new portion to be displayed.
        // but this is really slow. So instead we create a polygonItem and display that instead.

        //mPolygons << path.subtracted(mLastSubpath).toFillPolygon(); // So this was the slow part... the rest is actually quite smooth. Note: don't call subtracted() on large paths.

        mLastSubpath = path;
        mPolygons << path.toFillPolygon();

        QGraphicsPolygonItem* pi = new QGraphicsPolygonItem(mPolygons.last(), this);
        initPolygonItem(pi);

        mPolygonItems << pi;


        mPath.addPath(path);

        // Force update, otherwise the new subpath won't be painted
        //QRectF subpathRect = path.boundingRect();
        //qDebug() << "adding path with boundingrect: " << subpathRect;
        //qDebug() << "total bounding rect: " << this->boundingRect();
        //subpathRect.translated(-(boundingRect().topLeft());

        //update(); // this gets slow if called on a large path.

        // For now
       // mCurrentStroke->mPolygons = mCurrentStroke->mPath.toFillPolygons();
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
    //}


}

void UBGraphicsStroke::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (mShouldPaintPath) {
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setBrush(QBrush(mColorOnLightBackground));
        painter->setPen(Qt::NoPen);
        painter->drawPath(mPath);
    }

    //Delegate()->postpaint(painter, &styleOption, widget);
}


/**
 * @brief Erase the portion of the stroke contained in the given polygon
 * @return A pair of sets: the removed polygonItems, and the added polygonItems
 */
QPair<QSet<QGraphicsItem *>, QSet<QGraphicsItem*> > UBGraphicsStroke::erase(const QPolygonF &polygon)
{
    QPainterPath path;
    path.addPolygon(polygon);
    return erase(path);
}

/**
 * @brief Erase the portion of the stroke contained in the given path
 * @return A pair of sets: the removed polygonItems, and the added polygonItems
 */
QPair<QSet<QGraphicsItem *>, QSet<QGraphicsItem*> > UBGraphicsStroke::erase(const QPainterPath& path)
{

    QPainterPath eraserPath = mapFromScene(path);

    // option 1: most likely wayyyyy too slow: just call path.subtracted()

    // To allow undoing / redoing, we keep track of which polygonItems were deleted or modified.
    // PolygonItems that are completely contained in the eraser path are just deleted; the ones
    // that intersect the path are deleted and replaced by a modified item.
    // The more straight-forward approach would be to simply modify their shape, but this would
    // greatly complicate undo/redo operations.

    QSet<QGraphicsItem*> removedPolygonItems;
    QSet<QGraphicsItem*> addedPolygonItems;

    QMutableListIterator<QGraphicsPolygonItem*> it(mPolygonItems);
    while (it.hasNext()) {
        QGraphicsPolygonItem* poly = it.next();
        if (poly->collidesWithPath(eraserPath)) {
            //poly->setBrush(QBrush(Qt::black));

            QPainterPath polyPath = poly->shape();

            poly->scene()->removeItem(poly);
            it.remove();

            if (eraserPath.contains(polyPath)) {
                removedPolygonItems << poly;
            }

            else {
                polyPath = polyPath.subtracted(eraserPath);
                //poly->setPolygon(polyPath.toFillPolygon());

                QGraphicsPolygonItem* pi = new QGraphicsPolygonItem(polyPath.toFillPolygon(), this);
                initPolygonItem(pi);

                removedPolygonItems << poly;
                addedPolygonItems << pi;
            }
        }
    }
    foreach(QGraphicsItem* p, addedPolygonItems)
        mPolygonItems << dynamic_cast<QGraphicsPolygonItem*>(p);

    return QPair<QSet<QGraphicsItem*>, QSet<QGraphicsItem*> >(removedPolygonItems, addedPolygonItems);
}


/**
 * @brief Initialize a QGraphicsPolygonItem with color, flags etc.
 *
 * This function also reparents the polygonItem to the current UBGraphicsStroke instance.
 */
void UBGraphicsStroke::initPolygonItem(QGraphicsPolygonItem* polygonItem)
{
    polygonItem->setParentItem(this);

    polygonItem->setFlag(ItemStacksBehindParent, true);
    polygonItem->setFlag(ItemIsSelectable, false);
    polygonItem->setFlag(ItemIsMovable, false);

    polygonItem->setBrush(QBrush(mColorOnLightBackground));
    polygonItem->setPen(Qt::NoPen);

}

QVariant UBGraphicsStroke::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsItem::itemChange(change, newValue);
}

