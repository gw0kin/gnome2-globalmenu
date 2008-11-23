using GLib;

[CCode (cprefix = "GMarkup", lower_case_cprefix = "g_markup_")]
namespace GMarkup {
	public class DocumentParser {
		private weak Node current;
		private DocumentModel document;
		private enum ParseType {
			CHILD, /*create a new child node*/
			ROOT, /*create a new document*/
			UPDATE, /*update a node nonrecursively*/
		}
		private int pos;
		private bool first;
		private weak Node result;
		private ParseType type;
		private weak string propname;
		public DocumentParser(DocumentModel document){
			this.document = document;
		}
		[NoArrayLength]
		private void StartElement (MarkupParseContext context, string element_name, string[] attribute_names, string[] attribute_values) {
			weak DocumentParser parser = (DocumentParser) this;
			weak string[] names = attribute_names;
			weak string[] values = attribute_values;
			names.length = (int) strv_length(attribute_names);
			values.length = (int) strv_length(attribute_values);
			switch(parser.type) {
				case ParseType.ROOT:
				case ParseType.CHILD:
					Tag node = parser.document.CreateTagWithAttributes(element_name,
						names,
						values
						);
					if((parser.type == ParseType.CHILD) && parser.first) {
						parser.first = false;
						if(parser.current != null)
							parser.current.insert(node, parser.pos);
						message("ref_count = %u", node.ref_count);
						parser.result = node;
					} else if(parser.current != null)
						parser.current.append(node);
							
					parser.current = node;
				break;
				case ParseType.UPDATE:
					if(parser.first) {
						(parser.current).freeze();
						if(parser.propname == null) {
							(parser.current as Tag).unset_all();
						} else {
							(parser.current as Tag).unset(parser.propname);
						}
						for(int i=0; i< names.length; i++) {
							if(parser.propname == null || names[i] == parser.propname)
								(parser.current as Tag).set(names[i], values[i]);
						}
						(parser.current).unfreeze();
						if(parser.propname == null) {
							parser.document.updated(parser.current, null);
						} else  {
							parser.document.updated(parser.current, parser.propname);
						}
						parser.first = false;
					}
				break;
			}
		}
		
		private void EndElement (MarkupParseContext context, string element_name) {
			weak DocumentParser parser = (DocumentParser) this;
			if(parser.type == ParseType.UPDATE) return;
			parser.current = parser.current.parent;
		}
		
		private void Text (MarkupParseContext context, string text, ulong text_len) {
			weak DocumentParser parser = (DocumentParser) this;
			if(parser.type == ParseType.UPDATE) return;
			if(text_len > 0) {
				string newtext = text.ndup(text_len);
				GMarkup.Text node = parser.document.CreateText(newtext);
				parser.current.append(node);
			}
		}
		
		private void Passthrough (MarkupParseContext context, string passthrough_text, ulong text_len) {
			weak DocumentParser parser = (DocumentParser) this;
			if(parser.type == ParseType.UPDATE) return;
			if(text_len > 0) {
				string newtext = passthrough_text.ndup(text_len);
				Special node = parser.document.CreateSpecial(newtext);
				parser.current.append(node);
			}
		}
		
		private static void Error (MarkupParseContext context, GLib.Error error) {

		}
		public bool parse (string foo) {
			MarkupParser parser_funcs = { StartElement, EndElement, Text, Passthrough, Error};
			MarkupParseContext context = new MarkupParseContext(parser_funcs, 0, (void*)this, null);
			current = document.root;
			type = ParseType.ROOT;
			try {
				context.parse(foo, foo.size());
			} catch(MarkupError e) {
				warning("%s", e.message);
				return false;
			}
			return true;
		}
		public Node? parse_tag (string foo) {
			MarkupParser parser_funcs = { StartElement, EndElement, Text, Passthrough, Error};
			MarkupParseContext context = new MarkupParseContext(parser_funcs, 0, (void*)this, null);
			type = ParseType.CHILD;
			current = null;
			this.pos = -1;
			this.first = true;
			this.result = null;
			try {
				context.parse(foo, foo.size());
			} catch(MarkupError e) {
				warning("%s", e.message);
				return null;
			}
			return result;
		}
		public bool parse_child(Node parent, string foo, int pos) {
			MarkupParser parser_funcs = { StartElement, EndElement, Text, Passthrough, Error};
			MarkupParseContext context = new MarkupParseContext(parser_funcs, 0, (void*)this, null);
			type = ParseType.CHILD;
			this.pos = pos;
			this.first = true;
			current = parent;
			try {
				context.parse(foo, foo.size());
			} catch(MarkupError e) {
				warning("%s", e.message);
				return false;
			}
			return true;
		}
		public bool update_tag(Tag node, string? propname, string foo) {
			MarkupParser parser_funcs = { StartElement, EndElement, Text, Passthrough, Error};
			MarkupParseContext context = new MarkupParseContext(parser_funcs, 0, (void*)this, null);
			type = ParseType.UPDATE;
			current = node;
			first = true;
			this.propname = propname;
			try {
				context.parse(foo, foo.size());
			} catch(MarkupError e) {
				warning("%s", e.message);
				return false;
			}
			return true;

		}
		public static int test (string [] args){
			DocumentModel document = new Document();
			DocumentParser parser = new DocumentParser(document);
			parser.parse(
"""
<html><title>title</title>
<body name="body">
<div name="header">
	<h1> This is a header</h1>
</div>
<div name="content"></div>
<div name="tail"><br/></div>
</body>
"""
			);
			print("back to string %s\n", parser.document.root.to_string());
			return 0;
		}
	}
}