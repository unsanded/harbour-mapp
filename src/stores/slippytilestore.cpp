#include "slippytilestore.h"

SlippyTileStore::SlippyTileStore(QObject *parent) :
    QObject(parent)
{

}

bool SlippyTileStore::hasTile( const SlippyCoordinates &coords){
    return getTile(coords)!=0;
}


