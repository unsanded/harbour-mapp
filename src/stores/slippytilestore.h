#ifndef SLIPPYTILESTORE_H
#define SLIPPYTILESTORE_H

#include "../tile.h"

#include <QObject>

/**
 * @brief The SlippyTileStore class
 * an interface for defining ways to store tiles locally. What it should be capable of is:
 * for each layer type
 *  for each unique SlippyCoordinates
 *   save a tile, and possibly load a file(if it is available)
 */

class SlippyTileStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString layerName READ layerName WRITE setlayerName NOTIFY layerNameChanged)

    QString m_layerName;

public:

    /**
     * @brief SlippyTileStore
     * @param parent standard qobject parent. Should probably be the SlippyCache
     */
    explicit SlippyTileStore( QObject *parent = 0);

    virtual bool hasTile( const SlippyCoordinates& coords );

    /**
     * @brief getTile
     * This should retrieve the tile from the store, or return 0 if the tile is not available
     * @param coords the coordinates that point to the specified tile
     * @return
     */
    virtual Tile* getTile(const SlippyCoordinates& coords )=0;
    virtual void StoreTile(Tile* tile)=0;

    virtual QString getPrettyName()=0;


QString layerName() const
{
    return m_layerName;
}

signals:

void layerNameChanged(QString arg);

public slots:

void setlayerName(QString arg)
{
    if (m_layerName != arg) {
        m_layerName = arg;
        emit layerNameChanged(arg);
    }
}
};

#endif // SLIPPYTILESTORE_H
