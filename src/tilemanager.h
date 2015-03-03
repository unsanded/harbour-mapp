#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <QObject>
#include <QQuickItem>
#include "tile.h"
#include "slippyprovider.h"

struct Layer{
   SlippyCache* cache;
   SlippyProvider* provider;
   QString name;
};


class TileManager : public QQuickItem
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
    explicit TileManager(QQuickItem *parent = 0);
    ~TileManager();
    virtual Tile* getTile(SlippyCoordinates coordinates );
    virtual Tile* getTile( int x, int y, int zoom );

    Layer activeLayer();

    Layer operator[](int index);
    Layer operator[](QString name);

    /**
     * @brief getDefault to get the application-global tile-manager
     * @return
     */
    static TileManager *getDefault();

signals:
    void activeLayerChanged( int index );


public slots:
    void addLayer(SlippyProvider* provider);
    void selectLayer(int index);
    void selectLayer(QString name);


    // QQmlParserStatus interface
public:
    void classBegin(){
        QQuickItem::classBegin();
    }
    void componentComplete(){
        QQuickItem::componentComplete();
    }
};

#endif // TILEMANAGER_H
