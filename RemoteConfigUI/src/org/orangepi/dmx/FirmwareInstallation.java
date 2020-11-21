/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.IOException;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;

public class FirmwareInstallation extends JDialog {
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	private JCheckBox chckbxTFTPOn;
	private JButton btnClose;
	private JTextField textNodeInfo;
	private TFTPClient client;
	private JCheckBox chckbxRunTFTPClient;
	private JCheckBox chckbxTFTPOff;
	
	private static Object lock = new Object();
	private JCheckBox chckbxReboot;
	private JTextField textVersion;
	
	/**
	 * Create the dialog.
	 */
	public FirmwareInstallation(OrangePi Node, RemoteConfig remoteConfig) {
		InitComponents();
		CreateEvents();

		setTitle("Workflow Firmware Installation");
		textNodeInfo.setText(Node.getNodeId());

		Node.doSetTFTP(true);
		chckbxTFTPOn.setSelected(Node.doGetTFTP().contains("On"));

		if (!Node.doGetTFTP().contains("On")) {
			// Error
		} else {
			TFTPClient frame = new TFTPClient("", Node.getAddress());
			frame.setVisible(true);

			Thread t = new Thread() {
				public void run() {
					chckbxRunTFTPClient.setSelected(true);
					synchronized (lock) {
						while (frame.isVisible())
							try {
								lock.wait();
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						
						System.out.println("TFTP Client Closed with result " + frame.result());
						
						Node.doSetTFTP(false);
						chckbxTFTPOff.setSelected(!Node.doGetTFTP().contains("On"));
						
						if (Node.doGetTFTP().contains("On")) {
							// Error
						} else {
							if (frame.result() > 0) {
								chckbxReboot.setSelected(true);								
								try {
									Node.doReboot();
									String version;
									String progress = "";
									
									textVersion.setForeground(Color.BLACK);
									do {
										progress = progress.concat(".");
										textVersion.setText(progress);
										try {
											sleep(1000);
										} catch (InterruptedException e) {
											e.printStackTrace();
										}
										version = Node.doVersion();
										System.out.println("[" + version + "]");
									} while (version.contains("tftp") || (version.contains("ERROR")));
									
									textVersion.setForeground(Color.BLUE);
									textVersion.setText(version.substring(version.indexOf("[")));
									remoteConfig.refresh();
									
								} catch (IOException e) {
									e.printStackTrace();
								}
							}
						}
					}
				}
			};
			
			t.start();

			frame.addWindowListener(new WindowListener() {

				@Override
				public void windowOpened(WindowEvent e) {
				}

				@Override
				public void windowIconified(WindowEvent e) {
				}

				@Override
				public void windowDeiconified(WindowEvent e) {
				}

				@Override
				public void windowDeactivated(WindowEvent e) {
				}

				@Override
				public void windowClosing(WindowEvent e) {
				}

				@Override
				public void windowClosed(WindowEvent e) {
					synchronized (lock) {
						frame.setVisible(false);
						lock.notify();
					}
				}

				@Override
				public void windowActivated(WindowEvent e) {
				}
			});
		}
	}
	
	private void InitComponents() {
		setBounds(100, 100, 423, 258);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		chckbxTFTPOn = new JCheckBox("TFTP On");
		chckbxTFTPOn.setEnabled(false);
		textNodeInfo = new JTextField();
		textNodeInfo.setBackground(Color.LIGHT_GRAY);
		textNodeInfo.setEditable(false);
		textNodeInfo.setColumns(10);
		
		chckbxRunTFTPClient = new JCheckBox("Run TFTP Client");
		chckbxRunTFTPClient.setEnabled(false);
		
		chckbxTFTPOff = new JCheckBox("TFTP Off");
		chckbxTFTPOff.setEnabled(false);
		
		chckbxReboot = new JCheckBox("Reboot");
		chckbxReboot.setEnabled(false);
		
		textVersion = new JTextField();
		textVersion.setBackground(new Color(192, 192, 192));
		textVersion.setEditable(false);
		textVersion.setColumns(10);
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(textVersion, GroupLayout.DEFAULT_SIZE, 360, Short.MAX_VALUE)
						.addComponent(chckbxTFTPOn)
						.addComponent(chckbxRunTFTPClient)
						.addComponent(chckbxTFTPOff)
						.addComponent(chckbxReboot)
						.addComponent(textNodeInfo, GroupLayout.DEFAULT_SIZE, 378, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addComponent(textNodeInfo, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addGap(11)
					.addComponent(chckbxTFTPOn)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxRunTFTPClient)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxTFTPOff)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxReboot)
					.addPreferredGap(ComponentPlacement.RELATED, 8, Short.MAX_VALUE)
					.addComponent(textVersion, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addContainerGap())
		);
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				btnClose = new JButton("Close");
				btnClose.setActionCommand("Close");
				buttonPane.add(btnClose);
				getRootPane().setDefaultButton(btnClose);
			}
		}
		
	}
	
	private void CreateEvents() {
		btnClose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (client != null) {
					client.dispose();
				}
				dispose();
			}
		});
	}
}
