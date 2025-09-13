

#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QDebug>
#include <QSplitter>

// -------- SplitPlotWidget Implementation --------
SplitPlotWidget::SplitPlotWidget(int variableIndex, QString variableName, QString unit, QColor color, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    series = new QSplineSeries();
    series->setColor(color);
    chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->legend()->hide();
    chart->setTitle("Real-Time PMU Data Visualization");
    QList<QAbstractAxis*> axesX = chart->axes(Qt::Horizontal);
    if (!axesX.isEmpty()) axesX.first()->setTitleText("Time (s)");
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if (!axesY.isEmpty()) axesY.first()->setTitleText(unit);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    layout->addWidget(new QLabel(variableName));
    layout->addWidget(chartView);
}

void SplitPlotWidget::updateData(const QVector<double>& x, const QVector<double>& y)
{
    QVector<QPointF> points;
    int N = qMin(x.size(), y.size());
    for(int i = 0; i < N; ++i)
        points.append(QPointF(x[i], y[i]));
    series->replace(points);

    // Axis handling
    QList<QAbstractAxis*> axesX = chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if(!points.isEmpty()) {
        if (!axesX.isEmpty())
            axesX.first()->setRange(points.first().x(), points.last().x());
        double minY = points.first().y(), maxY = points.first().y();
        for(const QPointF &pt : points) {
            if(pt.y() < minY) minY = pt.y();
            if(pt.y() > maxY) maxY = pt.y();
        }
        double yPad = (maxY - minY) * 0.1;
        if(yPad == 0) yPad = 1.0;
        if (!axesY.isEmpty())
            axesY.first()->setRange(minY - yPad, maxY + yPad);
    }
}

// -------- MainWindow Implementation --------

QString MainWindow::getYAxisUnit(int variableIndex) {
    if(variableIndex == 0 || variableIndex == 3 || variableIndex == 6) return "Volts";
    if(variableIndex == 1 || variableIndex == 4 || variableIndex == 7) return "rad";
    if(variableIndex == 2 || variableIndex == 5 || variableIndex == 8) return "deg";
    if(variableIndex == 9) return "Hz";
    if(variableIndex == 10) return "Hz/s";
    if(variableIndex >= 11 && variableIndex <= 14) return "a.u.";
    return "";
}

QString MainWindow::variableLabel(int idx) const {
    switch(idx) {
    case 0: return "Phase 1 Magnitude";
    case 1: return "Phase 1 Angle (rad)";
    case 2: return "Phase 1 Angle (deg)";
    case 3: return "Phase 2 Magnitude";
    case 4: return "Phase 2 Angle (rad)";
    case 5: return "Phase 2 Angle (deg)";
    case 6: return "Phase 3 Magnitude";
    case 7: return "Phase 3 Angle (rad)";
    case 8: return "Phase 3 Angle (deg)";
    case 9: return "Frequency";
    case 10: return "ROCOF";
    case 11: return "Analog 1";
    case 12: return "Analog 2";
    case 13: return "Analog 3";
    case 14: return "Analog 4";
    default: return "Var";
    }
}

