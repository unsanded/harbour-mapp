#include "tile.h"
#include <math.h>
#include <QDebug>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>

QSGNode *Tile::makeNode(QQuickWindow* window)
{
    if(node && !nodeMadeWhileReady && ready()){
        //drop the texture that was made while we were not ready
        delete texture;
        texture=window->createTextureFromImage(m_image);
        ((QSGSimpleTextureNode*)node)->setTexture(texture);

        node->markDirty(QSGNode::DirtyMaterial);
        nodeMadeWhileReady=true;
    }

    if(!node){
        qDebug() << "Creating node " ;
        node=new QSGSimpleTextureNode;
        transformNode=new QSGTransformNode;

        nodeMadeWhileReady=ready();
        texture=window->createTextureFromImage(m_image);
        ((QSGSimpleTextureNode*)node)->setTexture(texture);
        ((QSGSimpleTextureNode*)node)->setRect(0,0,256,256);
        node->markDirty(QSGNode::DirtyMaterial);


        transformNode->setFlag(QSGNode::OwnedByParent, false);
        node->setFlag(QSGNode::OwnedByParent, false);

        transformNode->appendChildNode(node);
    }
    return transformNode;
}

void Tile::dropNode()
{
    if(transformNode){
            transformNode->removeChildNode(node);//remove the node from the grid
            delete transformNode;
            transformNode=0;
    }
    if(node) delete node;
    if(texture) delete texture;
    node=0;
    texture=0;
    qDebug() << "dropping node done";
}

Tile::Tile(SlippyCoordinates coordinates, QObject *parent) :
    QObject(parent),
    m_image(256, 256, QImage::Format_ARGB32),
    coords(coordinates)
{
    nodeMadeWhileReady=false;
    node=0;
    transformNode=0;
    texture=0;


    next=0;
    previous=0;
    m_ready=false;
    m_image.fill(QColor(100,100,200,200));

    //just to test
    QPainter p;
    p.begin(&m_image);
        p.setPen(Qt::green);
        p.drawText(20,100, QString("(%1 : %2,%3)").arg(coords.zoom()).arg(coords.x()).arg(coords.y()) );
        p.drawArc(0,0,256,256,0,270*16);
    p.end();


}




