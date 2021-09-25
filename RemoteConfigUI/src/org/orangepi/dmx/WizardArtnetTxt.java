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
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.text.NumberFormat;
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
import javax.swing.text.NumberFormatter;

public class WizardArtnetTxt extends JDialog {
	//
	private static final String TXT_FILE = "artnet.txt";
	//
	private static final long serialVersionUID = 1L;
	String nodeId = null;
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;

	private final JPanel contentPanel = new JPanel();
	private JButton btnCancel;
	private JButton btnSetDefaults;
	private JButton btnSave;
	private JFormattedTextField formattedTextFieldUniverseB;
	private JFormattedTextField formattedTextFieldUniverseA;
	private JFormattedTextField formattedTextFieldUniverseC;
	private JFormattedTextField formattedTextFieldUniverseD;
	private JComboBox<String> comboBoxDirectionPortA;
	private JComboBox<String> comboBoxDirectionPortB;
	private JComboBox<String> comboBoxDirectionPortC;
	private JComboBox<String> comboBoxDirectionPortD;
	private JComboBox<String>  comboBoxMergePortA;
	private JComboBox<String>  comboBoxMergePortB;
	private JComboBox<String>  comboBoxMergePortC;
	private JComboBox<String>  comboBoxMergePortD;
	private JComboBox<String>  comboBoxProtocolPortA;
	private JComboBox<String>  comboBoxProtocolPortB;
	private JComboBox<String>  comboBoxProtocolPortC;
	private JComboBox<String>  comboBoxProtocolPortD;
	private JCheckBox chckbxEnablePortD;
	private JCheckBox chckbxEnablePortC;
	private JCheckBox chckbxEnablePortB;
	private JCheckBox chckbxEnablePortA;
	
	private int artnetNet = 0;
	private int artnetSubnet = 0;
	
	private int artnetAdressPortA;
	private int artnetAdressPortB;
	private int artnetAdressPortC;
	private int artnetAdressPortD;
	
	private JCheckBox chckbxMapUniverse0;
	
	private JLabel lblDirection;
	
	private JFormattedTextField formattedIP1PortA;
	private JFormattedTextField formattedIP2PortA;
	private JFormattedTextField formattedIP3PortA;
	private JFormattedTextField formattedIP4PortA;
	
	private JFormattedTextField formattedIP1PortB;
	private JFormattedTextField formattedIP2PortB;
	private JFormattedTextField formattedIP3PortB;
	private JFormattedTextField formattedIP4PortB;
	
	private JFormattedTextField formattedIP1PortC;
	private JFormattedTextField formattedIP2PortC;
	private JFormattedTextField formattedIP3PortC;
	private JFormattedTextField formattedIP4PortC;
	
	private JFormattedTextField formattedIP1PortD;
	private JFormattedTextField formattedIP2PortD;
	private JFormattedTextField formattedIP3PortD;
	private JFormattedTextField formattedIP4PortD;
	
	private JLabel lblUniversePortA;
	private JLabel lblUniversePortB;
	private JLabel lblUniversePortC;
	private JLabel lblUniversePortD;
	

