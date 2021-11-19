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

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.text.NumberFormatter;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.text.NumberFormat;
import java.awt.event.ActionEvent;
import javax.swing.JLabel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JComboBox;
import javax.swing.JFormattedTextField;
import javax.swing.JCheckBox;
import javax.swing.SwingConstants;

public class WizardDisplayTxt extends JDialog {
	//
	private static final String TXT_FILE = "display.txt";
	//
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	final static String propertieKeys[] = { "#", "title", "board_name", "ip_address", "version", "universe", "active_ports",
			"short_name", "hostname", "universe_port_a", "universe_port_b", "universe_port_c", "universe_port_d",
			"net_mask", "dmx_start_address", "destination_ip_port_a", "destination_ip_port_b", "destination_ip_port_c",
			"destination_ip_port_d", "default_gateway", "dmx_direction" };
	//
	private JButton btnCancel;
	private JButton btnSave;
	private JComboBox<String> comboBoxLine1;
	private JComboBox<String> comboBoxLine2;
	private JComboBox<String> comboBoxLine3;
	private JComboBox<String> comboBoxLine4;
	private JComboBox<String> comboBoxLine5;
	private JComboBox<String> comboBoxLine6;
	private JFormattedTextField formattedTextFieldIntensity;
	private JFormattedTextField formattedTextFieldSleepTimeout;
	private JLabel lblMinutes;
	private JButton btnSetDefaults;
	private JCheckBox chckbxNewCheckBox;

