#ifndef SLIPPYVIEW_H
#define SLIPPYVIEW_H

#include "slippymap.h"


class SlippyView : public QQuickItem
{
    Q_OBJECT
    SlippyMap map;
public:
    explicit SlippyView(QQuickItem *parent = 0);

    virtual void touchEvent(QTouchEvent *e);

    virtual QSGNode* updatePaintNode(QSGNode *node, UpdatePaintNodeData *data);


signals:

public slots:

};

#endif // SLIPPYVIEW_H
