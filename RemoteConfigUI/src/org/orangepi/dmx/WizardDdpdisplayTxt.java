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
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.text.NumberFormatter;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

public class WizardDdpdisplayTxt extends JDialog {
	//
	private static final String TXT_FILE = "ddpdisp.txt";
	//
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	private JButton btnSave;
	private JButton btnCancel;
	private JButton btnSetDefaults;
	//
	private JComboBox<String> comboBoxMapping;
	private JComboBox<String> comboBoxType;
	private JFormattedTextField formattedTextFieldPort1;
	private JFormattedTextField formattedTextFieldPort2;
	private JFormattedTextField formattedTextFieldPort3;
	private JFormattedTextField formattedTextFieldPort4;
	private JFormattedTextField formattedTextFieldPort5;
	private JFormattedTextField formattedTextFieldPort6;
	private JFormattedTextField formattedTextFieldPort7;
	private JFormattedTextField formattedTextFieldPort8;
	private NumberFormatter formatterCount;
	private int ledsPerPixel = 3;
	private JLabel lblChannelsPort1;
	private JLabel lblChannelsPort2;
	private JLabel lblChannelsPort3;
	private JLabel lblChannelsPort4;
	private JLabel lblChannelsPort5;
	private JLabel lblChannelsPort6;
	private JLabel lblChannelsPort7;
	private JLabel lblChannelsPort8;

	public WizardDdpdisplayTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;
		
		setTitle(nodeId);
		
		InitComponents();
		CreateEvents();
		
