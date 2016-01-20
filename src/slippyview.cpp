#include "slippyview.h"

#include <QtQuick>
#include <math.h>

#include <src/tile.h>


using namespace std;


SlippyView::SlippyView(SlippyCache *cache, QQuickItem *parent) :
    QQuickItem(parent),
    m_tileManager(0),
    m_ready(false),
    currentLocation(4,3,3),
    cache(cache)
{
    matrices.zoomFromLevel=1;
#ifndef _DEBUG
    matrices.movementOffset = QPointF(-512,-512);
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

    //qDebug() << "STEPPING " << dx << ',' << dy;
    if(dx<HTILEBUFFER && dy<VTILEBUFFER && dx>-HTILEBUFFER && dy>-VTILEBUFFER )
    {
        QRect field(0, 0, HTILEBUFFER, VTILEBUFFER);

        QRect moveBlockFrom(field.intersected(field.translated(-dx,-dy)));
        QRect moveBlockTo(moveBlockFrom.translated(dx,dy));
        int movePropagationX;
        int movePropagationY;
        int loopstartX;
        int loopstartY;


        //qDebug() << "MOVING block" << moveBlockFrom;
        //qDebug() << "    to block" << moveBlockTo;


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
            //qDebug() << "TILE" << i << j;
            if(!moveBlockFrom.contains(i,j, false)){//The tile is not necessary anymore, so drop its node;
                queueForDropNode(drawnTiles[i][j]);
            }
            if(moveBlockTo.contains(i,j, false)){//we already have this tile, but somewhere else in the grid
                drawnTiles[i][j]=drawnTiles[i-dx][j-dy];
                //qDebug() << "  move" << (i-dx) << (j-dy);
            }
            else
            {
                drawnTiles[i][j]=tileManager()->getTile(tileX+i,tileY+j, currentLocation.zoom());
                connect(
                            drawnTiles[i][j], SIGNAL(ready(Tile*)),
                            this			, SLOT(onTileReady(Tile*))
                            );
                //qDebug() << "  get";
            }
        }
    }
    else
        return reTile();
    //    else//stepped more than four, so drop all, and fill the matrix again
#ifdef _DEBUG
    //do some sanity checks, but only if compiled with debug
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

    //qDebug()<< "Nodes queued for dropping:" << dropNodeQueue.size();
}

void SlippyView::reTile()
{
    //qDebug() << "retiling";
    int i, j;
    int tileX = currentLocation.tilePos().x();
    int tileY = currentLocation.tilePos().y();
    int zoom = currentLocation.zoom();

    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
    {
        queueForDropNode(drawnTiles[i][j]);
        drawnTiles[i][j]=tileManager()->getTile(tileX+i,tileY+j, zoom);
        changes.changedTiles.append(drawnTiles[i][j]);
    }

    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
    {
        dropNodeQueue.remove(drawnTiles[i][j]);
            connect(
                drawnTiles[i][j], SIGNAL(ready(Tile*)),
                this			, SLOT(onTileReady(Tile*))
            );
    }

    changes.gridChanged=true;
    changes.tileChanged=true;
}

void SlippyView::updateCompleteMatrix(bool upd)
{
    qDebug() << "updateIng matrix";
    changes.matrixChanged=true;

    if (changes.movementChanged){
        changes.movementChanged=false;

        //ok, so stepping is the point in tile-coordinates that has to stay withing the bounding box of 256
        QPointF stepping(matrices.movementFromTile);
        if(!(QRectF(-1*256.0,-1*256.0,2*256.0,2*256.0)).contains(stepping))
        {
           //qDebug() << "stepping" << matrices.movementFromTile;
           //step all the tiles one way
           if(ready())
           {
            stepTile(stepping.x()/256, stepping.y()/256);

            //move the whole grid exactly the other way
            QPointF stepBack((int)(stepping.x()/256),(int)(stepping.y()/256) );
            stepBack*=256;

            //stepBack=matrices.zoom.map(stepBack);
            matrices.movementFromTile -= stepBack;
            matrices.move.setToIdentity();
            matrices.move.translate(matrices.movementFromTile.x(), matrices.movementFromTile.y(), 0);
            //qDebug() << "      to" << matrices.movementFromTile;
            //the grid has changed now
            changes.gridChanged=true;
           }
        }
    }

    if(matrices.zoomFromLevel<0.5)
    {
        //TODO: correct for zooming out halfway a tile


        currentLocation.setzoom(currentLocation.zoom()-1);

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.moveByPixels(-QPointF(TILE_SIZE/2 * halfWayX, TILE_SIZE/2 * halfWayY));

        matrices.zoomFromLevel*=2;
        matrices.movementFromTile/=2;
        reTile();
    }

    else
    if(matrices.zoomFromLevel>2)
    {

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.setzoom(currentLocation.zoom()+1);
        matrices.zoomFromLevel/=2;
        matrices.movementFromTile*=2;
        currentLocation.moveByPixels( TILE_SIZE/2 * QPointF(halfWayX, halfWayY)  );

        reTile();

    }
    matrices.zoom.setToIdentity();
    matrices.zoom.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
    matrices.zoom.scale(matrices.zoomFromLevel);
    matrices.zoom.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);




    //rotation about the rotationCenter

    //this is the final matrix from tile to screen
    matrices.complete=matrices.rotate*matrices.zoom*matrices.move;

    if(upd && ready()) update();
    //update here, because the following stuff is all gui-thread, instead of graphics

    matrices.inverses.zoomAndRotate = (matrices.zoom*matrices.rotate).inverted();

    matrices.inverses.complete=matrices.complete.inverted();

}

