/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

package org.orangepi.dmx;

import java.awt.Color;
import java.awt.Desktop;
import java.awt.Font;
import java.io.IOException;
import java.net.URI;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.JTextPane;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;

public class About extends JDialog {
	private static final long serialVersionUID = 989628609152233931L;
	private final JPanel contentPanel = new JPanel();
	private JTextField txtArjan;
	private JTextField txtExample;

	public static void main(String[] args) {
		try {
			About dialog = new About();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public About() {
		getContentPane().setBackground(Color.WHITE);
		setModal(true);
		setResizable(false);
		setTitle("About");
		setBounds(100, 100, 385, 170);
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		contentPanel.setLayout(null);

		JTextPane htmlURL = new JTextPane();
		htmlURL.setEditable(false);
		htmlURL.setContentType("text/html");
		htmlURL.setText("<html>Website <a href=\"http://www.orangepi-dmx.org\">http://www.orangepi-dmx.org</a></html>");
		htmlURL.addHyperlinkListener(new HyperlinkListener() {
			@Override
			public void hyperlinkUpdate(HyperlinkEvent e) {
				if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
					String url = e.getURL().toString();
					try {
						Desktop.getDesktop().browse(URI.create(url));
					} catch (IOException e1) {
						e1.printStackTrace();
					}
				}
			}
		});

		JTextPane htmlGitHub = new JTextPane();
		htmlGitHub.setEditable(false);
		htmlGitHub.setContentType("text/html");
		htmlGitHub.setText(
				"<html>Source <a href=\"https://github.com/vanvught/rpidmx512\">https://github.com/vanvught/rpidmx512</a></html>\"");
		htmlGitHub.addHyperlinkListener(new HyperlinkListener() {
			@Override
			public void hyperlinkUpdate(HyperlinkEvent e) {
				if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
					String url = e.getURL().toString();
					try {
						Desktop.getDesktop().browse(URI.create(url));
					} catch (IOException e1) {
						e1.printStackTrace();
					}
				}
			}
		});

		JTextPane htmlDonate = new JTextPane();
		htmlDonate.setEditable(false);
		htmlDonate.setContentType("text/html");
		htmlDonate.setText(
				"<html>PayPal.Me <b>Donate</b> <a href=\"https://paypal.me/AvanVught\">https://paypal.me/AvanVught</a></html>\"");
		htmlDonate.addHyperlinkListener(new HyperlinkListener() {
			@Override
			public void hyperlinkUpdate(HyperlinkEvent e) {
				if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
					String url = e.getURL().toString();
					try {
						Desktop.getDesktop().browse(URI.create(url));
					} catch (IOException e1) {
						e1.printStackTrace();
					}
				}
			}
		});

		txtArjan = new JTextField();
		txtArjan.setBorder(null);
		txtArjan.setHorizontalAlignment(SwingConstants.CENTER);
		txtArjan.setText("(C) 2019-2020 Arjan van Vught");
		txtArjan.setEditable(false);
		txtArjan.setColumns(10);

		txtExample = new JTextField();
		txtExample.setBorder(null);
		txtExample.setFont(new Font("Courier New", Font.PLAIN, 10));
		txtExample.setEditable(false);
		txtExample.setText("Example");
		txtExample.setColumns(10);

		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(htmlURL, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(htmlDonate, GroupLayout.PREFERRED_SIZE, 388, GroupLayout.PREFERRED_SIZE)
							.addGap(37)
							.addComponent(contentPanel, GroupLayout.PREFERRED_SIZE, 457, GroupLayout.PREFERRED_SIZE))
						.addComponent(htmlGitHub, GroupLayout.PREFERRED_SIZE, 365, GroupLayout.PREFERRED_SIZE)))
				.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
					.addGroup(Alignment.LEADING, groupLayout.createSequentialGroup()
						.addContainerGap()
						.addComponent(txtExample, GroupLayout.PREFERRED_SIZE, 55, GroupLayout.PREFERRED_SIZE))
					.addComponent(txtArjan, Alignment.LEADING, GroupLayout.PREFERRED_SIZE, 381, GroupLayout.PREFERRED_SIZE))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(7)
					.addComponent(htmlURL, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(htmlGitHub, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addGap(8)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(contentPanel, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(htmlDonate, GroupLayout.PREFERRED_SIZE, 26, GroupLayout.PREFERRED_SIZE))
					.addGap(18)
					.addComponent(txtArjan, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(txtExample, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addContainerGap())
		);
		getContentPane().setLayout(groupLayout);

	}
}
