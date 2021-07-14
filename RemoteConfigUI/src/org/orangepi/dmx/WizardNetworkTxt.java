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
import java.awt.Color;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.NumberFormat;
import java.util.TimeZone;
import java.util.prefs.Preferences;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.EmptyBorder;
import javax.swing.text.NumberFormatter;

public class WizardNetworkTxt extends JDialog {
	private final JPanel contentPanel = new JPanel();
	private static final long serialVersionUID = 1L;
	//
	static final float UtcValidOffets[] = { -12.0f, -11.0f, -10.0f, -9.5f, -9.0f, -8.0f, -7.0f, -6.0f, -5.0f, -4.0f,
			-3.5f, -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 5.75f, 6.0f, 6.5f, 7.0f,
			8.0f, 8.75f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 12.0f, 12.75f, 13.0f, 14.0f };
	//
	private Preferences prefs = Preferences.userRoot().node(getClass().getName());
	static final String LAST_USED_NTP_SERVER = "org.orangepi.dmx";
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	private JButton btnCancel;
	private JButton btnSave;
	private JCheckBox chckbxEnableDHCP;
	private JLabel lblDhcpRetry;
	private JSpinner spinner;
	private JLabel lblStaticIp;
	private JLabel lblLocal;
	private JLabel lblNetmask;
	private JLabel lblNtpServerIp;
	private JLabel lblUtcOffset;
	private JComboBox<String> comboBoxUtcOffset;
	private JLabel lblHostname;
	private JTextField textFieldHostname;
	private JTextField textFieldNetmask;
	private JFormattedTextField formattedStaticIP1;
	private JFormattedTextField formattedStaticIP2;
	private JFormattedTextField formattedStaticIP3;
	private JFormattedTextField formattedStaticIP4;
	private JFormattedTextField formattedCdir;
	private JLabel lblsep;
	private JFormattedTextField formattedNtpServerIP1;
	private JFormattedTextField formattedNtpServerIP2;
	private JFormattedTextField formattedNtpServerIP3;
	private JFormattedTextField formattedNtpServerIP4;
	private JCheckBox chckbxEnableNtpServer;
	private JButton btnSetDefaults;
	private JLabel lblGateway;
	private JFormattedTextField formattedGatewayIP1;
	private JFormattedTextField formattedGatewayIP2;
	private JFormattedTextField formattedGatewayIP3;
	private JFormattedTextField formattedGatewayIP4;

	public WizardNetworkTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		for (int i = 0; i < UtcValidOffets.length; i++) {
			comboBoxUtcOffset.addItem(formatUtcOffset(UtcValidOffets[i]));
		}

