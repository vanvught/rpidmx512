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

package org.orangepi.dmx;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;

import javax.swing.ButtonGroup;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SwingUtilities;
import javax.swing.border.EmptyBorder;
import java.awt.Color;

public class NetworkInterfaces extends JDialog {

	/**
	 * 
	 */
	private static final long serialVersionUID = 8437167617114467625L;
	private final JPanel contentPanel = new JPanel();
	
	private InetAddress localAddress;
	private static final String ipv4Pattern = "(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])";
	
	private JButton okButton;
	private final ButtonGroup buttonGroup = new ButtonGroup();
	private JTextField txtFieldIf0;
	private JTextField txtFieldIf1;
	private JTextField txtFieldIf2;
	private JTextField txtFieldIf3;
	private JTextField txtFieldIf4;
	private JRadioButton rdbtnIf0;
	private JRadioButton rdbtnIf1;
	private JRadioButton rdbtnIf2;
	private JRadioButton rdbtnIf3;
	private JRadioButton rdbtnIf4;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		try {
			InetAddress address = null;
			
			NetworkInterface ifDefault = FirstNetworkInterface.get();
			
			Enumeration<InetAddress> enumIP = ifDefault.getInetAddresses(); 
			
			while (enumIP.hasMoreElements()) {
				address = (InetAddress) enumIP.nextElement();
	
				if (address.getHostAddress().matches(ipv4Pattern)) {
					break;
				}
			}
			
			System.out.println("|" + address.getHostAddress());
			
			NetworkInterfaces dialog = new NetworkInterfaces(address);
			
			dialog.Show();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public InetAddress Show() {
		setVisible(true);
		return localAddress;
	}

	/**
	 * Create the dialog.
	 */
	public NetworkInterfaces(InetAddress localAddress) {
		this.localAddress = localAddress;

		InitComponents();
		CreateEvents();
		
		getNetworkInterfaceList();

	}

	private void InitComponents() {
		setModal(true);
		setTitle("Network Interfaces");
		setBounds(100, 100, 369, 231);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		rdbtnIf0 = new JRadioButton("Not used");
		rdbtnIf0.setEnabled(false);
		buttonGroup.add(rdbtnIf0);
		
		rdbtnIf1 = new JRadioButton("Not used");
		rdbtnIf1.setEnabled(false);
		buttonGroup.add(rdbtnIf1);
		
		rdbtnIf2 = new JRadioButton("Not used");
		rdbtnIf2.setEnabled(false);
		buttonGroup.add(rdbtnIf2);
		
		rdbtnIf3 = new JRadioButton("Not used");
		rdbtnIf3.setEnabled(false);
		buttonGroup.add(rdbtnIf3);
		
		rdbtnIf4 = new JRadioButton("Not used");
		rdbtnIf4.setEnabled(false);
		buttonGroup.add(rdbtnIf4);
		
		txtFieldIf0 = new JTextField();
		txtFieldIf0.setBackground(Color.LIGHT_GRAY);
		txtFieldIf0.setEditable(false);
		txtFieldIf0.setColumns(10);
		
		txtFieldIf1 = new JTextField();
		txtFieldIf1.setBackground(Color.LIGHT_GRAY);
		txtFieldIf1.setEditable(false);
		txtFieldIf1.setColumns(10);
		
		txtFieldIf2 = new JTextField();
		txtFieldIf2.setBackground(Color.LIGHT_GRAY);
		txtFieldIf2.setEditable(false);
		txtFieldIf2.setColumns(10);
		
		txtFieldIf3 = new JTextField();
		txtFieldIf3.setBackground(Color.LIGHT_GRAY);
		txtFieldIf3.setEditable(false);
		txtFieldIf3.setColumns(10);
		
		txtFieldIf4 = new JTextField();
		txtFieldIf4.setBackground(Color.LIGHT_GRAY);
		txtFieldIf4.setEditable(false);
		txtFieldIf4.setColumns(10);
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.TRAILING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnIf0, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE)
						.addComponent(rdbtnIf1, GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE)
						.addComponent(rdbtnIf2, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE)
						.addComponent(rdbtnIf3, GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE)
						.addComponent(rdbtnIf4, GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(txtFieldIf0, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE)
						.addComponent(txtFieldIf1, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE)
						.addComponent(txtFieldIf2, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE)
						.addComponent(txtFieldIf3, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE)
						.addComponent(txtFieldIf4, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnIf0)
						.addComponent(txtFieldIf0, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnIf1)
						.addComponent(txtFieldIf1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnIf2)
						.addComponent(txtFieldIf2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnIf3)
						.addComponent(txtFieldIf3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnIf4)
						.addComponent(txtFieldIf4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
		);
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				okButton = new JButton("OK");
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
		}
	}

