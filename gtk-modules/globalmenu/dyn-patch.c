#include <gtk/gtk.h>
#include "dyn-patch.h"
extern void dyn_patch_widget_patcher();
extern void	dyn_patch_menu_shell_patcher();
extern void	dyn_patch_menu_bar_patcher();
extern void dyn_patch_widget_unpatcher();
extern void	dyn_patch_menu_shell_unpatcher();
extern void	dyn_patch_menu_bar_unpatcher();

/*
 * _USE_CLOSURES doesn't help improving the performance.
 * */

static GQuark __MENUBAR__ = 0;
static GQuark __DIRTY__ = 0;
static GQuark __OLD_SUBMENU__ = 0;
static GQuark __ITEM__  =  0;
static GQuark __LABEL_NOTIFY_CLOSURE__ = 0;
static GQuark __SUBMENU_NOTIFY_CLOSURE__ = 0;
static gulong SIGNAL_NOTIFY = 0;
static GQuark DETAIL_SUBMENU = 0;
static GQuark DETAIL_LABEL = 0;

static GTimer * timer = NULL;
static gulong buffered_changes = 0;

void dyn_patch_init () {
	__MENUBAR__ = g_quark_from_string("__menubar__");
	__DIRTY__ = g_quark_from_string("__dirty__");
	__OLD_SUBMENU__ = g_quark_from_string("__old_submenu__");
	__ITEM__ = g_quark_from_string("__item__");
	__LABEL_NOTIFY_CLOSURE__ = g_quark_from_string("__label_notify_closure__");
	__SUBMENU_NOTIFY_CLOSURE__ = g_quark_from_string("__submenu_notify_closure__");
	SIGNAL_NOTIFY = g_signal_lookup("notify", G_TYPE_OBJECT);
	DETAIL_SUBMENU = g_quark_from_string("submenu");
	DETAIL_LABEL = g_quark_from_string("label");

	dyn_patch_type_r(GTK_TYPE_WIDGET, dyn_patch_widget_patcher);
	dyn_patch_type_r(GTK_TYPE_MENU_SHELL, dyn_patch_menu_shell_patcher);
	dyn_patch_type_r(GTK_TYPE_MENU_BAR, dyn_patch_menu_bar_patcher);

	timer = g_timer_new();
	g_timer_stop(timer);
}
void dyn_patch_uninit() {
	g_timer_destroy(timer);
	dyn_patch_type_r(GTK_TYPE_WIDGET, dyn_patch_widget_unpatcher);
	dyn_patch_type_r(GTK_TYPE_MENU_SHELL, dyn_patch_menu_shell_unpatcher);
	dyn_patch_type_r(GTK_TYPE_MENU_BAR, dyn_patch_menu_bar_unpatcher);
}
void dyn_patch_type_r(GType type, DynPatcherFunc patcher) {
	GType * children = g_type_children(type, NULL);
	int i;
	patcher(type);
	for(i = 0; children[i]; i++) {
		dyn_patch_type_r(children[i], patcher);
	}
	g_free(children);
}
static gboolean _dyn_patch_emit_changed(GtkMenuBar * menubar) {
	g_message("Changed: %p", menubar);
	g_object_set_qdata((GObject*)menubar, __DIRTY__, NULL);
	g_signal_emit_by_name(menubar, "changed", 0, NULL);
	g_message("_dyn_patch_set_menu_bar_r consumption: %lf, buffered_changes = %ld ", g_timer_elapsed(timer, NULL), buffered_changes);
	buffered_changes = 0;

	g_timer_reset(timer);
	g_timer_stop(timer);
	return FALSE;
}
void dyn_patch_queue_changed(GtkMenuBar * menubar, GtkWidget * widget) {
	buffered_changes++;
	if(g_object_get_qdata((GObject*)menubar, __DIRTY__)) return;
	g_object_set_qdata((GObject*) menubar, __DIRTY__, GINT_TO_POINTER(1));
	g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc) _dyn_patch_emit_changed, g_object_ref(menubar), g_object_unref);
}

