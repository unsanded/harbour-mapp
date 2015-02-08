#ifndef OSMPROVIDER_H
#define OSMPROVIDER_H
#include "../slippyprovider.h"
#include <QNetworkAccessManager>

class OsmProvider;

class OsmTile: public Tile{
    friend class OsmProvider;
    Q_OBJECT

protected:
    QNetworkReply *reply;

    OsmProvider* provider;

    OsmTile(SlippyCoordinates coords, OsmProvider* parent);

public slots:
    void receiveData();
};



/**
 * @brief The OsmProvider class
 *provides the openstreetmap.org slippymap backend
 */
class OsmProvider : public SlippyProvider
{
    Q_OBJECT
    QNetworkAccessManager netmanager;

protected:
    QString userAgent;
    QString tileUrl;
public:
    explicit OsmProvider(QQuickItem *parent = 0);

signals:

public slots:


public:
    Tile *makeTile(SlippyCoordinates coords);
    bool isOverlay(){return false;}

    QString getPrettyName()
    {
        return "OpenStreetMap";
    }
    bool isOnline(){ return true; }
};

#endif // OSMPROVIDER_H
