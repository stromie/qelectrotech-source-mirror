/*
	Copyright 2006-2012 Xavier Guerrin
	This file is part of QElectroTech.
	
	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.
	
	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "customelementgraphicpart.h"

/**
	Ecrit les attributs de style dans un element XML
	@param qde L'element XML a modifier
	
*/
void CustomElementGraphicPart::stylesToXml(QDomElement &qde) const {
	QString css_like_styles;
	
	css_like_styles += "line-style:";
	if      (_linestyle == DashedStyle) css_like_styles += "dashed";
	if      (_linestyle == DottedStyle) css_like_styles += "dotted";
	else if (_linestyle == NormalStyle) css_like_styles += "normal";
	
	css_like_styles += ";line-weight:";
	if      (_lineweight == NoneWeight)   css_like_styles += "none";
	else if (_lineweight == ThinWeight)   css_like_styles += "thin";
	else if (_lineweight == NormalWeight) css_like_styles += "normal";
	
	css_like_styles += ";filling:";
	if      (_filling == NoneFilling)  css_like_styles += "none";
	else if (_filling == BlackFilling) css_like_styles += "black";
	else if (_filling == WhiteFilling) css_like_styles += "white";
	
	css_like_styles += ";color:";
	if      (_color == WhiteColor) css_like_styles += "white";
	else if (_color == BlackColor) css_like_styles += "black";
	
	qde.setAttribute("style", css_like_styles);
	qde.setAttribute("antialias", _antialiased ? "true" : "false");
}

/**
	Lit les attributs de style depuis un element XML
	@param qde L'element XML a analyser
*/
void CustomElementGraphicPart::stylesFromXml(const QDomElement &qde) {
	resetStyles();
	
	// recupere la liste des couples style / valeur
	QStringList styles = qde.attribute("style").split(";", QString::SkipEmptyParts);
	
	// analyse chaque couple
	QRegExp rx("^\\s*([a-z-]+)\\s*:\\s*([a-z-]+)\\s*$");
	foreach (QString style, styles) {
		if (!rx.exactMatch(style)) continue;
		QString style_name = rx.cap(1);
		QString style_value = rx.cap(2);
		if (style_name == "line-style") {
			if      (style_value == "dashed") _linestyle = DashedStyle;
			if      (style_value == "dotted") _linestyle = DottedStyle;
			else if (style_value == "normal") _linestyle = NormalStyle;
			// il n'y a pas de else car les valeurs non conformes sont ignorees (idem par la suite)
		} else if (style_name == "line-weight") {
			if      (style_value == "thin")   _lineweight = ThinWeight;
			else if (style_value == "normal") _lineweight = NormalWeight;
			else if (style_value == "none")   _lineweight = NoneWeight;
		} else if (style_name == "filling") {
			if      (style_value == "white") _filling = WhiteFilling;
			else if (style_value == "black") _filling = BlackFilling;
			else if (style_value == "none")  _filling = NoneFilling;
		} else if (style_name == "color") {
			if      (style_value == "black") _color = BlackColor;
			else if (style_value == "white") _color = WhiteColor;
		}
	}
	
	// recupere l'antialiasing
	_antialiased = qde.attribute("antialias") == "true";
}

/**
	Remet les styles par defaut
*/
void CustomElementGraphicPart::resetStyles() {
	_linestyle = NormalStyle;
	_lineweight = NormalWeight;
	_filling = NoneFilling;
	_color = BlackColor;
	_antialiased = false;
}

/**
	Applique les styles a un Qpainter
	@param painter QPainter a modifier
*/
void CustomElementGraphicPart::applyStylesToQPainter(QPainter &painter) const {
	// recupere le QPen et la QBrush du QPainter
	QPen pen = painter.pen();
	QBrush brush = painter.brush();
	
	// applique le style de trait
	if      (_linestyle == DashedStyle) pen.setStyle(Qt::DashLine);
	if      (_linestyle == DottedStyle) pen.setStyle(Qt::DotLine);
	else if (_linestyle == NormalStyle) pen.setStyle(Qt::SolidLine);
	
	// applique l'epaisseur de trait
	if      (_lineweight == NoneWeight)   pen.setColor(QColor(0, 0, 0, 0));
	else if (_lineweight == ThinWeight)   pen.setWidth(0);
	else if (_lineweight == NormalWeight) pen.setWidthF(1.0);
	
	// applique le remplissage
	if (_filling == NoneFilling) {
		brush.setStyle(Qt::NoBrush);
	} else if (_filling == BlackFilling) {
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::black);
	} else if (_filling == WhiteFilling) {
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::white);
	}
	
	// applique la couleur de trait
	if      (_color == WhiteColor) pen.setColor(QColor(255, 255, 255, pen.color().alpha()));
	else if (_color == BlackColor) pen.setColor(QColor(  0,   0,   0, pen.color().alpha()));
	
	
	// applique l'antialiasing
	painter.setRenderHint(QPainter::Antialiasing,          _antialiased);
	painter.setRenderHint(QPainter::TextAntialiasing,      _antialiased);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, _antialiased);
	
	painter.setPen(pen);
	painter.setBrush(brush);
}

/**
	Specifie la valeur d'une propriete de style donnee.
	@param property propriete a modifier. Valeurs acceptees :
		* line-style : type de trait (@see LineStyle)
		* line-weight : epaisseur du traut (@see LineWeight)
		* filling : couleur de remplissage (@see Color)
		* color : couleur du trait (@see Color)
		* antialias : utiliser l'antialiasing ou non (booleen)
	@param value Valeur a attribuer a la propriete
*/
void CustomElementGraphicPart::setProperty(const QString &property, const QVariant &value) {
	if (property == "line-style") {
		setLineStyle(static_cast<LineStyle>(value.toInt()));
	} else if (property == "line-weight") {
		setLineWeight(static_cast<LineWeight>(value.toInt()));
	} else if (property == "filling") {
		setFilling(static_cast<Filling>(value.toInt()));
	} else if (property == "color") {
		setColor(static_cast<Color>(value.toInt()));
	} else if (property == "antialias") {
		setAntialiased(value.toBool());
	}
}

/**
	Permet d'acceder a la valeur d'une propriete de style donnee.
	@param property propriete lue. Valeurs acceptees :
		* line-style : type de trait (@see LineStyle)
		* line-weight : epaisseur du traut (@see LineWeight)
		* filling : couleur de remplissage (@see Color)
		* color : couleur du trait (@see Color)
		* antialias : utiliser l'antialiasing ou non (booleen)
	@return La valeur de la propriete property
*/
QVariant CustomElementGraphicPart::property(const QString &property) {
	if (property == "line-style") {
		return(lineStyle());
	} else if (property == "line-weight") {
		return(lineWeight());
	} else if (property == "filling") {
		return(filling());
	} else if (property == "color") {
		return(color());
	} else if (property == "antialias") {
		return(antialiased());
	}
	return(QVariant());
}