	public static void main(String[] args) {
		try {
			WizardDisplayTxt dialog = new WizardDisplayTxt("Debug", null, null);
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public WizardDisplayTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		for (int i = 0; i < propertieKeys.length; i++) {
			comboBoxLine1.addItem(propertieKeys[i]);
			comboBoxLine2.addItem(propertieKeys[i]);
			comboBoxLine3.addItem(propertieKeys[i]);
			comboBoxLine4.addItem(propertieKeys[i]);
			comboBoxLine5.addItem(propertieKeys[i]);
			comboBoxLine6.addItem(propertieKeys[i]);
		}

		load();
	}

	private void initComponents() {
		setBounds(100, 100, 308, 334);

		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);

		JLabel lblLine6 = new JLabel("Line 6");
		JLabel lblLine5 = new JLabel("Line 5");
		JLabel lblLine4 = new JLabel("Line 4");
		JLabel lblLine3 = new JLabel("Line 3");
		JLabel lblLine2 = new JLabel("Line 2");
		JLabel lblLine1 = new JLabel("Line 1");

		comboBoxLine1 = new JComboBox<String>();
		comboBoxLine2 = new JComboBox<String>();
		comboBoxLine3 = new JComboBox<String>();
		comboBoxLine4 = new JComboBox<String>();
		comboBoxLine5 = new JComboBox<String>();
		comboBoxLine6 = new JComboBox<String>();

		NumberFormat format = NumberFormat.getInstance();

		NumberFormatter formatterIntensity = new NumberFormatter(format);
		formatterIntensity.setValueClass(Integer.class);
		formatterIntensity.setMinimum(1);
		formatterIntensity.setMaximum(255);
		formatterIntensity.setAllowsInvalid(false);
		formatterIntensity.setCommitsOnValidEdit(true);

		JLabel lblIntensity = new JLabel("Intensity");
		formattedTextFieldIntensity = new JFormattedTextField(formatterIntensity);
		formattedTextFieldIntensity.setColumns(2);

		NumberFormatter formatterSleepTimeout = new NumberFormatter(format);
		formatterSleepTimeout.setValueClass(Integer.class);
		formatterSleepTimeout.setMinimum(0);
		formatterSleepTimeout.setMaximum(255);
		formatterSleepTimeout.setAllowsInvalid(false);
		formatterSleepTimeout.setCommitsOnValidEdit(true);

		JLabel lblSleepTimeout = new JLabel("Sleep timout");
		formattedTextFieldSleepTimeout = new JFormattedTextField(formatterSleepTimeout);
		formattedTextFieldSleepTimeout.setToolTipText("0 is always on");
		formattedTextFieldSleepTimeout.setColumns(2);

		lblMinutes = new JLabel("Minutes");
		
		chckbxNewCheckBox = new JCheckBox("Flip vertically");
		chckbxNewCheckBox.setHorizontalAlignment(SwingConstants.RIGHT);

		GroupLayout groupLayout = new GroupLayout(contentPanel);
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addGroup(groupLayout.createSequentialGroup()
									.addComponent(lblLine2, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxLine2, 0, 248, Short.MAX_VALUE))
								.addGroup(groupLayout.createSequentialGroup()
									.addComponent(lblLine6)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxLine6, 0, 248, Short.MAX_VALUE))
								.addGroup(groupLayout.createSequentialGroup()
									.addComponent(lblLine1, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxLine1, 0, 248, Short.MAX_VALUE))
								.addGroup(groupLayout.createSequentialGroup()
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
											.addComponent(lblLine4, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE)
											.addComponent(lblLine3, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE))
										.addComponent(lblLine5, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE))
									.addPreferredGap(ComponentPlacement.RELATED)
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addComponent(comboBoxLine4, 0, 248, Short.MAX_VALUE)
										.addComponent(comboBoxLine3, 0, 248, Short.MAX_VALUE)
										.addComponent(comboBoxLine5, 0, 248, Short.MAX_VALUE)))
								.addGroup(groupLayout.createSequentialGroup()
									.addComponent(lblIntensity)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedTextFieldIntensity, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
									.addGap(18)
									.addComponent(lblSleepTimeout)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedTextFieldSleepTimeout, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(lblMinutes)))
							.addGap(0))
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(chckbxNewCheckBox)
							.addContainerGap(176, Short.MAX_VALUE))))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblIntensity)
						.addComponent(formattedTextFieldIntensity, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblSleepTimeout)
						.addComponent(formattedTextFieldSleepTimeout, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblMinutes))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblLine1)
						.addComponent(comboBoxLine1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxLine2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblLine2))
					.addGap(8)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxLine3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblLine3))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxLine4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblLine4))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxLine5, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblLine5))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblLine6)
						.addComponent(comboBoxLine6, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(8)
					.addComponent(chckbxNewCheckBox))
		);

		contentPanel.setLayout(groupLayout);

		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			
			{
				btnSave = new JButton("Save");
				buttonPane.add(btnSave);
				getRootPane().setDefaultButton(btnSave);
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
					if (line.contains("sleep_timeout")) {
						formattedTextFieldSleepTimeout.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("intensity")) {
						formattedTextFieldIntensity.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("flip_vertically")) {
						chckbxNewCheckBox.setSelected(Properties.getBool(line));
						continue;
					}
					for (int j = 1; j < propertieKeys.length; j++) {
						if (line.contains(propertieKeys[j])) {
							final int displayLine = Properties.getInt(line);
							switch (displayLine) {
							case 1:
								comboBoxLine1.setSelectedIndex(j);
								break;
							case 2:
								comboBoxLine2.setSelectedIndex(j);
								break;
							case 3:
								comboBoxLine3.setSelectedIndex(j);
								break;
							case 4:
								comboBoxLine4.setSelectedIndex(j);
								break;
							case 5:
								comboBoxLine5.setSelectedIndex(j);
								break;
							case 6:
								comboBoxLine6.setSelectedIndex(j);
								break;
							default:
								break;
							}
						}
					}
				}
			}
		}
	}

	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			//
			txtAppend.append(String.format("sleep_timeout=%d\n", formattedTextFieldSleepTimeout.getValue()));
			txtAppend.append(String.format("intensity=%d\n", formattedTextFieldIntensity.getValue()));
			txtAppend.append(String.format("flip_vertically=%c\n", chckbxNewCheckBox.isSelected() ? '1' : '0'));
			//
			txtAppend.append(String.format("%s=1\n", comboBoxLine1.getSelectedObjects()));
			txtAppend.append(String.format("%s=2\n", comboBoxLine2.getSelectedObjects()));
			txtAppend.append(String.format("%s=3\n", comboBoxLine3.getSelectedObjects()));
			txtAppend.append(String.format("%s=4\n", comboBoxLine4.getSelectedObjects()));
			txtAppend.append(String.format("%s=5\n", comboBoxLine5.getSelectedObjects()));
			txtAppend.append(String.format("%s=6\n", comboBoxLine6.getSelectedObjects()));
			//
			String txt = Properties.removeComments(opi.getTxt(TXT_FILE));

			try {
				opi.doSave(txt + "\n" + txtAppend.toString());
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			if (remoteConfig != null) {
				remoteConfig.setTextArea(opi.getTxt(TXT_FILE));
			}
			
			System.out.println(txtAppend.toString());
		}
	}
}
