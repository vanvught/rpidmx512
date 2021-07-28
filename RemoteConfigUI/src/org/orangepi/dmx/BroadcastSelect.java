/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.TreeMap;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;

public class BroadcastSelect extends JDialog {
	private final int BUFFERSIZE = 1024;
	
	private RemoteConfig remoteConfig;
	private DatagramSocket socketReceive;	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	private JButton btnDisplayOn;
	private JButton btnDisplayOff;
	private JButton btnReboot;
	private JButton btnUptime;
	private JButton btnVersion;
	private JButton btnList;
	private JButton btnEnableDHCP;
	private JTextArea textArea;

	public BroadcastSelect(RemoteConfig remoteConfig, DatagramSocket socketReceive) {
		this.remoteConfig = remoteConfig;
		this.socketReceive = socketReceive;
		
		setTitle("Broadcast");
				
		InitComponents();
		CreateEvents();
	}
		
	private void InitComponents() {
		setBounds(100, 100, 533, 335);

		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		btnDisplayOn = new JButton("Display On");
		btnDisplayOff = new JButton("Display Off");
		btnReboot = new JButton("Reboot all");
		btnReboot.setToolTipText("This will reboot ALL Orange Pi's");
		btnReboot.setForeground(Color.RED);
		btnUptime = new JButton("Uptime");	
		btnVersion = new JButton("Version");
		
		btnList = new JButton("List");
		
		btnEnableDHCP = new JButton("Enable DHCP");
		btnEnableDHCP.setToolTipText("This will clear all static ip configuration");
		
		JScrollPane scrollPane = new JScrollPane();

		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(btnDisplayOn)
						.addComponent(btnList)
						.addComponent(btnUptime)
						.addComponent(btnVersion)
						.addComponent(btnReboot, GroupLayout.PREFERRED_SIZE, 90, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnEnableDHCP))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(btnDisplayOff)
						.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 260, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnDisplayOff)
						.addComponent(btnDisplayOn))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(btnList)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnUptime)
							.addGap(7)
							.addComponent(btnVersion)
							.addPreferredGap(ComponentPlacement.RELATED, 37, Short.MAX_VALUE)
							.addComponent(btnEnableDHCP)
							.addGap(36)
							.addComponent(btnReboot)
							.addGap(19))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(21)
							.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 241, Short.MAX_VALUE))))
		);
		
		textArea = new JTextArea();
		scrollPane.setViewportView(textArea);
		contentPanel.setLayout(gl_contentPanel);
	}
	
	private void CreateEvents() {
		btnDisplayOn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.broadcast("!display#1");
			}
		});
		
		btnDisplayOff.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.broadcast("!display#0");
			}
		});
		
		btnUptime.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				ProcessBroadcast("?uptime#");
			}
		});
		
		btnVersion.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				ProcessBroadcast("?version#");
			}
		});
		
		btnList.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				ProcessBroadcast("?list#");
			}
		});
		
		btnReboot.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.broadcast("?reboot##");
				JOptionPane.showMessageDialog(null, "Reboot message has been sent.");
			}
		});
		
		btnEnableDHCP.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.broadcast("#network.txt\nuse_dhcp=1");
			}
		});
	}
	
	private void ProcessBroadcast(String cmd) {	
		TreeMap<Integer, String> treeMap = new TreeMap<Integer, String>();
		
		Graphics g = getGraphics();
		
		textArea.setText("Searching..\n");
		update(g);
		
		for (int i = 0; i < 2; i++) {
			try {
				remoteConfig.broadcast(cmd);
				while (true) {
					byte[] buffer = new byte[BUFFERSIZE];
					DatagramPacket dpack = new DatagramPacket(buffer, buffer.length);
					socketReceive.receive(dpack);

					textArea.append(dpack.getAddress().toString() + "\n");
					update(g);
					
					final String s = new String(dpack.getData());
					
					if (!s.startsWith("?")) {
						StringBuilder str = new StringBuilder();
						str.append(dpack.getAddress()).append(" ").append(s);
						treeMap.put(ByteBuffer.wrap(dpack.getAddress().getAddress()).getInt(), str.toString());
					}
				}
			} catch (SocketTimeoutException e) {
				textArea.append("No replies\n");
				update(g);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (!treeMap.isEmpty()) {
			textArea.setText("");
//			update(g);

			Collection<String> values = treeMap.values();

			for (String value : values) {
				try {
					final String lines[] = value.split("\n");
					textArea.append(lines[0] + "\n");
				} catch (Exception e) {

				}
			}
		} else {
			textArea.append("No node's found\n");
		}
	}
}

