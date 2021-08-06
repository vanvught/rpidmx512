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
import java.text.NumberFormat;

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
import javax.swing.text.NumberFormatter;

public class WizardE131Txt extends JDialog {
	private static final String TXT_FILE = "e131.txt";
	//
	private static final long serialVersionUID = 1L;
	String nodeId = null;
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;

	private final JPanel contentPanel = new JPanel();
	private JButton btnCancel;
	private JButton btnSetDefaults;
	private JButton btnSave;
	private JFormattedTextField formattedTextFieldUniverseA;
	private JFormattedTextField formattedTextFieldUniverseB;
	private JFormattedTextField formattedTextFieldUniverseC;
	private JFormattedTextField formattedTextFieldUniverseD;
	private JComboBox<String>  comboBoxMergePortA;
	private JComboBox<String>  comboBoxMergePortB;
	private JComboBox<String>  comboBoxMergePortC;
	private JComboBox<String>  comboBoxMergePortD;
	private JCheckBox chckbxEnablePortD;
	private JCheckBox chckbxEnablePortC;
	private JCheckBox chckbxEnablePortB;
	private JCheckBox chckbxEnablePortA;
	
	//
	private JComboBox<String> comboBoxDirection;
	private JFormattedTextField formattedTextFieldPriority;
	//