		load();
	}
	
	private void InitComponents() {
		setBounds(100, 100, 450, 374);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.WEST);
		{
			comboBoxType = new JComboBox<String>();
			comboBoxType.setModel(new DefaultComboBoxModel<String>(new String[] {"WS2801", "WS2811", "WS2812", "WS2812B", "WS2813", "WS2815", "SK6812", "SK6812W", "UCS1903", "UCS2903", "CS8812", "APA102", "\tSK9822", "\tP9813"}));
			comboBoxType.setSelectedIndex(3);
		}
		{
			comboBoxMapping = new JComboBox<String>();
			comboBoxMapping.setModel(new DefaultComboBoxModel<String>(new String[] {"Default", "RGB", "RBG", "GRB", "GBR", "BRG", "BGR"}));
		}
		
		JLabel lblMapping = new JLabel("Mapping");
		JLabel lblPort1 = new JLabel("Port 1");
		JLabel lblPort2 = new JLabel("Port 2");
		JLabel lblPort3 = new JLabel("Port 3");
		JLabel lblPort4 = new JLabel("Port 4");
		JLabel lblPort5 = new JLabel("Port 5");
		JLabel lblPort6 = new JLabel("Port 6");
		JLabel lblPort7 = new JLabel("Port 7");
		JLabel lblPort8 = new JLabel("Port 8");
		
	    NumberFormat format = NumberFormat.getInstance();
	    formatterCount = new NumberFormatter(format);
	    formatterCount.setValueClass(Integer.class);
	    formatterCount.setMinimum(0);
	    formatterCount.setMaximum(4 * (512 / ledsPerPixel));
	    formatterCount.setAllowsInvalid(false);
	    formatterCount.setCommitsOnValidEdit(true);
		
		formattedTextFieldPort1 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort1.setText("0");
		formattedTextFieldPort1.setColumns(3);
		
		formattedTextFieldPort2 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort2.setText("0");
		formattedTextFieldPort2.setColumns(3);
		
		formattedTextFieldPort3 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort3.setText("0");
		formattedTextFieldPort3.setColumns(3);
		
		formattedTextFieldPort4 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort4.setText("0");
		formattedTextFieldPort4.setColumns(3);
		
		formattedTextFieldPort5 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort5.setText("0");
		formattedTextFieldPort5.setColumns(3);
		
		formattedTextFieldPort6 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort6.setText("0");
		formattedTextFieldPort6.setColumns(3);
		
		formattedTextFieldPort7 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort7.setText("0");
		formattedTextFieldPort7.setColumns(3);
		
		formattedTextFieldPort8 = new JFormattedTextField(formatterCount);
		formattedTextFieldPort8.setText("0");
		formattedTextFieldPort8.setColumns(3);
		
		lblChannelsPort1 = new JLabel();
		
		lblChannelsPort2 = new JLabel();
		lblChannelsPort2.setText("0");
		
		lblChannelsPort3 = new JLabel();
		lblChannelsPort3.setText("0");
		
		lblChannelsPort4 = new JLabel();
		lblChannelsPort4.setText("0");
		
		lblChannelsPort5 = new JLabel();
		lblChannelsPort5.setText("0");
		
		lblChannelsPort6 = new JLabel();
		lblChannelsPort6.setText("0");
		
		lblChannelsPort7 = new JLabel();
		lblChannelsPort7.setText("0");
		
		lblChannelsPort8 = new JLabel();
		lblChannelsPort8.setText("0");
		
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(comboBoxType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(lblMapping)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(comboBoxMapping, GroupLayout.PREFERRED_SIZE, 110, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort1)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort1, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort2, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort2, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort2, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort3, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort3, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort3, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort4, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort4, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort4, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort5, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort5, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort5, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort6, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort6, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort6, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort7, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort7, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort7, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lblPort8, GroupLayout.PREFERRED_SIZE, 37, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldPort8, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblChannelsPort8, GroupLayout.PREFERRED_SIZE, 43, GroupLayout.PREFERRED_SIZE)))
					.addContainerGap(95, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(comboBoxType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(comboBoxMapping, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblMapping))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort1)
						.addComponent(formattedTextFieldPort1, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort1))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort2)
						.addComponent(formattedTextFieldPort2, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort2))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort3)
						.addComponent(formattedTextFieldPort3, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort3))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort4)
						.addComponent(formattedTextFieldPort4, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort4))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort5)
						.addComponent(formattedTextFieldPort5, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort5))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort6)
						.addComponent(formattedTextFieldPort6, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort6))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort7)
						.addComponent(formattedTextFieldPort7, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort7))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPort8)
						.addComponent(formattedTextFieldPort8, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblChannelsPort8))
					.addContainerGap(8, Short.MAX_VALUE))
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
	
	private void CreateEvents() {
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
		
		btnCancel.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
			}
		});
		
		comboBoxType.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				System.out.println(comboBoxType.getSelectedItem().toString());
				if (comboBoxType.getSelectedItem().toString().equals("SK6812W")) {
					ledsPerPixel = 4;
					comboBoxMapping.setEditable(false);
					comboBoxMapping.setEnabled(false);
					comboBoxMapping.setSelectedIndex(0);
				} else {
					ledsPerPixel = 3;
					comboBoxMapping.setEditable(true);
					comboBoxMapping.setEnabled(true);    
				}
				formatterCount.setMaximum(4 * (512 / ledsPerPixel));
				update();
			}
		});
		
		formattedTextFieldPort1.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort1.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort1.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort2.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort2.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort2.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort3.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort3.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort3.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort4.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort4.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort4.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort5.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort5.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort5.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort6.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort6.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort6.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort7.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort7.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort7.getText()) * ledsPerPixel));
			}
		});
		
		formattedTextFieldPort8.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				lblChannelsPort8.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort8.getText()) * ledsPerPixel));
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
					if (line.contains("led_type")) {
						comboBoxType.setSelectedItem(Properties.getString(line));
					}
					if (line.contains("led_rgb_mapping")) {
						if (line.startsWith("#")) {
							comboBoxMapping.setSelectedIndex(0);
						} else {
							comboBoxMapping.setSelectedItem(Properties.getString(line));
						}
					}
					//
					if (line.contains("count_port_1")) {
						formattedTextFieldPort1.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_2")) {
						formattedTextFieldPort2.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_3")) {
						formattedTextFieldPort3.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_4")) {
						formattedTextFieldPort4.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_5")) {
						formattedTextFieldPort5.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_6")) {
						formattedTextFieldPort6.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_7")) {
						formattedTextFieldPort7.setValue(Properties.getInt(line));
						continue;
					}
					if (line.contains("count_port_8")) {
						formattedTextFieldPort8.setValue(Properties.getInt(line));
						continue;
					}
				}
			}
		}
		
	 update();
	}
	
	private void update() {
		lblChannelsPort1.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort1.getText()) * ledsPerPixel));
		lblChannelsPort2.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort2.getText()) * ledsPerPixel));
		lblChannelsPort3.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort3.getText()) * ledsPerPixel));
		lblChannelsPort4.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort4.getText()) * ledsPerPixel));
		lblChannelsPort5.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort5.getText()) * ledsPerPixel));
		lblChannelsPort6.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort6.getText()) * ledsPerPixel));
		lblChannelsPort7.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort7.getText()) * ledsPerPixel));
		lblChannelsPort8.setText(String.valueOf(Integer.parseInt(formattedTextFieldPort8.getText()) * ledsPerPixel));
	}

	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			//
			txtAppend.append(String.format("count_port_1=%s\n", formattedTextFieldPort1.getText()));
			txtAppend.append(String.format("count_port_2=%s\n", formattedTextFieldPort2.getText()));
			txtAppend.append(String.format("count_port_3=%s\n", formattedTextFieldPort3.getText()));
			txtAppend.append(String.format("count_port_4=%s\n", formattedTextFieldPort4.getText()));
			txtAppend.append(String.format("count_port_5=%s\n", formattedTextFieldPort5.getText()));
			txtAppend.append(String.format("count_port_6=%s\n", formattedTextFieldPort6.getText()));
			txtAppend.append(String.format("count_port_7=%s\n", formattedTextFieldPort7.getText()));
			txtAppend.append(String.format("count_port_8=%s\n", formattedTextFieldPort8.getText()));
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
