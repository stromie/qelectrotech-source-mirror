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
#include "titleblockpropertieswidget.h"
#include "qeticons.h"
#include "templatescollection.h"

/**
	Constructeur
	@param titleblock TitleBlockProperties a afficher
	@param current true pour afficher l'option "Date courante"
	@param parent QWidget parent
*/
TitleBlockPropertiesWidget::TitleBlockPropertiesWidget(const TitleBlockProperties &titleblock, bool current, QWidget *parent) :
	QWidget(parent),
	display_current_date(false),
	tbt_collection_(0)
{
	initWidgets(titleblock);
	initLayouts();
	connect(tabbar, SIGNAL(currentChanged(int)), stack_layout, SLOT(setCurrentIndex(int)));
	
	titleblock_current_date -> setVisible(display_current_date = current);
	setTitleBlockProperties(titleblock);
	
	// by default, we do not display the template combo box
	titleblock_template_label -> setVisible(false);
	titleblock_template_name  -> setVisible(false);
	titleblock_template_button_ -> setVisible(false);
}

/// Destructeur
TitleBlockPropertiesWidget::~TitleBlockPropertiesWidget() {
}

/**
	@return Les proprietes affichees par le widget
*/
TitleBlockProperties TitleBlockPropertiesWidget::titleBlockProperties() const {
	TitleBlockProperties prop;
	prop.title    = titleblock_title -> text();
	prop.author   = titleblock_author -> text();
	prop.filename = titleblock_filename -> text();
	prop.folio    = titleblock_folio -> text();
	if (titleblock_no_date -> isChecked()) {
		prop.useDate = TitleBlockProperties::UseDateValue;
		prop.date = QDate();
	} else if (titleblock_fixed_date -> isChecked()) {
		prop.useDate = TitleBlockProperties::UseDateValue;
		prop.date = titleblock_date -> date();
	} else if (display_current_date && titleblock_current_date -> isChecked()) {
		prop.useDate = TitleBlockProperties::CurrentDate;
		prop.date = QDate::currentDate();
	}
	
	QString current_template_name = currentTitleBlockTemplateName();
	if (!current_template_name.isEmpty()) prop.template_name = current_template_name;
	
	for (int i = 0 ; i < additional_fields_table -> rowCount() ; ++ i) {
		QTableWidgetItem *qtwi_name  = additional_fields_table -> item(i, 0);
		QTableWidgetItem *qtwi_value = additional_fields_table -> item(i, 1);
		if (!qtwi_name || !qtwi_value) continue;
		
		QString key = qtwi_name -> text();
		if (key.isEmpty()) continue;
		
		QString value = qtwi_value -> text();
		prop.context.addValue(key, value);
	}
	
	return(prop);
}

/**
	Specifie les proprietes que le widget doit afficher
	@param titleblock nouvelles proprietes affichees par le widget
*/
void TitleBlockPropertiesWidget::setTitleBlockProperties(const TitleBlockProperties &titleblock) {
	titleblock_title    -> setText(titleblock.title);
	titleblock_author   -> setText(titleblock.author);
	titleblock_filename -> setText(titleblock.filename);
	titleblock_folio    -> setText(titleblock.folio);
	if (display_current_date) {
		if (titleblock.useDate == TitleBlockProperties::CurrentDate) {
			titleblock_current_date -> setChecked(true);
		} else {
			if (titleblock.date.isNull()) {
				titleblock_no_date -> setChecked(true);
			} else {
				titleblock_fixed_date -> setChecked(true);
				titleblock_date -> setDate(titleblock.date);
			}
		}
	} else {
		if (titleblock.useDate == TitleBlockProperties::CurrentDate) {
			titleblock_fixed_date -> setChecked(true);
			titleblock_date -> setDate(QDate::currentDate());
		} else {
			if (titleblock.date.isNull()) {
				titleblock_no_date -> setChecked(true);
			} else {
				titleblock_fixed_date -> setChecked(true);
				titleblock_date -> setDate(titleblock.date);
			}
		}
	}
	
	if (!titleblock.template_name.isEmpty()) {
		int matching_index = titleblock_template_name -> findData(titleblock.template_name);
		if (matching_index != -1) {
			titleblock_template_name -> setCurrentIndex(matching_index);
		}
	}
}

