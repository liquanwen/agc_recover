/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include "real_data.h"

#include "public_struct.h"

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axisX(new QValueAxis()),
    m_axisY(new QValueAxis()),
    m_step(0),
    m_x(0),
    m_y(0)
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &Chart::handleTimeout);
    m_timer.setInterval(1000);

    m_series = new QSplineSeries(this);

    //m_series = new QLineSeries(this);
    QPen green(Qt::red);
    green.setWidth(3);
    m_series->setPen(green);
    m_series->append(m_x, m_y);

    //m_series->append(2, 600);
    //m_series->append(3, 400);
    //m_series->append(4, 800);
    //m_series->append(7, 400);
    //m_series->append(10, 500);
   /* m_x = 11;*/

    addSeries(m_series);

    addAxis(m_axisX,Qt::AlignBottom);
    addAxis(m_axisY,Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_axisX->setTickCount(HorizontalAxis);
    m_axisX->setRange(0, HorizontalAxis);
    //m_axisX->setTickInterval(3);
    m_axisY->setRange(0, 3);
    m_axisY->setTickCount(VerticalAxis);
	m_axisX->setLabelFormat("%d");


    m_timer.start();
}

Chart::~Chart()
{

}

void Chart::handleTimeout()
{

    //qreal x = plotArea().width() / m_axisX->tickCount();
    qreal x = plotArea().width() / XMAXVALUE;
    qreal y = (m_axisX->max() - m_axisX->min()) / m_axisX->tickCount();
    m_x += 1;
    m_y = CRealDataManager::CreateInstance().m_cal_p_total * 0.001;

    m_series->append(m_x, m_y);



	this->addSeries(m_series); //给chart添加上这条线


	if (m_x >= HorizontalAxis && int(m_x) % HorizontalAxis == 0) {
		m_axisX->setMax(m_x + HorizontalAxis);
		m_axisX->setMin(m_x - 6);
	}
	this->addAxis(m_axisX, Qt::AlignBottom);
	m_series->attachAxis(m_axisX);


	this->addAxis(m_axisY, Qt::AlignLeft);
	m_series->attachAxis(m_axisY);

	this->legend()->setVisible(false);




    if (m_x > m_axisX->max())
    {
        //m_axisX->setRange(0, m_x + 1);
        scroll(x, 0);
    }

    //if (m_x >= XMAXVALUE)
    //{
    //    scroll(x, 0);
    //}

    if (m_y > m_axisY->max() && m_yl < m_y2)  //临时处理
    {
        m_axisY->setRange(0, m_y + 0.5);
    }
    m_yl = m_y;
    m_y2 = m_axisY->max();
    //if ((int)m_x % 3 == 1 && m_x >= 100)
    //{
 
    //}
    //if (m_x >= 300)
    //{
    //    m_x = 1;
    //}
    //    
}
