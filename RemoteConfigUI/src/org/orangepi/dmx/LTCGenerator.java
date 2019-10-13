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
import javax.swing.JSpinner;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SpinnerNumberModel;

public class LTCGenerator extends JDialog {
	private static final long serialVersionUID = -8623371832596143765L;
	static final int UDP_PORT = 21571;
	
	private InetAddress IPAddressLtcNode; 
    private DatagramSocket socket = null;
    
	private JButton btnStart;
	private JButton btnStop;
	private JButton btnResume;
	private JButton btnSetStart;
	private JButton btnSetStop;
	
	private JSpinner spinnerStartHours;
	private JSpinner spinnerStartMinutes;
	private JSpinner spinnerStartSeconds;
	private JSpinner spinnerStartFrames;
	private JSpinner spinnerStopHours;
	private JSpinner spinnerStopMinutes;
	private JSpinner spinnerStopSeconds;
	private JSpinner spinnerStopFrames;
	private JSpinner Sn1Hours;
	private JSpinner Sn1Minutes;
	private JSpinner Sn1Seconds;
	private JSpinner Sn1Frames;
	private JSpinner Sn2Hours;
	private JSpinner Sn2Minutes;
	private JSpinner Sn2Seconds;
	private JSpinner Sn2Frames;
	private JSpinner Sn3Hours;
	private JSpinner Sn3Minutes;
	private JSpinner Sn3Seconds;
	private JSpinner Sn3Frames;
	private JSpinner Sn4Frames;
	private JSpinner Sn4Seconds;
	private JSpinner Sn4Minutes;
	private JSpinner Sn4Hours;
	
	private JButton btnSn1;
	private JButton btnSn2;
	private JButton btnSn3;
	private JButton btnSn4;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					LTCGenerator dialog = new LTCGenerator(InetAddress.getByName("192.168.2.120"));
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
		setBounds(100, 100, 325, 270);
		
		btnStart = new JButton("Start");		
		btnStop = new JButton("Stop");
		btnResume = new JButton("Resume");
		btnSetStart = new JButton("Set Start");
		btnSetStop = new JButton("Set Stop");
		
		spinnerStartHours = new JSpinner(new SpinnerNumberModel(0, 0, 23, 1));
		spinnerStartMinutes = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		spinnerStartSeconds = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		spinnerStartFrames = new JSpinner(new SpinnerNumberModel(0, 0, 30, 1));
		
		spinnerStopHours = new JSpinner(new SpinnerNumberModel(23, 0, 23, 1));
		spinnerStopMinutes = new JSpinner(new SpinnerNumberModel(59, 0, 59, 1));
		spinnerStopSeconds = new JSpinner(new SpinnerNumberModel(29, 0, 59, 1));
		spinnerStopFrames = new JSpinner(new SpinnerNumberModel(25, 0, 30, 1));
		
		btnSn1 = new JButton("Section 1");
		btnSn2 = new JButton("Section 2");
		btnSn3 = new JButton("Section 3");
		btnSn4 = new JButton("Section 4");
		
		Sn1Hours = new JSpinner(new SpinnerNumberModel(0, 0, 23, 1));
		Sn1Minutes = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn1Seconds = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn1Frames = new JSpinner(new SpinnerNumberModel(0, 0, 30, 1));
	
		Sn2Hours = new JSpinner(new SpinnerNumberModel(0, 0, 23, 1));
		Sn2Minutes = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn2Seconds = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn2Frames = new JSpinner(new SpinnerNumberModel(0, 0, 30, 1));
		
		Sn3Hours = new JSpinner(new SpinnerNumberModel(0, 0, 23, 1));
		Sn3Minutes = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn3Seconds = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn3Frames = new JSpinner(new SpinnerNumberModel(0, 0, 30, 1));