/**
	@return true si le widget affiche la proposition "Date courante", false sinon
*/
bool TitleBlockPropertiesWidget::displayCurrentDate() const {
	return(display_current_date);
}

/**
	@return true si ce widget est en lecture seule, false sinon
*/
bool TitleBlockPropertiesWidget::isReadOnly() const {
	return(titleblock_title -> isReadOnly());
}

/**
	@param ro true pour passer ce widget en lecture seule, false sinon
*/
void TitleBlockPropertiesWidget::setReadOnly(bool ro) {
	titleblock_title        -> setReadOnly(ro);
	titleblock_author       -> setReadOnly(ro);
	titleblock_date         -> setReadOnly(ro);
	titleblock_filename     -> setReadOnly(ro);
	titleblock_folio        -> setReadOnly(ro);
	titleblock_no_date      -> setDisabled(ro);
	titleblock_current_date -> setDisabled(ro);
	titleblock_fixed_date   -> setDisabled(ro);
	titleblock_template_name -> setDisabled(ro);
	additional_fields_table  -> setDisabled(ro);
}

/**
	@param templates List of template names the dedicated combo box should
	display.
*/
void TitleBlockPropertiesWidget::setTitleBlockTemplatesList(const QList<QString> &templates) {
	titleblock_template_name -> clear();
	titleblock_template_name -> addItem(QET::Icons::TitleBlock, tr("Mod\350le par d\351faut"), QString());
	foreach (QString template_name, templates) {
		titleblock_template_name -> addItem(QET::Icons::TitleBlock, template_name, template_name);
	}
}

/**
	@param tbt_collection Collection from which title block templates should be read.
*/
void TitleBlockPropertiesWidget::setTitleBlockTemplatesCollection(TitleBlockTemplatesCollection *tbt_collection) {
	if (!tbt_collection) return;
	if (tbt_collection_ && tbt_collection != tbt_collection_) {
		// forget any connection with the previous collection
		disconnect(tbt_collection_, 0, this, 0);
	}
	
	tbt_collection_ = tbt_collection;
	updateTemplateList();
	connect(tbt_collection_, SIGNAL(changed(TitleBlockTemplatesCollection*, QString)), this, SLOT(updateTemplateList()));
}

/**
	@param visible true to display the title block templates list, false to
	hide it.
*/
void TitleBlockPropertiesWidget::setTitleBlockTemplatesVisible(bool visible) {
	titleblock_template_name  -> setVisible(visible);
	titleblock_template_label -> setVisible(visible);
	titleblock_template_button_ -> setVisible(visible);
}

/**
	@return the name of the currenlty selected title block template.
*/
QString TitleBlockPropertiesWidget::currentTitleBlockTemplateName() const {
	int index = titleblock_template_name -> currentIndex();
	if (index != -1) {
		return(titleblock_template_name -> itemData(index).toString());
	}
	return(QString());
}

/**
	Set the currently selected title block template.
	@param template_name Template to be selected
*/
void TitleBlockPropertiesWidget::setCurrentTitleBlockTemplateName(const QString &template_name) {
	int matching_index = titleblock_template_name -> findData(template_name);
	if (matching_index != -1) {
		titleblock_template_name -> setCurrentIndex(matching_index);
	}
}

/**
	Sets the text describing the acceptable format for keys when adding extra
	key/value pairs.
*/
void TitleBlockPropertiesWidget::refreshFieldsFormatLabel() {
	QString format_text = tr(
		"Les noms ne peuvent contenir que des lettres minuscules, des "
		"chiffres et des tirets."
	);
	
	if (highlightNonAcceptableKeys()) {
		format_text = QString("<span style=\"color: red;\">%1</span>").arg(format_text);
	}
	additional_fields_format_label -> setText(format_text);
}

