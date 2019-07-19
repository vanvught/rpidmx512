/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;

public class LTCGenerator extends JDialog {
	private static final long serialVersionUID = -8623371832596143765L;
	private InetAddress IPAddressLtcNode; 
    private DatagramSocket socket = null;
	private JButton btnStart;
	private JButton btnStop;
	private JButton btnResume;
	
	static final int UDP_PORT = 21571;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					LTCGenerator dialog = new LTCGenerator(InetAddress.getByName("192.168.2.250"));
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.Show();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the dialog.
	 */
	public LTCGenerator(InetAddress IPAddressLtcNode) {
		InitComponents();
		CreateEvents();
		
		this.IPAddressLtcNode = IPAddressLtcNode;
		
		 try {
			socket = new DatagramSocket();
			setTitle("LTC Node " + IPAddressLtcNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error: LTC Node " + IPAddressLtcNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	public void Show() {
		setVisible(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 322, 70);
		
		btnStart = new JButton("Start");		
		btnStop = new JButton("Stop");
		btnResume = new JButton("Resume");
		
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(16)
					.addComponent(btnStart)
					.addGap(18)
					.addComponent(btnStop)
					.addGap(18)
					.addComponent(btnResume)
					.addContainerGap(47, Short.MAX_VALUE))
		);
		
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(Alignment.TRAILING, groupLayout.createSequentialGroup()
					.addContainerGap(68, Short.MAX_VALUE)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnStart)
						.addComponent(btnStop)
						.addComponent(btnResume))
					.addGap(59))
		);
		
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		btnStart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!start");
			}
		});
		
		btnStop.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!stop");
			}
		});
		
		btnResume.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!resume");
			}
		});
	}
	
	private void SendUpd(String message) {
		byte[] buffer = message.getBytes();

		
		DatagramPacket UDPPacket = new DatagramPacket(buffer, buffer.length, IPAddressLtcNode, UDP_PORT);
		try {
			socket.send(UDPPacket);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
