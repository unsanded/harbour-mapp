#include "slippytilestore.h"

SlippyTileStore::SlippyTileStore(QObject *parent) :
    QObject(parent),
    m_layerName(layerName)
{

}

bool SlippyTileStore::hasTile( const SlippyCoordinates &coords){
    return getTile(coords)!=0;
}


