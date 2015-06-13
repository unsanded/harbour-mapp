#include "slippyview.h"

#include <QtQuick>
#include <math.h>

#include <src/tile.h>


using namespace std;


SlippyView::SlippyView(SlippyCache *cache, QQuickItem *parent) :
    QQuickItem(parent),
    currentLocation(4,3,3),
    cache(cache),
    m_tileManager(0),
    m_ready(false)
{
    nodes.scrollTransform=0;
    matrices.zoomFromLevel=1;
#ifndef _DEBUG
    matrices.movementOffset=QPointF(-512,-512);
    matrices.movementFromTile=matrices.movementOffset;
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
                queueForDropNode(drawnTiles[i][j]);
            }
            if(moveBlockTo.contains(i,j, false)){//we already have this tile, but somewhere else in the grid
                drawnTiles[i][j]=drawnTiles[i-dx][j-dy];
                qDebug() << "  move" << (i-dx) << (j-dy);
            }
            else
            {
                drawnTiles[i][j]=tileManager()->getTile(tileX+i,tileY+j, currentLocation.zoom());
                connect(
                            drawnTiles[i][j], SIGNAL(ready(Tile*)),
                            this			, SLOT(onTileReady(Tile*))
                            );
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
    qDebug() << "retiling";
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
    changes.matrixChanged=true;

    //ok, so stepping is the point in tile-coordinates that has to stay withing the bounding box of 256
    QPointF stepping(matrices.movementFromTile-matrices.movementOffset);
    if(!(QRectF(-1*256.0,-1*256.0,2*256.0,2*256.0)).contains(stepping))
    {
       qDebug() << "stepping" << matrices.movementFromTile;
       //step alle the tiles one way
       if(ready())
        stepTile(stepping.x()/256, stepping.y()/256);

       //move the whole grid exactly the other way
       QPointF stepBack((int)(stepping.x()/256),(int)(stepping.y()/256) );
       stepBack*=256;

       //stepBack=matrices.zoom.map(stepBack);
       matrices.movementFromTile-=stepBack;


        qDebug() << "      to" << matrices.movementFromTile;
        //the grid has changed now
        changes.gridChanged=true;
    }

    matrices.move.setToIdentity();
    QVector3D movement( matrices.movementFromTile);

    matrices.move.translate(movement.x(), movement.y());
    

    if(matrices.zoomFromLevel<0.5)
    {
        //TODO: correct for zooming out halfway a tile


        currentLocation.setzoom(currentLocation.zoom()-1);

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.moveByPixels(-QPointF(128*halfWayX, 128*halfWayY));

        matrices.zoomFromLevel*=2;
        matrices.movementFromTile/=2;
        reTile();
    }
    if(matrices.zoomFromLevel>2)
    {

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        currentLocation.setzoom(currentLocation.zoom()+1);
        matrices.zoomFromLevel/=2;
        matrices.movementFromTile*=2;
        currentLocation.settilePos(currentLocation.tilePos()+QPoint(halfWayX, halfWayY));

        reTile();


    }
    matrices.zoom.setToIdentity();
    matrices.zoom.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
    matrices.zoom.scale(matrices.zoomFromLevel);
    matrices.zoom.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);




    //rotation about the rotationCenter
    matrices.rotate.setToIdentity();
    matrices.rotate.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
    matrices.rotate.rotate(matrices.rotation*180.0/M_PI, 0,0,1);
    matrices.rotate.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);

    //this is the final matrix from tile to screen
    matrices.complete=matrices.rotate*matrices.zoom*matrices.move;

    if(upd && ready()) update();
    //update here, because the following stuff is all gui-thread, instead of graphics

    matrices.inverses.zoomAndRotate = (matrices.zoom*matrices.rotate).inverted();

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
    matrices.zoomFromLevel = pow(2, arg-currentLocation.zoom());
    if(ready())
       updateCompleteMatrix(true);


}

void SlippyView::onLayerChanged(int layerIndex)
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



void SlippyView::touchEvent(QTouchEvent *event)
{
    QPointF movementRelative ;
    QPointF pivot(event->touchPoints()[0].pos());
    qDebug() <<"pivot "<< pivot;

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

        pivot-=matrices.rotationCenter;
        pivot=matrices.inverses.zoomAndRotate.map(pivot);
        qDebug() << "pivot length: " << (pivot + matrices.movementFromTile);
        matrices.movementFromTile += matrices.inverses.zoomAndRotate.map(pivot + matrices.movementFromTile)*(1-zoomFactor);



        if(!lockRotation())
        {

            qreal angle=acos(QVector2D::dotProduct(vector.normalized(), QVector2D(1,0) ));
            if(vector.y()>0)
                angle=-angle;
            qreal oldAngle=acos(QVector2D::dotProduct(oldVector.normalized(), QVector2D(1,0) ));
            if(oldVector.y()>0)
                oldAngle = -oldAngle;


            angle -= oldAngle;

            matrices.rotation -= angle;


            //make sure we rotate about a finger instead of the Just the center

            //pivot is now the vector around wich to rotate

            QMatrix4x4 rotationRelative;
            //rotationRelative.translate( matrices.rotationCenter.x(), matrices.rotationCenter.y(), 0);
            rotationRelative.rotate(angle*180.0/M_PI, 0,0,1);
            //rotationRelative.translate(-matrices.rotationCenter.x(),-matrices.rotationCenter.y(), 0);

            QPointF rotationCorrection(pivot);
            rotationCorrection=rotationRelative.map(rotationCorrection);
            rotationCorrection-=pivot;



            qDebug()<< "PIVOT" << pivot;
            qDebug()<< "rotationCorrection" << rotationCorrection;

//            movementRelative+=rotationCorrection;
            qDebug() << "rotating by " << angle << " to " << matrices.rotation;
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

    matrices.movementFromTile += movementRelative;
    updateCompleteMatrix();
}


QSGNode *SlippyView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
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

        node->markDirty(QSGNode::DirtyMaterial);
        qDebug() << "node changed";
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
    qDebug() << "Width: " << width();
    qDebug() << "height: " << height();

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




