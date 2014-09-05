#include "slippyview.h"

#include <QtQuick>
#include <math.h>

#include <src/tile.h>

using namespace std;


SlippyView::SlippyView(SlippyCache *cache, QQuickItem *parent) :
    QQuickItem(parent),
    cache(cache),
    currentLocation(4,3,3)
{
    matrices.move.translate(-256, -128,0);
    nodes.scrollTransform=0;
    setFlag(ItemHasContents);
    setFlag(ItemClipsChildrenToShape);
    for(int i=     0       ; i< HTILEBUFFER  ; i++)
    for(int j=     0       ; j< VTILEBUFFER  ; j++)
    {
        drawnTiles[i][j]=cache->getTile(currentLocation.tilePos().x()+i,currentLocation.tilePos().y()+j, currentLocation.zoom());
    }
}

void SlippyView::stepTile(int dx, int dy)
{
    int i,j;
    currentLocation.settilePos(currentLocation.tilePos()+ QPoint(dx,dy));
    int tileX = currentLocation.tilePos().x();
    int tileY = currentLocation.tilePos().y();
    int zoom = currentLocation.zoom();
    
    qDebug() << "STEPPING " << dx << ',' << dy;
    if(dx<HTILEBUFFER && dy<VTILEBUFFER && dx>-HTILEBUFFER && dy>-VTILEBUFFER && false)
    {
        QStack<QSGTransformNode*> freenodes;

        //drop tiles that move off the grid
        for(i=(dx<0?0:HTILEBUFFER-dx); i< (dx<0?dx:HTILEBUFFER); i++)// the tiles next to the block that can be moved
        for(j=     0       ; j< VTILEBUFFER          ; j++)// over  the full hight
        {
            qDebug() << "DEL " << i << j;
            freenodes.push(drawnTiles[i][j]->transformNode);
            drawnTiles[i][j]->dropNode();
        }

        for(i=(dx<0?0:dx); i<(dx<0?dx+HTILEBUFFER:HTILEBUFFER); i++)    // the tiles above or below the block that can be moved
        for(j=(dy<0?0:VTILEBUFFER-dy); j< (dy<0?-dy:VTILEBUFFER); j++)
        {
            qDebug() << "DEL " << i << j;
            freenodes.push(drawnTiles[i][j]->transformNode);
            drawnTiles[i][j]->dropNode();
        }

        
        //move the tiles that can be moved
        for(i=min(VTILEBUFFER-1, dx+HTILEBUFFER); i>=max(0,dx); i--)
        for(j=min(VTILEBUFFER-1, dy+VTILEBUFFER); j>=max(0,dy); j--)
        {
            qDebug() << "MOVE" << i-dx << j-dy << "to" << i << j;
            drawnTiles[i][j]=drawnTiles[i-dx][j-dy];
        }
                    
        //get the new tiles
        for(i=(dx>=0)?0:dx+HTILEBUFFER;  i<(dx>0?dx:HTILEBUFFER)   ;i++)//the tiles next to the block that has been moved
        for(j=0;j<VTILEBUFFER;j++)//over the full height
        {
            qDebug() << "ADD " << i << j;
            drawnTiles[i][j]=cache->getTile(tileX+i,tileY+j,zoom);
            drawnTiles[i][j]->transformNode=freenodes.pop();//give it one of the available transformnodes
        }
        
        for(i=(dx>=0)?dx:0;  i<(dx>0?HTILEBUFFER:dx)   ;i++)//the tiles above or below the block that has been moved
        for(j=(dy<0)?dy+VTILEBUFFER:0;j<(dy>0?dy:VTILEBUFFER);j++)
        {
            qDebug() << "ADD " << i << j;
            drawnTiles[i][j]=cache->getTile(tileX+i,tileY+j,zoom);
            drawnTiles[i][j]->transformNode=freenodes.pop();//give it one of the available transformnodes
        }
    }
    else//stepped more than four, so drop all, and fill the matrix again
    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
    {
        qDebug() << "RES " << i << j;
        dropNodeQueue.insert(drawnTiles[i][j]);
        drawnTiles[i][j]=cache->getTile(tileX+i,tileY+j,zoom);
    }

    for(i=0;i<HTILEBUFFER;i++)
    for(j=0;j<VTILEBUFFER;j++)
        dropNodeQueue.remove(drawnTiles[i][j]);

    qDebug()<< "Nodes queued for dropping:" << dropNodeQueue.size();
}

void SlippyView::updateCompleteMatrix(bool upd)
{
    changes.matrixChanged=true;
    if(!QRectF(-1,-1,2,2).contains(matrices.movementFromTile))
    {
        stepTile((int) matrices.movementFromTile.x(), (int) matrices.movementFromTile.y());
        matrices.move.translate((int)matrices.movementFromTile.x()*256, (int)matrices.movementFromTile.y()*256, 0);
        matrices.movementFromTile.rx()-=(int)matrices.movementFromTile.x();
        matrices.movementFromTile.ry()-=(int)matrices.movementFromTile.y();
        changes.gridChanged=true;
    }

    matrices.complete=matrices.rotate*matrices.zoom*matrices.move;
    if(upd) update();
}

void SlippyView::onTileReady(Tile *tile)
{
   changes.tileChanged=true;
   changes.changedTiles.push(tile);
   update();
}



void SlippyView::touchEvent(QTouchEvent *event)
{
    if(event->touchPoints().size()!=1)
        return;
    if(event->touchPointStates() == Qt::TouchPointPressed)
        return;
    event->accept();
    QPointF movementRelative = event->touchPoints()[0].lastPos()-event->touchPoints()[0].pos();

    matrices.movementFromTile+=movementRelative/256;

    matrices.move.translate(-movementRelative.x(), -movementRelative.y(), 0);
    updateCompleteMatrix();
    update();

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
            translation.translate(i*256, j*256,0);
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
}

void SlippyView::componentComplete()
{
    connect(cache, SIGNAL(tileReady(Tile*)), this, SLOT(onTileReady(Tile*)));
}
