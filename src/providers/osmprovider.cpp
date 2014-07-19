#include "osmprovider.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>




OsmTile::OsmTile(SlippyCoordinates coords, OsmProvider *parent):Tile(coords, parent)
{
    provider=parent;
}

void OsmTile::receiveData()
{
    qDebug() << "ready " << coords;
    if(!reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower().startsWith("image/")){
        qWarning()<<"wrong docType" << coords;
        return;
    }
    QByteArray data( reply->readAll());

    if(image.loadFromData(data)){
        m_ready=true;
        emit ready(this);
    }else{
        qWarning() << "failed to load image";

    }
}

OsmProvider::OsmProvider(QObject *parent) :
    SlippyProvider(parent)
{

    tileUrl   = "http://b.tile.openstreetmap.org/%1/%2/%3.png";
    userAgent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0";
}

Tile* OsmProvider::makeTile(SlippyCoordinates coords)
{

    //request a tile from osm.org
    QUrl url(QString(tileUrl).arg(coords.zoom()).arg( coords.tilePos().x()).arg(coords.tilePos().y()));
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent", userAgent.toLatin1());

    OsmTile* result = new OsmTile(coords, this);
    result->reply = netmanager.get(request);
    connect(result, SIGNAL(ready(Tile*)), this, SLOT(onTileLoaded(Tile*)));
    connect(result->reply, SIGNAL(finished()), result, SLOT(receiveData()));


    return result;
}