QColor MainWindow::variableColor(int idx) const {
    static const QVector<QColor> colors = {
        Qt::red, Qt::blue, Qt::darkGreen, Qt::magenta, Qt::darkCyan, Qt::darkYellow, Qt::darkRed,
        Qt::darkBlue, Qt::green, Qt::cyan, Qt::yellow, Qt::black, Qt::gray, Qt::darkMagenta, Qt::darkGray
    };
    return colors[idx % colors.size()];
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    dataBuffers.resize(15);
    for(auto& buffer : dataBuffers) buffer.reserve(10000);
    timeBuffer.reserve(10000);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);

    // Auto-connect on startup
    socket->connectToHost("localhost", 4712);
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Top controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    controlsLayout->addWidget(new QLabel("Select Variable:"));
    variableCombo = new QComboBox();
    for(int phase = 1; phase <= 3; ++phase) {
        variableCombo->addItem(QString("Phase %1 Magnitude").arg(phase));
        variableCombo->addItem(QString("Phase %1 Angle (rad)").arg(phase));
        variableCombo->addItem(QString("Phase %1 Angle (deg)").arg(phase));
    }
    variableCombo->addItem("Frequency");
    variableCombo->addItem("ROCOF");
    for(int analog = 1; analog <= 4; ++analog)
        variableCombo->addItem(QString("Analog %1").arg(analog));
    connect(variableCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboChanged);
    controlsLayout->addWidget(variableCombo);

    controlsLayout->addSpacing(15);

    controlsLayout->addWidget(new QLabel("Window Size (s):"));
    windowSpinBox = new QDoubleSpinBox();
    windowSpinBox->setRange(0.1, 100.0);
    windowSpinBox->setDecimals(2);
    windowSpinBox->setSingleStep(0.1);
    windowSpinBox->setValue(windowSizeSec);
    connect(windowSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onWindowSizeChanged);
    controlsLayout->addWidget(windowSpinBox);

    controlsLayout->addSpacing(15);

    splitViewButton = new QPushButton("Split View");
    closeSplitButton = new QPushButton("Close Split");
    connect(splitViewButton, &QPushButton::clicked, this, &MainWindow::onSplitViewClicked);
    connect(closeSplitButton, &QPushButton::clicked, this, &MainWindow::onCloseSplitView);
    controlsLayout->addWidget(splitViewButton);
    controlsLayout->addWidget(closeSplitButton);
    closeSplitButton->setVisible(false);

    mainLayout->addLayout(controlsLayout);

    // Splitter for plots
    splitter = new QSplitter(Qt::Horizontal);
    mainPlotWidget = new QWidget();
    QVBoxLayout *mainPlotLayout = new QVBoxLayout(mainPlotWidget);

    // Main plot setup
    series = new QSplineSeries();
    series->setColor(variableColor(0));
    chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->legend()->hide();
    chart->setTitle("Real-Time PMU Data Visualization");
    QList<QAbstractAxis*> axesX = chart->axes(Qt::Horizontal);
    if (!axesX.isEmpty())
        axesX.first()->setTitleText("Time (s)");
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if (!axesY.isEmpty())
        axesY.first()->setTitleText(getYAxisUnit(0));
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(350);
    mainPlotLayout->addWidget(chartView);
    mainPlotWidget->setLayout(mainPlotLayout);

    splitter->addWidget(mainPlotWidget);
    mainLayout->addWidget(splitter);

    // Scrollbar
    QHBoxLayout *scrollLayout = new QHBoxLayout();
    hScrollBar = new QScrollBar(Qt::Horizontal);
    int maxPoints = static_cast<int>(windowSizeSec / 0.02);
    hScrollBar->setRange(0, 0);
    hScrollBar->setPageStep(maxPoints);
    connect(hScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScrollBarChanged);
    connect(hScrollBar, &QScrollBar::sliderMoved, this, [this](){ autoScrollEnabled = false; });
    connect(hScrollBar, &QScrollBar::sliderReleased, this, [this](){
        if(hScrollBar->value() == hScrollBar->maximum())
            autoScrollEnabled = true;
    });
    scrollLayout->addWidget(hScrollBar);
    mainLayout->addLayout(scrollLayout);

    setCentralWidget(central);
}

void MainWindow::onReadyRead()
{
    while(socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        QList<QByteArray> values = line.split(',');

        if(values.size() != 15) continue;

        timeBuffer.append(timeBuffer.size());
        for(int i = 0; i < 15; ++i) {
            bool ok;
            double value = values[i].toDouble(&ok);
            if(ok) dataBuffers[i].append(value);
        }

        int dataSize = timeBuffer.size();
        int maxPoints = static_cast<int>(windowSizeSec / 0.02);
        hScrollBar->setRange(0, qMax(0, dataSize - maxPoints));
        hScrollBar->setPageStep(maxPoints);

        if(autoScrollEnabled && dataSize > maxPoints)
            hScrollBar->setValue(dataSize - maxPoints);

        updatePlot();
        updateSplitPlot();
    }
}

void MainWindow::onComboChanged(int index)
{
    currentVariable = index;
    series->setColor(variableColor(index));
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if (!axesY.isEmpty())
        axesY.first()->setTitleText(getYAxisUnit(index));
    updatePlot();
}

