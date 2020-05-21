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
import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

import javax.swing.ButtonGroup;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JDialog;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JCheckBox;

public class TCNet extends JDialog {
	private static final long serialVersionUID = -7775636436383674851L;
	private static final int UDP_PORT = 2762;
	
	private InetAddress IPAddressLtcNode; 
    private DatagramSocket socket = null;
    private JTextField txtLayer;
    private JTextField txtType;
    private JRadioButton rdbtnLayer1;
    private JRadioButton rdbtnLayerA;
    private JRadioButton rdbtnLayerB;
    private JRadioButton rdbtnLayerM;
    private JRadioButton rdbtnLayerC;
    private JRadioButton rdbtnLayer4;
    private JRadioButton rdbtnLayer3;
    private JRadioButton rdbtnLayer2;
    private JRadioButton rdbtn30FPS;
    private JRadioButton rdbtn29FPS;
    private JRadioButton rdbtn25FPS;
    private JRadioButton rdbtn24FPS;
    private final ButtonGroup buttonGroupLayer = new ButtonGroup();
    private final ButtonGroup buttonGroupType = new ButtonGroup();
    private JCheckBox chckbxUseTimecode;

	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					TCNet dialog = new TCNet(InetAddress.getByName("192.168.2.120"));
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	public TCNet(InetAddress IPAddressLtcNode) {
		InitComponents();
		CreateEvents();
	
		this.IPAddressLtcNode = IPAddressLtcNode;
		
		 try {
			socket = new DatagramSocket();
			setTitle("TCNet: LTC Node " + IPAddressLtcNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error: LTC Node " + IPAddressLtcNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	public void Show() {
		setVisible(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 299, 187);
		
		txtLayer = new JTextField();
		txtLayer.setBorder(null);
		txtLayer.setDragEnabled(false);
		txtLayer.setBackground(new Color(238, 238, 238));
		txtLayer.setText("Layer");
		txtLayer.setEditable(false);
		txtLayer.setColumns(10);
		
		rdbtnLayer1 = new JRadioButton("1");
		buttonGroupLayer.add(rdbtnLayer1);
		
		rdbtnLayer2 = new JRadioButton("2");
		buttonGroupLayer.add(rdbtnLayer2);
		
		rdbtnLayer4 = new JRadioButton("4");
		buttonGroupLayer.add(rdbtnLayer4);
		
		rdbtnLayer3 = new JRadioButton("3");
		buttonGroupLayer.add(rdbtnLayer3);
		
		rdbtnLayerA = new JRadioButton("A");
		buttonGroupLayer.add(rdbtnLayerA);
		
		rdbtnLayerB = new JRadioButton("B");
		buttonGroupLayer.add(rdbtnLayerB);
		
		rdbtnLayerM = new JRadioButton("M");
		buttonGroupLayer.add(rdbtnLayerM);
		
		rdbtnLayerC = new JRadioButton("C");
		buttonGroupLayer.add(rdbtnLayerC);
		
		txtType = new JTextField();
		txtType.setText("Type");
		txtType.setEditable(false);
		txtType.setDragEnabled(false);
		txtType.setColumns(10);
		txtType.setBorder(null);
		txtType.setBackground(new Color(238, 238, 238));
		
		rdbtn24FPS = new JRadioButton("24");
		buttonGroupType.add(rdbtn24FPS);
		
		rdbtn25FPS = new JRadioButton("25");
		buttonGroupType.add(rdbtn25FPS);
		
		rdbtn29FPS = new JRadioButton("29.97");
		buttonGroupType.add(rdbtn29FPS);
		
		rdbtn30FPS = new JRadioButton("30");
		buttonGroupType.add(rdbtn30FPS);
		
		chckbxUseTimecode = new JCheckBox("Use TimeCode");
		
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(18)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(chckbxUseTimecode)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(txtType, GroupLayout.PREFERRED_SIZE, 48, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtn24FPS)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtn25FPS)
							.addGap(2)
							.addComponent(rdbtn29FPS)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtn30FPS))
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(txtLayer, GroupLayout.PREFERRED_SIZE, 48, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnLayer1)
								.addComponent(rdbtnLayerA, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
								.addComponent(rdbtnLayer2, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE)
								.addComponent(rdbtnLayerB, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnLayer3, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE)
								.addComponent(rdbtnLayerM))
							.addGap(9)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnLayerC)
								.addComponent(rdbtnLayer4, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE))))
					.addGap(68))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addGap(16)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnLayer1)
								.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
									.addComponent(rdbtnLayer2)
									.addComponent(rdbtnLayer3)
									.addComponent(rdbtnLayer4)))
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnLayerC)
								.addComponent(rdbtnLayerM)
								.addComponent(rdbtnLayerB)
								.addComponent(rdbtnLayerA)))
						.addGroup(groupLayout.createSequentialGroup()
							.addGap(34)
							.addComponent(txtLayer, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(txtType, GroupLayout.PREFERRED_SIZE, 16, GroupLayout.PREFERRED_SIZE)
						.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
							.addComponent(rdbtn24FPS)
							.addComponent(rdbtn25FPS)
							.addComponent(rdbtn29FPS)
							.addComponent(rdbtn30FPS)))
					.addGap(18)
					.addComponent(chckbxUseTimecode)
					.addContainerGap(62, Short.MAX_VALUE))
		);
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		rdbtnLayer1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#1");
			}
		});
		
		rdbtnLayer2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#2");
			}
		});
		
		rdbtnLayer3.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#3");
			}
		});
		
		rdbtnLayer4.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#4");
			}
		});
		
		rdbtnLayerA.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#A");
			}
		});
		
		rdbtnLayerB.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#B");
			}
		});
		
		rdbtnLayerM.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#M");
			}
		});
		
		rdbtnLayerC.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!layer#C");
			}
		});
		
		rdbtn24FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!type#24");
			}
		});
		
		rdbtn25FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!type#25");
			}
		});
		
		rdbtn29FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!type#29");
			}
		});
		
		rdbtn30FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("tcnet!type#30");
			}
		});
		
		chckbxUseTimecode.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (chckbxUseTimecode.isSelected()) {
					SendUpd("tcnet!timecode#y");
					DisablerdbtnFPS();
				} else {
					SendUpd("tcnet!timecode#n");
					EnablerdbtnFPS();
				}
			}
		});
	}
	
	private void DisablerdbtnFPS() {
		rdbtn24FPS.setEnabled(false);
		rdbtn25FPS.setEnabled(false);
		rdbtn29FPS.setEnabled(false);
		rdbtn30FPS.setEnabled(false);
	}

	private void EnablerdbtnFPS() {
		rdbtn24FPS.setEnabled(true);
		rdbtn25FPS.setEnabled(true);
		rdbtn29FPS.setEnabled(true);
		rdbtn30FPS.setEnabled(true);
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
