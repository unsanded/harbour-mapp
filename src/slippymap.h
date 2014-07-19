#ifndef QSLIPPYMAP_H
#define QSLIPPYMAP_H

#include <math.h>
#include <QQuickPaintedItem>

#include "slippycache.h"
#include "slippyprovider.h"
#include "gps/point.h"

/**
 * @brief The QSlippyMap class
 * QTquick item that draws a slippymap
 * 
 */
class SlippyMap : public QQuickPaintedItem
{
    friend class SlippyView;
    Q_OBJECT

    bool dragging;

    bool drawOffset;


    //the zoom as a float
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged);
    qreal m_zoom;

protected:
    //a cache-object for each provider.
    QList<SlippyCache*> providers;

    SlippyCoordinates currentLocation;

public:
    explicit SlippyMap(QQuickItem *parent = 0);
    virtual ~SlippyMap();



    virtual void paint(QPainter *painter);

    void addProvider(SlippyProvider* provider);

    virtual void makeCache(SlippyProvider* provider);

    virtual SlippyCoordinates coordinates();

    qreal zoom() const
    {
        return m_zoom;
    }

signals:

    void tileChanged();
    void zoomChanged(qreal arg);

public slots:
    virtual void setCoordinates(SlippyCoordinates newCoords);
    void onTileChanged();

    /**
     * @brief reTile
     *move by the given amount of tiles
     * @param dx
     * @param dy
     */
    virtual void reTile(int dx, int dy);

    void setZoom(qreal arg) ;

    // QQuickItem interface

    // QQmlParserStatus interface
public:
    virtual void componentComplete();
};



#endif // QSLIPPYMAP_H
