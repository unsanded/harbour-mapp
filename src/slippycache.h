#ifndef SLIPPYCACHE_H
#define SLIPPYCACHE_H

#include <QObject>
#include <QMap>
#include "src/slippyprovider.h"
#include <QLinkedList>
#include <QMutex>

/**
 * @brief The SlippyCache class
 * An object which caches map-tiles.
 */

class SlippyCache : public QObject
{
    Q_OBJECT
    //tiles that have not been loaded

    QMutex mutex;
    SlippyProvider* provider;
    QMap<SlippyCoordinates, Tile*> ramCache;

    // the list that keeps track when a tile was last used.
    // so that the oldest tile can be deleted.
    QLinkedList<Tile *> deathRow;
public:
    explicit SlippyCache(SlippyProvider* provider, QObject *parent = 0);

    virtual ~SlippyCache();
    /**
     * @brief getTile
     * gets a tile from the cache, or if it's not in there, from the provider.
     * @param coords the slippyCoordinates from which to get the tile
     * @return the tile belonging to the coordinates
     */
    virtual Tile* getTile(const SlippyCoordinates coords);
    Tile* getTile(int x, int y, int zoom );



signals:

public slots:

    virtual void deleteAllTiles();

protected:
    int capacity;

private:
    // implement a simple doublyLinked list to keep track of tile-age
    Tile* newest;
    Tile* oldest;


};

#endif // SLIPPYCACHE_H