void SlippyView::rotateBy(qreal radians, QPointF pivot)
{
    changes.rotationChanged = true;
    QMatrix4x4 rotationRelative;
    //rotate about the center so that the grid won't rotate out of view
    rotationRelative.rotate(-radians*180.0/M_PI, 0,0,1);

    matrices.rotation -= radians;
    //todo: make more efficient.
    matrices.rotate.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);
    matrices.rotate = rotationRelative * matrices.rotate;
    matrices.rotate.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);

    updateInverseMatrices();

    pivot -= matrices.rotationCenter;

    // How much does the pivot move as a qonsequence of rotation.
    QPointF rotationCorrection(pivot);
    rotationCorrection = rotationRelative.map(rotationCorrection);
    rotationCorrection-= pivot;

    // move back by this amount.
//    moveBy(-rotationCorrection);
}

void SlippyView::zoomBy(qreal zoom, QPointF pivot)
{

}

void SlippyView::moveBy(QPointF by)
{
    by = matrices.inverses.zoomAndRotate.map(by);
    matrices.movementFromTile += by;
    matrices.move.translate(by.x(), by.y());
    changes.movementChanged = true;
    updateCompleteMatrix();
}

void SlippyView::onTileReady(Tile *tile)
{
   changes.tileChanged=true;
   changes.changedTiles.push(tile);
   update();
}

void SlippyView::setzoom(qreal arg)
{
    matrices.zoomFromLevel = pow(2, arg-currentLocation.zoom());
    if(ready())
       updateCompleteMatrix(true);


}

void SlippyView::onLayerChanged(int /*layerIndex*/)
{
    if(!ready())
    {
        qDebug() << "Map becomes ready";
        m_ready=true;
        emit readyChanged(true);
        emit becomesReady();
    }
    reTile();
    update();
}

