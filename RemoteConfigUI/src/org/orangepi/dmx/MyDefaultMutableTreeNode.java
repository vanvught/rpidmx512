
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
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
