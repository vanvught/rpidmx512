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
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.Collection;
import java.util.TreeMap;

import javax.swing.ButtonGroup;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.JCheckBox;

public class PixelTestPattern extends JDialog {
	private static final long serialVersionUID = 1L;
	private RemoteConfig remoteConfig;
	private static final String patterns[] = {"None (Data)", "Rainbow", "Theater chase", "Colour wipe", "Scanner", "Fade"};
	//
	private final JPanel contentPanel = new JPanel();
	private JButton btnRefresh;
	private final ButtonGroup buttonGroup = new ButtonGroup();
	private JTextArea textArea;
	private JButton btnApply;
	private JRadioButton rdbtnNone_0;
	private JRadioButton rdbtnRainbow_1;
	private JRadioButton rdbtnTheaterChase_2;
	private JRadioButton rdbtnColourWipe_3;
	private JRadioButton rdbtnScanner_4;
	private JRadioButton rdbtnFade_5;
	private JCheckBox chckbxReboot;

	public PixelTestPattern(RemoteConfig remoteConfig) {
		this.remoteConfig = remoteConfig;
		
		setTitle("Pixel Controller Test Pattern Selection");
				
		InitComponents();
		CreateEvents();
		
		Display();
	}
	
	private void InitComponents() {
		setBounds(100, 100, 535, 335);

		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		btnRefresh = new JButton("Refresh");
		
		JScrollPane scrollPane = new JScrollPane();
		
		btnApply = new JButton("Apply");
		
		rdbtnNone_0 = new JRadioButton("None (Data)");
		rdbtnNone_0.setSelected(true);
		buttonGroup.add(rdbtnNone_0);
		
		rdbtnRainbow_1 = new JRadioButton("Rainbow");
		buttonGroup.add(rdbtnRainbow_1);
		
		rdbtnTheaterChase_2 = new JRadioButton("Theater chase");
		buttonGroup.add(rdbtnTheaterChase_2);
		
		rdbtnColourWipe_3 = new JRadioButton("Colour wipe");
		buttonGroup.add(rdbtnColourWipe_3);
		
		rdbtnScanner_4 = new JRadioButton("Scanner");
		buttonGroup.add(rdbtnScanner_4);
		
		rdbtnFade_5 = new JRadioButton("Fade");
		buttonGroup.add(rdbtnFade_5);
		
		chckbxReboot = new JCheckBox("Reboot");
		chckbxReboot.setSelected(true);

		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(rdbtnNone_0)
								.addComponent(rdbtnRainbow_1)
								.addComponent(rdbtnTheaterChase_2)
								.addComponent(rdbtnColourWipe_3)
								.addComponent(rdbtnScanner_4)
								.addComponent(rdbtnFade_5)
								.addComponent(btnApply)))
						.addComponent(btnRefresh)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addComponent(chckbxReboot)))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addComponent(btnRefresh)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(rdbtnNone_0)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtnRainbow_1)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtnTheaterChase_2)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtnColourWipe_3)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtnScanner_4)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtnFade_5)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnApply)
							.addPreferredGap(ComponentPlacement.RELATED, 18, Short.MAX_VALUE)
							.addComponent(chckbxReboot))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGap(6)
							.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 285, Short.MAX_VALUE)))
					.addContainerGap())
		);
		
		textArea = new JTextArea();
		textArea.setEditable(false);
		scrollPane.setViewportView(textArea);
		contentPanel.setLayout(gl_contentPanel);
	}
	
	private void CreateEvents() {
		btnRefresh.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				remoteConfig.constructTree();
				Display();
			}
		});
		
		btnApply.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				ProcessApply();
				Display();
			}
		});
	}

	private void Display() {	
		TreeMap<Integer, OrangePi> treeMap = remoteConfig.getTreeMap();
		
		if (!treeMap.isEmpty()) {
			textArea.setText("");

			final Collection<OrangePi> entries = treeMap.values();
			 
			for (OrangePi entry : entries) {		
				final String id = entry.getNodeId();
				
				System.out.println("id=" + id);
				
				String txtFile = "devices.txt";
				
				if (id.toLowerCase().contains("ddp")) {
					txtFile = "ddpdisp.txt";
				}
				
				if (id.toLowerCase().contains("pixel")) {
					textArea.append(id + " - " + handleDevicesTxt(entry, txtFile) + "\n");
				}
			}
		} else {
			textArea.setText("No pixel controllers found");
		}
	}
	
	private String handleDevicesTxt(OrangePi OPi, String txtFile) {
		final String txt = OPi.getTxt(txtFile);
		final String lines[] = txt.split("\n");
		
		for (int i = 0; i < lines.length; i++) {
			if (lines[i].startsWith("#")) {
				continue;
			}
			if (lines[i].toLowerCase().contains("test_pattern")) {
				final int index = lines[i].indexOf("=") + 1;
				if ((index > 0) && (index < lines[i].length())) {
					final String value = lines[i].substring(index);
					return patterns[Integer.parseInt(value)];
				}
			}
		}
		
		return patterns[0];
	}
	
	private void ProcessApply() {
		TreeMap<Integer, OrangePi> treeMap = remoteConfig.getTreeMap();
		
		if ((treeMap == null)||(treeMap.isEmpty())) {
			return;
		}
				
		String test_pattern = "test_pattern=";
		
		if (rdbtnNone_0.isSelected()) {
			test_pattern = test_pattern + "0";
		} else if (rdbtnRainbow_1.isSelected()) {
			test_pattern = test_pattern + "1";
		} else if (rdbtnTheaterChase_2.isSelected()) {
			test_pattern = test_pattern + "2";
		} else if (rdbtnColourWipe_3.isSelected()) {
			test_pattern = test_pattern + "3";
		} else if (rdbtnScanner_4.isSelected()) {
			test_pattern = test_pattern + "4";
		} else if (rdbtnFade_5.isSelected()) {
			test_pattern = test_pattern + "5";
		}  
		
		textArea.setText("");
		
		final Collection<OrangePi> values = treeMap.values();
		 
		for (OrangePi value : values) {
			final String id = value.getNodeId();
			
			System.out.println("id=" + id);
			
			String txtFile = "devices.txt";
			
			if (id.toLowerCase().contains("ddp")) {
				txtFile = "ddpdisp.txt";
			}
		
			if (id.toLowerCase().contains("pixel")) {
				String devicesTxt = value.getTxt(txtFile);
				devicesTxt = devicesTxt + "\n" + test_pattern;
				try {
					value.doSave(devicesTxt);
					textArea.append(value + " - " + handleDevicesTxt(value, txtFile) + "\n");
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		
		if (chckbxReboot.isSelected()) {
			for (OrangePi value : values) {
				final String id = value.getNodeId();
				if (id.toLowerCase().contains("pixel")) {
					try {
						value.doReboot();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
	}
}
