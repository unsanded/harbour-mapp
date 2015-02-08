#include "folderstore.h"
#include "slippytilestore.h"

FolderStore::FolderStore(QString folder, QObject *parent) :
    SlippyTileStore( parent),
    m_path(folder)
{
}

Tile *FolderStore::getTile(QString layerName, SlippyCoordinates &coords)
{
    QString path = QString("%1/%2/%3/%3/%5").arg(m_path).arg(layerName).arg(coords.zoom()).arg(coords.x()).arg(coords.y());

    QFile file(path);
    if(!file.exists()) return;

    Tile* tile=new Tile(coords, this);
    tile->image=QImage(path);
    tile->m_ready=true;

}
