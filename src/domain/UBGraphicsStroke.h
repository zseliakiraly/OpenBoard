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

#ifndef UBGRAPHICSSTROKE_H_
#define UBGRAPHICSSTROKE_H_

#include <QtGui>
#include <QPainterPath>

#include "core/UB.h"
#include "UBItem.h"

typedef QPair<QPointF, qreal> strokePoint;

class UBGraphicsStroke : public QGraphicsItem, public UBItem, public UBGraphicsItem
{
    //friend class UBDrawingController; // for now. but maybe drawingController should just interface via public functions

    public:
        UBGraphicsStroke(bool smooth = false, QGraphicsItem* parent = NULL);
        UBGraphicsStroke(QList<QPolygonF> polygons, bool smooth = false, QGraphicsItem* parent = NULL);
        virtual ~UBGraphicsStroke();

        enum { Type = UBGraphicsItemType::StrokeItemType };
        virtual int type() const { return Type; }

        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;

        void setUuid(const QUuid &pUuid);

        bool hasAlpha() const;
        void setColor(QColor lightBackground, QColor darkBackground);
        QColor color(bool lightBackground = true) const;

        void addPoint(const QPointF& point, qreal width);
        void addPolygonItem(QGraphicsPolygonItem* polygonItem); // for use by undo/redo commands

        const QList<QPair<QPointF, qreal> >& points() { return mDrawnPoints; }

        void simplify();
        bool hasPressure();

        QRectF boundingRect() const;

        QPair<QSet<QGraphicsItem*>, QSet<QGraphicsItem*> > erase(const QPolygonF &polygon);
        QPair<QSet<QGraphicsItem*>, QSet<QGraphicsItem*> > erase(const QPainterPath &path);

    protected:
        QPainterPath shape () const;

        /*
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        */
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    private:

        void initPolygonItem(QGraphicsPolygonItem* polygonItem);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);


        QList<QPolygonF> mPolygons; // why is this here? remove if no longer necessary
        QList<QGraphicsPolygonItem*> mPolygonItems; // individual polygons making up the stroke. Has to be done this way because updating a path each time a point is added is much too slow
        QPainterPath mLastSubpath;

        QPainterPath mPath;  // mPath represents the entire stroke

        bool mShouldPaintPath; // If true, mPath is painted

        /// Points that were drawn by the user (i.e, actually received through input device)
        QList<QPair<QPointF, qreal> > mReceivedPoints;

        /// All the points (including interpolated) that are used to draw the stroke
        QList<QPair<QPointF, qreal> > mDrawnPoints;

        bool mSmoothDrawing;


        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;
};

#endif /* UBGRAPHICSSTROKE_H_ */
