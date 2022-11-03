#include "RealTimeCurveQChartWidget.h"
#include <QDateTime>
#include <QHBoxLayout>
RealTimeCurveQChartWidget::RealTimeCurveQChartWidget(QWidget *parent) : QWidget(parent) {
    maxSize = 31; // ֻ�洢���µ� 31 ������
    maxX = 300;
    maxY = 100;
    splineSeries = new QSplineSeries();
    scatterSeries = new QScatterSeries();
    scatterSeries->setMarkerSize(8);
    chart = new QChart();
    chart->addSeries(splineSeries);
    chart->addSeries(scatterSeries);
    chart->legend()->hide();
    chart->setTitle("ʵʱ��̬����");
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, 300);
    chart->axisY()->setRange(0, maxY);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    setLayout(layout);
    timerId = startTimer(200);
    qsrand(QDateTime::currentDateTime().toTime_t());
}
RealTimeCurveQChartWidget::~RealTimeCurveQChartWidget() {
}
void RealTimeCurveQChartWidget::timerEvent(QTimerEvent *event) {
    // ����һ�����ݣ�ģ�ⲻͣ�Ľ��յ�������
    if (event->timerId() == timerId) {
        int newData = qrand() % (maxY + 1);
        dataReceived(newData);
    }
}
void RealTimeCurveQChartWidget::dataReceived(int value) {
    data << value;
    // ���ݸ��������������������ɾ�����Ƚ��յ������ݣ�ʵ��������ǰ�ƶ�
    while (data.size() > maxSize) {
        data.removeFirst();
    }
    // ���汻���غ��û�б�Ҫ�������ݵ�������
    if (isVisible()) {
        splineSeries->clear();
        scatterSeries->clear();
        int dx = maxX / (maxSize - 1);
        int less = maxSize - data.size();
        for (int i = 0; i < data.size(); ++i) {
            splineSeries->append(less*dx + i * dx, data.at(i));
            scatterSeries->append(less*dx + i * dx, data.at(i));
        }
    }
}