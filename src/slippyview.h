#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include <QQuickItem>
#include <QStack>
#include <QSGSimpleRectNode>

#include <src/slippycache.h>
#include <src/providers/osmprovider.h>
#include "tilemanager.h"


#undef _DEBUG


#ifdef _DEBUG
#define HTILEBUFFER (3)
#define VTILEBUFFER (3)
#else
#define HTILEBUFFER (6)
#define VTILEBUFFER (6)
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
    explicit SlippyView(QQuickItem *parent = 0);

    SlippyCoordinates currentLocation;

    //Zoomlevel + 2log(zoomFactor)
    Q_PROPERTY(qreal zoom READ zoom WRITE setzoom NOTIFY zoomChanged)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(qreal mapRotation READ mapRotation WRITE setmapRotation NOTIFY mapRotationChanged)
    Q_PROPERTY(QGeoCoordinate location READ location WRITE setlocation NOTIFY locationChanged)
    Q_PROPERTY(bool lockRotation READ lockRotation WRITE setlockRotation NOTIFY lockRotationChanged)
    Q_PROPERTY(bool lockZoom READ lockZoom WRITE setlockZoom NOTIFY lockZoomChanged)
    Q_PROPERTY(TileManager* tileManager READ tileManager WRITE settileManager NOTIFY tileManagerChanged)

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)




protected:

    void updateInverseMatrices(){
        bool invertible = true;
        movement.inverses.complete = movement.navigationMatrix.inverted(&invertible);
        if(!invertible){
            qWarning() << "complete matrix not ivertible";
        }
    }

    //this struct can only be used in the UpdatePaintNode functions thread
    struct {
        //QSGTransformNode* scrollTransform;
       // QSGTransformNode* tileTransform[HTILEBUFFER][VTILEBUFFER];

    } nodes;
    // que of tiles whose node is to be dropped in updatePaintNode()
    // not in nodes, because this is allowed to be used outside of updatePaintnode
    QSet<Tile*> dropNodeQueue;

    /**
     * @brief The matrices struct holds all the matrices used to transform tiles to screen.
     * All the tiles are mapped through matrices.complete, which is built up from a move, zoom and rotate component.
     * The transformation is in that order, so all tiles are moved first, then scaled and then rotated.
     */
    struct {
        QPointF rotationCenter;
        QRectF  stepBounds;

        QMatrix4x4 navigationMatrix;//the complete matrix which will be applied to the grid of tiles
        //TODO: use this;
        QMatrix4x4 perspectiveMatrix;
        QMatrix4x4 completeMatrix;

        struct{
            QMatrix4x4 complete; //this matrix should at all times map from component pixels to tilespace pixels
        } inverses;

#ifndef _NO_MOUSE
        QPointF lastMousePos;
#endif

    } movement;


    struct {
        QSGSimpleRectNode* debugNode;
        QRectF debugPos;
        QSGSimpleRectNode* debugNode2;
        QRectF debugPos2;


    } debug;

    QMap<int,QPointF> oldTouchPoints;

    struct {
        bool matrixChanged;
        bool matrixNodeChanged;
        bool inverseMatrixOutdated;
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

    void rotateBy(qreal radians, QPointF pivot );

    void zoomBy(qreal zoom, QPointF pivot);

    void moveBy(QPointF by);

    QPointF tileToReal(QPointF tileCoords);
    QPointF realToTile(QPointF realCoords);
signals:


    void becomesReady();

    //// CHANGE NOTIFIERS
    void zoomChanged(qreal zoom);
    void zoomLevelChanged(int zoomLevel);
    void locationChanged(QGeoCoordinate arg);
    void mapRotationChanged(qreal arg);
    void lockRotationChanged(bool arg);
    void lockZoomChanged(bool arg);
    void tileManagerChanged(TileManager* arg);
    void readyChanged(bool arg);
    void zoomFactorChanged(qreal zoomFactor);

protected slots:
    void onResize();

public slots:

    /**
     * @brief onTileReady lets the view now that a tile has become ready. ie. that it is loaded. 
     * @param tile
     */
    void onTileReady(Tile* tile);

    // QQuickItem interface
    void setzoom(qreal arg);

    void onLayerChanged(int layerIndex);

    void setlocation(QGeoCoordinate arg);

    void setmapRotation(qreal arg);

    void setlockRotation(bool arg);

    void setlockZoom(bool arg);

    void settileManager(TileManager* arg);
    
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
    /**
 * @brief zoom
 * @return the zoomlevel including the scaling.
 *  ie. the zoom 1.5 means one of two things:
 *  - zoomlevel one, but scaled by 2^0.5
 *  - zoomlevel two, but scaled by 2^-0.5
 *
 * Which one it is, usually depends on whether the user has just zoomed in, or just zoomed out.
 * Because the zoomlevel is lazy (has hysteresis) to prevent it from fetching tiles unnecessearily.
 */
qreal zoom() const {
    return zoomLevel() + log2(zoomFactor());
}
/**
 * @brief zoomLevel indicates the depth of zoom.
 *  * Zero means that the whole world is 1 tile.
 *  * One means that the whole world is 2x2 tiles.
 *  * etc.
 *  the maximum zoomlevel depends on the backend being used.
 * @return the slippy zoomlevel.
 */
qreal zoomLevel() const {
    return currentLocation.zoom();
}

qreal zoomFactor() const {
    //assuming here, that zoom is equal in x and y (as it should be)
    QVector3D zero;
    QVector3D reference(1., 0., 0.);
    reference = movement.navigationMatrix.map(reference);
    zero = movement.navigationMatrix.map(zero);

    return (reference - zero).length();

}

QGeoCoordinate location() ;
qreal mapRotation() const ;



bool lockRotation() const { return m_lockRotation; }
bool lockZoom() const { return m_lockZoom; }

TileManager* tileManager() const { return m_tileManager; }

/**
 * @brief ready means that the view has a tile-source, and is ready to draw.
 * @return whether the view is ready;
 */
bool ready() const
{
    return m_ready;
}

protected:
#ifndef _NO_MOUSE
virtual void mousePressEvent(QMouseEvent *event) override;
virtual void mouseMoveEvent(QMouseEvent *event) override;
virtual void wheelEvent(QWheelEvent *event) override;
#endif

};

#endif // SLIPPYVIEW_H
