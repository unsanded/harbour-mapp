#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include <QQuickItem>
#include <QStack>

#include <src/slippycache.h>
#include <src/providers/osmprovider.h>



#define HTILEBUFFER (5)
#define VTILEBUFFER (5)

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
    //not in nodes, because this is allowed to be used outside of updatePaintnode
    QSet<Tile*> dropNodeQueue;

    struct {
        QMatrix4x4 zoom;
        QMatrix4x4 move;
        QMatrix4x4 rotate;
        QMatrix4x4 complete;
        QPointF movementFromTile;//if this becomes more than one in either direction we have to swap tiles

    } matrices;

    struct {
        bool matrixChanged;
        bool gridChanged;
        bool tileChanged;
    QStack<Tile*> changedTiles;
    } changes;

    Tile* drawnTiles[HTILEBUFFER][VTILEBUFFER];

    SlippyCache* cache;

    void stepTile(int dx, int dy);

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
