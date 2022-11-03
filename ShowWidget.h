#pragma once

#include <QWidget>
#include "ui_ShowWidget.h"

class ShowWidget : public QWidget
{
	Q_OBJECT

public:
	ShowWidget(QWidget *parent = nullptr);
	~ShowWidget();

private:
	Ui::ShowWidgetClass ui;
};
