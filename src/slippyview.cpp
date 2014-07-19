#include "slippyview.h"
#include <QSGSimpleRectNode>
#include  <QTouchEvent>
#include <QQuickItem>
#include <QSGClipNode>
#include <QSGSimpleRectNode>
#include <QQuickWindow>



SlippyView::SlippyView(QQuickItem *parent) :
    QQuickItem(parent),
    m_Location(gps::Point(53.460477, 6.836115), 10)
{


    painter=painterthread.getPainter();

   setzoom(10);

    setClip(true);
    setTransformOrigin(Center);
    setFlag(ItemHasContents);
    setFlag(ItemClipsChildrenToShape);


    texture=0;


    connect(this, &QQuickItem::heightChanged,
            this, &SlippyView::sizeChanged);
    connect(this, &QQuickItem::widthChanged,
            this, &SlippyView::sizeChanged);

    connect(painter, &SlippyPainter::pixmapChanged,
            this,    &SlippyView::onPixmapChange);
}




void SlippyView::touchEvent(QTouchEvent *event)
{

    event->accept();


    QTouchEvent::TouchPoint point = event->touchPoints()[0];

    if(event->touchPoints().size()==1){
        if(point.state() == Qt::TouchPointPressed){
            QVector<int> id;
            id << point.id();
            grabTouchPoints(id);
            qDebug()<<"grabbed " << point.id();

            qDebug()<<"on location " << posToCoordinates(point.pos()).toGps();

            setKeepTouchGrab(true);
        }
        if(point.state()==Qt::TouchPointMoved){
            QPointF offset(point.pos()-point.lastPos());
            m_Location.moveByPixels(-offset);
            updateMatrix();



        }
    }

    else
    // there are two fingers on the screen. So we are being pinched
    if(event->touchPoints().size()==2 ){


        QTouchEvent::TouchPoint point2=event->touchPoints()[1];
        //START OF ZOOMING
        if(event->touchPointStates()& Qt::TouchPointPressed){

            QVector<int> ids;
            ids << point.id();
            ids << point2.id();
            grabTouchPoints(ids);

            zoomStartMercatorPos=Location().MercatorPos();
            zoomStartPos=point.pos();
            zoomStart=zoom();

            qDebug()<<"STARTED";
            zoomStartVector=QVector2D(point2.pos() - point.pos());
            qDebug() <<zoomStartVector;
        }

       if( event->touchPointStates()&Qt::TouchPointMoved){




            // vector from finger one to finger two
            QVector2D vector   (point2.pos()-point.pos());

            qDebug() << "PINCHING";
            qDebug() << point.id() << ' ' << point2.id();
            qDebug() << point2.pos();
            qDebug() << point.pos();

            //for now, just use the ratio to zoom
            double lengthRatio=vector.length()/zoomStartVector.length();

            qDebug() << "ZOOMING BY " << lengthRatio;

            /*
            setmapOffset(
                        zoomStartOffset-
                        (
                            (point.pos()-zoomStartPos)
                            +
                            zoomStartPos*(1-lengthRatio)
                        )
            );//*/

            setzoom(zoomStart*lengthRatio);
            //todo zoom


            //todo rotate
        }
    }
}




void SlippyView::settransformationMatrix(QMatrix4x4 arg)
{
     if (m_transformationMatrix != arg)
    {
        m_transformationMatrix = arg;
        m_transformationMatrixInverse=m_transformationMatrix.inverted();
        emit transformationMatrixChanged(arg);
        matrixChanged=true;
        update();
    }
}


void SlippyView::setrotation(qreal arg)
{
    if (m_rotation != arg) {
        m_rotation = arg;
        while(m_rotation>360)
            m_rotation-=360;
        while(m_rotation<=0)
            m_rotation+=360;
        emit rotationChanged(arg);

    }
    updateMatrix();
}

void SlippyView::setzoom(qreal arg)
{
    if (m_zoom != arg) {

        //the zoomlevel has changed, so we need to paint a scaled version of what we have now in the buffer.
        // this makes zooming just a little smoother
        m_zoom = arg;

        if(abs(m_zoom-arg)>1)
            painter->scale(pow(
                              2,
                              floor( abs(m_zoom-arg))
                          ) );
        m_Location.setzoom(zoom());
            painter->getMap()->currentLocation=Location();

        emit zoomChanged(arg);
    }

    updateMatrix();
}



SlippyCoordinates SlippyView::posToCoordinates(QPointF pos)
{
    SlippyCoordinates res;



    QVector3D posv(pos.x(), pos.y(), 0);

    QVector3D offsetv = m_transformationMatrixInverse*posv;

    SlippyCoordinates mapOrigin(Location().zoom(), Location().x(), Location().y());
    mapOrigin.moveByPixels(QPointF(offsetv.x(), offsetv.y()));
    return mapOrigin;

}

