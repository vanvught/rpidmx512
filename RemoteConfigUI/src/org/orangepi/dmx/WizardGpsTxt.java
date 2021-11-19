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
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.TimeZone;

import javax.swing.DefaultComboBoxModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;

public class WizardGpsTxt extends JDialog {
	private static final String TXT_FILE = "gps.txt";
	//
	private static final long serialVersionUID = 1L;
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	private final JPanel contentPanel = new JPanel();
	//
	private JComboBox<String> comboBoxModule;
	private JComboBox<String> comboBoxUtcOffset;
	private JCheckBox chckbxEnable;
	//
	private JButton btnSetDefaults;
	private JButton btnCancel;
	private JButton btnSave;
	//
	static final float UtcValidOffets[] = { -12.0f, -11.0f, -10.0f, -9.5f, -9.0f, -8.0f, -7.0f, -6.0f, -5.0f, -4.0f,
			-3.5f, -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 5.75f, 6.0f, 6.5f, 7.0f,
			8.0f, 8.75f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 12.0f, 12.75f, 13.0f, 14.0f };
	private JLabel lblModule;

	final static String MODULES[] = {"Generic", "ATGM336H", "ublox-NEO7", "MTK3339"};

	public static void main(String[] args) {
		try {
			WizardGpsTxt dialog = new WizardGpsTxt();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public WizardGpsTxt() {
		initComponents();
		createEvents();
	}
	
	public WizardGpsTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		load();
	}

	private void initComponents() {
		setBounds(100, 100, 291, 184);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		chckbxEnable = new JCheckBox("Enable");
		
		JLabel lblUtcOffset = new JLabel("UTC offset");
		
		comboBoxUtcOffset = new JComboBox<String>();
		for (int i = 0; i < UtcValidOffets.length; i++) {
			comboBoxUtcOffset.addItem(formatUtcOffset(UtcValidOffets[i]));
		}
		lblModule = new JLabel("Module");
		comboBoxModule = new JComboBox<String>();
		comboBoxModule.setModel(new DefaultComboBoxModel<String>(MODULES));
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING)
						.addComponent(chckbxEnable)
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
							.addComponent(lblModule)
							.addComponent(lblUtcOffset, GroupLayout.PREFERRED_SIZE, 67, GroupLayout.PREFERRED_SIZE)))
					.addGap(18)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(comboBoxUtcOffset, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE)
						.addComponent(comboBoxModule, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(55, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblModule)
						.addComponent(comboBoxModule, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblUtcOffset)
						.addComponent(comboBoxUtcOffset, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(chckbxEnable)
					.addContainerGap(35, Short.MAX_VALUE))
		);
		
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				btnSave = new JButton("Save");
				buttonPane.add(btnSave);
			}
			{
				btnSetDefaults = new JButton("Set default");
				buttonPane.add(btnSetDefaults);
			}
			{
				btnCancel = new JButton("Cancel");
				buttonPane.add(btnCancel);
			}
			getRootPane().setDefaultButton(btnCancel);
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
				remoteConfig.setTextArea(opi.doDefaults(TXT_FILE));
				load();
			}
		});
	}
	
	private void load() {
		if (opi != null) {
			final String txt = opi.getTxt(TXT_FILE);
			if (txt != null) {
				final String[] lines = txt.split("\n");
				for (int i = 0; i < lines.length; i++) {
					final String line = lines[i];
					if (line.contains("module")) {
						for (int j = 1; j < MODULES.length; j++) {
							if (line.toUpperCase().contains(MODULES[j].toUpperCase())) {
								comboBoxModule.setSelectedIndex(j);
								break;
							}
						}
						continue;
					}
					//
					if (line.contains("utc_offset")) {
						if (line.startsWith("#")) {
							TimeZone timeZone = TimeZone.getDefault();
							final float offsetUtc = (float) timeZone.getOffset(System.currentTimeMillis()) / 3600000;
							comboBoxUtcOffset.setSelectedItem(formatUtcOffset(offsetUtc));
						} else {
							comboBoxUtcOffset.setSelectedItem(formatUtcOffset(Float.parseFloat(Properties.getString(line))));
						}
						continue;
					}
					//
					if (line.contains("enable")) {
						chckbxEnable.setSelected(Properties.getBool(line));
						continue;
					}
				}
			}
		}
	}
	
	private String formatUtcOffset(float f) {
		if (f > 0) {
			return String.format("+%1.2f", f);
		}
		return String.format("%1.2f", f);
	}
	
	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			//
			txtAppend.append(String.format("module=%s\n", comboBoxModule.getSelectedItem()));
			//
			String utcOffset = comboBoxUtcOffset.getSelectedItem().toString().replace(',', '.');
			
			if (utcOffset.startsWith("+")) {
				utcOffset = utcOffset.substring(1);
			}
			txtAppend.append(String.format("utc_offset=%s\n", utcOffset));
			//
			txtAppend.append(String.format("enable=%d\n", chckbxEnable.isSelected() ? 1 : 0));
				
			String txt = Properties.removeComments(opi.getTxt(TXT_FILE));
					
			try {
				opi.doSave(txt + "\n" + txtAppend.toString());
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			if (remoteConfig != null) {
				remoteConfig.setTextArea(opi.getTxt(TXT_FILE));
			}
		}
	}
}
