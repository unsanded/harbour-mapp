#include "osmprovider.h"

#ifndef GOOGLEMAPSPROVIDER_H
#define GOOGLEMAPSPROVIDER_H

class GoogleMapsProvider : public OsmProvider
{
public:
    Q_PROPERTY(QString apiKey READ apiKey WRITE setapiKey NOTIFY apiKeyChanged)
    GoogleMapsProvider();


public:

    virtual bool isOverlay(){ return false;}
    virtual QString getPrettyName(){return "Google Maps";}
    virtual bool isOnline(){return true;};
};

#endif // GOOGLEMAPSPROVIDER_H
