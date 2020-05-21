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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;

import javax.swing.DefaultListModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.border.EtchedBorder;

public class BroadcastSelect extends JDialog {
	private final int BUFFERSIZE = 1024;
	private static final int PORT = 0x2905;
	
	private static InetAddress localAddress;
	private static DatagramSocket socketReceive;	
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
	private JList<String> list;
	private JButton btnList;
	private JButton btnEnableDHCP;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {	
		try {
			localAddress = InetAddress.getByName("192.168.2.133");
			try {
				try {
					socketReceive = new DatagramSocket(null);
					socketReceive.setReuseAddress(true);
					SocketAddress sockaddr = new InetSocketAddress(localAddress, PORT);
					socketReceive.bind(sockaddr);
					socketReceive.setSoTimeout(1500);
				} catch (SocketException e) {
					e.printStackTrace();
				}
				
				BroadcastSelect dialog = new BroadcastSelect(localAddress, socketReceive);
				dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
				dialog.setVisible(true);
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (UnknownHostException e1) {
			e1.printStackTrace();
		}
	}

	public BroadcastSelect(InetAddress localAddress, DatagramSocket socketReceive) {
		BroadcastSelect.localAddress = localAddress;
		BroadcastSelect.socketReceive = socketReceive;
		
		setTitle("Broadcast");
				
		InitComponents();
		CreateEvents();
	}
	
	public void Show() {
		setVisible(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 413, 335);

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
		
		list = new JList<String>();
		list.setBorder(new EtchedBorder(EtchedBorder.LOWERED, null, null));
		
		btnList = new JButton("List");
		
		btnEnableDHCP = new JButton("Enable DHCP");
		btnEnableDHCP.setToolTipText("This will clear all static ip configuration");

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
						.addComponent(list, GroupLayout.DEFAULT_SIZE, 264, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnDisplayOff)
						.addComponent(btnDisplayOn))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(btnList)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnUptime)
							.addGap(7)
							.addComponent(btnVersion)
							.addPreferredGap(ComponentPlacement.RELATED, 43, Short.MAX_VALUE)
							.addComponent(btnEnableDHCP)
							.addGap(36)
							.addComponent(btnReboot))
						.addComponent(list, GroupLayout.DEFAULT_SIZE, 237, Short.MAX_VALUE))
					.addGap(19))
		);
		contentPanel.setLayout(gl_contentPanel);
	}
	
	private void CreateEvents() {
		btnDisplayOn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				broadcast("!display#1");
			}
		});
		
		btnDisplayOff.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				broadcast("!display#0");
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
				broadcast("?reboot##");
				JOptionPane.showMessageDialog(null, "Reboot message has been sent.");
			}
		});
		
		btnEnableDHCP.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				broadcast("#network.txt\nuse_dhcp=1");
			}
		});
	}
	
	private void ProcessBroadcast(String cmd) {	
		byte[] buffer = new byte[BUFFERSIZE];
		DatagramPacket dpack = new DatagramPacket(buffer, buffer.length);

		DefaultListModel<String> listModel = new DefaultListModel<>();

		try {
			broadcast(cmd);

			while (true) {
				socketReceive.receive(dpack);
				System.out.println(dpack.getAddress());

				StringBuilder str = new StringBuilder();

				str.append(dpack.getAddress());
				str.append(" ");
				str.append(new String(dpack.getData()));

				listModel.addElement(str.toString());
			}
		} catch (SocketTimeoutException e) {
//			 Toolkit.getDefaultToolkit().beep();
			System.out.println("No more messages.");
		} catch (IOException e) {
			e.printStackTrace();
		}

		ArrayList<String> collection = Collections.list(listModel.elements());
		Collections.sort(collection);
		listModel.clear();

		for (Object o : collection) {
			listModel.addElement((String) o);
		}

		list.setModel(listModel);
	}
	
	private void broadcast(String broadcastMessage)  {
		byte[] buffer = broadcastMessage.getBytes();

		DatagramPacket packet;
		try {
			packet = new DatagramPacket(buffer, buffer.length, InetAddress.getByName("255.255.255.255"), PORT);
			DatagramSocket socketBroadcast;
			try {
				socketBroadcast = new DatagramSocket(null);
				socketBroadcast.setReuseAddress(true);
				SocketAddress sockaddr = new InetSocketAddress(localAddress, PORT);
				socketBroadcast.bind(sockaddr);
				socketBroadcast.setBroadcast(true);
				socketBroadcast.send(packet);
				socketBroadcast.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (UnknownHostException e1) {
			e1.printStackTrace();
		}
	}
}

