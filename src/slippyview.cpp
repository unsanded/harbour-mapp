#include "slippyview.h"

#include <QtQuick>
#include <math.h>

#include <src/tile.h>


using namespace std;


SlippyView::SlippyView(SlippyCache *cache, QQuickItem *parent) :
    QQuickItem(parent),
    currentLocation(4,3,3),
    cache(cache),
    m_tileManager(0)
{
    nodes.scrollTransform=0;
    matrices.zoomFromLevel=1;
#ifndef _DEBUG
    matrices.movementOffset=QPointF(-256,-256);
    setFlag(ItemClipsChildrenToShape);
#endif
    setFlag(ItemHasContents);
    setFlag(ItemAcceptsInputMethod);

}

void SlippyView::stepTile(int dx, int dy)
{
    int i,j;
    currentLocation.settilePos(currentLocation.tilePos()- QPoint(dx,dy));
    int tileX = currentLocation.tilePos().x();
    int tileY = currentLocation.tilePos().y();
    int zoom = currentLocation.zoom();

    qDebug() << "STEPPING " << dx << ',' << dy;
    if(dx<HTILEBUFFER && dy<VTILEBUFFER && dx>-HTILEBUFFER && dy>-VTILEBUFFER )
    {
        QRect field(0, 0, HTILEBUFFER, VTILEBUFFER);

        QRect moveBlockFrom(field.intersected(field.translated(-dx,-dy)));
        QRect moveBlockTo(moveBlockFrom.translated(dx,dy));
        int movePropagationX;
        int movePropagationY;
        int loopstartX;
        int loopstartY;


        qDebug() << "MOVING block" << moveBlockFrom;
        qDebug() << "    to block" << moveBlockTo;


        if(dy<=0)
        {
            loopstartY=0;
            movePropagationY=1;
        }
        else
        {
            loopstartY=VTILEBUFFER-1;
            movePropagationY=-1;
        }
        if(dx<=0)
        {
            loopstartX=0;
            movePropagationX=1;
        }
        else
        {
            loopstartX=HTILEBUFFER-1;
            movePropagationX=-1;
        }



        for(i=loopstartX;i<HTILEBUFFER && i>=0;i+=movePropagationX)
        for(j=loopstartY;j<VTILEBUFFER && j>=0;j+=movePropagationY)
        {
            qDebug() << "TILE" << i << j;
            if(!moveBlockFrom.contains(i,j, false)){//The tile is not necessary anymore, so drop its node;
                dropNodeQueue.insert(drawnTiles[i][j]);
                qDebug() << "   drop";
            }
            if(moveBlockTo.contains(i,j, false)){//we already have this tile, but somewhere else in the grid
                drawnTiles[i][j]=drawnTiles[i-dx][j-dy];
                qDebug() << "  move" << (i-dx) << (j-dy);
            }
            else
            {
                drawnTiles[i][j]=tileManager()->getTile(tileX+i,tileY+j, currentLocation.zoom());
                qDebug() << "  get";
            }

        }

    }
    else
        return reTile();
    //    else//stepped more than four, so drop all, and fill the matrix again
#ifdef _DEBUG
    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
    {
     //   dropNodeQueue.insert(drawnTiles[i][j]);
        if(drawnTiles[i][j]==tileManager()->getTile(tileX+i,tileY+j,zoom)){
        }else{
            qDebug() << "  NOT GOOD";
            qDebug() << "  tile is   " << drawnTiles[i][j]->coords;
            qDebug() << "  should be " << SlippyCoordinates(zoom,tileX+i, tileY+j);
        }

    }
#endif

    //this is just in case: make sure none of the tiles on the grid are scheduled for dropping
    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
        dropNodeQueue.remove(drawnTiles[i][j]);

    qDebug()<< "Nodes queued for dropping:" << dropNodeQueue.size();
}

void SlippyView::reTile()
{
    int i, j;
    int tileX = currentLocation.tilePos().x();
    int tileY = currentLocation.tilePos().y();
    int zoom = currentLocation.zoom();

    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
    {
        dropNodeQueue.insert(drawnTiles[i][j]);
        drawnTiles[i][j]=tileManager()->getTile(tileX+i,tileY+j, zoom);
    }

    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
        dropNodeQueue.remove(drawnTiles[i][j]);
    changes.gridChanged=true;
    changes.gridChanged=true;
}

