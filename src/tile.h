#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "gps/point.h"

struct SlippyCoordinates{
    int zoom;
    int x;
    int y;

    bool m_hasOffset;
    double offsetX;
    double offsetY;
    /**
     * @brief operator <
     * comparision operator to sort coordinates
     * @param other
     * @return
     */
    bool operator<(const SlippyCoordinates& other) const{
        if(zoom<other.zoom) return true;
        if(zoom>other.zoom) return false;

        if(x<other.x) return true;
        if(x>other.x) return false;

        if(y<other.y) return true;
        if(y>other.y) return false;

        return false; // they are equal;
    }

    SlippyCoordinates(int zoom, int x, int y):
        zoom(zoom),
        x(x),
        y(y),
        m_hasOffset(false)
    {

    }

SlippyCoordinates(gps::Point location, int zoom);
    SlippyCoordinates()
    {
        m_hasOffset=false;
    }
    void setOffset(double x, double y){
        offsetX=x;
        offsetY=y;
        m_hasOffset=true;
    }


    double yOffset(){
        return offsetY;
    }
    double xOffset(){
        return offsetX;
    }

    int xOffsetPixels(){
        return offsetX*256.0;
    }
    int yOffsetPixels(){
        return offsetY*256.0;
    }

public:
    bool hasOffset() const;

    SlippyCoordinates operator+(QPoint offset);
    SlippyCoordinates operator +(QPointF offset);

    //yes, i was to lazy to write operator-()
    template<typename T>
    inline SlippyCoordinates operator -(T offset){
        return operator+(T()-offset);
    }
};

class Tile : public QObject
{
    Q_OBJECT
public:
    SlippyCoordinates coords;
    explicit Tile(SlippyCoordinates coords, QObject *parent = 0);

    QImage image;

signals:

    void ready(Tile*);
public slots:
    void reload(){
        //TODO
    }
};

#endif // TILE_H
