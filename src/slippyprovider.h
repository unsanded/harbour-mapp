#ifndef SLIPPYPROVIDER_H
#define SLIPPYPROVIDER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "tile.h"

/**
 * @brief The SlippyProvider class
 * An abstract class providing a source of maps.
 */
class SlippyProvider : public QObject
{
    Q_OBJECT
public:
    explicit SlippyProvider(QObject *parent = 0) :
        QObject(parent)
    {
    }

    /**
     * @brief makeTile
     * Make or fetch a tile from a mapservice. Caching is not necessary, that will be handled separately.
     * @param coords the SlippyCoordinates that should be fetched or created.
     * @return The created tile
     */
    virtual Tile* makeTile(SlippyCoordinates coords);

public:
    /**
     * @brief isLayer
     * @return whether the provided tiles are usable as overlay. Like openseamap for instance.
     */
    virtual bool isOverlay();

    /**
     * @brief getPrettyName
     * @return the name people will see. eg. "OpenStreetMap"
     */
    virtual QString getPrettyName();

     /**
     * @brief isOnline
     * @return whether the provider needs an internet-connection
     */
    virtual bool isOnline()
    {
         return false;
    }


            signals:
    void tileChanged(Tile*);
    void tileChanged();

public slots:

    void onTileLoaded(Tile* tile);
};



#endif // SLIPPYPROVIDER_H
