
#include "stdafx.h"

#include "ImageButton.h"


ImageButton::ImageButton(QWidget* parent, bool expandToFit) :
	QPushButton(parent),
	m_imageReplaced(false),
	m_expandToFit(expandToFit)
{
}


ImageButton::~ImageButton()
{
}

bool ImageButton::hasHeightForWidth() const
{
	if (image.isDetached() || image.isNull())
	{
		return false;
	}
	return false;
	//return true;
}

int ImageButton::heightForWidth(int w) const
{
	if (image.isDetached() || image.isNull())
	{
		return -1;
	}
	return image.size().height() * 1.0 / image.size().width() * w;
}

void ImageButton::paintEvent(QPaintEvent *event) 
{
	QPushButton::paintEvent(event);

	if (!image.isNull())
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);

		QSize pixSize = image.size();
		if (m_expandToFit)
		{
			pixSize.scale(event->rect().size(), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
		}
		else
		{
			pixSize.scale(event->rect().size(), Qt::AspectRatioMode::KeepAspectRatio);
		}

		pixSize.setWidth(pixSize.width() + 2);
		pixSize.setHeight(pixSize.height() + 2);
		//pixSize.expandedTo()

		if (scaledImage.rect().size() != pixSize || m_imageReplaced)
		{
			scaledImage = image.scaled(pixSize,
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation
			);
			m_imageReplaced = false;
		}

		//painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
		//painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceAtop);
		//painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_Overlay);
		painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_Darken);

		painter.drawPixmap(
			QPoint(event->rect().center().x() - pixSize.width()/2.0, 
				event->rect().center().y() - pixSize.height()/2.0),
			scaledImage);

		//painter.drawPixmap(QPoint(), scaledImage);
	}
}

const QPixmap* ImageButton::pixmap() const 
{
	return &image;
}

void ImageButton::setPixmap(const QPixmap &pixmap) 
{
	// force redraw each time image is set
	m_imageReplaced = true;

	image = pixmap;
}