	private void CreateEvents() {
		okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					if (rdbtnIf0.isSelected()) {
						localAddress = InetAddress.getByName(txtFieldIf0.getText());
					} else if (rdbtnIf1.isSelected()) {
						localAddress = InetAddress.getByName(txtFieldIf1.getText());
					} else if (rdbtnIf2.isSelected()) {
						localAddress = InetAddress.getByName(txtFieldIf2.getText());
					} else if (rdbtnIf3.isSelected()) {
						localAddress = InetAddress.getByName(txtFieldIf3.getText());
					} else if (rdbtnIf4.isSelected()) {
						localAddress = InetAddress.getByName(txtFieldIf4.getText());
					}
				} catch (UnknownHostException e1) {
					e1.printStackTrace();
				}

				Component component = (Component) e.getSource();
				JDialog dialog = (JDialog) SwingUtilities.getRoot(component);
				dialog.dispose();
			}
		});
	}

	private void SetButtonText(JRadioButton button, JTextField textField, NetworkInterface ni, InetAddress ip) throws SocketException {
		button.setText(ni.getDisplayName());
		button.setEnabled(true);
		textField.setText(ip.getHostAddress());
		
		if (ip.getHostAddress().equals(localAddress.getHostAddress())) {
			button.setSelected(true);
		}
	}

	private Boolean checkValidNetwork(NetworkInterface ni) throws SocketException {
		System.out.println(ni.getDisplayName());

		if (ni.isPointToPoint()) {
			System.out.println("isPointToPoint");
			return false;
		}
		
		if (ni.isLoopback()) {
			System.out.println("isLoopback");
			return false;
		}

		return true;
	}
	
	int SetButton(int nButton, NetworkInterface ni) throws SocketException {
		System.out.println(ni.getDisplayName());
				
		Enumeration<InetAddress> enumIP = ni.getInetAddresses();
				
		while (enumIP.hasMoreElements()) {
			InetAddress ip = (InetAddress) enumIP.nextElement();
			System.out.println(ip);
			

			if (ip.getHostAddress().matches(ipv4Pattern)) {
				
				switch (nButton) {
				case 0:
					SetButtonText(rdbtnIf0, txtFieldIf0, ni,ip);
					break;
				case 1:
					SetButtonText(rdbtnIf1, txtFieldIf1, ni,ip);
					break;
				case 2:
					SetButtonText(rdbtnIf2, txtFieldIf2, ni,ip);
					break;
				case 3:
					SetButtonText(rdbtnIf3, txtFieldIf3, ni,ip);
					break;
				case 4:
					SetButtonText(rdbtnIf4, txtFieldIf4, ni,ip);
					break;
				default:
					break;
				}
				
				nButton = nButton + 1;
			}
		}
		
		return nButton;
	}

	private void getNetworkInterfaceList() {
		try {
			Enumeration<NetworkInterface> networks = NetworkInterface.getNetworkInterfaces();
			NetworkInterface ni = null;

			int nButtonIndex = 0;

			while (networks.hasMoreElements()) {
				ni = networks.nextElement();
				if (checkValidNetwork(ni)) {
					if ((nButtonIndex = SetButton(nButtonIndex, ni)) > 4) {
						break;
					}
				} else {
					continue;
				}
			}

		} catch (SocketException e) {
			e.printStackTrace();
		}
	}
}
