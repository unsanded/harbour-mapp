#include "slippyview.h"
#include <QSGSimpleRectNode>
#include  <QTouchEvent>

SlippyView::SlippyView(QQuickItem *parent) :
    QQuickItem(parent)
{

    map.setWidth(2000);
    map.setHeight(2000);
    childItems().append(&map);

    map.setVisible(true);
    map.setX(50);
    map.setY(50);

    setFlag(ItemHasContents);

    map.setFlag(ItemHasContents);



    setKeepTouchGrab(true);
    update();
}

void SlippyView::touchEvent(QTouchEvent *e)
{
    qDebug() << "touchEvent";
    if(e->touchPoints().size()>0){


    }
}

QSGNode *SlippyView::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData* data)
{
  QSGSimpleRectNode* n = static_cast<QSGSimpleRectNode *>(node);
  if (!n) {
      n = new QSGSimpleRectNode();
      n->setColor(Qt::red);

      n->setRect(10, 10,500,500);
  }


  return n;

}
