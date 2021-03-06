#include "slippycache.h"

SlippyCache::SlippyCache(SlippyProvider *provider, QObject *parent) :
    QObject(parent),
    provider(provider)
{

    //start the linkedlist
    newest=oldest=new Tile(SlippyCoordinates(666,666,666), this);
    newest->next=oldest;
    oldest->previous=newest;

    connect(provider, SIGNAL(tileChanged(Tile*)), this, SLOT(onTileReady(Tile*)));
    capacity=500;
}

SlippyCache::~SlippyCache()
{
    deleteAllTiles();
    delete(provider);
}

Tile *SlippyCache::getTile(int x, int y, int zoom )
{
    return SlippyCache::getTile(SlippyCoordinates(zoom,x,y));
}

void SlippyCache::onTileReady(Tile *tile)
{
    emit tileReady(tile);
}

Tile *SlippyCache::getTile(const SlippyCoordinates coords)
{
    QMutexLocker l(&mutex);
    Tile* retval;
    if(ramCache.contains(coords)){
        retval = ramCache[coords];

         //remove from deathrow
        retval->next->previous=retval->previous;
        retval->previous->next=retval->next;
    }
    else
    {
        //not found in cache, so create it.
        retval = provider->makeTile(coords);
            ramCache.insert(coords, retval) ;
            //qDebug()<< "made tile " << retval->coords;
    }

        //insert at the front
        retval->previous=newest;
        newest->next->previous=retval;
        retval->next=newest->next;

        newest->next=retval;


        retval->cache=this;

        if(ramCache.size()>capacity){
            Tile* removeTile= oldest->previous;

            removeTile->previous->next=oldest;
            oldest->previous=removeTile->previous;
            ramCache.remove(oldest->previous->coords);
            delete removeTile;
        }
        return retval;
}



void SlippyCache::deleteAllTiles(){
    qDebug()<< "deleting tiles in cache: " << ramCache.size();
    QMutexLocker l(&mutex);
    while(oldest->previous!=newest){

        Tile* rmTile=oldest->previous;


        oldest->previous=rmTile->previous;

        ramCache.remove(rmTile->coords);
        delete rmTile;
    }
    ramCache.clear();
    newest->next=oldest;
    oldest->previous=newest;
}