/**
	Adds a row in the additional fields table if needed.
*/
void TitleBlockPropertiesWidget::checkTableRows() {
	refreshFieldsFormatLabel();
	if (!nameLessRowsCount()) {
		int new_idx = additional_fields_table -> rowCount();
		additional_fields_table -> setRowCount(new_idx + 1);
		additional_fields_table -> setItem(new_idx, 0, new QTableWidgetItem(""));
		additional_fields_table -> setItem(new_idx, 1, new QTableWidgetItem(""));
	}
}

/**
	Update the title block templates list.
*/
void TitleBlockPropertiesWidget::updateTemplateList() {
	if (!tbt_collection_) return;
	
	QString current_template_name = currentTitleBlockTemplateName();
	setTitleBlockTemplatesList(tbt_collection_ -> templates());
	setCurrentTitleBlockTemplateName(current_template_name);
}

/**
	Edit the currently selected title block template
*/
void TitleBlockPropertiesWidget::editCurrentTitleBlockTemplate() {
	emit(editTitleBlockTemplate(currentTitleBlockTemplateName(), false));
}

/**
	Duplicate the currently selected title block template (the user is asked
	for a name), then edit it.
*/
void TitleBlockPropertiesWidget::duplicateCurrentTitleBlockTemplate() {
	emit(editTitleBlockTemplate(currentTitleBlockTemplateName(), true));
}