void SlippyView::updateCompleteMatrix(bool upd)
{
    changes.matrixChanged=true;
    //if we have moved further than one tile
    if(!(QRectF(-1*256.0,-1*256.0,2*256.0,2*256.0)).contains(matrices.movementFromTile))
    {
        qDebug() << "stepping" << matrices.movementFromTile;
        //step alle the tiles one way
        stepTile(matrices.movementFromTile.x()/256,  matrices.movementFromTile.y()/256);

        //move the whole grid exactly the other way
        QPointF stepBack((int)(matrices.movementFromTile.x()/256),(int)(matrices.movementFromTile.y()/256) );
       stepBack*=256;

       //stepBack=matrices.zoom.map(stepBack);
       matrices.movementFromTile-=stepBack;


        qDebug() << "      to" << matrices.movementFromTile;
        //the grid has changed now
        changes.gridChanged=true;
    }

    matrices.move.setToIdentity();
    QVector3D movement( matrices.movementFromTile+matrices.movementOffset);

    matrices.move.translate(movement.x(), movement.y());

    if(matrices.zoomFromLevel<0.5)
    {
        //TODO: correct for zooming out halfway a tile


        currentLocation.setzoom(currentLocation.zoom()-1);

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.moveByPixels(-QPointF(128*halfWayX, 128*halfWayY));

        matrices.zoomFromLevel*=2;
        reTile();
    }
    if(matrices.zoomFromLevel>2)
    {

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.setzoom(currentLocation.zoom()+1);
        matrices.zoomFromLevel/=2;
        currentLocation.settilePos(currentLocation.tilePos()+QPoint(halfWayX, halfWayY));

        reTile();


    }
    matrices.zoom.setToIdentity();
    //matrices.zoom.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
    matrices.zoom.scale(matrices.zoomFromLevel);
    //matrices.zoom.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);




    //rotation about the rotationCenter
    matrices.rotate.setToIdentity();
    matrices.rotate.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
    matrices.rotate.rotate(matrices.rotation*180.0/M_PI, 0,0,1);
    matrices.rotate.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);

    matrices.complete=matrices.rotate*matrices.zoom*matrices.move;

    if(upd) update();
    //update here, because the following stuff is all gui-thread, instead of graphics

    matrices.inverses.zoomAndRotate=(matrices.zoom*matrices.rotate).inverted();

    matrices.inverses.complete=matrices.complete.inverted();

}

void SlippyView::onTileReady(Tile *tile)
{
   changes.tileChanged=true;
   changes.changedTiles.push(tile);
   update();
}

void SlippyView::setzoom(qreal arg)
{
    int intPart=arg;
    qreal fpart=pow(2, arg-intPart);
    if(intPart!=currentLocation.zoom())
    {
        currentLocation.setzoom(intPart);
        emit zoomLevelChanged(intPart);
    }

    if(fpart!=matrices.zoomFromLevel){
        matrices.zoomFromLevel=fpart;
        emit zoomChanged(arg);
    }
}

void SlippyView::setlocation(QGeoCoordinate arg)
{
    SlippyCoordinates nw(arg, currentLocation.zoom());

    if(nw.MercatorPos()!=currentLocation.MercatorPos())
    {
        currentLocation=nw;
        updateCompleteMatrix();
        emit locationChanged(arg);
    }
}

void SlippyView::setmapRotation(qreal arg)
{
    if(matrices.rotation==arg) return;

    matrices.rotation=arg;
    updateCompleteMatrix();
    emit rotationChanged();
}

void SlippyView::setlockRotation(bool arg)
{
    if (m_lockRotation != arg) {
        m_lockRotation = arg;
        emit lockRotationChanged(arg);
    }
}

void SlippyView::setlockZoom(bool arg)
{
    if (m_lockZoom != arg) {
        m_lockZoom = arg;
        emit lockZoomChanged(arg);
    }
}



