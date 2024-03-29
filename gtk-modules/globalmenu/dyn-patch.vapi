[CCode (cheader_filename = "dyn-patch.h")]
namespace DynPatch {
[CCode (cname = "dyn_patch_init")]
	public void init();
[CCode (cname = "dyn_patch_uninit_vfuncs")]
	public void uninit_vfuncs();
[CCode (cname = "dyn_patch_uninit_final")]
	public void uninit_final();
[CCode (cname = "dyn_patch_get_window")]
	public weak Gtk.Window? get_window(Gtk.MenuBar menubar);
[CCode (cname = "dyn_patch_get_menubar")]
	public weak Gtk.MenuBar? get_menubar(Gtk.Widget widget);
[CCode (cname = "dyn_patch_get_is_local")]
	public bool get_is_local(Gtk.MenuBar menubar);
[CCode (cname = "dyn_patch_set_is_local")]
	public void set_is_local(Gtk.MenuBar menubar, bool is_local);
}
