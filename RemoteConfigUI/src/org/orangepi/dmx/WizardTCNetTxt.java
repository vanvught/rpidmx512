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
import java.text.ParseException;

import javax.swing.DefaultComboBoxModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.text.MaskFormatter;

public class WizardTCNetTxt extends JDialog {
	private static final String TXT_FILE = "tcnet.txt";
	//
	private static final long serialVersionUID = 1L;
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	private final JPanel contentPanel = new JPanel();
	//
	private JFormattedTextField formattedTextFieldNodeName;
	private JComboBox<String> comboBoxLayer;
	private JComboBox<String> comboBoxTimecodeType;
	private JCheckBox chckbxUseTimecode;
	//
	private JButton btnCancel;
	private JButton btnSetDefaults;
	private JButton btnSave;
	//
	static final String LAYER[] = { "1", "2", "3", "4", "A", "B", "M", "C" };
	static final String TIMECODE_STRING[] = { "Film 24fps ", "EBU 25fps  ", "DF 29.97fps", "SMPTE 30fps" };
	static final int TIMECODE_INT[] = { 24, 25, 29, 30 };

	
	public static void main(String[] args) {
		try {
			WizardTCNetTxt dialog = new WizardTCNetTxt();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public WizardTCNetTxt() {
		initComponents();
		createEvents();
	}
	
	public WizardTCNetTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		load();
	}

	private void initComponents() {
		setBounds(100, 100, 317, 222);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
	
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		JLabel lblNodeName = new JLabel("Node Name");
		JLabel lblLayer = new JLabel("Layer");
		JLabel lblTimecodeType = new JLabel("Timecode Type");
		
		chckbxUseTimecode = new JCheckBox("Use Timecode");
		
		comboBoxLayer = new JComboBox<String>();
		comboBoxLayer.setModel(new DefaultComboBoxModel<String>(LAYER));
			
		try {
			formattedTextFieldNodeName = new JFormattedTextField(new MaskFormatter("********"));
		} catch (ParseException e) {
			e.printStackTrace();
		}
		
		comboBoxTimecodeType = new JComboBox<String>();
		comboBoxTimecodeType.setModel(new DefaultComboBoxModel<String>(TIMECODE_STRING));

		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addComponent(lblTimecodeType)
									.addPreferredGap(ComponentPlacement.UNRELATED)
									.addComponent(comboBoxTimecodeType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
										.addComponent(lblNodeName)
										.addComponent(lblLayer))
									.addGap(32)
									.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
										.addComponent(formattedTextFieldNodeName, GroupLayout.PREFERRED_SIZE, 77, GroupLayout.PREFERRED_SIZE)
										.addComponent(comboBoxLayer, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))))
						.addComponent(chckbxUseTimecode))
					.addContainerGap(119, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNodeName)
						.addComponent(formattedTextFieldNodeName, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblLayer)
						.addComponent(comboBoxLayer, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(18)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblTimecodeType)
						.addComponent(comboBoxTimecodeType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(chckbxUseTimecode)
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
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
					//
					if (line.contains("node_name")) {
						formattedTextFieldNodeName.setText(Properties.getString(line).trim());
						continue;
					}
					//
					if (line.contains("layer")) {
						for (int j = 1; j < LAYER.length; j++) {
							if (Properties.getString(line).equals(LAYER[j].toUpperCase()) ) {
								comboBoxLayer.setSelectedIndex(j);
								break;
							}
						}
						continue;
					}
					//
					if (line.contains("timecode_type")) {
						for (int j = 1; j < TIMECODE_INT.length; j++) {
							if (Properties.getInt(line) == TIMECODE_INT[j]) {
								comboBoxTimecodeType.setSelectedIndex(j);
								break;
							}
						}
					}
					//
					if (line.contains("use_timecode")) {
						chckbxUseTimecode.setSelected(Properties.getBool(line));
						continue;
					}
				}
			}
		}
	}
	
	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			//
			txtAppend.append(String.format("node_name=%s\n", formattedTextFieldNodeName.getText().trim()));
			txtAppend.append(String.format("layer=%s\n", comboBoxLayer.getSelectedItem()));
			txtAppend.append(String.format("timecode_type=%d\n", TIMECODE_INT[comboBoxTimecodeType.getSelectedIndex()]));
			txtAppend.append(String.format("use_timecode=%d\n", chckbxUseTimecode.isSelected() ? 1 : 0));
				
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
