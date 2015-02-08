#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <QObject>
#include "tile.h"
#include "slippyprovider.h"

struct Layer{
   SlippyCache* cache;
   SlippyProvider* provider;
   QString name;
};


class TileManager : public QObject
{
    Q_OBJECT
    int activeLayerIndex;
    SlippyCache* currentCache;
    QMap<QString, int> layerIndices;
    QVector<Layer> layers;

    /**
     * @brief stores the tilestores available to the manager
     */
    QVector<SlippyTileStore*> stores;

public:
    explicit TileManager(QObject *parent = 0);
    ~TileManager();
    Tile* getTile(SlippyCoordinates& coordinates const);

    Layer activeLayer();

    Layer operator[](int index);
    Layer operator[](QString name);

    /**
     * @brief getDefault to get the application-global tile-manager
     * @return
     */
    TileManager *getDefault();

signals:
    activeLayerChanged( int index );


public slots:
    void addLayer(SlippyProvider* provider);
    void selectLayer(int index);
    void selectLayer(QString name);

};

#endif // TILEMANAGER_H