/**
	Builds the various child widgets for this widget
*/
void TitleBlockPropertiesWidget::initWidgets(const TitleBlockProperties &titleblock) {
	// actions
	titleblock_template_edit_ = new QAction(tr("\311diter ce mod\350le", "menu entry"), this);
	titleblock_template_duplicate_ = new QAction(tr("Dupliquer et editer ce mod\350le", "menu entry"), this);
	
	connect(titleblock_template_edit_, SIGNAL(triggered()), this, SLOT(editCurrentTitleBlockTemplate()));
	connect(titleblock_template_duplicate_, SIGNAL(triggered()), this, SLOT(duplicateCurrentTitleBlockTemplate()));
	
	// menu
	titleblock_template_menu_ = new QMenu(tr("Title block templates actions"));
	titleblock_template_menu_ -> addAction(titleblock_template_edit_);
	titleblock_template_menu_ -> addAction(titleblock_template_duplicate_);
	
	// widgets
	titleblock_template_label = new QLabel(tr("Mod\350le :"), this);
	titleblock_template_name = new QComboBox(this);
	titleblock_template_button_ = new QPushButton(QET::Icons::TitleBlock, QString());
	titleblock_template_button_ -> setMenu(titleblock_template_menu_);
	
	titleblock_title    = new QLineEdit(this);
	titleblock_author   = new QLineEdit(this);
	titleblock_filename = new QLineEdit(this);
	
	titleblock_folio = new QLineEdit(this);
	folio_tip = new QLabel(
		tr(
			"Les variables suivantes sont utilisables dans le champ Folio :\n"
			"  - %id : num\351ro du sch\351ma courant dans le projet\n"
			"  - %total : nombre total de sch\351mas dans le projet"
		)
	);
	folio_tip -> setWordWrap(true);
	
	QButtonGroup *date_policy_group = new QButtonGroup(this);
	titleblock_no_date = new QRadioButton(tr("Pas de date"), this);
	titleblock_current_date = new QRadioButton(tr("Date courante"), this);
	titleblock_fixed_date = new QRadioButton(tr("Date fixe : "), this);
	date_policy_group -> addButton(titleblock_no_date);
	date_policy_group -> addButton(titleblock_current_date);
	date_policy_group -> addButton(titleblock_fixed_date);
	titleblock_date = new QDateEdit(QDate::currentDate(), this);
	titleblock_date -> setEnabled(titleblock_fixed_date -> isChecked());
	titleblock_current_date -> setVisible(display_current_date);
	connect(titleblock_fixed_date, SIGNAL(toggled(bool)), titleblock_date, SLOT(setEnabled(bool)));
	titleblock_date -> setCalendarPopup(true);
	
	// we add a bunch of tooltips for users to know how they can put these
	// values into their title block templates
	QString variable_tooltip = tr("Disponible en tant que %1 pour les mod\350les de cartouches.");
	titleblock_title -> setToolTip(QString(variable_tooltip).arg("%title"));
	titleblock_author -> setToolTip(QString(variable_tooltip).arg("%author"));
	titleblock_filename -> setToolTip(QString(variable_tooltip).arg("%filename"));
	titleblock_folio -> setToolTip(QString(variable_tooltip).arg("%folio"));
	QString date_variable_tooltip = QString(variable_tooltip).arg("%date");
	titleblock_current_date -> setToolTip(date_variable_tooltip);
	titleblock_fixed_date -> setToolTip(date_variable_tooltip);
	titleblock_date -> setToolTip(date_variable_tooltip);
	folio_tip -> setToolTip(tr("%id et %total sont disponibles en tant que %{folio-id} et %{folio-total} (respectivement) pour les mod\350les de cartouches."));
	
	// widgets for users to enter their own name/value pairs
	additional_fields_label = new QLabel(
		tr(
			"Vous pouvez d\351finir ici vos propres associations noms/valeurs pour"
			" que le cartouche en tienne compte. Exemple : associer le nom "
			"\"volta\" et la valeur \"1745\" remplacera %{volta} par 1745 dans le "
			"cartouche."
		)
	);
	additional_fields_label -> setWordWrap(true);
	additional_fields_label -> setAlignment(Qt::AlignJustify);
	additional_fields_format_label = new QLabel();
	additional_fields_format_label -> setWordWrap(true);
	additional_fields_format_label -> setAlignment(Qt::AlignJustify);
	
	int num_rows = titleblock.context.keys().count() + 1;
	additional_fields_table = new QTableWidget(num_rows, 2);
	additional_fields_table -> setHorizontalHeaderLabels(QStringList() << tr("Nom") << tr("Valeur"));
	additional_fields_table -> horizontalHeader() -> setStretchLastSection(true);
	
	int i = 0;
	foreach (QString key, titleblock.context.keys()) {
		additional_fields_table -> setItem(i, 0, new QTableWidgetItem(key));
		additional_fields_table -> setItem(i, 1, new QTableWidgetItem(titleblock.context[key].toString()));
		++ i;
	}
	
	refreshFieldsFormatLabel();
	connect(additional_fields_table, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(checkTableRows()));
	
	tabbar = new QTabBar(this);
	tabbar -> addTab(tr("Principales"));
	tabbar -> addTab(tr("Personnalis\351es"));
	tabbar -> setShape(QTabBar::RoundedSouth);
}

