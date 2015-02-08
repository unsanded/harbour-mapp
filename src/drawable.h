#ifndef DRAWABLES_DRAWABLE_H
#define DRAWABLES_DRAWABLE_H

#include <QObject>
#include <QRectF>

namespace drawables {

class Drawable : public QObject
{
    Q_OBJECT
    /**
      bounding box in mercator coordinates
      */
    Q_PROPERTY(QRectF boundingBox READ boundingBox NOTIFY boundingBoxChanged)
    QRectF m_boundingBox;

public:
    explicit Drawable(QObject *parent = 0);



QRectF boundingBox() const
{
    return m_boundingBox;
}

signals:

void boundingBoxChanged(QRectF arg);

public slots:

};

} // namespace drawables

#endif // DRAWABLES_DRAWABLE_H
