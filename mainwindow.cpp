#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QPen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , pixmapItem(nullptr)
    , currentLine(nullptr)
    , drawing(false)
    , scaleFactor(3.5)
    , rotationAngle(0.0)
    , isDrawing(false)
    , isEraserMode(false)
    , eraserSize(100.0)
{
    ui->setupUi(this);
    // 设置 QGraphicsView 和场景
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    ui->graphicsView->installEventFilter(this);  // 安装事件过滤器
    ui->uploadImageButton->installEventFilter(this);
    // 安装事件过滤器
    ui->graphicsView->viewport()->installEventFilter(this);

    // 设置图片缩放比例为两倍
    // pixmapItem->setScale(2.0);


    // 连接上传图片按钮
    connect(ui->rotateLeftButton, &QPushButton::clicked, this, &MainWindow::on_rotateLeftButton_clicked);
    connect(ui->rotateRightButton, &QPushButton::clicked, this, &MainWindow::on_rotateRightButton_clicked);
    connect(ui->scaleUpButton, &QPushButton::clicked, this, &MainWindow::on_scaleUpButton_clicked);
    connect(ui->scaleDownButton, &QPushButton::clicked, this, &MainWindow::on_scaleDownButton_clicked);
    connect(ui->rotationSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::rotateImage);
    connect(ui->horizontalScaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onScaleChanged);
    connect(ui->verticalScaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onScaleChanged);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::exportLinePoints);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_eraserButton_clicked() {
    isEraserMode = !isEraserMode;

    if (isEraserMode) {
        qDebug() << isEraserMode << ' ' << __FUNCTION__;
        ui->eraserButton->setStyleSheet("background-color: red; color: white;");
    } else {
        qDebug() << isEraserMode << ' ' << __FUNCTION__;
        ui->eraserButton->setStyleSheet("background-color: gray; color: white;");
    }
}


// 上传图片按钮点击事件
void MainWindow::on_uploadImageButton_clicked()
{
    // 获取单个文件路径
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    qDebug() << fileName;

    if (!fileName.isEmpty()) {
        // 加载图片
        QPixmap image(fileName);
        if (!image.isNull()) {
            // 为图片创建一个新的 QGraphicsPixmapItem
            QGraphicsPixmapItem* newPixmapItem = new QGraphicsPixmapItem(image);

            // 设置图片的旋转中心为图片中心
            newPixmapItem->setTransformOriginPoint(newPixmapItem->boundingRect().center());

            // 缩放图片
            newPixmapItem->setScale(scaleFactor);


            // 将图片项添加到场景中
            scene->addItem(newPixmapItem);

            // 设置图片可拖动
            // newPixmapItem->setFlag(QGraphicsItem::ItemIsMovable);

            // 存储图片项
            pixmapItem = newPixmapItem;

            // 设置X轴和Y轴范围，并使用统一的线条画刻度和坐标轴
            int xAxisLength = 2000;   // X轴的长度
            int yAxisLength = 1500;   // Y轴的长度
            int tickInterval = 400;   // 刻度线间隔

            addCoordinateSystemWithAxes(scene, xAxisLength, yAxisLength, tickInterval);

            // 设置图片左上角位置为 (0, 0)
            newPixmapItem->setPos(0, 0);
        }
    }

    QMessageBox::information(this, "提示", "请在图片上选择上表面");

    // 进入选择上表面状态
    isSelectingUpperSurface = true;
}

void MainWindow::handleMousePress(QMouseEvent *event) {
    QPointF scenePos = ui->graphicsView->mapToScene(event->pos());  // 将点击位置映射到场景坐标
    qDebug() << "Clicked position in scene:" << scenePos;

    if (isSelectingUpperSurface) {
        // 记录上表面位置
        upperSurfacePos = scenePos;
        isSelectingUpperSurface = false;
        isSelectingLowerSurface = true;

        // 提示用户选择下表面
        QMessageBox::information(this, "提示", "请在图片上选择下表面");

    } else if (isSelectingLowerSurface) {
        // 记录下表面位置
        lowerSurfacePos = scenePos;
        isSelectingLowerSurface = false;

        // 提示选择完成，并要求输入实际工件高度
        bool ok;
        double actualHeight = QInputDialog::getDouble(this, "输入实际高度", "请输入工件的实际高度（单位：毫米）", 0, 0, 10000, 2, &ok);

        if (ok) {
            // 计算像素高度与实际高度之间的比例
            double pixelHeight = lowerSurfacePos.y() - upperSurfacePos.y();
            heightPerPixel = actualHeight / pixelHeight;  // 每个像素对应的实际高度

            qDebug() << "实际工件高度:" << actualHeight << "像素高度:" << pixelHeight << "每像素对应的高度:" << heightPerPixel;

            // 提示选择完成
            QMessageBox::information(this, "提示", "上表面和下表面已选择完成！您可以继续绘图。");
        }
    }
}