void SlippyView::sizeChanged(){
    if(painter->getMap()){
        QSizeF rect(boundingRect().size());
        rect+=QSizeF(512.0,512.0);
        painter->getMap()->setSize(rect);
    }

    qDebug() << "Resized to: "<< boundingRect();
    paintMapBuffer();
}

void SlippyView::onPixmapChange(QPixmap *pixmap)
{
    Q_UNUSED(pixmap);
    qDebug()<<"have new pixmap... upating";
    bufferChanged=true;
    update();
}






bool SlippyView::isTextureProvider()
{
    return false;
}

void SlippyView::classBegin()
{
}


void SlippyView::componentComplete()
{
    
    qDebug()<< "Complete" << boundingRect();
    qDebug()<< childrenRect();
    qDebug()<< "childcount:" << childItems().size();

    connect(&redrawTimer, &QTimer::timeout,
            this, &SlippyView::checkQueuedUpdate);

    connect(&m_Location, &SlippyCoordinates::tileOffsetChanged,
            this, &SlippyView::updateMatrix);
    connect(&m_Location, &SlippyCoordinates::tilePosChanged,
            this, &SlippyView::paintMapBuffer);

    setKeepMouseGrab(true);
    redrawTimer.setSingleShot(true);
    setClip(true);

    redrawTimer.setInterval(500);

    sizeChanged();
}




QSGNode *SlippyView::updatePaintNode(QSGNode *node, UpdatePaintNodeData * data)
{
    Q_UNUSED(data)

    //Initial setup of nodes
    if(!node){

        transformNode = new QSGTransformNode;
        node=transformNode;
        matrixChanged=true;

        QSGSimpleRectNode* rect=new QSGSimpleRectNode;
        rect->setRect(110,110,100,100);
        rect->setColor(Qt::cyan);
        transformNode->appendChildNode(rect);
        textureNode=new QSGSimpleTextureNode;

        //temporary texture;
        QImage img(200,200, QImage::Format_ARGB32);
        img.fill(Qt::gray);
        QSGTexture* texture = window()->createTextureFromImage(img);

        textureNode->setTexture(texture);

        transformNode->appendChildNode(textureNode);
        bufferChanged=true;

    }
    // just in case...

    if(transformNode!=node)
        qWarning()<< "nodes not equal";
    transformNode=static_cast<QSGTransformNode*> (node);

    if(matrixChanged){
        matrixChanged=false;
        transformNode->setMatrix(m_transformationMatrix);
        transformNode->markDirty(QSGNode::DirtyMatrix);

    }


    // this should only happen when switching tile-coordinates or zoomlevel
    if(bufferChanged ){
        bufferChanged=false;
        if(texture)
            delete texture;

       texture = this->window()->createTextureFromImage(painter->getPixmap()->toImage());
       textureNode->setTexture(texture);
       textureNode->setRect(QRectF(QPointF(0,0), painter->getPixmap()->size()) );
       textureNode->markDirty(QSGNode::DirtyMaterial);

       offsetChanged=true;
    }
    return transformNode;
}

void SlippyView::updateMatrix()
{
    QMatrix4x4 newMatrix;
    newMatrix.scale(pow(2.0, zoom()-floor(zoom()) ) );
    newMatrix.rotate(rotation(), 0,0,1 );
    newMatrix.translate(-mapOffset().x()*TILE_SIZE, -mapOffset().y()*TILE_SIZE);
    settransformationMatrix(newMatrix);
}

void SlippyView::paintMapBuffer()
{

    if(!painter->getMap())
        return;
    if(painter->getMap()->currentLocation.tilePos()==m_Location.tilePos())
        return;

    QPoint offset=m_Location.tilePos()-painter->getMap()->currentLocation.tilePos();

    offset*=-TILE_SIZE;

    painter->getMap()->currentLocation=m_Location;

    painter->move(offset);//at this point the buffer hasn't been repainted, so we have to shift the ex

    qDebug() << "repainting map" << Location();
    painterthread.queueRepaint();

}


void SlippyView::queueUpdate()
{
    if(!hasRedrawQueued){
        hasRedrawQueued=true;
        redrawTimer.start();
    }
}

void SlippyView::checkQueuedUpdate()
{
    if(hasRedrawQueued){
        update();
        hasRedrawQueued=false;
        redrawTimer.stop();
    }
}




void SlippyView::touchUngrabEvent()
{
    qDebug()<< "UNGRABBED";
}


