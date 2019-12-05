
#pragma once

#include <QtWidgets>
#include <qpushbutton.h>


class ImageButton:
	public QPushButton
{
public:
	ImageButton(QWidget* parent, bool expandToFit = false);
	~ImageButton();
	const QPixmap* pixmap() const;

public slots:
	void setPixmap(const QPixmap&);

	virtual bool hasHeightForWidth() const;
	virtual int heightForWidth(int w) const;

protected:
	void paintEvent(QPaintEvent *);

private:
	QPixmap image;
	QPixmap scaledImage;
	bool m_imageReplaced;
	bool m_expandToFit;
};

