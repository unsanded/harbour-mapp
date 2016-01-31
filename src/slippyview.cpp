#include "slippyview.h"

#include <QtQuick>
#include <math.h>

#include <src/tile.h>


using namespace std;



SlippyView::SlippyView( QQuickItem *parent) :
    QQuickItem(parent),
    m_lockRotation(false),
    m_lockZoom(false),
    m_tileManager(0),
    m_ready(false),
    currentLocation(4,3,3)
{
    debug.debugPos = QRectF(0,0, 20, 20);
    debug.debugPos2 = QRectF(10,10, 30, 30);
    setFlag(ItemHasContents);
    setFlag(ItemAcceptsInputMethod);

#ifndef _NO_MOUSE
    setAcceptedMouseButtons(Qt::AllButtons);
#endif

    connect(this, SIGNAL(widthChanged()), this, SLOT(onResize()));
    connect(this, SIGNAL(heightChanged()), this, SLOT(onResize()));

    movement.stepBounds =
                QRectF(HTILEBUFFER*HTILESIZE/2 - HTILESIZE,
                       VTILEBUFFER*VTILESIZE/2 - HTILESIZE,
                       HTILESIZE*2,
                       VTILESIZE*2);

}

void SlippyView::stepTile(int dx, int dy)
{
    int i,j;

    if(dx==0 && dy ==0)
    {
        qDebug() << "STEPPING ZERO";
        return;
    }
    changes.tileChanged = true;
    changes.gridChanged = true;

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
    qDebug() << "updateing matrix";

    //ok, so stepping is the point in tile-coordinates that has to stay withing the bounding box of 256

    QPointF stepping( realToTile(movement.rotationCenter) );


    debug.debugPos .moveTo( stepping );
    debug.debugPos2 = movement.stepBounds;

    // check if we have to step to a different tile.
    if(!movement.stepBounds.contains(stepping))
    {
        qDebug() << "stepping" << stepping;
        if(ready())
        {
            stepping -= movement.stepBounds.center();

            //step all the tiles one way
            stepTile(stepping.x()/-256, stepping.y()/-256);

            //move the whole grid exactly the other way
            QPointF stepBack( (int)(stepping.x()/256),(int)(stepping.y()/256) );
            stepBack *= 256;

//            stepBack = movement.navigationMatrix.map(stepBack);

            QMatrix4x4 backStep;
            backStep.translate(stepBack.x(), stepBack.y());

            movement.navigationMatrix.translate(stepBack.x(), stepBack.y(),0);

            //movement.navigationMatrix = backStep * movement.navigationMatrix;
        }
    }

    qreal factor =  zoomFactor();

    if(factor < 0.5)
    {
        //TODO: correct for zooming out halfway a tile

        currentLocation.setzoom(currentLocation.zoom()-1);

        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        //currentLocation.moveByPixels(-QPointF(TILE_SIZE * halfWayX, TILE_SIZE * halfWayY));

        movement.navigationMatrix.scale(2.0);
        // move x and y half a tile, if the tile position of the previous layer was odd (ie. we're now halfway a tile for that direcion);
        movement.navigationMatrix.translate(QVector3D(tileToReal(
        {(qreal)(halfWayX*TILE_SIZE/2.),
         (qreal)(halfWayY*TILE_SIZE/2.)})));
        reTile();
    }
    else
    if(factor > 2)
    {
        bool halfWayX = currentLocation.tilePos().x()&1;//so, we are halfway the tile while zooming out
        bool halfWayY = currentLocation.tilePos().y()&1;

        qDebug() << "halfway" << halfWayX << halfWayY;

        currentLocation.setzoom(currentLocation.zoom()+1);
        movement.navigationMatrix.scale(0.5);

        QMatrix4x4 zoomFix;
        zoomFix.translate(QVector3D(TILE_SIZE * QPointF(-halfWayX, -halfWayY)));

//        movement.navigationMatrix = zoomFix * movement.navigationMatrix;

        reTile();
    }



    //rotation about the rotationCenter

    //this is the final matrix from tile to screen
    movement.completeMatrix = movement.perspectiveMatrix * movement.navigationMatrix;

    changes.matrixChanged = true;
    if(upd && ready()) update();
    //update here, because the following stuff is all gui-thread, instead of graphics

    updateInverseMatrices();
}

void SlippyView::rotateBy(qreal radians, QPointF pivot)
{
    QMatrix4x4 rotate;

    rotate.translate( pivot.x(),  pivot.y());
    rotate.rotate   (radians*180. * M_1_PI, 0, 0, 1 );
    rotate.translate(-pivot.x(), -pivot.y());

    // insert rotation before existing transformation
    movement.navigationMatrix = rotate * movement.navigationMatrix;

    changes.matrixChanged = true;
    updateCompleteMatrix();
}

void SlippyView::zoomBy(qreal zoom, QPointF pivot)
{
    QMatrix4x4 scale;
    scale.translate( pivot.x(),  pivot.y());
    scale.scale(zoom);
    scale.translate(-pivot.x(), -pivot.y());

    movement.navigationMatrix = scale * movement.navigationMatrix;

    changes.matrixChanged=true;
    updateCompleteMatrix();
}

void SlippyView::moveBy(QPointF by)
{
    QMatrix4x4 move;
    move.translate(by.x(), by.y());
    //multiply before in stead of after;
    movement.navigationMatrix = move * movement.navigationMatrix;
    updateCompleteMatrix();
}