SlippyPainter::SlippyPainter():
    pixmap1(new QPixmap(1024, 1024)),
    pixmap2(new QPixmap(1024, 1024))
{


    map=new SlippyMap;

    connect(
        getMap(), SIGNAL(tileChanged()),
        this, SLOT (repaint())
       );
    connect(
        getMap(), SIGNAL(heightChanged()),
        this, SLOT (updateSize())
        );
    connect(
        getMap(), SIGNAL(widthChanged()),
        this, SLOT (updateSize())
        );

    updateChecker.setInterval(10000);
    connect(
        &updateChecker, &QTimer::timeout,
        this, &SlippyPainter::repaint
        );

    updateChecker.start();
    repaint();
}

void PainterThread::run()
{
    SlippyPainter painter;
    this->painter=&painter;
    exec();
    this->painter=0;
}

void PainterThread::queueRepaint()
{
    QMetaObject::invokeMethod(painter, "repaint", Qt::QueuedConnection);
}

void SlippyPainter::repaint()
{
    QMutexLocker l(&mutex);
    needRepaint=1;
    while(needRepaint){
    needRepaint=0;//threads are weird

    qDebug()<<"Repainting";
    QPixmap* otherpixmap;
    if(currentPixmap==pixmap1)
    {
        qDebug()<< "PAINTING on buffer one";
        otherpixmap=pixmap2;
    }
    else
    {
        qDebug()<< "PAINTING on buffer two";
        otherpixmap=pixmap1;
    }
    otherpixmap->fill(Qt::darkBlue);
    QPainter p(otherpixmap);
    getMap()->paint(&p);

    qDebug() << "one" << pixmap1;
    qDebug() << "two" << pixmap2;
    qDebug() << "now on buffer " << currentPixmap;

    currentPixmap=otherpixmap;


    emit pixmapChanged(currentPixmap);
    }//while
}


void SlippyPainter::updateSize()
{

    QMutexLocker l(&mutex);

    QPixmap* oldPixmap1=pixmap1;
    QPixmap* oldPixmap2=pixmap2;

    qDebug()<< "Updating size";

        pixmap2=new QPixmap(getMap()->width(), getMap()->height());
        pixmap1=new QPixmap(getMap()->width(), getMap()->height());
        pixmap1->fill(Qt::red);
        pixmap2->fill(Qt::red);
    delete oldPixmap1;
    delete oldPixmap2;
}

void SlippyPainter::scale(double scale){
    QPixmap* oldMapBuffer;

    qDebug()<<"scale";
    QMutexLocker l(&mutex);
    qDebug()<<"lock";

    if(currentPixmap==pixmap1){

        oldMapBuffer=pixmap2;
        pixmap2=new QPixmap(pixmap2->size());

        pixmap2->fill(Qt::darkRed);

        QPainter p;
        p.begin(pixmap2);

        qDebug() << "drawing old map onto the new one with halve scale" << scale;
        if(scale<1)
            p.drawPixmap(
                        QRect({0,0}, pixmap2->size()*scale),
                        *pixmap1,
                        QRect({0,0}, pixmap2->size())
                        );

        else
            p.drawPixmap(
                        QRect({0,0}, pixmap2->size()),
                        *pixmap1,
                        QRect({0,0}, pixmap2->size()/scale)
                        );

        p.end();
        currentPixmap=pixmap2;
    }
    else
    {

        oldMapBuffer=pixmap1;

        pixmap1=new QPixmap(pixmap1->size());

        pixmap1->fill(Qt::darkRed);

        QPainter p;
        p.begin(pixmap2);

        qDebug() << "drawing old map onto the new one";
        if(scale<1)
            p.drawPixmap(
                        QRect({0,0}, pixmap1->size()*scale),
                        *pixmap2,
                        QRect({0,0}, pixmap1->size())
                        );

        else
            p.drawPixmap(
                        QRect({0,0}, pixmap1->size()),
                        *pixmap2,
                        QRect({0,0}, pixmap1->size()/scale)
                        );

        p.end();
        currentPixmap=pixmap1;
    }
    delete oldMapBuffer;
    emit pixmapChanged(currentPixmap);
}

void SlippyPainter::move(QPoint offset)
{
    qDebug()<< "Move";
    QMutexLocker l(&mutex);

    QPixmap* otherpixmap;
    if(currentPixmap==pixmap1)
        otherpixmap=pixmap2;
    else
        otherpixmap=pixmap1;


    otherpixmap->fill(Qt::darkYellow);
    QPainter p(otherpixmap);
    p.drawPixmap(offset, *currentPixmap);

    currentPixmap=otherpixmap;
    emit pixmapChanged(currentPixmap);
}

