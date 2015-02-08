#include "tilemanager.h"

#include "slippycache.h"

//yes, it is unorthodox and all
static TileManager* globaltm=0;


TileManager::TileManager(QObject *parent) :
    QObject(parent)
{
    if(!getDefault()){
        globaltm=this;
    }
}

TileManager::~TileManager()
{
    if(getDefault()==this){
        globaltm=0;
    }
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

    return globaltm;
}

void TileManager::addLayer(SlippyProvider *provider)
{
    SlippyCache* newCache = new SlippyCache(provider);

    layers.append({newCache, provider, provider->name()});
    layerIndices.insert(provider->name(), layers.size()-1);

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