void MainWindow::onWindowSizeChanged(double newSizeSec)
{
    windowSizeSec = newSizeSec;
    int dataSize = timeBuffer.size();
    int maxPoints = static_cast<int>(windowSizeSec / 0.02);
    hScrollBar->setRange(0, qMax(0, dataSize - maxPoints));
    hScrollBar->setPageStep(maxPoints);
    if(autoScrollEnabled && dataSize > maxPoints)
        hScrollBar->setValue(dataSize - maxPoints);
    updatePlot();
    updateSplitPlot();
}

void MainWindow::onScrollBarChanged(int /*value*/)
{
    updatePlot();
    updateSplitPlot();
}

void MainWindow::onSplitViewClicked()
{
    if (splitPlotWidget) return;

    QStringList varList;
    for(int i=0; i<15; ++i) {
        if(i == currentVariable) continue;
        varList << variableLabel(i);
    }
    bool ok = false;
    QString selected = QInputDialog::getItem(this, "Select Variable for Split View",
                                             "Variable:", varList, 0, false, &ok);
    if (!ok || selected.isEmpty()) return;

    int varIdx = -1;
    for(int i=0, j=0; i<15; ++i) {
        if(i == currentVariable) continue;
        if(varList[j] == selected) { varIdx = i; break; }
        ++j;
    }
    if(varIdx < 0) return;

    splitVariable = varIdx;
    splitPlotWidget = new SplitPlotWidget(varIdx, variableLabel(varIdx), getYAxisUnit(varIdx), variableColor(varIdx));
    splitter->addWidget(splitPlotWidget);
    splitter->setSizes(QList<int>() << 1 << 1);
    closeSplitButton->setVisible(true);
    updateSplitPlot();
}

void MainWindow::onCloseSplitView()
{
    if (!splitPlotWidget) return;
    splitter->widget(1)->deleteLater();
    splitPlotWidget = nullptr;
    splitVariable = -1;
    closeSplitButton->setVisible(false);
}

void MainWindow::updatePlot()
{
    if(dataBuffers[currentVariable].isEmpty()) return;

    int scrollPos = hScrollBar->value();
    int dataSize = timeBuffer.size();
    int bufferSize = dataBuffers[currentVariable].size();
    int start = qMax(0, scrollPos);
    int maxPoints = static_cast<int>(windowSizeSec / 0.02);
    int end = qMin(qMin(dataSize, bufferSize), start + maxPoints);

    QVector<QPointF> points;
    points.reserve(maxPoints);
    for(int i = start; i < end; ++i)
        points.append(QPointF(timeBuffer[i] * 0.02, dataBuffers[currentVariable][i]));

    series->replace(points);

    QList<QAbstractAxis*> axesX = chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if(!points.isEmpty()) {
        if (!axesX.isEmpty())
            axesX.first()->setRange(points.first().x(), points.last().x());
        double minY = points.first().y(), maxY = points.first().y();
        for(const QPointF &pt : points) {
            if(pt.y() < minY) minY = pt.y();
            if(pt.y() > maxY) maxY = pt.y();
        }
        double yPad = (maxY - minY) * 0.1;
        if(yPad == 0) yPad = 1.0;
        if (!axesY.isEmpty())
            axesY.first()->setRange(minY - yPad, maxY + yPad);
    }
}

void MainWindow::updateSplitPlot()
{
    if (!splitPlotWidget || splitVariable < 0) return;

    if(dataBuffers[splitVariable].isEmpty()) return;

    int scrollPos = hScrollBar->value();
    int dataSize = timeBuffer.size();
    int bufferSize = dataBuffers[splitVariable].size();
    int start = qMax(0, scrollPos);
    int maxPoints = static_cast<int>(windowSizeSec / 0.02);
    int end = qMin(qMin(dataSize, bufferSize), start + maxPoints);

    QVector<double> x, y;
    for(int i = start; i < end; ++i) {
        x.append(timeBuffer[i] * 0.02);
        y.append(dataBuffers[splitVariable][i]);
    }
    splitPlotWidget->updateData(x, y);
}
