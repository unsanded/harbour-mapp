#include "slippycache.h"

SlippyCache::SlippyCache(SlippyProvider *provider, QObject *parent) :
    QObject(parent),
    provider(provider)
{
}

Tile *SlippyCache::getTile(const SlippyCoordinates coords)
{
    if(ramCache.contains(coords))
        return ramCache[coords];
    Tile* newTile = provider->makeTile(coords);
    ramCache.insert(coords, newTile) ;
    return newTile;
}

