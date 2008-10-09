using GLib;
[CCode (cprefix = "G", lower_case_cprefix = "g_", cheader_filename = "glib.h")]
namespace GLibCompat {
	[Compact]
	[CCode (free_function = "g_string_chunk_free")]
	public class StringChunk {
		public StringChunk (ulong size);
		public weak string insert(string str);
		public weak string insert_const(string str);
		public weak string insert_len(void * buffer, ulong len);
		public void clear();
	}
	[Compact]
	[Immutable]
	[CCode (cname = "char", const_cname = "const char", copy_function = "g_strdup", free_function = "g_free", cheader_filename = "stdlib.h,string.h,glib.h", type_id = "G_TYPE_STRING", marshaller_type_name = "STRING", get_value_function = "g_value_get_string", set_value_function = "g_value_set_string", type_signature = "s")]
	public class String {
		[CCode (cname = "g_strcanon")]
		public weak string canon(string valid_chars, char substitutor);
	}
	[CCode (lower_case_csuffix = "object_class")]
	public class ObjectClass : TypeClass {
		public weak ParamSpec find_property (string property_name);
		public weak ParamSpec[] list_properties ();
		public void install_property  (uint property_id, ParamSpec pspec);
	}
	[CCode (ref_function = "g_object_ref", unref_function = "g_object_unref", marshaller_type_name = "OBJECT", get_value_function = "g_value_get_object", set_value_function = "g_value_set_object", cheader_filename = "glib-object.h")]
	public class Object : TypeInstance {
	
		public void add_toggle_ref(ToggleNotifyFunc notify, void* data);
		public void remove_toggle_ref(ToggleNotifyFunc notify, void* data);
		public static delegate void ToggleNotifyFunc (void* data, GLib.Object object, bool is_last);
	}

	public class ParamSpecBoolean : ParamSpec {
		[CCode (cname = "g_param_spec_boolean")]
		public ParamSpecBoolean (string name, string nick, string blurb, bool defaultvalue, ParamFlags flags);
	}
}
namespace Gdk {
[CCode (cname = "GDK_WINDOW_XID", cheader_filename="gdk/gdkx.h")]
public static ulong XWINDOW(Gdk.Drawable drawable);
}

[CCode (cprefix = "Gtk", lower_case_cprefix = "gtk_")]
namespace GtkAQD {
	[CCode (cheader_filename = "gtk/gtk.h", cname = "GtkMenuShell")]
	public class MenuShell : Gtk.Container, Atk.Implementor, Gtk.Buildable {
		public GLib.List<Gtk.MenuItem> children;
		public weak Gtk.Widget active_menu_item;
		public weak Gtk.Widget parent_menu_shell;
		public uint button;
		public uint activate_time;
		public uint active;
		public uint have_grab;
		public uint have_xgrab;
		public uint ignore_leave;
		public uint menu_flag;
		public uint ignore_enter;
		public void activate_item (Gtk.Widget menu_item, bool force_deactivate);
		public void append ([CCode (type = "GtkWidget*")] Gtk.MenuItem child);
		public void deselect ();
		public bool get_take_focus ();
		public void prepend (Gtk.Widget child);
		public void select_first (bool search_sensitive);
		public void set_take_focus (bool take_focus);
		[NoWrapper]
		public virtual int get_popup_delay ();
		public virtual void select_item (Gtk.Widget menu_item);
		public bool take_focus { get; set; }
		public virtual signal void activate_current (bool force_hide);
		[HasEmitter]
		public virtual signal void cancel ();
		public virtual signal void cycle_focus (Gtk.DirectionType p0);
		[HasEmitter]
		public virtual signal void deactivate ();
		public virtual signal void move_current (Gtk.MenuDirectionType direction);
		public virtual signal bool move_selected (int distance);
		public virtual signal void selection_done ();
/* Below are in GtkAQD*/
		[HasEmitter]
		public virtual signal void insert(Gtk.Widget widget, int pos);
	}

	[CCode (cheader_filename = "gtk/gtk.h")]
	public class MenuBar : MenuShell, Atk.Implementor, Gtk.Buildable {
		public Gtk.PackDirection get_child_pack_direction ();
		public Gtk.PackDirection get_pack_direction ();
		[CCode (type = "GtkWidget*")]
		public MenuBar ();
		public void set_child_pack_direction (Gtk.PackDirection child_pack_dir);
		public void set_pack_direction (Gtk.PackDirection pack_dir);
		public Gtk.PackDirection child_pack_direction { get; set; }
		public Gtk.PackDirection pack_direction { get; set; }
		[NoAccessorMethod]
		public bool local {get; set;}
	}

	[CCode (cheader_filename = "gtk/gtk.h")]
	public class MenuItem : Gtk.Item, Atk.Implementor, Gtk.Buildable {
		public weak Gdk.Window event_window;
		public ushort toggle_size;
		public ushort accelerator_width;
		public weak string accel_path;
		public uint show_submenu_indicator;
		public uint submenu_placement;
		public uint submenu_direction;
		public uint right_justify;
		public uint timer_from_keypress;
		public uint from_menubar;
		public uint timer;
		public bool get_right_justified ();
		public weak Gtk.Widget get_submenu ();
		[CCode (type = "GtkWidget*")]
		public MenuItem ();
		[CCode (type = "GtkWidget*")]
		public MenuItem.with_label (string label);
		[CCode (type = "GtkWidget*")]
		public MenuItem.with_mnemonic (string label);
		public void set_accel_path (string accel_path);
		public void set_right_justified (bool right_justified);
		public void set_submenu (Gtk.Widget submenu);
		public Gtk.Menu submenu { get; set; }
		[HasEmitter]
		public virtual signal void activate ();
		public virtual signal void activate_item ();
		[HasEmitter]
		public virtual signal void toggle_size_allocate (int allocation);
		[HasEmitter]
		public virtual signal void toggle_size_request (void* requisition);
		public signal void label_set(Gtk.Label? label);
	}
}
[CCode (cprefix = "Gtk", lower_case_cprefix = "gtk_")]
namespace GtkCompat {
	[CCode (cheader_filename = "gtk/gtk.h", cname="GtkNotebook")]
	public class Notebook : Gtk.Container, Atk.Implementor, Gtk.Buildable {
		public int page_num(Gtk.Widget child);
	}
	[CCode (cheader_filename = "gtk/gtk.h")]
	public class Container : Gtk.Widget, Atk.Implementor, Gtk.Buildable {
		public virtual void forall (Gtk.Callback callback);
	}
	[CCode (cheader_filename = "gtk/gtk.h")]
	public class Widget : Gtk.Object, Atk.Implementor, Gtk.Buildable {
		public virtual signal void style_set (Gtk.Style? previous_style);
	}


}