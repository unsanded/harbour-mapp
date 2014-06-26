#ifndef QSLIPPYMAP_H
#define QSLIPPYMAP_H

#include <math.h>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QQuickImageProvider>

#include "slippycache.h"
#include "slippyprovider.h"
#include "gps/point.h"

/**
 * @brief The QSlippyMap class
 * QTquick item that draws a slippymap
 * this class does not provide scrolling. It is just a static image.
 * To enable scrolling, use SlippyView
 */
class SlippyMap : public QQuickPaintedItem
{
    Q_OBJECT

    bool dragging;
    QPoint dragStart;
    SlippyCoordinates dragStartCoordinates;

    //the zoom as a float
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged);
    qreal m_zoom;

protected:
    //a cache-object for each provider.
    QList<SlippyCache*> providers;

    SlippyCoordinates currentLocation;

public:
    explicit SlippyMap(QQuickItem *parent = 0);

    virtual void paint(QPainter *painter);

    void addProvider(SlippyProvider* provider);

    virtual void makeCache(SlippyProvider* provider);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void touchEvent(QTouchEvent *event);

    qreal zoom() const
    {
        return m_zoom;
    }

signals:

    void zoomChanged(qreal arg);

public slots:
    virtual void setCoordinates(SlippyCoordinates newCoords);
    /**
     * @brief reTile
     *move by the given amount of tiles
     * @param dx
     * @param dy
     */
    virtual void reTile(int dx, int dy);

    void setZoom(qreal arg) ;
};



#endif // QSLIPPYMAP_H