void MainWindow::on_rotateLeftButton_clicked()
{
    if (pixmapItem) {
        rotationAngle -= 10; // 左旋转 10 度
        pixmapItem->setRotation(rotationAngle);
    }
}

void MainWindow::on_rotateRightButton_clicked()
{
    if (pixmapItem) {
        rotationAngle += 10; // 右旋转 10 度
        pixmapItem->setRotation(rotationAngle);
    }
}

void MainWindow::on_scaleUpButton_clicked()
{
    if (pixmapItem) {
        scaleFactor += 0.1; // 放大 10%
        pixmapItem->setScale(scaleFactor);
    }
}

void MainWindow::on_scaleDownButton_clicked()
{
    if (pixmapItem) {
        scaleFactor -= 0.1; // 缩小 10%
        if (scaleFactor > 0) {
            pixmapItem->setScale(scaleFactor);
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {

    if (isDrawing && event->button() == Qt::LeftButton) {
        qDebug()<< event->pos();
        lastPoint = ui->graphicsView->mapToScene(event->pos());  // 将鼠标位置映射到场景坐标
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << isDrawing << ' ' << event->buttons() <<' ' << event->pos() << ' ' << __FUNCTION__;
    if (isDrawing && (event->buttons() & Qt::LeftButton)) {
        QPointF currentPoint = ui->graphicsView->mapToScene(event->pos());
        scene->addLine(QLineF(lastPoint, currentPoint), QPen(Qt::black, 2));  // 绘制线条
        lastPoint = currentPoint;  // 更新上一个点为当前点
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && isDrawing) {
        isDrawing = false;  // 停止绘图
        lastPoint = QPointF();  // 重置 lastPoint
    }
}

void MainWindow::rotateImage(int angle)
{
    if (pixmapItem) {
        pixmapItem->setRotation(angle); // 设置图像的旋转角度
    }
}

void MainWindow::on_paintButton_clicked() {
    isDrawing = !isDrawing;
    if (isDrawing) {
        // 设置按钮为红色
        ui->paintButton->setStyleSheet("background-color: red; color: white;");
    } else {
        // 设置按钮为灰色
        ui->paintButton->setStyleSheet("background-color: gray; color: white;");
    }
    qDebug() << isDrawing<< __FUNCTION__;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->graphicsView->viewport()) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        // 鼠标按下事件
        if (event->type() == QEvent::MouseButtonPress) {
            if (mouseEvent->button() == Qt::LeftButton) {
                QPointF currentPoint = ui->graphicsView->mapToScene(mouseEvent->pos());

                if (isEraserMode) {
                    // 橡皮擦模式下，删除当前橡皮擦区域内的线条
                    QList<QGraphicsItem *> items = scene->items(QRectF(currentPoint.x() - eraserSize / 2, currentPoint.y() - eraserSize / 2, eraserSize, eraserSize));

                    for (QGraphicsItem *item : items) {
                        QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem *>(item);
                        if (lineItem) {
                            scene->removeItem(lineItem);
                            delete lineItem;
                        }
                    }
                } else if (isDrawing) {
                    // 绘图模式，记录初始点
                    lastPoint = currentPoint;
                } else {
                    // 如果有其他鼠标点击逻辑，比如选择上表面或下表面
                    handleMousePress(mouseEvent);
                }
                return true;  // 事件已处理
            }
        }

        // 鼠标移动事件
        else if (event->type() == QEvent::MouseMove) {
            if (isEraserMode && (mouseEvent->buttons() & Qt::LeftButton)) {
                QPointF currentPoint = ui->graphicsView->mapToScene(mouseEvent->pos());

                // 橡皮擦模式下，删除当前区域内的线条
                QList<QGraphicsItem *> items = scene->items(QRectF(currentPoint.x() - eraserSize / 2, currentPoint.y() - eraserSize / 2, eraserSize, eraserSize));

                for (QGraphicsItem *item : items) {
                    QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem *>(item);
                    if (lineItem) {
                        scene->removeItem(lineItem);
                        delete lineItem;
                    }
                }
                return true;  // 事件已处理
            } else if (isDrawing && (mouseEvent->buttons() & Qt::LeftButton)) {
                // 绘图模式，绘制线条
                QPointF currentPoint = ui->graphicsView->mapToScene(mouseEvent->pos());
                scene->addLine(QLineF(lastPoint, currentPoint), QPen(Qt::blue, 2));
                // 计算当前点相对于上表面的像素高度
                double pixelDepth = currentPoint.y() - upperSurfacePos.y();
                double actualDepth = pixelDepth * heightPerPixel;
                qDebug() << "当前点像素深度:" << pixelDepth << "实际深度:" << actualDepth;
                lastPoint = currentPoint;
                linePoints.append(currentPoint);  // 记录每个绘制点

                return true;  // 事件已处理
            }
        }

        // 鼠标松开事件
        else if (event->type() == QEvent::MouseButtonRelease) {
            if (mouseEvent->button() == Qt::LeftButton) {
                if (isDrawing) {
                    // 停止绘图
                    lastPoint = QPointF();
                    // 打印所有绘制的点
                    for (const QPointF& point : linePoints) {
                        qDebug() << "Point in line:" << point;
                    }
                    drawModifiedLine();  // 绘制修改后的线条
                }
                return true;  // 事件已处理
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);  // 继续其他默认处理
}

void MainWindow::onScaleChanged() {
    if (pixmapItem) {
        // 获取当前的缩放倍数
        double horizontalScaleFactor = ui->horizontalScaleSpinBox->value();
        double verticalScaleFactor = ui->verticalScaleSpinBox->value();

        // 计算新的尺寸
        int newWidth = static_cast<int>(originalPixmap.width() * horizontalScaleFactor);
        int newHeight = static_cast<int>(originalPixmap.height() * verticalScaleFactor);

        // 根据新的尺寸缩放图像
        QPixmap scaledPixmap = originalPixmap.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        pixmapItem->setPixmap(scaledPixmap);
    }
}

void MainWindow::exportLinePoints() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Line Points"), "", tr("CSV Files (*.csv);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "X,Y\n";

        for (const QPointF &point : linePoints) {
            stream << point.x() << "," << point.y() << "\n";
        }

        file.close();
        QMessageBox::information(this, tr("Export Successful"), tr("Line points exported successfully!"));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Unable to open file for writing."));
    }
}

void MainWindow::drawModifiedLine() {
    // 清空新的点数组
    newLinePoints.clear();

    // 遍历 linePoints，并对每个点的 Y 值加上一个随机数
    for (const QPointF &point : linePoints) {
        // 生成 -0.5 到 0.5 之间的随机数
        double randomYOffset = QRandomGenerator::global()->bounded(-10, 10);
        QPointF newPoint(point.x(), point.y() + randomYOffset);
        newLinePoints.append(newPoint);  // 添加修改后的点
    }

    // 遍历 newLinePoints 并绘制红线
    for (int i = 0; i < newLinePoints.size() - 1; ++i) {
        scene->addLine(QLineF(newLinePoints[i], newLinePoints[i + 1]), QPen(Qt::red, 2));  // 用红色笔绘制线
    }
}

// 函数：添加带X轴和Y轴的坐标系，并使用统一的线条画刻度和坐标轴
void MainWindow::addCoordinateSystemWithAxes(QGraphicsScene* scene, int xAxisLength, int yAxisLength, int tickInterval) {

    // 创建统一的画笔，用于坐标轴和刻度线
    QPen pen(Qt::black, 1);  // 黑色线条，统一用于绘制坐标轴和刻度线

    QFont font("Arial", 8);

    // 绘制X轴（包括负坐标）
    // scene->addLine(-xAxisLength, 0, xAxisLength, 0, pen);

    // 绘制Y轴（包括负坐标）
    // scene->addLine(0, -yAxisLength, 0, yAxisLength, pen);

    // 在X轴上添加正负刻度线和标签
    for (int i = -xAxisLength; i <= xAxisLength; i += tickInterval) {

        // 绘制垂直的长刻度线
        scene->addLine(i, -yAxisLength, i, yAxisLength, pen);

        // 绘制X轴上的标签
        // QGraphicsTextItem* textItem = scene->addText(QString::number(i), font);
        // textItem->setPos(i - 10, 10);  // 调整标签位置

    }

    // 在Y轴上添加正负刻度线和标签
    for (int i = -yAxisLength; i <= yAxisLength; i += tickInterval) {

        // 绘制水平的长刻度线
        scene->addLine(-xAxisLength, i, xAxisLength, i, pen);

        // 绘制Y轴上的标签
        // QGraphicsTextItem* textItem = scene->addText(QString::number(-i), font);
        // textItem->setPos(-25, i - 5);  // 调整标签位置

    }

    // 添加原点标签
    // QGraphicsTextItem* originText = scene->addText("0", font);
    // originText->setPos(-15, 10);
}
