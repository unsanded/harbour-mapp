#include "slippyprovider.h"
#include "slippyview.h"




SlippyProvider::SlippyProvider(QQuickItem *parentitem): QQuickItem(parentitem){
    if(parentitem){
        SlippyView* view=qobject_cast<SlippyView*>(parentitem);
        if(view){
            qDebug() << "Found parent; registering provider " << getPrettyName();
        }
    }
}

Tile *SlippyProvider::makeTile(SlippyCoordinates coords)
{
    Q_UNUSED(coords)
    qDebug() << "Slippy maketile";

    return new Tile(coords);
}

bool SlippyProvider::isOverlay(){
    return false;
}

QString SlippyProvider::getPrettyName()
{
    return "ABSTRACT" ;
}

void SlippyProvider::onTileLoaded(Tile *tile)
{
    emit tileChanged();
    emit tileChanged(tile);
}

void SlippyProvider::classBegin()
{
    qDebug() << parentItem();
}

void SlippyProvider::componentComplete()
{
    if(m_name.isEmpty()){
        m_name=getPrettyName();
    }

    QQuickItem* parentitem = parentItem();
    if(parentitem){
        SlippyView* view=qobject_cast<SlippyView*>(parentitem);
        if(view){
            qDebug() << "Found parent; registering provider " << getPrettyName();
            view->addLayer(this);
        }
    }
}