		load();
	}

	private void initComponents() {
		setBounds(100, 100, 316, 385);
		chckbxEnableDHCP = new JCheckBox("Enable DHCP");
		lblDhcpRetry = new JLabel("DHCP retry");	
		spinner = new JSpinner(new SpinnerNumberModel(0, 0, 5, 1));
		spinner.setToolTipText("0 is no retry");
		
		JLabel lblMinutes = new JLabel("Minutes");
		
		lblStaticIp = new JLabel("Static");
		lblLocal = new JLabel("IP");
		lblNetmask = new JLabel("Netmask");
		lblNtpServerIp = new JLabel("Server IP");
		lblUtcOffset = new JLabel("UTC offset");
		comboBoxUtcOffset = new JComboBox<String>();
		lblHostname = new JLabel("Hostname");
		textFieldHostname = new JTextField();
		textFieldHostname.setColumns(10);
		
		textFieldNetmask = new JTextField();
		textFieldNetmask.setEditable(false);
		textFieldNetmask.setBackground(Color.LIGHT_GRAY);
		textFieldNetmask.setColumns(10);
		
		NumberFormat format = NumberFormat.getInstance();
		NumberFormatter formatterIP = new NumberFormatter(format);
		formatterIP.setValueClass(Integer.class);
		formatterIP.setMinimum(0);
		formatterIP.setMaximum(255);
		formatterIP.setAllowsInvalid(false);
		formatterIP.setCommitsOnValidEdit(true);
		
		formattedStaticIP1 = new JFormattedTextField(formatterIP);
		formattedStaticIP1.setColumns(2);
		
		formattedStaticIP2 = new JFormattedTextField(formatterIP);
		formattedStaticIP2.setColumns(2);
		
		formattedStaticIP3 = new JFormattedTextField(formatterIP);
		formattedStaticIP3.setColumns(2);
		
		formattedStaticIP4 = new JFormattedTextField(formatterIP);
		formattedStaticIP4.setColumns(2);
		
		NumberFormatter formatterCdir = new NumberFormatter(format);
		formatterCdir.setValueClass(Integer.class);
		formatterCdir.setMinimum(1);
		formatterCdir.setMaximum(32);
		formatterCdir.setAllowsInvalid(false);
		formatterCdir.setCommitsOnValidEdit(true);
		
		formattedCdir = new JFormattedTextField(formatterCdir);
		formattedCdir.setColumns(2);
		
		lblsep = new JLabel("/");
		
		formattedNtpServerIP1 = new JFormattedTextField(formatterIP);
		formattedNtpServerIP1.setColumns(2);
		
		formattedNtpServerIP2 = new JFormattedTextField(formatterIP);
		formattedNtpServerIP2.setColumns(2);
		
		formattedNtpServerIP3 = new JFormattedTextField(formatterIP);
		formattedNtpServerIP3.setColumns(2);
		
		formattedNtpServerIP4 = new JFormattedTextField(formatterIP);
		formattedNtpServerIP4.setColumns(2);
		
		chckbxEnableNtpServer = new JCheckBox("Enable NTP Client");
				
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		lblGateway = new JLabel("Gateway");
		
		formattedGatewayIP1 = new JFormattedTextField(formatterIP);
		formattedGatewayIP1.setColumns(2);
		
		formattedGatewayIP2 = new JFormattedTextField(formatterIP);
		formattedGatewayIP2.setColumns(2);
		
		formattedGatewayIP3 = new JFormattedTextField(formatterIP);
		formattedGatewayIP3.setColumns(2);
		
		formattedGatewayIP4 = new JFormattedTextField(formatterIP);
		formattedGatewayIP4.setColumns(2);
		
		GroupLayout groupLayout = new GroupLayout(contentPanel);
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addContainerGap()
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addGroup(groupLayout.createSequentialGroup()
									.addGap(2)
									.addComponent(lblDhcpRetry)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(spinner, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(lblMinutes)
									.addGap(24))
								.addComponent(chckbxEnableDHCP)
								.addComponent(lblStaticIp)
								.addGroup(groupLayout.createSequentialGroup()
									.addGap(6)
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addComponent(lblNetmask)
										.addComponent(lblLocal)
										.addComponent(lblGateway)
										.addComponent(lblHostname))
									.addGap(24)
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addGroup(groupLayout.createSequentialGroup()
											.addComponent(textFieldNetmask, 144, 144, 144)
											.addGap(75))
										.addGroup(groupLayout.createSequentialGroup()
											.addComponent(formattedGatewayIP1, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addGap(4)
											.addComponent(formattedGatewayIP2, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addGap(6)
											.addComponent(formattedGatewayIP3, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addGap(6)
											.addComponent(formattedGatewayIP4, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE))
										.addGroup(Alignment.TRAILING, groupLayout.createSequentialGroup()
											.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
												.addComponent(textFieldHostname, Alignment.LEADING, GroupLayout.DEFAULT_SIZE, 205, Short.MAX_VALUE)
												.addGroup(groupLayout.createSequentialGroup()
													.addComponent(formattedStaticIP1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
													.addGap(4)
													.addComponent(formattedStaticIP2, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedStaticIP3, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedStaticIP4, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(lblsep, GroupLayout.DEFAULT_SIZE, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedCdir, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
											.addGap(14))))))
						.addComponent(chckbxEnableNtpServer)
						.addGroup(groupLayout.createSequentialGroup()
							.addContainerGap()
							.addComponent(lblNtpServerIp)
							.addGap(31)
							.addComponent(formattedNtpServerIP1, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(formattedNtpServerIP2, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(formattedNtpServerIP3, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(formattedNtpServerIP4, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE))
						.addGroup(groupLayout.createSequentialGroup()
							.addContainerGap()
							.addComponent(lblUtcOffset)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(comboBoxUtcOffset, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE)))
					.addGap(0))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addComponent(chckbxEnableDHCP)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(spinner, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblMinutes)
						.addComponent(lblDhcpRetry))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblStaticIp)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblLocal)
						.addComponent(formattedStaticIP1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedStaticIP4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblsep)
						.addComponent(formattedStaticIP3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedStaticIP2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedCdir, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNetmask)
						.addComponent(textFieldNetmask, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
							.addComponent(formattedGatewayIP1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(lblGateway))
						.addComponent(formattedGatewayIP2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedGatewayIP3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedGatewayIP4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(textFieldHostname, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblHostname))
					.addGap(8)
					.addComponent(chckbxEnableNtpServer)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNtpServerIp)
						.addComponent(formattedNtpServerIP1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedNtpServerIP2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedNtpServerIP3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(formattedNtpServerIP4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxUtcOffset, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblUtcOffset))
					.addGap(22))
		);
		
		contentPanel.setLayout(groupLayout);
		
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);

			{
				btnSave = new JButton("Save");
				buttonPane.add(btnSave);
			}
			{
				btnSetDefaults = new JButton("Set defaults");
				buttonPane.add(btnSetDefaults);
			}
			{
				btnCancel = new JButton("Cancel");
				buttonPane.add(btnCancel);
				getRootPane().setDefaultButton(btnCancel);
			}
		}
	}
	
	private void createEvents() {
		btnCancel.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
			}
		});
		
		btnSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				save();
			}
		});

		chckbxEnableDHCP.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				update();
			}
		});
		
		btnSetDefaults.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.setTextArea(opi.doDefaults("network.txt"));
				load();
			}
		});
		
		formattedCdir.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				textFieldNetmask.setText(convertCidrToString((int)formattedCdir.getValue()));
			}
		});
	}
	
	private String formatUtcOffset(float f) {
		if (f > 0) {
			return String.format("+%1.2f", f);
		}
		return String.format("%1.2f", f);
	}
	
	private int convertNetmaskToCIDR(InetAddress netmask) {
		byte[] netmaskBytes = netmask.getAddress();
		int cidr = 0;
		boolean zero = false;
		for (byte b : netmaskBytes) {
			int mask = 0x80;
			for (int i = 0; i < 8; i++) {
				int result = b & mask;
				if (result == 0) {
					zero = true;
				} else if (zero) {
					throw new IllegalArgumentException("Invalid netmask.");
				} else {
					cidr++;
				}
				mask >>>= 1;
			}
		}
		return cidr;
	}
	
	private String convertCidrToString(int cidrMask) {
		final long bits = 0xffffffff ^ (1 << 32 - cidrMask) - 1;
		return String.format("%d.%d.%d.%d", (bits & 0x0000000000ff000000L) >> 24, (bits & 0x0000000000ff0000) >> 16, (bits & 0x0000000000ff00) >> 8, bits & 0xff);
	}
	
	private void load() {
		if (opi != null) {
			final String txt = opi.getTxt("network.txt");
			if (txt != null) {
				final String[] lines = txt.split("\n");
				for (int i = 0; i < lines.length; i++) {
					final String line = lines[i];
					if (line.contains("use_dhcp")) {
						chckbxEnableDHCP.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("dhcp_retry_time")) {
						spinner.setValue(Properties.getInt(line));
						continue;
					}
					//
					if (line.contains("ip_address")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedStaticIP1.setValue(Integer.parseInt(parts[0]));
							formattedStaticIP2.setValue(Integer.parseInt(parts[1]));
							formattedStaticIP3.setValue(Integer.parseInt(parts[2]));
							formattedStaticIP4.setValue(Integer.parseInt(parts[3]));
						}
					}
					if (line.contains("default_gateway")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedGatewayIP1.setValue(Integer.parseInt(parts[0]));
							formattedGatewayIP2.setValue(Integer.parseInt(parts[1]));
							formattedGatewayIP3.setValue(Integer.parseInt(parts[2]));
							formattedGatewayIP4.setValue(Integer.parseInt(parts[3]));
						}
					}
					if (line.contains("net_mask")) {
						textFieldNetmask.setText(Properties.getString(line));
						try {
							formattedCdir.setValue(convertNetmaskToCIDR(InetAddress.getByName(Properties.getString(line))));
						} catch (UnknownHostException e) {
							e.printStackTrace();
						}
						continue;	
					}
					if (line.contains("hostname")) {
						textFieldHostname.setText(Properties.getString(line));
						continue;
					}
					//
					if (line.contains("ntp_server")) {
						String value;
						
						if (line.startsWith("#")) {
							value = prefs.get(LAST_USED_NTP_SERVER, "0.0.0.0").replace('.', '\n');
							chckbxEnableNtpServer.setSelected(false);
						} else {
							value = Properties.getString(line).replace('.', '\n');
							chckbxEnableNtpServer.setSelected(true);
						}
						
						final String parts[] = value.split("\n");
						
						if (parts.length == 4) {
							try {
								formattedNtpServerIP1.setValue(Integer.parseInt(parts[0]));
								formattedNtpServerIP2.setValue(Integer.parseInt(parts[1]));
								formattedNtpServerIP3.setValue(Integer.parseInt(parts[2]));
								formattedNtpServerIP4.setValue(Integer.parseInt(parts[3]));	
							} catch (Exception e) {
							}
						}
					}
					if (line.contains("ntp_utc_offset")) {
						if (line.startsWith("#")) {
							TimeZone timeZone = TimeZone.getDefault();
							final float offsetUtc = (float) timeZone.getOffset(System.currentTimeMillis()) / 3600000;
							comboBoxUtcOffset.setSelectedItem(formatUtcOffset(offsetUtc));
						} else {
							comboBoxUtcOffset.setSelectedItem(formatUtcOffset(Float.parseFloat(Properties.getString(line))));
						}
						continue;
					}
				}
			}
			
			update();
		}
	}
	
	private void update() {
		spinner.setEnabled(chckbxEnableDHCP.isSelected());
		//
		formattedStaticIP1.setEditable(!chckbxEnableDHCP.isSelected());
		formattedStaticIP2.setEditable(!chckbxEnableDHCP.isSelected());
		formattedStaticIP3.setEditable(!chckbxEnableDHCP.isSelected());
		formattedStaticIP4.setEditable(!chckbxEnableDHCP.isSelected());
		formattedCdir.setEditable(!chckbxEnableDHCP.isSelected());
		formattedGatewayIP1.setEditable(!chckbxEnableDHCP.isSelected());
		formattedGatewayIP2.setEditable(!chckbxEnableDHCP.isSelected());
		formattedGatewayIP3.setEditable(!chckbxEnableDHCP.isSelected());
		formattedGatewayIP4.setEditable(!chckbxEnableDHCP.isSelected());
		textFieldHostname.setEditable(!chckbxEnableDHCP.isSelected());
	}	
	
	private void save() {
		if (opi != null) {
			StringBuffer networkTxt = new StringBuffer("#network.txt\n");
			
			if (chckbxEnableDHCP.isSelected()) {
				// DHCP
				networkTxt.append("use_dhcp=1\n");
				networkTxt.append(String.format("dhcp_retry_time=%d\n", spinner.getValue()));
			} else {
				// Static IP
				networkTxt.append("use_dhcp=0\n");
				networkTxt.append(String.format("ip_address=%d.%d.%d.%d\n", formattedStaticIP1.getValue(),formattedStaticIP2.getValue(),formattedStaticIP3.getValue(),formattedStaticIP4.getValue()));
				networkTxt.append(String.format("net_mask=%s\n", textFieldNetmask.getText()));
				networkTxt.append(String.format("default_gateway=%d.%d.%d.%d\n", formattedGatewayIP1.getValue(),formattedGatewayIP2.getValue(),formattedGatewayIP3.getValue(),formattedGatewayIP4.getValue()));
				
				final String hostname = textFieldHostname.getText();
				
				if (hostname.length() > 63) {
					networkTxt.append(String.format("hostname=%s\n", hostname.substring(0, 63)));
					textFieldHostname.setText(hostname.substring(0, 63));
				} else {
					networkTxt.append(String.format("hostname=%s\n", hostname));
				}
			}
			
			if (chckbxEnableNtpServer.isSelected()) {
				// NTP Server
				final String ip = String.format("%d.%d.%d.%d", formattedNtpServerIP1.getValue(), formattedNtpServerIP2.getValue(), formattedNtpServerIP3.getValue(), formattedNtpServerIP4.getValue());
				networkTxt.append(String.format("ntp_server=%s\n", ip));
				prefs.put(LAST_USED_NTP_SERVER, ip);

				String utcOffset = comboBoxUtcOffset.getSelectedItem().toString().replace(',', '.');
				
				if (utcOffset.startsWith("+")) {
					utcOffset = utcOffset.substring(1);
				}
				networkTxt.append(String.format("ntp_utc_offset=%s\n", utcOffset));
			}
					
			try {
				opi.doSave(networkTxt.toString());
			} catch (IOException e) {
				e.printStackTrace();
			}

			if (remoteConfig != null) {
				remoteConfig.setTextArea(opi.getTxt("network.txt"));
			}

			System.out.println(networkTxt.toString());
		}
	}
}
