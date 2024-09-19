#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QFileDialog>
#include <QInputDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    QList<QGraphicsPixmapItem*> pixmapItems;   // 存储所有的图片项
    QPointF startPoint;
    QGraphicsLineItem *currentLine;
    bool drawing;
    qreal scaleFactor;
    qreal rotationAngle;

    bool isDrawing;  // 判断是否进入画笔模式
    QPointF lastPoint;  // 保存鼠标的上一个位置
    bool eventFilter(QObject *obj, QEvent *event);
    QPixmap originalPixmap;
    bool isEraserMode;  // 判断是否处于橡皮擦模式
    qreal eraserSize;   // 橡皮擦的大小
    QVector<QPointF> linePoints;
    QVector<QPointF> newLinePoints;

    bool isSelectingUpperSurface = false;  // 是否正在选择上表面
    bool isSelectingLowerSurface = false;  // 是否正在选择下表面
    QPointF upperSurfacePos;  // 上表面的坐标
    QPointF lowerSurfacePos;  // 下表面的坐标

    void handleMousePress(QMouseEvent *event);  // 处理鼠标点击事件的逻辑

    double heightPerPixel;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void addCoordinateSystemWithAxes(QGraphicsScene* scene, int xAxisLength, int yAxisLength, int tickInterval);


private slots:
    void on_uploadImageButton_clicked();
    void on_rotateLeftButton_clicked();
    void on_rotateRightButton_clicked();
    void on_scaleUpButton_clicked();
    void on_scaleDownButton_clicked();
    void rotateImage(int angle);
    void on_paintButton_clicked();
    void onScaleChanged();
    void on_eraserButton_clicked();
    void exportLinePoints();
    void drawModifiedLine();
};
#endif // MAINWINDOW_H