	public static void main(String[] args) {
		try {
			WizardArtnetTxt dialog = new WizardArtnetTxt();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public WizardE131Txt() {
		initComponents();
		createEvents();

	}
	
	public WizardE131Txt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.nodeId = nodeId;
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		load();
	}
	
	private void initComponents() {
		setBounds(100, 100, 307, 288);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		NumberFormat format = NumberFormat.getInstance();
		NumberFormatter formatterUniverse = new NumberFormatter(format);
		formatterUniverse.setValueClass(Integer.class);
		formatterUniverse.setMinimum(0);
		formatterUniverse.setMaximum(32768);
		formatterUniverse.setAllowsInvalid(false);
		formatterUniverse.setCommitsOnValidEdit(true);
		
		JLabel lblUniversePortA = new JLabel("Port A");
		JLabel lblUniversePortB = new JLabel("Port B");
		JLabel lblUniversePortC = new JLabel("Port C");
		JLabel lblUniversePortD = new JLabel("Port D");
		
		formattedTextFieldUniverseA = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseA.setColumns(6);
		formattedTextFieldUniverseB = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseB.setColumns(6);
		formattedTextFieldUniverseC = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseC.setColumns(6);
		formattedTextFieldUniverseD = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseD.setColumns(6);
		
		comboBoxMergePortA = new JComboBox<String> ();
		comboBoxMergePortA.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortB = new JComboBox<String> ();
		comboBoxMergePortB.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortC = new JComboBox<String> ();
		comboBoxMergePortC.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortD = new JComboBox<String> ();
		comboBoxMergePortD.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		
		chckbxEnablePortA = new JCheckBox("Enable");
		chckbxEnablePortB = new JCheckBox("Enable");
		chckbxEnablePortC = new JCheckBox("Enable");
		chckbxEnablePortD = new JCheckBox("Enable");
		
		comboBoxDirection = new JComboBox<String>();
		comboBoxDirection.setModel(new DefaultComboBoxModel<String>(new String[] {"Output", "Input"}));
		
		JLabel lblPriority = new JLabel("Priority");
		
		NumberFormatter formatterPriority = new NumberFormatter(format);
		formatterPriority.setValueClass(Integer.class);
		formatterPriority.setMinimum(1);
		formatterPriority.setMaximum(200);
		formatterPriority.setAllowsInvalid(false);
		formatterPriority.setCommitsOnValidEdit(true);
		
		formattedTextFieldPriority = new JFormattedTextField(formatterPriority);
		formattedTextFieldPriority.setEditable(false);
		formattedTextFieldPriority.setText("100");
		formattedTextFieldPriority.setColumns(3);
				
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(lblUniversePortB, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblUniversePortA)
						.addComponent(lblUniversePortD)
						.addComponent(lblUniversePortC, GroupLayout.PREFERRED_SIZE, 38, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(comboBoxDirection, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(lblPriority)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(formattedTextFieldPriority, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addComponent(formattedTextFieldUniverseD, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxMergePortD, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE))
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addComponent(formattedTextFieldUniverseC, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxMergePortC, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE))
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addComponent(formattedTextFieldUniverseB, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxMergePortB, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE))
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addComponent(formattedTextFieldUniverseA, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(comboBoxMergePortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(chckbxEnablePortC, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE)
								.addComponent(chckbxEnablePortD, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE)
								.addComponent(chckbxEnablePortB, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE)
								.addComponent(chckbxEnablePortA))))
					.addGap(41))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxDirection, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblPriority)
						.addComponent(formattedTextFieldPriority, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(2)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblUniversePortA)
								.addComponent(formattedTextFieldUniverseA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(chckbxEnablePortA))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblUniversePortB)
								.addComponent(formattedTextFieldUniverseB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(chckbxEnablePortB)
							.addPreferredGap(ComponentPlacement.RELATED)))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(formattedTextFieldUniverseC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(chckbxEnablePortC)))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(11)
							.addComponent(lblUniversePortC)))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblUniversePortD)
						.addComponent(formattedTextFieldUniverseD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(comboBoxMergePortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(chckbxEnablePortD))
					.addGap(209))
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
				
		comboBoxDirection.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				formattedTextFieldPriority.setEditable(comboBoxDirection.getSelectedIndex() == 1);
				formattedTextFieldPriority.setEnabled(comboBoxDirection.getSelectedIndex() == 1);
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
					if (line.contains("direction")) {
						if (Properties.getString(line).equals("input")) {
							comboBoxDirection.setSelectedIndex(1);
						} else {
							comboBoxDirection.setSelectedIndex(0);
						}
					}
					if (line.contains("universe_port_a")) {
						formattedTextFieldUniverseA.setValue(Properties.getInt(line));
						chckbxEnablePortA.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_b")) {
						formattedTextFieldUniverseB.setValue(Properties.getInt(line));
						chckbxEnablePortB.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_c")) {
						formattedTextFieldUniverseC.setValue(Properties.getInt(line));
						chckbxEnablePortC.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_d")) {
						formattedTextFieldUniverseD.setValue(Properties.getInt(line));
						chckbxEnablePortD.setSelected(!line.startsWith("#"));
						continue;
					}
					//
					if (line.contains("merge_mode_port_a")) {
						comboBoxMergePortA.setSelectedIndex(Properties.getString(line).equals("ltp") ? 1 : 0);
						continue;
					}
					if (line.contains("merge_mode_port_b")) {
						comboBoxMergePortB.setSelectedIndex(Properties.getString(line).equals("ltp") ? 1 : 0);
						continue;
					}
					if (line.contains("merge_mode_port_c")) {
						comboBoxMergePortC.setSelectedIndex(Properties.getString(line).equals("ltp") ? 1 : 0);
						continue;
					}
					if (line.contains("merge_mode_port_d")) {
						comboBoxMergePortD.setSelectedIndex(Properties.getString(line).equals("ltp") ? 1 : 0);
						continue;
					}
					// Input
					if (line.contains("priority")) {
						formattedTextFieldPriority.setValue(Properties.getInt(line));
					}
				}
			}
		}
	}
	
	private String getComment(JCheckBox checkBox) {
		return checkBox.isSelected() ? "" : "#";
	}
	
	private String getMergeMode(JComboBox<String> comboBox) {
		if (comboBox.getSelectedItem().toString().toLowerCase().equals("ltp")) {
			return "ltp";
		}
		return "htp";
	}
	
	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			
			txtAppend.append(String.format("direction=%s\n", comboBoxDirection.getSelectedIndex() == 1 ? "input" : "output"));
						
			txtAppend.append(String.format("%suniverse_port_a=%s\n", getComment(chckbxEnablePortA), formattedTextFieldUniverseA.getValue()));
			txtAppend.append(String.format("%suniverse_port_b=%s\n", getComment(chckbxEnablePortB), formattedTextFieldUniverseB.getValue()));
			txtAppend.append(String.format("%suniverse_port_c=%s\n", getComment(chckbxEnablePortC), formattedTextFieldUniverseC.getValue()));
			txtAppend.append(String.format("%suniverse_port_d=%s\n", getComment(chckbxEnablePortD), formattedTextFieldUniverseD.getValue()));

			txtAppend.append(String.format("merge_mode_port_a=%s\n", getMergeMode(comboBoxMergePortA)));
			txtAppend.append(String.format("merge_mode_port_b=%s\n", getMergeMode(comboBoxMergePortB)));
			txtAppend.append(String.format("merge_mode_port_c=%s\n", getMergeMode(comboBoxMergePortC)));
			txtAppend.append(String.format("merge_mode_port_d=%s\n", getMergeMode(comboBoxMergePortD)));
			
			txtAppend.append(String.format("priority=%s\n", formattedTextFieldPriority.getValue()));
					
			String txt = Properties.removeComments(opi.getTxt(TXT_FILE));
			txt = txt.replaceAll("direction", "#direction");
			txt = txt.replaceAll("universe_port_", "#universe_port_");
					
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