/**
	Builds the various layouts for this widget
*/
void TitleBlockPropertiesWidget::initLayouts() {
	// layouts for tab #0
	QGridLayout *layout_date = new QGridLayout();
	layout_date -> addWidget(titleblock_no_date,      0, 0);
	layout_date -> addWidget(titleblock_current_date, 1, 0);
	layout_date -> addWidget(titleblock_fixed_date,   2, 0);
	layout_date -> addWidget(titleblock_date,         2, 1);
	layout_date -> setColumnStretch(0, 1);
	layout_date -> setColumnStretch(1, 500);
	
	QWidget *widget_main_fields = new QWidget(this);
	QGridLayout *layout_main_fields = new QGridLayout(widget_main_fields);
	layout_main_fields -> addWidget(new QLabel(tr("Titre : ")),   0, 0);
	layout_main_fields -> addWidget(titleblock_title,             0, 1);
	layout_main_fields -> addWidget(new QLabel(tr("Auteur : ")),  1, 0);
	layout_main_fields -> addWidget(titleblock_author,            1, 1);
	layout_main_fields -> addWidget(new QLabel(tr("Date : ")),    2, 0, Qt::AlignTop);
	layout_main_fields -> addLayout(layout_date,                  2, 1);
	layout_main_fields -> addWidget(new QLabel(tr("Fichier : ")), 3, 0);
	layout_main_fields -> addWidget(titleblock_filename,          3, 1);
	layout_main_fields -> addWidget(new QLabel(tr("Folio : ")),   4, 0);
	layout_main_fields -> addWidget(titleblock_folio,             4, 1);
	layout_main_fields -> addWidget(folio_tip,                    5, 1, Qt::AlignTop);
	layout_main_fields -> setContentsMargins(0, 0, 0, 0);
	layout_main_fields -> setRowStretch(5, 500);
	
	// layouts for tab #1
	QWidget *widget_user_fields = new QWidget(this);
	QVBoxLayout *layout_user_fields = new QVBoxLayout(widget_user_fields);
	layout_user_fields -> addWidget(additional_fields_label);
	layout_user_fields -> addWidget(additional_fields_format_label);
	layout_user_fields -> addWidget(additional_fields_table);
	layout_user_fields -> setContentsMargins(0, 0, 0, 0);
	
	// stacked layout
	stack_layout = new QStackedLayout();
	stack_layout -> addWidget(widget_main_fields);
	stack_layout -> addWidget(widget_user_fields);
	stack_layout -> setContentsMargins(0, 0, 0, 0);
	stack_layout -> setCurrentIndex(0);
	
	// template layout
	QHBoxLayout *template_layout = new QHBoxLayout();
	template_layout -> addWidget(titleblock_template_label);
	template_layout -> addWidget(titleblock_template_name);
	template_layout -> addWidget(titleblock_template_button_);
	template_layout -> setStretch(0, 1);
	template_layout -> setStretch(1, 500);
	
	// groupbox layout
	QVBoxLayout *groupbox_layout = new QVBoxLayout();
	groupbox_layout -> addLayout(template_layout);
	groupbox_layout -> addLayout(stack_layout);
	groupbox_layout -> addWidget(tabbar);
	
	// groupbox
	QGroupBox *titleblock_infos = new QGroupBox(tr("Informations du cartouche"), this);
	titleblock_infos -> setLayout(groupbox_layout);
	titleblock_infos -> setMinimumSize(300, 330);
	
	// widget layout
	QVBoxLayout *this_layout = new QVBoxLayout();
	this_layout -> setContentsMargins(0, 0, 0, 0);
	this_layout -> addWidget(titleblock_infos);
	setLayout(this_layout);
}

/**
	@return The count of name-less rows in the additional fields table.
*/
int TitleBlockPropertiesWidget::nameLessRowsCount() const {
	int name_less_rows_count = 0;
	for (int i = 0 ; i < additional_fields_table -> rowCount() ; ++ i) {
		QTableWidgetItem *qtwi_name  = additional_fields_table -> item(i, 0);
		if (qtwi_name && qtwi_name -> text().isEmpty()) ++ name_less_rows_count;
	}
	return(name_less_rows_count);
}

/**
	Highlight keys that would not be accepted by a DiagramContext object.
	@return the number of highlighted keys.
*/
int TitleBlockPropertiesWidget::highlightNonAcceptableKeys() {
	static QRegExp re(DiagramContext::validKeyRegExp());
	
	QBrush fg_brush = additional_fields_table -> palette().brush(QPalette::WindowText);
	
	int invalid_keys = 0;
	for (int i = 0 ; i < additional_fields_table -> rowCount() ; ++ i) {
		QTableWidgetItem *qtwi_name  = additional_fields_table -> item(i, 0);
		if (!qtwi_name) continue;
		bool highlight = false;
		if (!qtwi_name -> text().isEmpty()) {
			if (!re.exactMatch(qtwi_name -> text())) {
				highlight = true;
				++ invalid_keys;
			}
		}
		qtwi_name -> setForeground(highlight ? Qt::red : fg_brush);
	}
	return(invalid_keys);
}