	public static void main(String[] args) {
		try {
			WizardArtnetTxt dialog = new WizardArtnetTxt();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public WizardArtnetTxt() {
		initComponents();
		createEvents();

	}
	
	public WizardArtnetTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.nodeId = nodeId;
		this.opi = opi;
		this.remoteConfig = remoteConfig;

		setTitle(nodeId);

		initComponents();
		createEvents();

		load();
	}
	
	private void initComponents() {
		setBounds(100, 100, 666, 280);
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
		
		lblUniversePortA = new JLabel("Port A");
		lblUniversePortB = new JLabel("Port B");
		lblUniversePortC = new JLabel("Port C");
		lblUniversePortD = new JLabel("Port D");
		
		formattedTextFieldUniverseA = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseA.setColumns(6);
		formattedTextFieldUniverseB = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseB.setColumns(6);
		formattedTextFieldUniverseC = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseC.setColumns(6);
		formattedTextFieldUniverseD = new JFormattedTextField(formatterUniverse);
		formattedTextFieldUniverseD.setColumns(6);
		
		comboBoxDirectionPortA = new JComboBox<String> ();
		comboBoxDirectionPortA.setModel(new DefaultComboBoxModel<String>(new String[] {"Output", "Input"}));
		comboBoxDirectionPortB = new JComboBox<String>();
		comboBoxDirectionPortB.setModel(new DefaultComboBoxModel<String>(new String[] {"Output", "Input"}));
		comboBoxDirectionPortC = new JComboBox<String>();
		comboBoxDirectionPortC.setModel(new DefaultComboBoxModel<String>(new String[] {"Output", "Input"}));
		comboBoxDirectionPortD = new JComboBox<String>();
		comboBoxDirectionPortD.setModel(new DefaultComboBoxModel<String>(new String[] {"Output", "Input"}));
		
		comboBoxMergePortA = new JComboBox<String> ();
		comboBoxMergePortA.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortB = new JComboBox<String> ();
		comboBoxMergePortB.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortC = new JComboBox<String> ();
		comboBoxMergePortC.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		comboBoxMergePortD = new JComboBox<String> ();
		comboBoxMergePortD.setModel(new DefaultComboBoxModel<String> (new String[] {"HTP", "LTP"}));
		
		comboBoxProtocolPortA = new JComboBox<String> ();
		comboBoxProtocolPortA.setModel(new DefaultComboBoxModel<String> (new String[] {"Art-Net", "sACN"}));
		comboBoxProtocolPortB = new JComboBox<String> ();
		comboBoxProtocolPortB.setModel(new DefaultComboBoxModel<String> (new String[] {"Art-Net", "sACN"}));
		comboBoxProtocolPortC = new JComboBox<String> ();
		comboBoxProtocolPortC.setModel(new DefaultComboBoxModel<String> (new String[] {"Art-Net", "sACN"}));		
		comboBoxProtocolPortD = new JComboBox<String> ();
		comboBoxProtocolPortD.setModel(new DefaultComboBoxModel<String> (new String[] {"Art-Net", "sACN"}));
				
		chckbxEnablePortA = new JCheckBox("Enable");
		chckbxEnablePortB = new JCheckBox("Enable");
		chckbxEnablePortC = new JCheckBox("Enable");
		chckbxEnablePortD = new JCheckBox("Enable");
		
		chckbxMapUniverse0 = new JCheckBox("Art-Net 4 Map Universe 0");
		
		NumberFormatter formatterIP = new NumberFormatter(format);
		formatterIP.setValueClass(Integer.class);
		formatterIP.setMinimum(0);
		formatterIP.setMaximum(255);
		formatterIP.setAllowsInvalid(false);
		formatterIP.setCommitsOnValidEdit(true);
		
		formattedIP1PortA = new JFormattedTextField(formatterIP);
		formattedIP1PortA.setColumns(2);
		formattedIP2PortA = new JFormattedTextField(formatterIP);
		formattedIP2PortA.setColumns(2);
		formattedIP3PortA = new JFormattedTextField(formatterIP);
		formattedIP3PortA.setColumns(2);
		formattedIP4PortA = new JFormattedTextField(formatterIP);
		formattedIP4PortA.setColumns(2);
		
		formattedIP1PortB = new JFormattedTextField(formatterIP);
		formattedIP1PortB.setColumns(2);
		formattedIP2PortB = new JFormattedTextField(formatterIP);
		formattedIP2PortB.setColumns(2);
		formattedIP3PortB = new JFormattedTextField(formatterIP);
		formattedIP3PortB.setColumns(2);
		formattedIP4PortB = new JFormattedTextField(formatterIP);
		formattedIP4PortB.setColumns(2);
		
		formattedIP1PortC = new JFormattedTextField(formatterIP);
		formattedIP1PortC.setColumns(2);		
		formattedIP2PortC = new JFormattedTextField(formatterIP);
		formattedIP2PortC.setColumns(2);
		formattedIP3PortC = new JFormattedTextField(formatterIP);
		formattedIP3PortC.setColumns(2);
		formattedIP4PortC = new JFormattedTextField(formatterIP);
		formattedIP4PortC.setColumns(2);
		
		formattedIP1PortD = new JFormattedTextField(formatterIP);
		formattedIP1PortD.setColumns(2);
		formattedIP2PortD = new JFormattedTextField(formatterIP);
		formattedIP2PortD.setColumns(2);
		formattedIP3PortD = new JFormattedTextField(formatterIP);
		formattedIP3PortD.setColumns(2);
		formattedIP4PortD = new JFormattedTextField(formatterIP);
		formattedIP4PortD.setColumns(2);
		
		JLabel lblMergeMode = new JLabel("Merge Mode");
		lblMergeMode.setFont(new Font("Lucida Grande", Font.PLAIN, 12));
		
		JLabel lblProtocol = new JLabel("Protocol");
		lblProtocol.setFont(new Font("Lucida Grande", Font.PLAIN, 12));
		
		JLabel lblDestinationIP = new JLabel("Destination IP Address");
		lblDestinationIP.setFont(new Font("Lucida Grande", Font.PLAIN, 12));
		
		lblDirection = new JLabel("Direction");
		lblDirection.setFont(new Font("Lucida Grande", Font.PLAIN, 12));
										
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(lblUniversePortA, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblUniversePortB, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblUniversePortC, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblUniversePortD, GroupLayout.PREFERRED_SIZE, 40, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(formattedTextFieldUniverseD, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedTextFieldUniverseB, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedTextFieldUniverseC, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedTextFieldUniverseA, GroupLayout.PREFERRED_SIZE, 39, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(comboBoxDirectionPortD, GroupLayout.PREFERRED_SIZE, 100, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortC, GroupLayout.PREFERRED_SIZE, 100, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortB, GroupLayout.PREFERRED_SIZE, 100, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortA, GroupLayout.PREFERRED_SIZE, 100, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblDirection))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(lblMergeMode)
								.addComponent(comboBoxMergePortA, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortB, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortC, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortD, GroupLayout.PREFERRED_SIZE, 81, GroupLayout.PREFERRED_SIZE))
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addGap(7)
									.addComponent(comboBoxProtocolPortA, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedIP1PortA, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedIP2PortA, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedIP3PortA, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(formattedIP4PortA, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(chckbxEnablePortA, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE))
								.addGroup(gl_contentPanel.createSequentialGroup()
									.addPreferredGap(ComponentPlacement.RELATED)
									.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
										.addGroup(gl_contentPanel.createSequentialGroup()
											.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
												.addGroup(gl_contentPanel.createSequentialGroup()
													.addGap(12)
													.addComponent(lblProtocol, GroupLayout.PREFERRED_SIZE, 67, GroupLayout.PREFERRED_SIZE))
												.addComponent(comboBoxProtocolPortD, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE))
											.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
												.addGroup(gl_contentPanel.createSequentialGroup()
													.addGap(6)
													.addComponent(formattedIP1PortD, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedIP2PortD, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedIP3PortD, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(formattedIP4PortD, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
													.addPreferredGap(ComponentPlacement.RELATED)
													.addComponent(chckbxEnablePortD, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE))
												.addGroup(gl_contentPanel.createSequentialGroup()
													.addGap(15)
													.addComponent(lblDestinationIP, GroupLayout.PREFERRED_SIZE, 153, GroupLayout.PREFERRED_SIZE))))
										.addGroup(gl_contentPanel.createSequentialGroup()
											.addComponent(comboBoxProtocolPortC, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP1PortC, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP2PortC, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP3PortC, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP4PortC, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(chckbxEnablePortC, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE))
										.addGroup(gl_contentPanel.createSequentialGroup()
											.addComponent(comboBoxProtocolPortB, GroupLayout.PREFERRED_SIZE, 105, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP1PortB, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP2PortB, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP3PortB, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(formattedIP4PortB, GroupLayout.PREFERRED_SIZE, 34, GroupLayout.PREFERRED_SIZE)
											.addPreferredGap(ComponentPlacement.RELATED)
											.addComponent(chckbxEnablePortB, GroupLayout.PREFERRED_SIZE, 73, GroupLayout.PREFERRED_SIZE)))))
							.addGap(210))
						.addComponent(chckbxMapUniverse0))
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblProtocol, GroupLayout.PREFERRED_SIZE, 15, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblDestinationIP, GroupLayout.PREFERRED_SIZE, 15, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED, 12, Short.MAX_VALUE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblMergeMode)
								.addComponent(lblDirection, GroupLayout.PREFERRED_SIZE, 15, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.UNRELATED)))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
							.addComponent(lblUniversePortA)
							.addComponent(formattedTextFieldUniverseA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(comboBoxDirectionPortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(comboBoxMergePortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(comboBoxProtocolPortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(formattedIP1PortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(formattedIP2PortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(formattedIP3PortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addComponent(formattedIP4PortA, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(2)
							.addComponent(chckbxEnablePortA)))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblUniversePortB)
								.addComponent(formattedTextFieldUniverseB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxProtocolPortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP1PortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP2PortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP3PortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP4PortB, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(8)
							.addComponent(chckbxEnablePortB)))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblUniversePortC)
								.addComponent(formattedTextFieldUniverseC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxProtocolPortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP1PortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP2PortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP3PortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP4PortC, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(8)
							.addComponent(chckbxEnablePortC)))
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblUniversePortD)
								.addComponent(formattedTextFieldUniverseD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxDirectionPortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxMergePortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(comboBoxProtocolPortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP1PortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP2PortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP3PortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(formattedIP4PortD, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(chckbxMapUniverse0))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(8)
							.addComponent(chckbxEnablePortD)))
					.addGap(11))
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
		
		formattedTextFieldUniverseB.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				try {
					formattedTextFieldUniverseB.commitEdit();
					artnetAdressPortA = setNetSubNet((int)formattedTextFieldUniverseB.getValue());
					update('A');
				} catch (ParseException e1) {
					formattedTextFieldUniverseB.setValue(getUniverseFromAddress(artnetAdressPortA));
				}
			}
		});
		
		formattedTextFieldUniverseA.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				try {
					formattedTextFieldUniverseA.commitEdit();
					artnetAdressPortB = setNetSubNet((int)formattedTextFieldUniverseA.getValue());
					update('B');
				} catch (ParseException e1) {
					formattedTextFieldUniverseA.setValue(getUniverseFromAddress(artnetAdressPortB));
				}
			}
		});
		
		formattedTextFieldUniverseC.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				try {
					formattedTextFieldUniverseC.commitEdit();
					artnetAdressPortC = setNetSubNet((int)formattedTextFieldUniverseC.getValue());
					update('C');
				} catch (ParseException e1) {
					formattedTextFieldUniverseC.setValue(getUniverseFromAddress(artnetAdressPortC));
				}
			}
		});
		
		formattedTextFieldUniverseD.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				try {
					formattedTextFieldUniverseD.commitEdit();
					artnetAdressPortD = setNetSubNet((int)formattedTextFieldUniverseD.getValue());
					update('D');
				} catch (ParseException e1) {
					formattedTextFieldUniverseD.setValue(getUniverseFromAddress(artnetAdressPortD));
				}
			}
		});
		
		comboBoxDirectionPortA.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				final Boolean isSelected = (comboBoxDirectionPortA.getSelectedIndex() == 1);
				comboBoxMergePortA.setEnabled(!isSelected);
				comboBoxProtocolPortA.setEnabled(!isSelected);
				//
				formattedIP1PortA.setEnabled(isSelected);
				formattedIP2PortA.setEnabled(isSelected);
				formattedIP3PortA.setEnabled(isSelected);
				formattedIP4PortA.setEnabled(isSelected);
			}
		});
		
		comboBoxDirectionPortB.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				final Boolean isSelected = (comboBoxDirectionPortB.getSelectedIndex() == 1);
				comboBoxMergePortB.setEnabled(!isSelected);
				comboBoxProtocolPortB.setEnabled(!isSelected);
				//
				formattedIP1PortB.setEnabled(isSelected);
				formattedIP2PortB.setEnabled(isSelected);
				formattedIP3PortB.setEnabled(isSelected);
				formattedIP4PortB.setEnabled(isSelected);
			}
		});
		
		comboBoxDirectionPortC.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				final Boolean isSelected = (comboBoxDirectionPortC.getSelectedIndex() == 1);
				comboBoxMergePortC.setEnabled(!isSelected);
				comboBoxProtocolPortC.setEnabled(!isSelected);
				//
				formattedIP1PortC.setEnabled(isSelected);
				formattedIP2PortC.setEnabled(isSelected);
				formattedIP3PortC.setEnabled(isSelected);
				formattedIP4PortC.setEnabled(isSelected);
			}
		});
		
		comboBoxDirectionPortD.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				final Boolean isSelected = (comboBoxDirectionPortD.getSelectedIndex() == 1);
				comboBoxMergePortD.setEnabled(!isSelected);
				comboBoxProtocolPortD.setEnabled(!isSelected);
				//
				formattedIP1PortD.setEnabled(isSelected);
				formattedIP2PortD.setEnabled(isSelected);
				formattedIP3PortD.setEnabled(isSelected);
				formattedIP4PortD.setEnabled(isSelected);
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
					if (line.contains("direction_port_a")) {
						if (Properties.getString(line).equals("input")) {
							comboBoxDirectionPortA.setSelectedIndex(1);
						} else {
							comboBoxDirectionPortA.setSelectedIndex(0);
						}
						continue;
					}
					if (line.contains("direction_port_b")) {
						if (Properties.getString(line).equals("input")) {
							comboBoxDirectionPortB.setSelectedIndex(1);
						} else {
							comboBoxDirectionPortB.setSelectedIndex(0);
						}
						continue;
					}
					if (line.contains("direction_port_c")) {
						if (Properties.getString(line).equals("input")) {
							comboBoxDirectionPortC.setSelectedIndex(1);
						} else {
							comboBoxDirectionPortC.setSelectedIndex(0);
						}
						continue;
					}
					if (line.contains("direction_port_d")) {
						if (Properties.getString(line).equals("input")) {
							comboBoxDirectionPortD.setSelectedIndex(1);
						} else {
							comboBoxDirectionPortD.setSelectedIndex(0);
						}
						continue;
					}
					//
					if ((line.contains("net") && (!line.contains("subnet")) && (!line.contains("artnet") && (!line.contains("network"))))) {
						artnetNet = Properties.getInt(line);
						System.out.println("artnetNet->" + artnetNet);
						continue;
					}
					if (line.contains("subnet")) {
						artnetSubnet = Properties.getInt(line);
						System.out.println("artnetSubnet->" + artnetSubnet);
						continue;
					}
					if (line.contains("universe_port_a")) {
						artnetAdressPortA = Properties.getInt(line);
						formattedTextFieldUniverseA.setValue(getUniverseFromAddress(artnetAdressPortA));
						chckbxEnablePortA.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_b")) {
						artnetAdressPortB = Properties.getInt(line);
						formattedTextFieldUniverseB.setValue(getUniverseFromAddress(artnetAdressPortB));
						chckbxEnablePortB.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_c")) {
						artnetAdressPortC = Properties.getInt(line);
						formattedTextFieldUniverseC.setValue(getUniverseFromAddress(artnetAdressPortC));
						chckbxEnablePortC.setSelected(!line.startsWith("#"));
						continue;
					}
					if (line.contains("universe_port_d")) {
						artnetAdressPortD = Properties.getInt(line);
						formattedTextFieldUniverseD.setValue(getUniverseFromAddress(artnetAdressPortD));
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
					//
					if (line.contains("protocol_port_a")) {
						comboBoxProtocolPortA.setSelectedIndex(Properties.getString(line).equals("sacn") ? 1 : 0);
						continue;
					}
					if (line.contains("protocol_port_b")) {
						comboBoxProtocolPortB.setSelectedIndex(Properties.getString(line).equals("sacn") ? 1 : 0);
						continue;
					}
					if (line.contains("protocol_port_c")) {
						comboBoxProtocolPortC.setSelectedIndex(Properties.getString(line).equals("sacn") ? 1 : 0);
						continue;
					}
					if (line.contains("protocol_port_d")) {
						comboBoxProtocolPortD.setSelectedIndex(Properties.getString(line).equals("sacn") ? 1 : 0);
						continue;
					}
					// Input
					if (line.contains("destination_ip_port_a")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedIP1PortA.setValue(Integer.parseInt(parts[0]));
							formattedIP2PortA.setValue(Integer.parseInt(parts[1]));
							formattedIP3PortA.setValue(Integer.parseInt(parts[2]));
							formattedIP4PortA.setValue(Integer.parseInt(parts[3]));
						}
					}
					if (line.contains("destination_ip_port_b")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedIP1PortB.setValue(Integer.parseInt(parts[0]));
							formattedIP2PortB.setValue(Integer.parseInt(parts[1]));
							formattedIP3PortB.setValue(Integer.parseInt(parts[2]));
							formattedIP4PortB.setValue(Integer.parseInt(parts[3]));
						}
					}
					if (line.contains("destination_ip_port_c")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedIP1PortC.setValue(Integer.parseInt(parts[0]));
							formattedIP2PortC.setValue(Integer.parseInt(parts[1]));
							formattedIP3PortC.setValue(Integer.parseInt(parts[2]));
							formattedIP4PortC.setValue(Integer.parseInt(parts[3]));
						}
					}
					if (line.contains("destination_ip_port_d")) {
						final String value = Properties.getString(line).replace('.', '\n');
						final String parts[] = value.split("\n");
						if (parts.length == 4) {
							formattedIP1PortD.setValue(Integer.parseInt(parts[0]));
							formattedIP2PortD.setValue(Integer.parseInt(parts[1]));
							formattedIP3PortD.setValue(Integer.parseInt(parts[2]));
							formattedIP4PortD.setValue(Integer.parseInt(parts[3]));
						}
					}
					//
					if (line.contains("map_universe0")) {
						chckbxMapUniverse0.setSelected(Properties.getBool(line));
						continue;
					}
				}
			}
		}
	}
	
	private int setNetSubNet(int universe) {
		artnetNet = (universe >> 8) & 0x7F;
		artnetSubnet  = (universe >> 4) & 0x0F;
		final int artnetAddress = universe & 0x0F;
		System.out.println("[" + universe + "] " + artnetNet + ":" + artnetSubnet + ":" + artnetAddress);
		return artnetAddress;
	}
	
	private int getUniverseFromAddress(int address) {
		int universe = (artnetNet & 0x7F) << 8;		// Net : Bits 14-8
		universe |= ((artnetSubnet & 0x0F) << 4);	// Sub-Net : Bits 7-4
		universe |= address & 0x0F;	
		
		System.out.println("" + artnetNet + ":" +  artnetSubnet + "->" + universe);
		
		return universe;
	}
	
	private String getComment(JCheckBox checkBox) {
		return checkBox.isSelected() ? "" : "#";
	}
	
	private String getComment(JComboBox<String> comboBox) {
		return comboBox.getSelectedItem().toString() == "Input" ? "" : "#";
	}
	
	private void update(char port) {
		if (port != 'A')
			formattedTextFieldUniverseB.setValue(getUniverseFromAddress(artnetAdressPortA));
		if (port != 'B')
			formattedTextFieldUniverseA.setValue(getUniverseFromAddress(artnetAdressPortB));
		if (port != 'C')
			formattedTextFieldUniverseC.setValue(getUniverseFromAddress(artnetAdressPortC));
		if (port != 'D')
			formattedTextFieldUniverseD.setValue(getUniverseFromAddress(artnetAdressPortD));
	}
	
	private String getDirection(JComboBox<String> comboBox) {
		if (comboBox.getSelectedItem().toString().toLowerCase().equals("input")) {
			return "input";
		}
		return "output";
	}
	
	private String getProtocol(JComboBox<String> comboBox) {
		if (comboBox.getSelectedItem().toString().toLowerCase().equals("sacn")) {
			return "sacn";
		}
		return "artnet";
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
						
			txtAppend.append(String.format("net=%d\n", artnetNet));
			txtAppend.append(String.format("subnet=%d\n", artnetSubnet));
			
			txtAppend.append(String.format("direction_port_a=%s\n", getDirection(comboBoxDirectionPortA)));
			txtAppend.append(String.format("direction_port_b=%s\n", getDirection(comboBoxDirectionPortB)));
			txtAppend.append(String.format("direction_port_c=%s\n", getDirection(comboBoxDirectionPortC)));
			txtAppend.append(String.format("direction_port_d=%s\n", getDirection(comboBoxDirectionPortD)));
			
			txtAppend.append(String.format("%suniverse_port_a=%s\n", getComment(chckbxEnablePortA), artnetAdressPortA));
			txtAppend.append(String.format("%suniverse_port_b=%s\n", getComment(chckbxEnablePortB), artnetAdressPortB));
			txtAppend.append(String.format("%suniverse_port_c=%s\n", getComment(chckbxEnablePortC), artnetAdressPortC));
			txtAppend.append(String.format("%suniverse_port_d=%s\n", getComment(chckbxEnablePortD), artnetAdressPortD));
			
			// Output
			
			txtAppend.append(String.format("protocol_port_a=%s\n", getProtocol(comboBoxProtocolPortA)));
			txtAppend.append(String.format("protocol_port_b=%s\n", getProtocol(comboBoxProtocolPortB)));
			txtAppend.append(String.format("protocol_port_c=%s\n", getProtocol(comboBoxProtocolPortC)));
			txtAppend.append(String.format("protocol_port_d=%s\n", getProtocol(comboBoxProtocolPortD)));

			txtAppend.append(String.format("merge_mode_port_a=%s\n", getMergeMode(comboBoxMergePortA)));
			txtAppend.append(String.format("merge_mode_port_b=%s\n", getMergeMode(comboBoxMergePortB)));
			txtAppend.append(String.format("merge_mode_port_c=%s\n", getMergeMode(comboBoxMergePortC)));
			txtAppend.append(String.format("merge_mode_port_d=%s\n", getMergeMode(comboBoxMergePortD)));
				
			// Input
			txtAppend.append(String.format("%sdestination_ip_port_a=%d.%d.%d.%d\n", getComment(comboBoxDirectionPortA), formattedIP1PortA.getValue(),formattedIP2PortA.getValue(),formattedIP3PortA.getValue(),formattedIP4PortA.getValue()));
			txtAppend.append(String.format("%sdestination_ip_port_b=%d.%d.%d.%d\n", getComment(comboBoxDirectionPortB), formattedIP1PortB.getValue(),formattedIP2PortB.getValue(),formattedIP3PortB.getValue(),formattedIP4PortB.getValue()));
			txtAppend.append(String.format("%sdestination_ip_port_c=%d.%d.%d.%d\n", getComment(comboBoxDirectionPortC), formattedIP1PortC.getValue(),formattedIP2PortC.getValue(),formattedIP3PortC.getValue(),formattedIP4PortC.getValue()));
			txtAppend.append(String.format("%sdestination_ip_port_d=%d.%d.%d.%d\n", getComment(comboBoxDirectionPortD), formattedIP1PortD.getValue(),formattedIP2PortD.getValue(),formattedIP3PortD.getValue(),formattedIP4PortD.getValue()));
			
			//
			txtAppend.append(String.format("map_universe0=%d\n", chckbxMapUniverse0.isSelected() ? 1 : 0));
			
			String txt = Properties.removeComments(opi.getTxt(TXT_FILE));
			txt = txt.replaceAll("direction", "#direction");
			txt = txt.replaceAll("universe_port_", "#universe_port_");
			txt = txt.replaceAll("destination_ip_port", "#destination_ip_port");
					
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