		Sn4Hours = new JSpinner(new SpinnerNumberModel(0, 0, 23, 1));
		Sn4Minutes = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn4Seconds = new JSpinner(new SpinnerNumberModel(0, 0, 59, 1));
		Sn4Frames = new JSpinner(new SpinnerNumberModel(0, 0, 30, 1));
		
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addContainerGap()
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(btnSn1)
								.addComponent(btnSn2)
								.addComponent(btnSn3)
								.addComponent(btnSn4))
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(Sn4Hours, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn3Hours, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn2Hours, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn1Hours, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(Sn4Minutes, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn3Minutes, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn2Minutes, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn1Minutes, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(Sn4Seconds, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn3Seconds, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn2Seconds, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn1Seconds, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(Sn4Frames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn3Frames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn2Frames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(Sn1Frames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 42, GroupLayout.PREFERRED_SIZE)
								.addComponent(spinnerStopFrames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(spinnerStartFrames, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
						.addGroup(groupLayout.createSequentialGroup()
							.addGap(12)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING, false)
								.addComponent(btnSetStop, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
								.addComponent(btnStart)
								.addComponent(btnSetStart, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addGroup(groupLayout.createSequentialGroup()
									.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
										.addComponent(spinnerStartHours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
										.addComponent(spinnerStopHours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
									.addPreferredGap(ComponentPlacement.RELATED)
									.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING, false)
										.addComponent(spinnerStartMinutes, Alignment.LEADING, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
										.addComponent(spinnerStopMinutes, Alignment.LEADING, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
								.addComponent(btnStop))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(spinnerStopSeconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(spinnerStartSeconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(btnResume, GroupLayout.PREFERRED_SIZE, 90, GroupLayout.PREFERRED_SIZE))))
					.addContainerGap(16, Short.MAX_VALUE))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(btnStart)
						.addComponent(btnStop)
						.addComponent(btnResume))
					.addGap(18)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(spinnerStartFrames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStartSeconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStartMinutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStartHours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSetStart))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(spinnerStopFrames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStopSeconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStopMinutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(spinnerStopHours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSetStop, GroupLayout.PREFERRED_SIZE, 25, GroupLayout.PREFERRED_SIZE))
					.addGap(24)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(btnSn1)
						.addComponent(Sn1Hours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn1Minutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn1Seconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn1Frames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(6)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(Sn2Frames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn2Seconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn2Minutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn2Hours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSn2))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(Sn3Frames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn3Seconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn3Minutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn3Hours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSn3))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(Sn4Frames, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn4Seconds, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn4Minutes, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(Sn4Hours, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnSn4))
					.addGap(69))
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
		
		btnSetStart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
	
				String ltcStart = String.format("ltc!start#%02d:%02d:%02d.%02d", 
						Integer.parseInt(spinnerStartHours.getValue().toString()),
						Integer.parseInt(spinnerStartMinutes.getValue().toString()), 
						Integer.parseInt(spinnerStartSeconds.getValue().toString()), 
						Integer.parseInt(spinnerStartFrames.getValue().toString()));

				SendUpd(ltcStart);
			}
		});
		
		btnSetStop.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {

				String ltcStop = String.format("ltc!stop#%02d:%02d:%02d.%02d", 
						Integer.parseInt(spinnerStopHours.getValue().toString()),
						Integer.parseInt(spinnerStopMinutes.getValue().toString()), 
						Integer.parseInt(spinnerStopSeconds.getValue().toString()), 
						Integer.parseInt(spinnerStopFrames.getValue().toString()));

				SendUpd(ltcStop);
			}
		});
		
		btnSn1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				
				String ltcStart = String.format("ltc!start!%02d:%02d:%02d.%02d", 
						Integer.parseInt(Sn1Hours.getValue().toString()),
						Integer.parseInt(Sn1Minutes.getValue().toString()), 
						Integer.parseInt(Sn1Seconds.getValue().toString()), 
						Integer.parseInt(Sn1Frames.getValue().toString()));

				SendUpd(ltcStart);
			}
		});
		
		btnSn2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				
				String ltcStart = String.format("ltc!start!%02d:%02d:%02d.%02d", 
						Integer.parseInt(Sn2Hours.getValue().toString()),
						Integer.parseInt(Sn2Minutes.getValue().toString()), 
						Integer.parseInt(Sn2Seconds.getValue().toString()), 
						Integer.parseInt(Sn2Frames.getValue().toString()));

				SendUpd(ltcStart);
			}
		});
		
		btnSn3.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				
				String ltcStart = String.format("ltc!start!%02d:%02d:%02d.%02d", 
						Integer.parseInt(Sn3Hours.getValue().toString()),
						Integer.parseInt(Sn3Minutes.getValue().toString()), 
						Integer.parseInt(Sn3Seconds.getValue().toString()), 
						Integer.parseInt(Sn3Frames.getValue().toString()));

				SendUpd(ltcStart);
			}
		});
		
		btnSn4.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				
				String ltcStart = String.format("ltc!start!%02d:%02d:%02d.%02d", 
						Integer.parseInt(Sn4Hours.getValue().toString()),
						Integer.parseInt(Sn4Minutes.getValue().toString()), 
						Integer.parseInt(Sn4Seconds.getValue().toString()), 
						Integer.parseInt(Sn4Frames.getValue().toString()));

				SendUpd(ltcStart);
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
