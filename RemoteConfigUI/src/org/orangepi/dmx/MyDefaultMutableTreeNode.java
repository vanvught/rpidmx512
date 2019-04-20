/*
 * http://tech.chitgoks.com/2009/11/15/sort-jtree/
 */

package org.orangepi.dmx;

import java.util.Collections;
import java.util.Comparator;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.MutableTreeNode;

public class MyDefaultMutableTreeNode extends DefaultMutableTreeNode {
	private static final long serialVersionUID = 6514969080472476543L;

	public MyDefaultMutableTreeNode() {
		super();
	}

	public MyDefaultMutableTreeNode(Object userObject, boolean allowsChildren) {
		super(userObject, allowsChildren);
	}

	public MyDefaultMutableTreeNode(Object userObject) {
		super(userObject);
	}

	@SuppressWarnings("unchecked")
	@Override
	public void insert(MutableTreeNode newChild, int childIndex) {
		super.insert(newChild, childIndex);
		Collections.sort(this.children, nodeComparator);
	}

	protected Comparator<?> nodeComparator = new Comparator<Object>() {
		@Override
		public int compare(Object o1, Object o2) {
			return o1.toString().compareToIgnoreCase(o2.toString());
		}

		@Override
		public boolean equals(Object obj) {
			return false;
		}
	};
}
