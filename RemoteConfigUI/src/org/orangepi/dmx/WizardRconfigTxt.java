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

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;

import java.awt.event.ActionListener;
import java.io.IOException;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

public class WizardRconfigTxt extends JDialog {
	private final JPanel contentPanel = new JPanel();
	private static final long serialVersionUID = 1L;
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	private JButton btnCancel;
	private JButton btnSave;
	private JTextField textFieldDisplayName;
	private JCheckBox chckbxDisable;
	private JCheckBox chckbxDisableWrite;
	private JCheckBox chckbxEnableUptime;
	private JCheckBox chckbxEnableReboot;
	private JCheckBox chckbxEnableFactoryDefaults;
	private JButton btnSetDefaults;

	public WizardRconfigTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		load();
	}

	private void initComponents() {
		setBounds(100, 100, 381, 259);
		
		chckbxDisable = new JCheckBox("Disable remote configuration");
		
		chckbxDisableWrite = new JCheckBox("Disable write");
		chckbxDisableWrite.setToolTipText("Read only");
		
		chckbxEnableUptime = new JCheckBox("Enable get uptime");
		
		chckbxEnableReboot = new JCheckBox("Enable reboot");
		
		chckbxEnableFactoryDefaults = new JCheckBox("Enable set factory defaults");
		
		JLabel lblDisplayname = new JLabel("Display name");
		
		textFieldDisplayName = new JTextField();
		textFieldDisplayName.setColumns(10);
		
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		GroupLayout groupLayout = new GroupLayout(contentPanel);
	
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(chckbxDisable)
						.addComponent(chckbxDisableWrite)
						.addComponent(chckbxEnableUptime)
						.addComponent(chckbxEnableReboot)
						.addComponent(chckbxEnableFactoryDefaults)
						.addGroup(groupLayout.createSequentialGroup()
							.addGap(9)
							.addComponent(lblDisplayname)
							.addGap(18)
							.addComponent(textFieldDisplayName, GroupLayout.PREFERRED_SIZE, 253, GroupLayout.PREFERRED_SIZE)))
					.addContainerGap())
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(Alignment.LEADING, groupLayout.createSequentialGroup()
					.addContainerGap()
					.addComponent(chckbxDisable)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxDisableWrite)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxEnableUptime)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxEnableReboot)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxEnableFactoryDefaults)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(textFieldDisplayName, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblDisplayname))
					.addContainerGap(44, Short.MAX_VALUE))
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
		
		btnSetDefaults.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.setTextArea(opi.doDefaults("rconfig.txt"));
				load();
			}
		});
	}
	
	private void load() {
		if (opi != null) {
			final String txt = opi.getTxt("rconfig.txt");
			if (txt != null) {
				final String[] lines = txt.split("\n");
				for (int i = 0; i < lines.length; i++) {
					final String line = lines[i];
					if (line.contains("disable")) {
						chckbxDisable.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("disable_write")) {
						chckbxDisableWrite.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("enable_reboot")) {
						chckbxEnableReboot.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("enable_uptime")) {
						chckbxEnableUptime.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("enable_factory")) {
						chckbxEnableFactoryDefaults.setSelected(Properties.getBool(line));
						continue;
					}
					if (line.contains("display_name")) {
						textFieldDisplayName.setText(Properties.getString(line));
					}
				}
			}
		}		
	}
		
	private void save() {
		if (opi != null) {
			StringBuffer rconfigTxt = new StringBuffer("#rconfig.txt\n");
			rconfigTxt.append(String.format("disable=%c\n",chckbxDisable.isSelected() ? '1' : '0'));
			rconfigTxt.append(String.format("disable_write=%c\n",chckbxDisableWrite.isSelected() ? '1' : '0'));
			rconfigTxt.append(String.format("enable_reboot=%c\n",chckbxEnableReboot.isSelected() ? '1' : '0'));
			rconfigTxt.append(String.format("enable_uptime=%c\n",chckbxEnableUptime.isSelected() ? '1' : '0'));		
			rconfigTxt.append(String.format("enable_factory=%c\n",chckbxEnableFactoryDefaults.isSelected() ? '1' : '0'));
		
			String displayName = textFieldDisplayName.getText().trim();

			if (displayName.length() != 0) {
				rconfigTxt.append("display_name=");
				if (displayName.length() > 24) {
					rconfigTxt.append(displayName.substring(0, 24));
					textFieldDisplayName.setText(displayName.substring(0, 24));
				} else {
					rconfigTxt.append(displayName);
				}
			}

			try {
				opi.doSave(rconfigTxt.toString());
			} catch (IOException e) {
				e.printStackTrace();
			}

			if (remoteConfig != null) {
				remoteConfig.setTextArea(opi.getTxt("rconfig.txt"));
			}

			System.out.println(rconfigTxt.toString());
		}
	}
}
