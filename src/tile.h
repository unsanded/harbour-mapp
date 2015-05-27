#ifndef TILE_H
#define TILE_H


#include <QImage>
#include <QPainter>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGTransformNode>
#include "math.h"
#include "slippycoordinates.h"

class SlippyCache;
class SlippyTileStore;


class Tile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setimage NOTIFY imageChanged)
    Q_PROPERTY(bool ready READ ready WRITE setready NOTIFY readyChanged)

    friend class SlippyCache;
    friend class SlippyTileStore;
    friend class SlippyView;

protected:

    QImage m_image;
    bool m_ready;

    QSGNode* node;
    QSGTransformNode* transformNode;//a link to the transformnode on the grid
    QSGTexture* texture;

    bool nodeMadeWhileReady;
    //these two functions may only be called from within the updatepaintnode function



    // to make them linkedListable
    SlippyCache* cache;
    Tile* next;
    Tile* previous;


public:
    virtual QSGNode* makeNode(QQuickWindow *window);
    virtual void dropNode();
    SlippyCoordinates coords;
    explicit Tile(SlippyCoordinates coords, QObject* parent=0);

    bool ready(){
        return m_ready;
    }

    QImage image() const
    {
        return m_image;
    }

signals:

    void ready(Tile*);
    void imageChanged(QImage arg);

    void readyChanged(bool arg);

public slots:
    virtual void reload(){
        //TODO
}
void setimage(QImage arg)
{
    if (m_image != arg) {
        m_image = arg;
        emit imageChanged(arg);
        setready(true);
    }

}
void setready(bool arg)
{
    if (m_ready != arg) {
        m_ready = arg;
        emit readyChanged(arg);
        if(arg)
            emit ready(this);
    }
}
};



#endif // TILE_H
