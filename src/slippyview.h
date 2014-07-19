#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include "slippymap.h"
#include <QMatrix4x4>
#include <QQuickItem>
#include <QSGClipNode>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QTimer>
#include <QMutex>

#define TILE_SIZE 256
class SlippyPainter;
class PainterThread:public QThread{
    Q_OBJECT


    // QThread interface
protected:
    virtual void run();
    SlippyPainter* painter;


public slots:
void queueRepaint();

signals:

public:
    SlippyPainter* getPainter(){return painter;}

    PainterThread(){
        painter=0;
        start();
        while(!painter){
            qDebug()<< "waiting for painter";
            msleep(50);
        }
    }
};

class SlippyPainter: public QObject{
Q_OBJECT

public:

private:
    volatile SlippyMap* map;
    //some double-buffering (this is twice the ram, but saves all the tedious mallocing every time)
    QPixmap* pixmap1;
    QPixmap* pixmap2;
    QPixmap* currentPixmap;
    QMutex mutex;
    bool needRepaint;
    QTimer updateChecker;

public:

    SlippyPainter();

    /**
     * @brief scale
     * scales the pixmap with existing data. So this function can be called to have at least some image to show while zooming.
     * @param scale
     */
    void scale(double scale);
    void move(QPoint offset);


 signals:
    void pixmapChanged(QPixmap* pixmap);
public slots:

    void repaint();

protected slots:
    void updateSize();

public:
    SlippyMap* getMap(){return (SlippyMap*) map;}
    QPixmap* getPixmap(){return currentPixmap;}

};


class SlippyView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QMatrix4x4 transformationMatrix READ transformationMatrix WRITE settransformationMatrix NOTIFY transformationMatrixChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setrotation NOTIFY rotationChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setzoom NOTIFY zoomChanged)

    Q_PROPERTY(SlippyCoordinates Location READ Location WRITE setLocation NOTIFY LocationChanged)

    qreal m_rotation;
    QSize bufferSize;

    qreal m_zoom;

    QPointF m_MercatorCoordinates;

    SlippyCoordinates m_Location;

    PainterThread painterthread;
    SlippyPainter* painter;

public:
    explicit SlippyView(QQuickItem *parent = 0);

    void touchEvent(QTouchEvent *e);



signals:


    //properties
    void transformationMatrixChanged(QMatrix4x4 arg);
    void mapOffsetChanged(QPointF arg);
    void rotationChanged(qreal arg);
    void zoomChanged(qreal arg);

    void MercatorCoordinatesChanged(QPointF arg);

    void LocationChanged(SlippyCoordinates arg);

public slots:


    //two interface functions for extendability
    virtual void updateMatrix();
    virtual void paintMapBuffer();
    void queueUpdate();

    virtual void checkQueuedUpdate();


    void settransformationMatrix(QMatrix4x4 arg);

    void setrotation(qreal arg);

    void setzoom(qreal arg);

    /**
     * @brief posToCoordinates
     * translates a pixel position to gps coordinates in the currently displayed map
     * @param pos
     * @return
     */
    SlippyCoordinates posToCoordinates(QPointF pos);

    void setMercatorCoordinates(QPointF arg)
    {
        if (m_MercatorCoordinates != arg) {
            m_MercatorCoordinates = arg;
            emit MercatorCoordinatesChanged(arg);
        }
    }

    void setLocation(SlippyCoordinates arg)
    {
        m_Location = arg;
        emit LocationChanged(arg);
    }

private slots:

    void sizeChanged();
    void onPixmapChange(QPixmap* pixmap);




public:
    

    // QQuickItem interface
public:
    virtual bool isTextureProvider();

    // QQmlParserStatus interface
public:
    virtual void classBegin();
    virtual void componentComplete();



    QMatrix4x4 transformationMatrix() const { return m_transformationMatrix; }

    QPointF mapOffset() const { return m_Location.tileOffset(); }

    qreal rotation() const { return m_rotation; }

    qreal zoom() const { return m_zoom; }

    QPointF MercatorCoordinates() const { return m_MercatorCoordinates; }

    SlippyCoordinates Location() const { return m_Location; }

protected:
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *data);


protected:
//only redraw what's necessary
    bool matrixChanged;
    bool bufferChanged;
    bool offsetChanged;

    bool hasRedrawQueued;

    QTimer redrawTimer;

    //stuff for rendering
    QMatrix4x4 m_transformationMatrix;
    QMatrix4x4 m_transformationMatrixInverse;

    QSGTransformNode* transformNode;
    QSGSimpleTextureNode* textureNode;
    QSGTexture* texture;

    //stuff for gestures
    QPointF   zoomStartMercatorPos;
    QPointF   zoomStartPos;
    QVector2D zoomStartVector;
    qreal     zoomStart;





    // QQuickItem interface
protected:
    virtual void touchUngrabEvent();
};

#endif // SLIPPYVIEW_H
