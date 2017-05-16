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


#include "UBGraphicsStroke.h";

UBGraphicsStroke::UBGraphicsStroke(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                           | GF_RESPECT_RATIO
                                           | GF_REVOLVABLE
                                           | GF_FLIPPABLE_ALL_AXIS));

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    mPath.setFillRule(Qt::WindingFill);
}

UBGraphicsStroke::UBGraphicsStroke(QList<QPolygonF> polygons, QGraphicsItem* parent)
    : UBGraphicsStroke(parent)
{
    mPolygons = polygons;
}

UBGraphicsStroke::~UBGraphicsStroke() {}

UBItem* UBGraphicsStroke::deepCopy() const
{

    // call copyitemparameters
}

void UBGraphicsStroke::copyItemParameters(UBItem *copy) const
{

}

void UBGraphicsStroke::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

bool UBGraphicsStroke::hasAlpha() const
{
    return (mColorOnDarkBackground.alpha() || mColorOnLightBackground.alpha());
}

void UBGraphicsStroke::simplify()
{
    // TODO

}

bool UBGraphicsStroke::hasPressure()
{
    // Returns true if the width isn't constant across all points.

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

void UBGraphicsStroke::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(QBrush(mColorOnLightBackground));
    painter->setPen(Qt::NoPen);
    painter->drawPath(mPath);

    //Delegate()->postpaint(painter, &styleOption, widget);
}


