/* vi: set sw=4 ts=4: */
/*
 * app-document.cpp
 *
 * This file is part of ________.
 *
 * Copyright (C) 2008 - ubuntu <ubuntu@gmail.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 * */

#include "app-document.h"

#include <QtGlobal>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QXmlStreamReader>

#include <QMenu>
#include <QAction>
#include <QMenuBar>

#include <KIcon>

class AppModel {
public:
	AppModel(AppModel *parent = NULL) 
		:_parent(parent)
	{
		if (parent)
			parent->addChild(this);
	}

	~AppModel() 
	{
		foreach(AppModel *child, _children) {
			delete child;
		}
	}

	inline AppModel *parent() { return _parent;}
	inline const QList<AppModel*>& children(){ return _children;}
	inline const QHash<QString, QString>& properties() { return _properties;}

	inline void addProperty(const QString &name, const QString &value)
	{
		_properties.insert(name, value);
	}

	inline bool hasProperty(const QString &name) {
		return _properties.contains(name);
	}

	inline QString getProperty(const QString &name) 
	{
		return _properties.value(name);
	}

	inline void addChild(AppModel *child) 
	{
		_children.push_back(child);
	}

	static void printModel(AppModel *model)
	{
		foreach(AppModel *m, model->children())
		{
			foreach(QString key, m->properties().keys())
			{
				qDebug() << key << ":" << m->properties().value(key);
			}
			printModel(m);
		}
	}

private:
	QHash<QString, QString> _properties;
	AppModel *_parent;
	QList<AppModel*> _children;
};


static QHash<QString, QString> _table;
static bool isIconTableAdded;

static void addIconEntries()
{
	if(isIconTableAdded)
		return;
	isIconTableAdded = true;

	_table.insert("gtk-about", "");
	_table.insert("gtk-close", "document-close");
	_table.insert("gtk-quit", "document-quit");
	_table.insert("gtk-open", "document-open");
	_table.insert("gtk-save", "document-save");
}

static QIcon* iconFromGtkStock(const QString &stock)
{
	if(!isIconTableAdded)
		addIconEntries();
	if(!_table.contains(stock))
		return new KIcon("");
	else 
		return new KIcon(_table[stock]);
}


AppDocument::AppDocument(QString service, QString path)
	:_service(service),
	 _path(path),
	 _menubar(NULL)
{
	qDebug() << "Create AppDocument" << _service;
	if (!QDBusConnection::sessionBus().isConnected()) {
		qDebug() << "Can't connect to session bus";
	}

	QDBusConnection::sessionBus().connect("org.freedesktop.DBus", 
			"/org/freedesktop/DBus",
			"org.freedesktop.DBus", "NameOwnerChanged", this, 
			SLOT(onNameOwnerChanged(QString, QString, QString)));

	QDBusConnection::sessionBus().connect(_service, _path,
			"org.gnome.GlobalMenu.Document", "Inserted", this, 
			SLOT(onInserted(QString, QString, int)));

	QDBusConnection::sessionBus().connect(_service, _path,
			"org.gnome.GlobalMenu.Document", "Removed", this, 
			SLOT(onRemoved(QString, QString)));

	QDBusConnection::sessionBus().connect(_service, _path,
			"org.gnome.GlobalMenu.Document", "Updated", this, 
			SLOT(onUpdated(QString, QString)));

	_model = new AppModel();
	parseDocument();
//	AppModel::printModel(_model);
}

void AppDocument::parseDocument()
{
	QDBusInterface _DBusInterface(_service, _path, 
							  	  "org.gnome.GlobalMenu.Document");

	QDBusReply<QString> reply = _DBusInterface.call("QueryRoot", 0);
	if (reply.isValid()) {
		qDebug() << __func__ <<":QueryRoot OK!";
	}
	else qDebug() << reply.error().message();

	QXmlStreamReader xml;
	xml.addData(reply);

	QString currentTag;
	AppModel *currentModel = _model;
	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			AppModel *newModel = new AppModel(currentModel);
			currentModel = newModel;
			currentTag = xml.name().toString();
			newModel->addProperty("tag", currentTag);
			if (xml.name() == "window") {
				_xid = xml.attributes().value("xid").toString();
				_name = xml.attributes().value("name").toString();
				newModel->addProperty("xid", _xid);
				newModel->addProperty("name", _name);
				qDebug() << "window add property:" << _xid << _name;
			} else if (xml.name() == "menubar") {
				newModel->addProperty("name",
						xml.attributes().value("name").toString());
			} else if (xml.name() == "menu") {
				newModel->addProperty("name", 
						xml.attributes().value("name").toString());
			} else if (xml.name() == "imageitem") {
				newModel->addProperty("label", 
						xml.attributes().value("label").toString());
				newModel->addProperty("name", 
						xml.attributes().value("name").toString());
				newModel->addProperty("no-show-all", 
						xml.attributes().value("no-show-all").toString());
				newModel->addProperty("icon-stock", 
						xml.attributes().value("icon-stock").toString());
			} else if (xml.name() == "item") {
				newModel->addProperty("name",
						xml.attributes().value("name").toString());
				newModel->addProperty("label",
						xml.attributes().value("label").toString());
				newModel->addProperty("no-show-all",
						xml.attributes().value("no-show-all").toString());
				newModel->addProperty("visible",
						xml.attributes().value("visible").toString());
			} else if (xml.name() == "check") {
				newModel->addProperty("name",
						xml.attributes().value("name").toString());
				newModel->addProperty("inconsistent",
						xml.attributes().value("inconsistent").toString());
				newModel->addProperty("label",
						xml.attributes().value("label").toString());
				newModel->addProperty("no-show-all",
						xml.attributes().value("no-show-all").toString());
				newModel->addProperty("active",
						xml.attributes().value("active").toString());
				newModel->addProperty("draw-as-radio",
						xml.attributes().value("draw-as-radio").toString());
				newModel->addProperty("sensitive",
						xml.attributes().value("sensitive").toString());
				
			}
		} else if (xml.isEndElement()) {
			currentModel = currentModel->parent();
//			qDebug() << "end elemnt" << xml.name().toString();
		} else if (xml.isCharacters() && !xml.isWhitespace()) {
			qDebug() << "text" << xml.text().toString();
		}
		if (xml.hasError()) {
			qDebug() << xml.errorString();
			break;
		}
	}
}