GtkMenuBar * dyn_patch_get_menubar(GtkWidget * widget) {
	if(GTK_IS_MENU_BAR(widget)) return widget;
	return g_object_get_qdata((GObject*)widget, __MENUBAR__);
}
void dyn_patch_set_menubar(GtkWidget * widget, GtkMenuBar * menubar) {
	if(menubar != NULL) {
		g_object_set_qdata_full((GObject*) widget, __MENUBAR__, g_object_ref(menubar), g_object_unref);
	} else {
		g_object_set_qdata((GObject*) widget, __MENUBAR__, NULL);
	}
}
static void _dyn_patch_simple_notify(GtkWidget * widget, GParamSpec * pspec, GtkMenuBar * menubar) {
	dyn_patch_queue_changed(menubar, widget);
}
static void _dyn_patch_submenu_notify(GtkWidget * widget, GParamSpec * pspec, GtkMenuBar * menubar) {
	GtkWidget * old_submenu = g_object_get_qdata((GObject*) widget, __OLD_SUBMENU__);
	GtkWidget * submenu = gtk_menu_item_get_submenu((GtkMenuItem*)widget);
	g_message("submenu changed %p %p", widget, submenu);
	if(submenu != old_submenu) {
		if(old_submenu) {
			dyn_patch_set_menubar_r(old_submenu, NULL);
		}
		if(submenu) {
			dyn_patch_set_menubar_r(submenu, menubar);
			g_object_set_qdata_full((GObject*) widget, __OLD_SUBMENU__, g_object_ref(submenu), g_object_unref); 
		} else {
			g_object_set_qdata((GObject*) widget, __OLD_SUBMENU__, NULL); 
		}
		/* although the property already hold a reference, 
		 * we want to ensure old_submenu above is still alive
		 * */
		dyn_patch_queue_changed(menubar, widget);
	}
}

void dyn_patch_set_menubar_r(GtkWidget * widget, GtkMenuBar * menubar) {
	g_timer_continue(timer);
	GtkWidget * old = (GtkWidget*) dyn_patch_get_menubar(widget);
	if(old != menubar) {
		if(old && GTK_IS_LABEL(widget))
			g_signal_handlers_disconnect_by_func(widget, 
					_dyn_patch_simple_notify, 
					menubar);
		if(old && GTK_IS_MENU_ITEM(widget)) {
			g_signal_handlers_disconnect_by_func(widget, 
					_dyn_patch_submenu_notify, 
					menubar);
			g_signal_handlers_disconnect_by_func(widget, 
					_dyn_patch_simple_notify, 
					menubar);
		}
		if(menubar && GTK_IS_CHECK_MENU_ITEM(widget)) {
			g_signal_handlers_disconnect_by_func(widget, 
					_dyn_patch_simple_notify, 
					menubar);
		}
	}
	g_timer_stop(timer);
	dyn_patch_set_menubar(widget, menubar);

	if(GTK_IS_CONTAINER(widget)) {
		GList * children = gtk_container_get_children((GtkContainer*)widget);
		GList * node;
		for(node = children; node; node = node->next) {
			dyn_patch_set_menubar_r(node->data, menubar);
		}
	}
	if(GTK_IS_MENU_ITEM(widget)) {
		GtkWidget * submenu = gtk_menu_item_get_submenu((GtkMenuItem*)widget);
		if(submenu) {
			g_object_set_qdata_full((GObject*) submenu, __ITEM__, g_object_ref(widget), g_object_unref);
			dyn_patch_set_menubar_r(submenu, menubar);
		}
	}
	g_timer_continue(timer);
	if(menubar != old) {
		if(menubar && GTK_IS_LABEL(widget)) {
			g_signal_connect(widget, "notify::label", 
					_dyn_patch_simple_notify, menubar);
		}
		if(menubar && GTK_IS_MENU_ITEM(widget)) {
			g_signal_connect(widget, "notify::submenu", 
					_dyn_patch_submenu_notify, menubar);
			g_signal_connect(widget, "notify::visible", 
					_dyn_patch_simple_notify, menubar);
		}
		if(menubar && GTK_IS_CHECK_MENU_ITEM(widget)) {
			g_signal_connect(widget, "notify::active", 
					_dyn_patch_simple_notify, menubar);
			g_signal_connect(widget, "notify::inconsistent", 
					_dyn_patch_simple_notify, menubar);
			g_signal_connect(widget, "notify::draw-as-radio", 
					_dyn_patch_simple_notify, menubar);
		}
	}
	g_timer_stop(timer);
}

