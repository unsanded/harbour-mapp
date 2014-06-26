#include "slippyprovider.h"




Tile *SlippyProvider::makeTile(SlippyCoordinates coords)
{
    Q_UNUSED(coords)
    qDebug() << "Slippy maketile";

    return new Tile(coords);
}

bool SlippyProvider::isOverlay(){
    return false;
}

QString SlippyProvider::getPrettyName()
{
    return "ABSTRACT" ;
}

void SlippyProvider::onTileLoaded(Tile *tile)
{
    emit tileChanged();
    emit tileChanged(tile);
}
