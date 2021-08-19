/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.text.NumberFormat;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.text.NumberFormatter;

public class UdpDmxTransmit extends JDialog {
	//
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	//
	static final int UDP_PORT = 5120;
	private InetAddress ipAddressNode; 
    private DatagramSocket datagramSocket = null;
	//
	private JButton btnBreakTime;
	private JButton btnMabTime;
	private JButton btnRefreshRate;
	private JButton btnSlots;
	private NumberFormatter formatterMaBTime;
	private JFormattedTextField formattedTextFieldMabTime;
	private JComboBox<Integer> comboBoxBreakTime;
	private JComboBox<Integer> comboBoxRefreshRate;
	private JComboBox<Integer> comboBoxSlots;
	private JButton btnReset;

	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					UdpDmxTransmit dialog = new UdpDmxTransmit(InetAddress.getByName("192.168.2.120"));
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	public UdpDmxTransmit(InetAddress ipAddress) {
		this.ipAddressNode = ipAddress;
		
		InitComponents();
		CreateEvents();
		
		 try {
			 datagramSocket = new DatagramSocket();
			setTitle("DMX Transmit: Node " + ipAddressNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error:  Node " + ipAddressNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	private void InitComponents() {
		setBounds(100, 100, 340, 178);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		JLabel lblUsBreak = new JLabel("μs");
		JLabel lblusMab = new JLabel("μs");
		
	    NumberFormat format = NumberFormat.getInstance();		
	    formatterMaBTime = new NumberFormatter(format);
	    formatterMaBTime.setValueClass(Integer.class);
	    formatterMaBTime.setMinimum(12);
	    formatterMaBTime.setMaximum(1000000);
	    formatterMaBTime.setAllowsInvalid(false);
	    formatterMaBTime.setCommitsOnValidEdit(true);

		formattedTextFieldMabTime = new JFormattedTextField(formatterMaBTime);
		formattedTextFieldMabTime.setText("12");
		formattedTextFieldMabTime.setColumns(16);
		
		btnBreakTime = new JButton("Set BREAK time");
		btnMabTime = new JButton("Set MAB time");
		btnRefreshRate = new JButton("Set refresh rate");
		btnSlots = new JButton("Set slots");
		
		comboBoxRefreshRate = new JComboBox<Integer>();
		comboBoxRefreshRate.setToolTipText("0 = as fast as possible");
		for (int refreshRate = 0; refreshRate <=44; refreshRate++) {
			comboBoxRefreshRate.addItem(refreshRate);
		}
		comboBoxRefreshRate.setSelectedIndex(40);
		
		comboBoxBreakTime = new JComboBox<Integer>();
		comboBoxBreakTime.setToolTipText("92 - 176");
		for (int breakTime = 92; breakTime <=176; breakTime++) {
			comboBoxBreakTime.addItem(breakTime);
		}
		comboBoxBreakTime.setSelectedItem(176);
		
		comboBoxSlots = new JComboBox<Integer>();
		comboBoxSlots.setToolTipText("2 - 512 (even slots only)");
		for (int slots = 2; slots <=512; slots = slots + 2) {
			comboBoxSlots.addItem(slots);
		}
		comboBoxSlots.setSelectedItem(512);
		
		btnReset = new JButton("Reset");
		btnReset.setToolTipText("Set all defaults [does not send]");
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(btnRefreshRate)
						.addComponent(btnSlots)
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING, false)
							.addComponent(btnMabTime, Alignment.LEADING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
							.addComponent(btnBreakTime, Alignment.LEADING, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING, false)
						.addComponent(comboBoxSlots, 0, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
						.addComponent(comboBoxRefreshRate, 0, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
						.addComponent(comboBoxBreakTime, 0, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
						.addComponent(formattedTextFieldMabTime, GroupLayout.DEFAULT_SIZE, 70, Short.MAX_VALUE))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(lblusMab, GroupLayout.PREFERRED_SIZE, 15, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblUsBreak))
							.addContainerGap(136, Short.MAX_VALUE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnReset))))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnBreakTime)
						.addComponent(comboBoxBreakTime, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblUsBreak))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnMabTime)
						.addComponent(formattedTextFieldMabTime, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblusMab))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnRefreshRate)
						.addComponent(comboBoxRefreshRate, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnSlots)
						.addComponent(comboBoxSlots, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnReset))
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
		);
		contentPanel.setLayout(gl_contentPanel);
	}
	
	private void CreateEvents() {
		btnBreakTime.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				SendUpd("dmx!break#".concat(Integer.toString((Integer)comboBoxBreakTime.getSelectedItem())));
			}
		});
		btnMabTime.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("dmx!mab#".concat(formattedTextFieldMabTime.getText().trim()));
			}
		});
		btnRefreshRate.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("dmx!refresh#".concat(Integer.toString((Integer)comboBoxRefreshRate.getSelectedItem())));
			}
		});
		btnSlots.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("dmx!slots#".concat(Integer.toString((Integer)comboBoxSlots.getSelectedItem())));
			}
		});
		btnReset.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				comboBoxBreakTime.setSelectedItem(176);
				formattedTextFieldMabTime.setText("12");
				comboBoxRefreshRate.setSelectedIndex(40);
				comboBoxSlots.setSelectedItem(512);
			}
		});
	}
	
	private void SendUpd(String message) {
		final byte[] buffer = message.getBytes();	
		try {
			datagramSocket.send(new DatagramPacket(buffer, buffer.length, ipAddressNode, UDP_PORT));
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
