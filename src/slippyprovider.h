#ifndef SLIPPYPROVIDER_H
#define SLIPPYPROVIDER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QQuickItem>
#include "tile.h"

/**
 * @brief The SlippyProvider class
 * An abstract class providing a source of maps. All it has to do is fetch or create (provide) tiles.
 */
class SlippyProvider : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setname NOTIFY nameChanged)
    QString m_name;

public:
    explicit SlippyProvider(QQuickItem *parent = 0);

    /**
     * @brief makeTile
     * Make or fetch a tile from a mapservice. Caching is not necessary, that will be handled separately.
     * @param coords the SlippyCoordinates that should be fetched or created.
     * @return The created tile
     */
    virtual Tile* makeTile(SlippyCoordinates coords);

public:
    /**
     * @brief isLayer
     * @return whether the provided tiles are usable as overlay. Like openseamap for instance.
     */
    virtual bool isOverlay();

    /**
     * @brief getPrettyName
     * @return the name people will see. eg. "OpenStreetMap"
     */
    virtual QString getPrettyName();

     /**
     * @brief isOnline
     * @return whether the provider needs an internet-connection
     */
    virtual bool isOnline()
    {
         return false;
    }


            signals:
    void tileChanged(Tile*);
    void tileChanged();

    void nameChanged(QString arg);

public slots:

    void onTileLoaded(Tile* tile);

    // QQuickItem interface

    // QQmlParserStatus interface
    void setname(QString arg)
    {
        if (m_name != arg) {
            m_name = arg;
            emit nameChanged(arg);
        }
    }

public:
    void classBegin();
    void componentComplete();
    QString name() const
    {
        return m_name;
    }
};



#endif // SLIPPYPROVIDER_H
