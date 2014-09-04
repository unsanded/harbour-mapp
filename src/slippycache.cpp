#include "slippycache.h"

SlippyCache::SlippyCache(SlippyProvider *provider, QObject *parent) :
    QObject(parent),
    provider(provider)
{
    //start the linkedlist
    newest=oldest=new Tile(SlippyCoordinates(666,666,666), this);
    newest->next=oldest;
    oldest->previous=newest;


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

Tile *SlippyCache::getTile(const SlippyCoordinates coords)
{
    qDebug() << "getting tile " << coords;
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

            qDebug()<< "cache full, removing tile" << removeTile->coords;
            removeTile->previous->next=oldest;
            oldest->previous=removeTile->previous;
            qDebug() << ramCache.remove(oldest->previous->coords);
            delete removeTile;
        }
        return retval;
}



void SlippyCache::deleteAllTiles(){
    qDebug()<< "deleting tiles in cache: " << ramCache.size();
    QMutexLocker l(&mutex);
    while(oldest->previous!=newest){

        Tile* rmTile=oldest->previous;

        qDebug()<< "    removing tile" << rmTile->coords;

        oldest->previous=rmTile->previous;

        qDebug()<< "    " << ramCache.remove(rmTile->coords);
        delete rmTile;
    }
    ramCache.clear();
    newest->next=oldest;
    oldest->previous=newest;
}
