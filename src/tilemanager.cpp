#include "tilemanager.h"

#include "slippycache.h"

//yes, it is unorthodox and all
static TileManager* globaltm=0;


TileManager::TileManager(QQuickItem *parent) :
    QQuickItem(parent),
    currentCache(0)
{
    if(!globaltm){
        globaltm=this;
    }
}

TileManager::~TileManager()
{
    if(getDefault()==this){
        globaltm=0;
    }
}

Tile *TileManager::getTile(SlippyCoordinates coordinates)
{
    return currentCache->getTile(coordinates);
}

Tile *TileManager::getTile(int x, int y, int zoom)
{
    return getTile(SlippyCoordinates(zoom, x, y));
}

Layer TileManager::activeLayer()
{
    return layers[activeLayerIndex];
}

Layer TileManager::operator[](int index)
{
    return layers[index];
}

Layer TileManager::operator[](QString name)
{
    if(layerIndices.contains(name)){
        return layers[layerIndices[name]];
    }
    else
        return {0,0,""};
}


TileManager* TileManager::getDefault()
{

    if(!globaltm)
        globaltm=new TileManager;
    return globaltm;
}

void TileManager::addLayer(SlippyProvider *provider)
{
    SlippyCache* newCache = new SlippyCache(provider);

    layers.append({newCache, provider, provider->name()});
    layerIndices.insert(provider->name(), layers.size()-1);
    qDebug() << "Inserted layer: " << provider->name();
    if(layers.size()==1)//this is the first layer, so select it.
        selectLayer(0);
}

void TileManager::selectLayer(int index)
{
    if(index<layers.size()){
        activeLayerIndex=index;
        currentCache=activeLayer().cache;
        emit activeLayerChanged(activeLayerIndex);
    }
}

void TileManager::selectLayer(QString name)
{
    if(layerIndices.contains(name)){
        selectLayer(layerIndices[name]);
    }
}





