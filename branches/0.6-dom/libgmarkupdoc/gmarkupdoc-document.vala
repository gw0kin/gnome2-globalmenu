using GLib;

[CCode (cprefix = "GMarkup", lower_case_cprefix = "g_markup_")]
namespace GMarkup {
	public class Document: Node {
		private HashTable<int, weak Node> _pool;
		private int _unique;
		/**
		 * A unique interger
		 */
		public int unique {get { return _unique++;}}
		public Document() {
			this.nodeType = NodeType.DOCUMENT;
			this.id = 0;
			this.name = "#DOCUMENT";
			this.anchored = true;
		}
		construct {
			_unique = 1;
			_pool = createPool();
			_pool.insert(this.id, this);
		}

		/**
		 * create a node of given type, should be overriden 
		 */
		public virtual weak Node createNode(NodeType type) {
			Node rt = new Node(this, unique, type);
			_pool.insert(rt.id, rt.ref() as Node);
			return _pool.lookup(rt.id);
		}
		/**
		 * create a Node that contains text in the document. 
		 *
		 *   @return a newly created node; never unref.
		 */
		public weak Node createText(string text) {
			weak Node rt = createNode(NodeType.TEXT);
			rt.name = "#TEXT";
			rt.value = text;
			return rt;
		}
		/**
		 * create a Node that contains special segments in the document. 
		 * Example, <?xml ....?>
		 *
		 *   @return a newly created node; never unref.
		 */
		public weak Node createSpecial(string text) {
			weak Node rt = createNode(NodeType.SPECIAL);
			rt.name = "#SPECIAL";
			rt.value = text;
			return rt;
		}
		/**
		 * create a Node that contains a tag in the document. 
		 * Example, <body name="mybody"/>
		 *	 @param tag  			'body' in our example;
		 *   @return a newly created node; never unref.
		 */
		public weak Node createElement(string tag) {
			weak Node rt = createNode(NodeType.ELEMENT);
			rt.name = tag;
			return rt;
		}
		/**
		 * find the node with the give id.
		 *
		 *   @return an existing node. never unref it. null if not found.
		 */
		public weak Node? getNode(int id) {
			return _pool.lookup(id);
		}
		/**
		 * find the first node matched by func
		 *
		 * @return the found node or null
		 */
		public weak Node? findNode(MatchNodeFunc func) {
			List<weak Node> list = _pool.get_values();
			foreach(weak Node node in list) {
				if(func(node)) return node;
			}	
			return null;
		}
		/**
		 * match a node
		 *
		 *   @return true if matched, false if not
		 */
		public delegate bool MatchNodeFunc (Node node);
		/**
		 * create a Node that contains a fragment
		 * Example, <body name="mybody"/>
		 *	 @param tag  			'body' in our example;
		 *   @return a newly created node; unref when not used. Never destroy it.
		 */
		public Node createFragment() {
			Node rt = new Node(this, -1,  NodeType.FRAGMENT);
			rt.name = "#FRAGMENT";
			return rt;
		}
		public Node createMeta() {
			Node rt = new Node(this, -1, NodeType.META);
			rt.name = "#META";
			rt.anchored = false;
			return rt;
		}
		/**
		 * create a Pool for storing nodes; used for implementation
		 *
		 */
		private HashTable<int, weak Node> createPool(){
			return new HashTable<int, weak Node>(direct_hash, direct_equal);
		}
		/**
		 * Destroy a Node if it is not used in any context. In other words,
		 * if it is a child of the orphan node.
         * Panic if the node is not a child of the orphan node.
		 *
		 *   @param node  		the node to be destroyed.
		 *
		 */
		public virtual void destroyNode(Node? node, bool recursive = true) {
			assert(node.parent == null);
			if(recursive) 
			node.transverse((node) => {
				debug("%d  %u", node.id, node.ref_count);
				if(node.ref_count != 1) {
					warning("maybe leaking a node!");
				}
				node.removeAll();
				this._pool.remove(node.id);
				node.unref();
			});
			else {
				this._pool.remove(node.id);
				node.unref();
			}
		}
		public void memcheck() {
			_pool.for_each((key, value) => {
						weak Node node = (Node) value;
						if(!node.anchored) 
							critical("node %d %p %s is not anchored; but exist in pool with ref_count=%u",
								node.id, node, node.name, node.ref_count
								);
					});
		}
		/**
		 * Emitted when an attribute of a node is updated
		 *
		 * 	 @param node 		the node
		 * 	 @param prop 		the name of the attribute.
		 */
		public abstract signal void updated(Node node, string? prop);
		/**
		 * Emitted when a node is inserted to another node.
		 * 	 @param parent 		the parent
		 * 	 @param node 		the node
		 * 	 @param pos 		the position where the insertion is made
		 */
		public abstract signal void inserted(Node parent, Node node, Node? ref_node);
		/**
		 * Emitted when a node is removed from another node. 
		 * Notice that the node is not destroyed by this removal.
		 * 	 @param parent 		the parent
		 * 	 @param node 		the node
		 */
		public abstract signal void removed(Node parent, Node node);
		public void change_id(Node node, int new_id) {
			_pool.remove(node.id);
			node.id = new_id;
			_pool.insert(node.id, node);
		}
		public void mergeMeta(Node meta, Node ref_root, Node? ref_node) {
			int meta_id;
			string  meta_name;
			weak Node meta_current;
			List<weak Node> children = meta.children.copy();
			foreach (weak Node node in children) {
				message("meta = %s", node.name);
				if(node.nodeType == NodeType.META) {
					meta_id = node.get("id").to_int();
					meta_name = node.get("name");
					List<weak Node> children2 = node.children.copy();
					foreach(weak Node child in children2) {
						message("child = %s", child.name);
						if(child.name == meta_name) {
							Node oldNode;
							oldNode = getNode(meta_id);
							if(oldNode != null) {
								message("old node = %s", oldNode.to_string());
							}
							if(oldNode != null && oldNode.anchored) {
								/*replace properties for an anchored node*/
								oldNode.clear();
								oldNode.value = child.value;
								oldNode.obtain_attributes(child);	
								mergeMeta(child, oldNode, null);
							}
							if(oldNode != null && !oldNode.anchored) {
							/*re id an unanchred node, then let the meta node take the id*/
								message("old id is notanchored");
								change_id(oldNode, unique);
								oldNode = null;
							}
							if(oldNode == null) {
								Node clone = child.clone();
								clone.id = meta_id;
								_pool.insert(meta_id, clone.ref() as Node);
								ref_root.insert(clone, ref_node);
								mergeMeta(child, clone, null);
							}
						} 
						node.remove(child);
						destroyNode(child);
					}
					meta.remove(node);
					node.unref();
				} else {
					meta.remove(node);
					destroyNode(node);
				}
			}
		}
	}
}