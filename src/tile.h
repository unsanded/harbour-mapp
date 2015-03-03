#ifndef TILE_H
#define TILE_H

#define TILE_SIZE 256

#include <QImage>
#include <QPainter>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGTransformNode>
#include <QtPositioning>
#include "math.h"

class SlippyCache;
class SlippyTileStore;

struct SlippyCoordinates: public QObject {

    Q_OBJECT


    Q_PROPERTY(QPoint tilePos READ tilePos WRITE settilePos NOTIFY tilePosChanged)
    Q_PROPERTY(QPointF tileOffset READ tileOffset WRITE settileOffset NOTIFY tileOffsetChanged)
    Q_PROPERTY(int x READ x WRITE setx NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE sety NOTIFY yChanged)
    Q_PROPERTY(int zoom READ zoom WRITE setzoom NOTIFY zoomChanged)
    Q_PROPERTY(QPointF MercatorPos READ MercatorPos WRITE setMercatorPos NOTIFY MercatorPosChanged)



private:
    int m_zoom;
    QPoint m_tilePos;
    QPointF m_tileOffset;
    QPointF m_MercatorPos;

public:

    SlippyCoordinates(QObject* parent=0): QObject(parent){ }

    /**
     * @brief operator <
     * comparision operator to sort coordinates
     * @param other
     * @return
     */
    bool operator<(const SlippyCoordinates& other) const;

    SlippyCoordinates(int zoom, int x, int y, QObject* parent=0);

    SlippyCoordinates(QGeoCoordinate location, int zoom, QObject* parent =0);

    SlippyCoordinates(const SlippyCoordinates& other):
        QObject(other.parent())
    {
        m_zoom=other.zoom();
        setMercatorPos(other.MercatorPos());
    }


    operator QGeoCoordinate(){
        return toGps();
    }

    QPointF OffsetPixels() const{ return tileOffset()*TILE_SIZE; }

    QGeoCoordinate toGps();


public:
    bool hasOffset() const;

    /**
     * @brief operator +
     * add to the coordinates in tiles
     * @param offset the offset in Tiles.
     * @return the sum of this, and offset
     */
    SlippyCoordinates operator+(QPoint offset);
    /**
     * @brief operator +
     * add to the coordinates in mercator coordinates
     * @param offset
     * @return
     */
    SlippyCoordinates operator +(QPointF offset);

    SlippyCoordinates& operator =(SlippyCoordinates other){
        m_zoom=other.zoom();
        setMercatorPos(other.MercatorPos());
        return *this;
    }

    //yes, i was to lazy to write operator-()
    template<typename T>
    inline SlippyCoordinates operator -(T offset){
        return operator+(T()-offset);
    }


    int zoom() const;
    QPoint tilePos() const
    {
        return m_tilePos;
    }
    int x() const
    {
        return m_tilePos.x();
    }

    int y() const
    {
        return m_tilePos.y();
    }

    QPointF MercatorPos() const
    {
        return m_MercatorPos;
    }

    QPointF tileOffset() const
    {
        return m_tileOffset;

    }

public slots:
    void moveByPixels(QPointF offset){
        settileOffset(tileOffset() + offset/TILE_SIZE);

    }

    void settilePos(QPoint arg)
    {
        if (m_tilePos != arg) {
            m_tilePos=arg;
            updateMercatorFromTiles();
            emit tilePosChanged(arg);
        }

    }
    void setx(int arg)
    {
        if (m_tilePos.x() != arg) {

            m_tilePos.setX(arg);
            updateMercatorFromTiles();
            emit xChanged(x());
            emit tilePosChanged(tilePos());

        }
    }

    void sety(int arg)
    {
        if (m_tilePos.y() != arg) {
            m_tilePos.setY( arg );
            emit tilePosChanged(tilePos());
            emit yChanged(y());
            emit MercatorPosChanged(MercatorPos());
        }
    }

    void setzoom(int arg)
    {
        if (m_zoom != arg && arg>0) {
            m_zoom = arg;
            updateTilesFromMercator();
            emit zoomChanged(arg);
        }
    }

    void setMercatorPos(QPointF arg)
    {
        if (m_MercatorPos != arg) {

            m_MercatorPos=arg;

            updateTilesFromMercator();

            emit MercatorPosChanged(m_MercatorPos);
        }
    }

    void settileOffset(QPointF arg)
    {
        if (m_tileOffset != arg) {
            m_tileOffset = arg;

            bool retiled=false;// have we gone to another tile

            while(m_tileOffset.y()<0){
                m_tileOffset.ry()+=1.0;
                m_tilePos.ry()--;
                retiled=true;
            }
            while(m_tileOffset.x()<0){
                m_tileOffset.rx()+=1.0;
                m_tilePos.rx()--;
                retiled=true;
            }

            while(m_tileOffset.y()>1){
                m_tileOffset.ry()-=1.0;
                m_tilePos.ry()++;
                retiled=true;
            }
            while(m_tileOffset.x()>1){
                m_tileOffset.rx()-=1.0;
                m_tilePos.rx()++;
                retiled=true;
            }

            updateMercatorFromTiles();
            if(retiled){
                emit tilePosChanged(m_tilePos);
            }
            emit tileOffsetChanged(tileOffset());
            emit MercatorPosChanged(MercatorPos());
        }
    }

signals:
    void tilePosChanged(QPoint arg);
    void xChanged(int arg);
    void yChanged(int arg);
    void zoomChanged(int arg);
    void MercatorPosChanged(QPointF arg);
    void tileOffsetChanged(QPointF arg);


private:
    void updateMercatorFromTiles(){
        int n = pow(2, zoom());

        setMercatorPos(
            (QPointF(tilePos().x(), tilePos().y()) + tileOffset())/n
        );
    }
    void updateTilesFromMercator(){
        int n = pow(2, zoom());

        QPointF zoomedMercator=MercatorPos()*n;

        settilePos(QPoint(zoomedMercator.x(), zoomedMercator.y()));
        settileOffset(QPointF(zoomedMercator.x()-floor(zoomedMercator.x()),
                              zoomedMercator.y()-floor(zoomedMercator.y())
                              ));


    }
};

class Tile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setimage NOTIFY imageChanged)
    Q_PROPERTY(bool ready READ ready WRITE setready NOTIFY readyChanged)

    friend class SlippyCache;
    friend class SlippyTileStore;
    friend class SlippyView;

protected:

    QImage m_image;
    bool m_ready;

    QSGNode* node;
    QSGTransformNode* transformNode;//a link to the transformnode on the grid
    QSGTexture* texture;

    bool nodeMadeWhileReady;
    //these two functions may only be called from within the updatepaintnode function



    // to make them linkedListable
    SlippyCache* cache;
    Tile* next;
    Tile* previous;


public:
    virtual QSGNode* makeNode(QQuickWindow *window);
    virtual void dropNode();
    SlippyCoordinates coords;
    explicit Tile(SlippyCoordinates coords, QObject* parent=0);

    bool ready(){
        return m_ready;
    }

    QImage image() const
    {
        return m_image;
    }

signals:

    void ready(Tile*);
    void imageChanged(QImage arg);

    void readyChanged(bool arg);

public slots:
    virtual void reload(){
        //TODO
}
void setimage(QImage arg)
{
    if (m_image != arg) {
        m_image = arg;
        emit imageChanged(arg);
        setready(true);
    }

}
void setready(bool arg)
{
    if (m_ready != arg) {
        m_ready = arg;
        emit readyChanged(arg);
        if(arg)
            emit ready(this);
    }
}
};



#endif // TILE_H
