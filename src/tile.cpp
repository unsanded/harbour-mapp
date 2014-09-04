#include "tile.h"
#include <math.h>
#include <QDebug>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>

QSGNode *Tile::makeNode(QQuickWindow* window)
{
    qDebug() << "Updating node " << coords;
    if(node && !nodeMadeWhileReady && isReady()){
        qDebug()<<"Reusing node";
        //drop the node that was made while we were not ready
        delete texture;
        texture=window->createTextureFromImage(image);
        ((QSGSimpleTextureNode*)node)->setTexture(texture);

        node->markDirty(QSGNode::DirtyMaterial);
        nodeMadeWhileReady=true;
    }

    if(!node){
        qDebug() << "Creating node " ;
        node=new QSGSimpleTextureNode;
        texture=window->createTextureFromImage(image);
        ((QSGSimpleTextureNode*)node)->setTexture(texture);
        ((QSGSimpleTextureNode*)node)->setRect(0,0,256,256);
        node->markDirty(QSGNode::DirtyMaterial);

        transformNode=new QSGTransformNode;

        transformNode->appendChildNode(node);
        nodeMadeWhileReady=isReady();
    }
    return transformNode;
}

void Tile::dropNode()
{
    qDebug() << "dropping node...";
    if(transformNode){
            transformNode->removeChildNode(node);//remove the node from the grid
            delete transformNode;
            transformNode=0;
    }
    if(node) delete node;
    if(texture) delete texture;
    node=0;
    texture=0;
    qDebug() << "dropping node done";
}

Tile::Tile(SlippyCoordinates coordinates, QObject *parent) :
    QObject(parent),
    coords(coordinates),
    image(256, 256, QImage::Format_ARGB32)
{
    nodeMadeWhileReady=false;
    node=0;
    transformNode=0;
    texture=0;

    next=0;
    previous=0;
    m_ready=false;
    image.fill(QColor(100,100,200,200));
    //just to test
    QPainter p;
    p.begin(&image);
    p.setPen(Qt::green);

    p.drawArc(0,0,256,256,0,270*16);
    p.end();


}

//secosine for convenience
inline double sec(double in){
    return 1.0/cos(in);
}

SlippyCoordinates::SlippyCoordinates(gps::Point location, int zoom, QObject *parent):
    QObject(parent)
{
    this->m_zoom=zoom;

    // for the calculation see:
    // http://wiki.openstreetmap.org/wiki/Tilenames#X_and_Y

    double latitude=location.latitude;
    double longitude=location.longitude;

    double mercatorx = ((longitude + 180) / 360);

    double mercatory = (1.0 - (log(tan(latitude/180.0*M_PI) + sec(latitude/180.0*M_PI)) / M_PI)) / 2.0;

    setMercatorPos(QPointF(mercatorx, mercatory));


}



int SlippyCoordinates::zoom() const
{
    return m_zoom;
}




SlippyCoordinates SlippyCoordinates::operator+(QPoint offset)
{
    SlippyCoordinates res(zoom(), x(), y());
    res.settilePos(res.tilePos()+ offset);
    return res;
}

SlippyCoordinates SlippyCoordinates::operator+(QPointF offset)
{
    SlippyCoordinates ret;
    ret.setMercatorPos(MercatorPos() + offset );
    return *this;
}


bool SlippyCoordinates::operator<(const SlippyCoordinates &other) const{
    if(zoom()<other.zoom()) return true;
    if(zoom()>other.zoom()) return false;

    if(tilePos().x()<other.tilePos().x()) return true;
    if(tilePos().x()>other.tilePos().x()) return false;

    if(tilePos().y()<other.tilePos().y()) return true;
    if(tilePos().y()>other.tilePos().y()) return false;

    return false; // they are equal;
}

SlippyCoordinates::SlippyCoordinates(int zoom, int x, int y, QObject *parent):
    QObject(parent),
    m_zoom(zoom),
    m_tilePos(x,y)
{
    updateMercatorFromTiles();
}

gps::Point SlippyCoordinates::toGps(){


    double lon_deg = MercatorPos().x() * 360.0 - 180.0;

    double lat_rad = atan(sinh(M_PI * (1 - 2 * MercatorPos().y())));
    double lat_deg = lat_rad * 180.0 / M_PI;

    return gps::Point(lat_deg, lon_deg);

}


