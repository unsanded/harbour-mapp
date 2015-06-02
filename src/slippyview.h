#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include <QQuickItem>
#include <QStack>

#include <src/slippycache.h>
#include <src/providers/osmprovider.h>
#include "tilemanager.h"


#undef _DEBUG


#ifdef _DEBUG
#define HTILEBUFFER (3)
#define VTILEBUFFER (3)
#else
#define HTILEBUFFER (5)
#define VTILEBUFFER (5)
#endif

#define HTILESIZE (256)
#define VTILESIZE (256)

/**
 * @brief The SlippyView class
 * This is the qt-quick component that shows the interactive map. It keeps a grid of n by n Slippy tiles, which are moved around by geastures.
 * For small movements, the whole grid is translated, but when the movements exceeds the size of one tile, the grid of tiles is shifted by one tile.
 */
class SlippyView : public QQuickItem
{
    Q_OBJECT


    bool m_lockRotation;
    bool m_lockZoom;
    TileManager* m_tileManager;

    bool m_ready;

public:
    explicit SlippyView(SlippyCache* cache=new SlippyCache(new OsmProvider),QQuickItem *parent = 0);

    SlippyCoordinates currentLocation;

    Q_PROPERTY(qreal zoom READ zoom WRITE setzoom NOTIFY zoomChanged)
    Q_PROPERTY(qreal mapRotation READ mapRotation WRITE setmapRotation NOTIFY mapRotationChanged)
    Q_PROPERTY(QGeoCoordinate location READ location WRITE setlocation NOTIFY locationChanged)
    Q_PROPERTY(bool lockRotation READ lockRotation WRITE setlockRotation NOTIFY lockRotationChanged)
    Q_PROPERTY(bool lockZoom READ lockZoom WRITE setlockZoom NOTIFY lockZoomChanged)
    Q_PROPERTY(TileManager* tileManager READ tileManager WRITE settileManager NOTIFY tileManagerChanged)

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)




protected:
    //this struct can only be used in the UpdatePaintNode functions thread
    struct {
        QSGTransformNode* scrollTransform;
       // QSGTransformNode* tileTransform[HTILEBUFFER][VTILEBUFFER];

    } nodes;
    //que of tiles whose node is to be dropped in updatePaintNode()
    //not in nodes, because this is allowed to be used outside of updatePaintnode
    QSet<Tile*> dropNodeQueue;

    /**
     * @brief The matrices struct holds all the matrices used to transform tiles to screen.
     * All the tiles are mapped through matrices.complete, which is built up from a move, zoom and rotate component.
     * The transformation is in that order, so all tiles are moved first, then scaled and then rotated.
     */
    struct {
        QPointF movementOffset;//offset so that the edge of the grid stays off the screen

        QPointF movementFromTile;//if this becomes more than one in either direction we have to swap tiles

        QMatrix4x4 move;

        qreal zoomFromLevel;
        QMatrix4x4 zoom;

        qreal rotation;
        QPointF rotationCenter; //the map rotate around this point
        QMatrix4x4 rotate;

        QMatrix4x4 complete;//the complete matrix which will be applied to the grid of tiles

        struct{
            QMatrix4x4 zoomAndRotate;
            QMatrix4x4 complete;
        } inverses;

    } matrices;

    struct {
        bool matrixChanged;
        bool gridChanged;
        bool tileChanged;
        QStack<Tile*> changedTiles;
    } changes;



    //the grid of tiles. Changes every time the view is moved more than one tile
    Tile* drawnTiles[HTILEBUFFER][VTILEBUFFER];
    QSGNode* drawnTileNodes[HTILEBUFFER][VTILEBUFFER];

    SlippyCache* cache;

    //steps the tiles in the grid with offsets dx and dy
    //reuses as many tiles as possible;
    void stepTile(int dx, int dy);
    //fill the grid again from scratch
    void reTile();

    //update the transformation matrix from the movement rotation and zoom variables.
    void updateCompleteMatrix(bool update=true);




signals:

    void zoomChanged(qreal zoom);
    void zoomLevelChanged(int zoomLevel);

    void locationChanged(QGeoCoordinate arg);

    void mapRotationChanged(qreal arg);

    void lockRotationChanged(bool arg);

    void lockZoomChanged(bool arg);

    void tileManagerChanged(TileManager* arg);

    void readyChanged(bool arg);
    void becomesReady();

public slots:

    void onTileReady(Tile* tile);

    // QQuickItem interface
    void setzoom(qreal arg);

    void onLayerChanged(int layerIndex);

    void setlocation(QGeoCoordinate arg);

    void setmapRotation(qreal arg);

    void setlockRotation(bool arg);

    void setlockZoom(bool arg);

    void settileManager(TileManager* arg)
    {
        if (m_tileManager != arg) {

            if(m_tileManager)
                disconnect(m_tileManager, SIGNAL(activeLayerChanged(int)),
                           this, SLOT(onLayerChanged(int)));

            m_tileManager = arg;

                connect(m_tileManager, SIGNAL(activeLayerChanged(int)),
                           this, SLOT(onLayerChanged(int)));

            emit tileManagerChanged(arg);
        }
    }

protected:
    virtual void touchEvent(QTouchEvent *event);
    virtual QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data);


    /**
      * add a tile to the queue to have it SG node dropped
      */
    virtual void queueForDropNode(Tile* tile);

    // QQmlParserStatus interface
public:
    virtual void classBegin();
virtual void componentComplete();
qreal zoom() const
{
    return currentLocation.zoom() + log2(matrices.zoomFromLevel);
}

QGeoCoordinate location() ;
qreal mapRotation() const ;

void selectLayer(QString name);



bool lockRotation() const
{
    return m_lockRotation;
}
bool lockZoom() const
{
    return m_lockZoom;
}

TileManager* tileManager() const
{
    return m_tileManager;
}

/**
 * @brief ready means that the view has a tile-source, and is ready to draw.
 * @return whether the view is ready;
 */
bool ready() const
{
    return m_ready;
}
};

#endif // SLIPPYVIEW_H
