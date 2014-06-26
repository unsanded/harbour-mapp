#ifndef SLIPPYCACHE_H
#define SLIPPYCACHE_H

#include <QObject>
#include <QMap>
#include "src/slippyprovider.h"
/**
 * @brief The SlippyCache class
 * An object which caches map-tiles.
 */

class SlippyCache : public QObject
{
    Q_OBJECT
    //tiles that have not been loaded

    SlippyProvider* provider;
    QMap<SlippyCoordinates, Tile*> ramCache;
public:
    explicit SlippyCache(SlippyProvider* provider, QObject *parent = 0);

    /**
     * @brief getTile
     * gets a tile from the cache, or if it's not in there, from the provider.
     * @param coords the slippyCoordinates from which to get the tile
     * @return the tile belonging to the coordinates
     */
    virtual Tile *getTile(const SlippyCoordinates coords);


signals:

public slots:

};

#endif // SLIPPYCACHE_H
