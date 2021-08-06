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
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class MIDI extends JDialog {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1236629594155800906L;

	private final JPanel contentPanel = new JPanel();
	
	static final int UDP_PORT = 0x4444;
	
	private InetAddress IPAddressNode; 
    private DatagramSocket socket = null;
    private JButton btnStart;
    private JButton btnStop;
    private JButton btnContinue;
    private JTextField textField;
    private JSlider slider;
    private JButton btnSetBPM;

	public static void main(String[] args) {
		try {
			MIDI dialog = new MIDI(InetAddress.getByName("192.168.2.120"));
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public MIDI(InetAddress IPAddressNode) {
		InitComponents();
		textField.setText(String.valueOf(slider.getValue()));
		CreateEvents();
		
		this.IPAddressNode = IPAddressNode;
		
		 try {
			socket = new DatagramSocket();
			setTitle("MIDI: LTC Node " + IPAddressNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error: LTC Node " + IPAddressNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	private void InitComponents() {
		setBounds(100, 100, 290, 146);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		btnStart = new JButton("Start");
		btnStop = new JButton("Stop");
		btnContinue = new JButton("Continue");
		
		slider = new JSlider();
		slider.setPaintLabels(true);
		slider.setMaximum(300);
		slider.setMinimum(8);
		slider.setValue(120);
		
		JLabel lblBPM = new JLabel("BPM");
		
		textField = new JTextField();

		textField.setColumns(10);
		
		btnSetBPM = new JButton("Set BPM");
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.TRAILING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(btnStart)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(btnStop)
							.addPreferredGap(ComponentPlacement.RELATED, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
							.addComponent(btnContinue))
						.addGroup(Alignment.TRAILING, gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addComponent(lblBPM)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(textField, GroupLayout.PREFERRED_SIZE, 63, GroupLayout.PREFERRED_SIZE)
							.addGap(94)
							.addComponent(btnSetBPM, GroupLayout.PREFERRED_SIZE, 75, GroupLayout.PREFERRED_SIZE)
							.addGap(5))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addComponent(slider, GroupLayout.DEFAULT_SIZE, 268, Short.MAX_VALUE)))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblBPM)
						.addComponent(textField, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSetBPM))
					.addGap(3)
					.addComponent(slider, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnStart)
						.addComponent(btnStop)
						.addComponent(btnContinue))
					.addContainerGap(11, Short.MAX_VALUE))
		);
		contentPanel.setLayout(gl_contentPanel);
	}
	
	private void CreateEvents() {
		btnStart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("midi!start");
			}
		});
		
		btnStop.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("midi!stop");
			}
		});
		
		btnContinue.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("midi!continue");
			}
		});
		
		slider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent e) {
				textField.setText(String.valueOf(slider.getValue()));
			}
		});
		
		textField.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			}
		});
		
		btnSetBPM.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (Integer.parseInt(textField.getText()) != 0) {
					slider.setValue(Integer.parseInt(textField.getText()));
				}
				String midiBPM = String.format("midi!bpm#%03d", Integer.parseInt(textField.getText()));
				SendUpd(midiBPM);
			}
		});
	}
	
	private void SendUpd(String message) {
		byte[] buffer = message.getBytes();

		DatagramPacket UDPPacket = new DatagramPacket(buffer, buffer.length, IPAddressNode, UDP_PORT);
		try {
			socket.send(UDPPacket);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
