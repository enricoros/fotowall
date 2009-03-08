#include "CPixmap.h"
#include <QtGui>

CPixmap::CPixmap(const QString &fileName) : QPixmap(fileName), m_filePath(fileName), m_isNVG(false), m_isBlackAndWhite(false)
{
}

CPixmap::CPixmap(const QPixmap &pixmap): QPixmap(pixmap){
}


void CPixmap::toNVG() {
	QImage img = this->toImage();
	QImage dest(img.size(), img.format());
	QColor pixel;
	for(int x=0; x<img.width();x++) {
		for (int y=0; y<img.height(); y++) {
			pixel = img.pixel(x, y);
			unsigned int average = (pixel.green()+ pixel.red() + pixel.blue()) / 3;
			pixel.setGreen(average);
			pixel.setBlue(average);
			pixel.setRed(average);
			dest.setPixel(x,y,pixel.rgb());
		}
	}
	 m_isNVG = true;
	*this = fromImage(dest);
}

void CPixmap::invertColors() {
	QImage img = this->toImage();
	img.invertPixels();
	*this = fromImage(img);
}

void CPixmap::flipH() {
	QImage img = this->toImage();
	QImage dest(img.size(), img.format());
	QRgb pixel;
	int width = img.width() -1;
	for(int x=0; x<width;x++) {
		for (int y=0; y<img.height(); y++) {
			pixel = img.pixel(x,y);
			dest.setPixel(width - x,y,pixel);
		}
	}
	*this = fromImage(dest);
}
void CPixmap::flipV() {
	QImage img = this->toImage();
	QImage dest(img.size(), img.format());
	QRgb pixel;
	int height = img.height() -1;
	for(int x=0; x<img.width() ;x++) {
		for (int y=0; y<height; y++) {
			pixel = img.pixel(x,y);
			dest.setPixel(x, height - y,pixel);
		}
	}
	*this = fromImage(dest);
}

void CPixmap::toBlackAndWhite() {
	QImage img = this->toImage();
	QImage dest(img.size(), img.format());
	QColor pixel;
	for(int x=0; x<img.width();x++) {
		for (int y=0; y<img.height(); y++) {
			pixel = img.pixel(x, y);
			unsigned int average = (pixel.green()+ pixel.red() + pixel.blue()) / 3;
			if(average > 127)
				average = 255;
			else
				average = 0;
			pixel.setGreen(average);
			pixel.setBlue(average);
			pixel.setRed(average);
			dest.setPixel(x,y,pixel.rgb());
		}
	}
	m_isBlackAndWhite = true;
	*this = fromImage(dest);
}

void CPixmap::luminosity(int value) {
	QImage img = this->toImage();
	QImage dest(img.size(), img.format());
	QColor pixel;
	for(int x=0; x<img.width();x++) {
		for (int y=0; y<img.height(); y++) {
			pixel = img.pixel(x, y);
			int green = pixel.green() + value;
			int red = pixel.red() + value;
			int blue = pixel.blue() + value;
			if(green > 255) green = 255;
			else if(green < 0) green = 0;
			if(red > 255) red = 255;
			else if(red < 0) red = 0;
			if(blue > 255) blue = 255;
			else if(blue <0) blue = 0;
			
			pixel.setGreen(green);
			pixel.setBlue(blue);
			pixel.setRed(red);
			dest.setPixel(x,y,pixel.rgb());
		}
	}
	*this = fromImage(dest);
}