void SlippyView::setlocation(QGeoCoordinate arg)
{
    SlippyCoordinates nw(arg, currentLocation.zoom());

    if(!nw.valid())return;

    if(nw.MercatorPos()!=currentLocation.MercatorPos())
    {
        currentLocation=nw;
        matrices.movementFromTile=nw.tileOffset()*TILE_SIZE;
        reTile();
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



//There is a bug somewhere which causes touchpoint::oldPos()
//to not actually be the pos from the previous call to this function.
//This is why oldTouchPoints is used to keep track of the last position.
void SlippyView::touchEvent(QTouchEvent *event)
{
    QPointF movementRelative ;
    QPointF pivot(event->touchPoints()[0].pos());
    QPointF oldPivot(pivot);
    int pivotId = event->touchPoints()[0].id();

    
    bool numberChanged = false;
    //Manage touch point memory
    if (event->touchPointStates() == Qt::TouchPointReleased){
        //completely released all fingers
        oldTouchPoints.clear();
        event->accept();
        return;
    }
    //insert and remove new and old touchpoints
    for (auto point : event->touchPoints()){
        qDebug() << "point " << point.id();
        if(point.state() == Qt::TouchPointPressed){
            qDebug()<< "\tadded";
            oldTouchPoints.insert(point.id(), point.pos());
            numberChanged=true;
        }
        else if(point.state() == Qt::TouchPointReleased){
            qDebug()<< "\temoved";
            int res = oldTouchPoints.remove(point.id());
            qDebug()<< "res " << res;
            numberChanged=true;
        }
    }


    //don't do moving on touch/release events
    if(numberChanged)
    {
        qDebug()<< "now got " << oldTouchPoints.size() << " points";
        event->accept();
        //TODO: tap recognition
        return;
    }

    if(oldTouchPoints.contains(pivotId)){
        oldPivot = oldTouchPoints[pivotId];
    }

    if(event->touchPoints().size()>=1)
    {
        event->accept();

        //these two should be in the tile vector space. That is the map before it is zoomed and rotated

        QPointF movementFrom= matrices.inverses.zoomAndRotate.map(oldPivot);
        QPointF movementTo  = matrices.inverses.zoomAndRotate.map(event->touchPoints()[0].screenPos());
        movementRelative = movementTo - movementFrom;

        moveBy(movementRelative);

    }
    if(event->touchPoints().size()==2){
        
        QPointF oldManu(oldTouchPoints[event->touchPoints()[1].id()]);

        //what it used to be.
        QVector2D oldVector(
            oldManu -
            oldPivot
        );
        QVector2D vector(
            event->touchPoints()[1].pos() -
            event->touchPoints()[0].pos()
        );

        qreal zoomFactor = vector.length()/oldVector.length();

        pivot -= matrices.rotationCenter;
        pivot  = matrices.inverses.zoomAndRotate.map(pivot);

        matrices.movementFromTile += matrices.inverses.zoomAndRotate.map(pivot + matrices.movementFromTile)*(1-matrices.zoomFromLevel);



        if(!lockRotation())
        {
            qreal angle=acos(QVector2D::dotProduct(vector.normalized(), QVector2D(1,0) ));
            if(vector.y()>0)
                angle=-angle;
            qreal oldAngle=acos(QVector2D::dotProduct(oldVector.normalized(), QVector2D(1,0) ));
            if(oldVector.y()>0)
                oldAngle = -oldAngle;


            angle -= oldAngle;

            rotateBy(angle, pivot);



        }


        if(!lockZoom())
        {
            matrices.zoomFromLevel*=zoomFactor;
            //don't change the zoomlevel here. That happens in updateCompleteMatrix
            emit zoomChanged(log2(matrices.zoomFromLevel) + currentLocation.zoom());
        }
        if(!lockRotation())
            emit mapRotationChanged(matrices.rotation);

    }

    qDebug() << "MOV:" << matrices.movementFromTile << "\t" << movementRelative;
    qDebug() << "ZOM:" << matrices.zoomFromLevel << "\t" << zoom();
    
    for (auto point : event->touchPoints() ) {
       if (point.state() == Qt::TouchPointMoved)
        {
           qDebug() << "moved " << point.id();
           oldTouchPoints[point.id()] = point.pos();
        }
    }
}


QSGNode *SlippyView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData* /*data*/)
{
    QSGTransformNode* node;

    if(!ready()) return 0;

    if(!oldNode)
    {
        node=new QSGTransformNode;

        //initialise transformation nodes for each tile;
        changes.gridChanged=true;
        node->markDirty(QSGNode::DirtyForceUpdate);
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
            //qDebug() << "reAdding node" << i << j;
            QMatrix4x4 translation;
            translation.translate(i*256 + matrices.movementOffset.x(), j*256 + matrices.movementOffset.y(), 0);
#ifdef _DEBUG
            translation.scale(0.95);//to show the borders between the tiles
#endif
            node->appendChildNode(drawnTiles[i][j]->makeNode(window()));
            drawnTiles[i][j]->transformNode->setMatrix(translation);
            drawnTiles[i][j]->transformNode->markDirty(QSGNode::DirtyMatrix);
        }
        if(!dropNodeQueue.empty())
        {
            //qDebug() << "dropping nodes";
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

        node->markDirty(QSGNode::DirtyMaterial);
        //qDebug() << "node changed";
    }
    return node;
}

void SlippyView::queueForDropNode(Tile *tile)
{
    dropNodeQueue.insert(tile);
    disconnect(tile, SIGNAL(ready(Tile*)),
               this, SLOT(onTileReady(Tile*))
                );
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
    //qDebug() << "Width: " << width();
    //qDebug() << "height: " << height();

    if(!tileManager()){
        settileManager(TileManager::getDefault());
    }

    //we fill the grid here, and from now on, it will always be full of tiles
    for(int i=     0       ; i< HTILEBUFFER  ; i++)
    for(int j=     0       ; j< VTILEBUFFER  ; j++)
    {
        drawnTiles[i][j]=new Tile(currentLocation + QPoint(i,j));
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




