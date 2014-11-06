#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include <QQuickItem>
#include <QStack>

#include <src/slippycache.h>
#include <src/providers/osmprovider.h>



#ifdef _DEBUG
#define HTILEBUFFER (3)
#define VTILEBUFFER (3)
#else
#define HTILEBUFFER (5)
#define VTILEBUFFER (5)
#endif

class SlippyView : public QQuickItem
{
    Q_OBJECT
public:
    explicit SlippyView(SlippyCache* cache=new SlippyCache(new OsmProvider),QQuickItem *parent = 0);

    SlippyCoordinates currentLocation;


protected:
    //this struct can only be used in the UpdatePaintNode functions thread
    struct {
        QSGTransformNode* scrollTransform;
       // QSGTransformNode* tileTransform[HTILEBUFFER][VTILEBUFFER];

    } nodes;
    //que of tiles whose node is to be dropped in updatePaintNode()
    //not in nodes, because this is allowed to be used outside of updatePaintnode
    QSet<Tile*> dropNodeQueue;

    struct {
        QPointF movementOffset;//offset so that the edge of the grid stays off the screen
        QPointF movementFromTile;//if this becomes more than one in either direction we have to swap tiles
        QPointF rotationCenter; //the map rotate around this point
        QMatrix4x4 move;

        qreal zoomFromLevel;
        QMatrix4x4 zoom;

        qreal rotation;
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

    SlippyCache* cache;

    //steps the tiles in the grid with offsets dx and dy
    //reuses as many tiles as possible;
    void stepTile(int dx, int dy);
    //fill the grid again from scratch
    void reTile();

    void updateCompleteMatrix(bool update=true);




signals:

public slots:

    void onTileReady(Tile* tile);

    // QQuickItem interface
protected:
    virtual void touchEvent(QTouchEvent *event);
    virtual QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data);

    // QQmlParserStatus interface
public:
    virtual void classBegin();
    virtual void componentComplete();
};

#endif // SLIPPYVIEW_H