void AppDocument::onInserted(QString pathName, QString nodeName, int pos)
{

}

void AppDocument::onRemoved(QString parentName, QString nodeName)
{
}

void AppDocument::onUpdated(QString nodeName, QString propName)
{
}

void AppDocument::onNameOwnerChanged(QString bus, 
									 QString oldOwner, 
									 QString newOwner)
{
	qDebug() << bus << oldOwner << newOwner;
}

static bool isMenu(AppModel *model)
{
	bool ret = false;

	if (model->getProperty("tag") == "imageitem" || 
		model->getProperty("tag") == "item") 
	{
		if (!model->children().isEmpty()) {
			AppModel *firstChild =  model->children().first();
			if (firstChild->getProperty("tag") == "menu")
				ret = true;
		}
	}

	return ret;
}

QMenu* AppDocument::createMenu(QWidget *parent, AppModel *model)
{
	AppModel *menuModel;
	QMenu *menu = NULL;
	QString label;
	menu = new QMenu(parent);
	
	// model itself is a item which describ the label and icon, 
	// and its first child should be a menu.
	menuModel = model->children().first(); 
	
	if (menuModel->getProperty("tag") != "menu") {
		qDebug() << __func__ << "Not a menu!!";
		return NULL;
	}


	foreach(AppModel *child, menuModel->children())
	{
		if (isMenu(child)) {
			menu->addMenu(createMenu(menu, child));
		} else if (child->getProperty("name").startsWith("GtkTearoffMenuItem") &&
					child->hasProperty("visible") &&
					child->getProperty("visible") == "true") {
			menu->setTearOffEnabled(true);
		} else
			menu->addAction(createAction(parent, child));
	}

	label = model->getProperty("label");
	label.replace(QString("_"), QString("&"));
	menu->setTitle(label);

	return menu;
}

QAction *AppDocument::createAction(QWidget *parent, AppModel *model)
{
	if (isMenu(model)) {
		qDebug() << __func__ << "Can't create action, it's menu rather that item";
		return NULL;
	}

	QAction *a = NULL;
	QString label = model->getProperty("label");

	a = new QAction(parent);
	a->setProperty("name", model->getProperty("name"));

    if (label.startsWith("|")) {// separator
		a->setSeparator(true);
		return a;
	}

	label.replace(QString("_"), QString("&"));
	a->setText(label);

	if (model->hasProperty("visible") && model->getProperty("visible") == "false")
		a->setVisible(false);

	if (model->getProperty("tag") == "imageitem" && model->hasProperty("icon-stock"))
	{
		QString iconStock = model->getProperty("icon-stock");
		QIcon *icon = iconFromGtkStock(iconStock);
		a->setIcon(*icon);
		delete icon;
	}

	if (model->getProperty("tag") == "check") 
	{
		a->setCheckable(true);
		if (model->hasProperty("inconsistent") && model->getProperty("inconsistent") == "false")
			qDebug() << __func__ << "inconsistent hasn't been implemented yet";

		if (model->hasProperty("active") && model->getProperty("active") == "true")
			a->setChecked(true);

		if (model->hasProperty("draw-as-radio") && model->getProperty("draw-as-radio") == "true")
			qDebug() << __func__ << "draw-as-radio hasn't been implemented yet";

	}

	connect(a, SIGNAL(triggered(bool)), this, SLOT(onActionTrigger(bool)));

	return a;
}

void AppDocument::onActionTrigger(bool triggerred)
{
//	qDebug() << __func__ << triggerred << sender()->property("name").toString();
	QString itemName = sender()->property("name").toString();
	QDBusInterface DBusIface(_service, _path, 
							  	"org.gnome.GlobalMenu.Document");
	DBusIface.call("Activate", itemName);
}

QMenuBar* AppDocument::createMenuBar()
{
	AppModel *it = _model;

	if(_menubar) 
		return _menubar;

	AppModel *child =  it->children().first()->children().first();
	if (child->getProperty("tag") == "menubar") {
		_menubar = new QMenuBar(NULL);
		foreach(AppModel *cchild, child->children()) {
			if (isMenu(cchild)) {
				_menubar->addMenu(createMenu(_menubar, cchild));
			} else 
				_menubar->addAction(createAction(_menubar, cchild));
		}
	}	

	return _menubar;
}



#include "app-document.moc"