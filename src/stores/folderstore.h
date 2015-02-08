#include "slippytilestore.h"

#ifndef FOLDERSTORE_H
#define FOLDERSTORE_H

class FolderStore : public SlippyTileStore
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    QString m_path;

public:
    explicit FolderStore(QString layerName, QObject *parent = 0);

QString path() const
{
    return m_path;
}

signals:

void pathChanged(QString arg);

public slots:

void setPath(QString arg)
{
    if (m_path != arg && QFile(arg).exists()) {
        m_path = arg;
        emit pathChanged(arg);
    }
}

// SlippyTileStore interface
public:
Tile *getTile(QString layerName, SlippyCoordinates &coords);
void StoreTile(Tile *tile);
QString getPrettyName();
};

#endif // FOLDERSTORE_H
