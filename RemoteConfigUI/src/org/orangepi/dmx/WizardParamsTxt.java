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

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.text.NumberFormatter;
import javax.swing.JLabel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JFormattedTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import java.awt.Color;

public class WizardParamsTxt extends JDialog {
	//
	private static final String TXT_FILE = "params.txt";
	//
	private static final long serialVersionUID = 1L;
	private final JPanel contentPanel = new JPanel();
	//
	OrangePi opi = null;
	RemoteConfig remoteConfig = null;
	//
	private JButton btnSetDefaults;
	private JButton btnSave;
	private JButton btnCancel;
	//
	private NumberFormatter formatterBreakTime;
	private NumberFormatter formatterRefreshRate;
	private JFormattedTextField formattedTextFieldBreakTime;
	private JFormattedTextField formattedTextFieldMaBTime;
	private JFormattedTextField formattedTextFieldRefreshRate;

	public static void main(String[] args) {
		try {
			WizardParamsTxt dialog = new WizardParamsTxt("Title", null, null);
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public WizardParamsTxt(String nodeId, OrangePi opi, RemoteConfig remoteConfig) {
		this.opi = opi;
		this.remoteConfig = remoteConfig;
		
		setTitle(nodeId);
		
		InitComponents();
		CreateEvents();
		
		load();
	}
	
	private void InitComponents() {
		setBounds(100, 100, 294, 199);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		JLabel lblBreakTime = new JLabel("BREAK time");
		JLabel lblMabTime = new JLabel("MAB time");
		JLabel lbRefreshRate = new JLabel("Refresh rate");
		
	    NumberFormat format = NumberFormat.getInstance();
	    formatterBreakTime = new NumberFormatter(format);
	    formatterBreakTime.setValueClass(Integer.class);
	    formatterBreakTime.setMinimum(92);
	    formatterBreakTime.setMaximum(176);
	    formatterBreakTime.setAllowsInvalid(false);
	    formatterBreakTime.setCommitsOnValidEdit(true);
		
		formattedTextFieldBreakTime = new JFormattedTextField(formatterBreakTime);
		formattedTextFieldBreakTime.setColumns(4);
		formattedTextFieldBreakTime.setText("176");
		
		formattedTextFieldMaBTime = new JFormattedTextField();
		formattedTextFieldMaBTime.setText("12");
		formattedTextFieldMaBTime.setColumns(4);
			
	    formatterRefreshRate = new NumberFormatter(format);
	    formatterRefreshRate.setValueClass(Integer.class);
	    formatterRefreshRate.setMinimum(0);
	    formatterRefreshRate.setMaximum(44);
	    formatterRefreshRate.setAllowsInvalid(false);
	    formatterRefreshRate.setCommitsOnValidEdit(true);
		
		formattedTextFieldRefreshRate = new JFormattedTextField(formatterRefreshRate);
		formattedTextFieldRefreshRate.setColumns(4);
		formattedTextFieldRefreshRate.setText("40");
		
		JLabel lblNewLabel = new JLabel("0 = as fast as possible");
		lblNewLabel.setForeground(Color.GRAY);
		
		JLabel lblNewLabel_1 = new JLabel("μs");
		
		JLabel lblNewLabel_1_1 = new JLabel("μs");
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(lbRefreshRate)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(formattedTextFieldRefreshRate, GroupLayout.PREFERRED_SIZE, 35, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(lblNewLabel))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(lblBreakTime)
								.addComponent(lblMabTime))
							.addGap(18)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING, false)
								.addComponent(formattedTextFieldMaBTime)
								.addComponent(formattedTextFieldBreakTime, GroupLayout.DEFAULT_SIZE, 30, Short.MAX_VALUE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(lblNewLabel_1_1, GroupLayout.PREFERRED_SIZE, 15, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblNewLabel_1))))
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblBreakTime)
						.addComponent(formattedTextFieldBreakTime, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblNewLabel_1))
					.addGap(18)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblMabTime)
						.addComponent(formattedTextFieldMaBTime, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblNewLabel_1_1))
					.addGap(18)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(lbRefreshRate)
						.addComponent(formattedTextFieldRefreshRate, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblNewLabel))
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
	}
	
	private void load() {
		if (opi != null) {
			final String txt = opi.getTxt(TXT_FILE);
			if (txt != null) {
				final String[] lines = txt.split("\n");
				for (int i = 0; i < lines.length; i++) {
					final String line = lines[i];
					if (line.contains("break_time")) {
						formattedTextFieldBreakTime.setText(Properties.getString(line));
					}
					if (line.contains("mab_time")) {
						formattedTextFieldMaBTime.setText(Properties.getString(line));
					}
					if (line.contains("refresh_rate")) {
						formattedTextFieldRefreshRate.setText(Properties.getString(line));
					}
				}
			}
		}
	}
	
	private void save() {
		if (opi != null) {
			StringBuffer txtAppend = new StringBuffer();
			//
			txtAppend.append(String.format("break_time=%s\n", formattedTextFieldBreakTime.getText()));
			txtAppend.append(String.format("mab_time=%s\n", formattedTextFieldMaBTime.getText()));
			txtAppend.append(String.format("refresh_rate=%s\n", formattedTextFieldRefreshRate.getText()));
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
