

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QScrollBar>
#include <QPushButton>
#include <QSplitter>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QColor>



class SplitPlotWidget : public QWidget
{
    Q_OBJECT
public:
    SplitPlotWidget(int variableIndex, QString variableName, QString unit, QColor color, QWidget *parent = nullptr);
    void updateData(const QVector<double>& x, const QVector<double>& y);

private:
    QChartView *chartView;
    QChart *chart;
    QSplineSeries *series;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onReadyRead();
    void onComboChanged(int index);
    void onWindowSizeChanged(double newSizeSec);
    void onScrollBarChanged(int value);
    void onSplitViewClicked();
    void onCloseSplitView();

private:
    void setupUI();
    void updatePlot();
    void updateSplitPlot();
    QString getYAxisUnit(int variableIndex);
    QString variableLabel(int idx) const;
    QColor variableColor(int idx) const;

    QTcpSocket *socket;
    QComboBox *variableCombo;
    QDoubleSpinBox *windowSpinBox;
    QScrollBar *hScrollBar;
    QPushButton *splitViewButton;
    QPushButton *closeSplitButton;

    QSplitter *splitter;
    QWidget *mainPlotWidget;
    SplitPlotWidget *splitPlotWidget = nullptr;

    QSplineSeries *series;
    QChart *chart;
    QChartView *chartView;

    QVector<QVector<double>> dataBuffers;
    QVector<double> timeBuffer;
    int currentVariable = 0;
    int splitVariable = -1;
    double windowSizeSec = 2.0;
    bool autoScrollEnabled = true;
};

#endif // MAINWINDOW_H