QPointF SlippyView::tileToReal(QPointF tileCoords)
{
    return movement.navigationMatrix.map(tileCoords);
}

QPointF SlippyView::realToTile(QPointF realCoords)
{
    return movement.inverses.complete.map(realCoords);
}

void SlippyView::onResize()
{
    movement.rotationCenter.rx() = width()/2;
    movement.rotationCenter.ry() = height()/2;
}

void SlippyView::onTileReady(Tile *tile)
{
   changes.tileChanged=true;
   changes.changedTiles.push(tile);
   update();
}

void SlippyView::setzoom(qreal arg)
{
    qreal zoomFromLevel = pow(2, arg-currentLocation.zoom());

    //TODO

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
   //     movement.movementFromTile = nw.tileOffset()*TILE_SIZE;
        reTile();
        updateCompleteMatrix();
        emit locationChanged(arg);
    }
}

void SlippyView::setmapRotation(qreal arg)
{
    //TODO
}

void SlippyView::setlockRotation(bool arg)
{
    //TODO
}

void SlippyView::setlockZoom(bool arg)
{
    if (m_lockZoom != arg) {
        m_lockZoom = arg;
        emit lockZoomChanged(arg);
    }
}

void SlippyView::settileManager(TileManager *arg)
{
    if (m_tileManager != arg) {

        if(m_tileManager)
            disconnect(m_tileManager, SIGNAL(activeLayerChanged(int)),
                       this, SLOT(onLayerChanged(int)));
        m_tileManager = arg;

        connect(m_tileManager, SIGNAL(activeLayerChanged(int)),
                this      , SLOT(onLayerChanged(int)));

        emit tileManagerChanged(arg);
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

        QPointF movementFrom= oldPivot;
        QPointF movementTo  = event->touchPoints()[0].pos();

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


        if(!lockRotation())
        {
            qreal angle=acos(QVector2D::dotProduct(vector.normalized(), QVector2D(1,0) ));
            if(vector.y()>0)
                angle=-angle;
            qreal oldAngle=acos(QVector2D::dotProduct(oldVector.normalized(), QVector2D(1,0) ));
            if(oldVector.y()>0)
                oldAngle = -oldAngle;

            angle -= oldAngle;

            rotateBy(-angle, pivot);



        }


        if(!lockZoom())
        {
            zoomBy(zoomFactor, pivot);
        }

    }

    for (auto point : event->touchPoints() )
    if (point.state() == Qt::TouchPointMoved)
    {
       oldTouchPoints[point.id()] = point.pos();
    }
}


QSGNode *SlippyView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData* /*data*/)
{
    QSGTransformNode* node;

    if(!ready()) return 0;

    if(!oldNode)
    {
        node=new QSGTransformNode;

        debug.debugNode = new QSGSimpleRectNode(debug.debugPos, QColor(255, 0,0,128));
        debug.debugNode2= new QSGSimpleRectNode(debug.debugPos2, QColor(0, 255,0,128));

        //initialise transformation nodes for each tile;
        changes.gridChanged=true;
        node->markDirty(QSGNode::DirtyForceUpdate);
    }
    else
        node=(QSGTransformNode*) oldNode;


    if(changes.matrixChanged){
        node->setMatrix(movement.navigationMatrix);
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
            translation.translate(i*256 , j*256 , 0);
#ifdef _DEBUG
            translation.scale(0.95);//to show the borders between the tiles
#endif
            node->appendChildNode(drawnTiles[i][j]->makeNode(window()));
            drawnTiles[i][j]->transformNode->setMatrix(translation);
            drawnTiles[i][j]->transformNode->markDirty(QSGNode::DirtyMatrix);
        }
        node->appendChildNode(debug.debugNode );
        node->appendChildNode(debug.debugNode2);


        if(!dropNodeQueue.empty())
        {
            //qDebug() << "dropping nodes";
            for( Tile* n : dropNodeQueue){
                n->dropNode();
            }
            dropNodeQueue.clear();
        }
    }

    debug.debugNode ->markDirty(QSGNode::DirtyForceUpdate);
    debug.debugNode2->markDirty(QSGNode::DirtyForceUpdate);
    debug.debugNode ->setRect(debug.debugPos);
    debug.debugNode2->setRect(debug.debugPos2);

    if(changes.tileChanged){
        changes.tileChanged = false;
        for( Tile* n : changes.changedTiles){
            if(n->node)
                    n->makeNode(window());
        }
        changes.changedTiles.clear();

        node->markDirty(QSGNode::DirtyMaterial);
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
    movement.rotationCenter.setX( width() /2);
    movement.rotationCenter.setY( height()/2);
    qDebug() << "Width: "  << width();
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
    QVector3D reference(1., 0., 0.);
    QVector3D rotated = movement.navigationMatrix.map(reference);

    if(rotated.y() > 0)
        return acos(rotated.x());
    else
        return M_PI*2 - acos(rotated.x());
}

void SlippyView::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    qDebug() << "PRESSED";
    movement.lastMousePos = event->pos();
}

#ifndef _NO_MOUSE
void SlippyView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)){
        return;
    }
    event->accept();
    QPointF movedBy = event -> pos() - movement.lastMousePos;
    moveBy(movedBy);
    movement.lastMousePos = event->pos();


}

void SlippyView::wheelEvent(QWheelEvent *event)
{
    qDebug() << "WHEEEL";

    qreal angle = event->angleDelta().y();
    angle /= 1800/M_PI;


    zoomBy(1.0 + angle, event->pos());

}
#endif