void SlippyView::touchEvent(QTouchEvent *event)
{
    QPointF movementRelative ;
    QPointF pivot(event->touchPoints()[0].pos());

    if(event->touchPoints().size()>=1)
    {
        if(event->touchPointStates() == Qt::TouchPointPressed)
            return;
        event->accept();

        //these two should be in the tile vector space. That is the map before it is zoomed and rotated

            QPointF movementFrom= matrices.inverses.zoomAndRotate.map(event->touchPoints()[0].lastScreenPos());
            QPointF movementTo= matrices.inverses.zoomAndRotate.map(event->touchPoints()[0].screenPos());

            movementRelative = movementTo-movementFrom;

            movementRelative *=2;

            if(event->touchPoints().size()==1)
                updateCompleteMatrix();
    }
    if(event->touchPoints().size()==2){
        QVector2D oldVector(
            event->touchPoints()[1].lastPos()-
            event->touchPoints()[0].lastPos()
        );
        QVector2D vector(
            event->touchPoints()[1].pos()-
            event->touchPoints()[0].pos()
        );
        qreal zoomFactor = vector.length()/oldVector.length();

        matrices.movementFromTile-=(pivot*(zoomFactor-1));

        qreal angle=acos(QVector2D::dotProduct(vector.normalized(), QVector2D(1,0) ));
        if(vector.y()>0)
            angle=-angle;
        qreal oldAngle=acos(QVector2D::dotProduct(oldVector.normalized(), QVector2D(1,0) ));
        if(oldVector.y()>0)
            oldAngle=-oldAngle;


        angle-=oldAngle;

        matrices.rotation-=angle;


        //make sure we rotate about a finger instead of the Just the center

        //pivot is now the vector around wich to compensate rotation

        //pivot should be in the tile vector-space for this
//        pivot-=matrices.rotationCenter;
        pivot=matrices.inverses.zoomAndRotate.map(pivot);
        pivot-=matrices.inverses.zoomAndRotate.map(matrices.rotationCenter);




        QMatrix4x4 rotationRelative;
        //rotationRelative.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
        rotationRelative.rotate(angle*180.0/M_PI, 0,0,1);
        //rotationRelative.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);



        QPointF rotationCorrection(pivot);
        rotationCorrection=rotationRelative.map(rotationCorrection);
        rotationCorrection-=pivot;





        qDebug()<< "PIVOT" << pivot;
        qDebug()<< "rotationCorrection" << rotationCorrection;


        movementRelative+=rotationCorrection;


        qDebug() << "rotating by " << angle << " to " << matrices.rotation;


        matrices.zoomFromLevel*=zoomFactor;
        emit zoomChanged(log2(matrices.zoomFromLevel) + currentLocation.zoom());
        emit mapRotationChanged(matrices.rotation);

    }

    matrices.movementFromTile+=movementRelative;
    updateCompleteMatrix();
}


QSGNode *SlippyView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    QSGTransformNode* node;

    if(!oldNode)
    {
        node=new QSGTransformNode;

        //initialise transformation nodes for each tile;
        for(int i=0; i<HTILEBUFFER;i++)
        for(int j=0; j<VTILEBUFFER;j++)
        {
            changes.gridChanged=true;

           // drawnTiles[i][j]->makeNode(window())->appendChildNode(new QSGSimpleRectNode(QRectF(10,10,110,110),QColor(Qt::red)));

            node->markDirty(QSGNode::DirtyForceUpdate);
        }
    }

    else
        node=(QSGTransformNode*) oldNode;

    if(changes.matrixChanged){
        node->setMatrix(matrices.complete);
        node->markDirty(QSGNode::DirtyMatrix);
        changes.matrixChanged=false;
    }
    if(changes.gridChanged){
        changes.gridChanged=false;
        node->removeAllChildNodes();
        for(int i=0; i<HTILEBUFFER;i++)
        for(int j=0; j<VTILEBUFFER;j++)
        {
            qDebug() << "reAdding node" << i << j;
            QMatrix4x4 translation;
            translation.translate(i*256, j*256, 0);
#ifdef _DEBUG
            translation.scale(0.95);//to show the borders between the tiles
#endif
            node->appendChildNode(drawnTiles[i][j]->makeNode(window()));
            drawnTiles[i][j]->transformNode->setMatrix(translation);
            drawnTiles[i][j]->transformNode->markDirty(QSGNode::DirtyMatrix);
        }
        if(!dropNodeQueue.empty())
        {
            qDebug() << "dropping nodes";
            for( Tile* n : dropNodeQueue){
                n->dropNode();
            }
            dropNodeQueue.clear();
        }
    }
    if(changes.tileChanged){
        changes.tileChanged=false;
        for( Tile* n : changes.changedTiles){
            if(n->node)
                    n->makeNode(window());
        }
        changes.changedTiles.clear();
    }
    return node;
}

void SlippyView::classBegin()
{
    QQuickItem::classBegin();
}

void SlippyView::componentComplete()
{
    QQuickItem::componentComplete();
    connect(cache, SIGNAL(tileReady(Tile*)), this, SLOT(onTileReady(Tile*)));
    matrices.rotationCenter.setX(width()/2);
    matrices.rotationCenter.setY(height()/2);
    qDebug() << "Width: " << width();
    qDebug() << "height: " << height();

    if(!tileManager()){
        settileManager(TileManager::getDefault());
    }

    //we fill the grid here, and from now on, it will always be full of tiles
    for(int i=     0       ; i< HTILEBUFFER  ; i++)
    for(int j=     0       ; j< VTILEBUFFER  ; j++)
    {
        drawnTiles[i][j]=tileManager()->getTile(currentLocation.tilePos().x()+i,currentLocation.tilePos().y()+j, currentLocation.zoom());
    }
}

QGeoCoordinate SlippyView::location()
{
    return currentLocation;
}

qreal SlippyView::mapRotation() const
{
    return matrices.rotation;
}


void SlippyView::selectLayer(QString name)
{
    if(layers.caches.contains(name))
    {
        cache=layers.caches[name];
        reTile();
        update();
    }
}



