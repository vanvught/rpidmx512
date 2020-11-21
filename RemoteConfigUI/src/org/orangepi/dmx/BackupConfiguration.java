/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.HashSet;
import java.util.Iterator;
import java.util.prefs.Preferences;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;

public class BackupConfiguration extends JDialog {
	private static final long serialVersionUID = -464535293787621412L;
	private final JPanel contentPanel = new JPanel();
	private static final String LAST_USED_FOLDER = "org.orangepi.dmx";
	
	private JTextField textBackupDirectory;
	private Preferences prefs = Preferences.userRoot().node(getClass().getName());
	private JButton btnDirectory;
	private JTextField textNodeInfo;
	private JButton btnBackup;
	private JTextField textStaticNode;
	private JTextArea textArea;
	
	HashSet<OrangePi> hashSet;
	
	public static void main(String[] args) {
		try {
			BackupConfiguration dialog = new BackupConfiguration();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public BackupConfiguration() {
		setTitle("Backup");
		InitComponents();
		CreateEvents();
		
		btnBackup.setEnabled(false);
		textBackupDirectory.setText(prefs.get(LAST_USED_FOLDER, System.getProperty("user.home")));
	}
	
	public void doSaveSelected(HashSet<OrangePi> hashSet) {
		this.hashSet = hashSet;
		
		if (hashSet.size() == 1) {
			Iterator<OrangePi> it = hashSet.iterator();		
			textNodeInfo.setText(it.next().getNodeId());
		} else {
			textNodeInfo.setText("<All>");
		}
	
		btnBackup.setEnabled(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 450, 300);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		btnDirectory = new JButton("Directory");
		
		textBackupDirectory = new JTextField();
		textBackupDirectory.setBackground(Color.LIGHT_GRAY);
		textBackupDirectory.setEditable(false);
		textBackupDirectory.setColumns(10);
		
		textNodeInfo = new JTextField();
		textNodeInfo.setBackground(Color.LIGHT_GRAY);
		textNodeInfo.setEditable(false);
		textNodeInfo.setColumns(10);
		
		textStaticNode = new JTextField();
		textStaticNode.setText("Node");
		textStaticNode.setBackground(Color.LIGHT_GRAY);
		textStaticNode.setEditable(false);
		textStaticNode.setColumns(10);
		
		JScrollPane scrollPane = new JScrollPane();
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 428, Short.MAX_VALUE))
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addContainerGap()
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(btnDirectory)
								.addComponent(textStaticNode, GroupLayout.PREFERRED_SIZE, 50, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
								.addComponent(textNodeInfo, GroupLayout.DEFAULT_SIZE, 326, Short.MAX_VALUE)
								.addComponent(textBackupDirectory, GroupLayout.DEFAULT_SIZE, 326, Short.MAX_VALUE))))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addGap(9)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnDirectory)
						.addComponent(textBackupDirectory, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(textStaticNode, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(textNodeInfo, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE))
		);
		
		textArea = new JTextArea();
		textArea.setEditable(false);
		scrollPane.setViewportView(textArea);
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			
			btnBackup = new JButton("Backup");
			buttonPane.add(btnBackup);
		}
	}
	
	private void CreateEvents() {
		btnDirectory.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser dirChooser = new JFileChooser(prefs.get(LAST_USED_FOLDER, textBackupDirectory.getText()));
				dirChooser.setDialogTitle("Select a backup directory");
				dirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		          
				if (dirChooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
					final File selectedFile = dirChooser.getSelectedFile();
					System.out.println(selectedFile.getAbsolutePath());
					textBackupDirectory.setText(selectedFile.getAbsolutePath());
					prefs.put(LAST_USED_FOLDER, selectedFile.getAbsolutePath());
				}
			}
		});
		
		btnBackup.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				DoBackup();
			}
		});
	}
	
	private void doWrite(OrangePi Node, String backupDirectory, String txtFile) {
		String fileName = backupDirectory + File.separator + txtFile;
		System.out.println(fileName);
		
		String txtData = Node.getTxt(txtFile);
		
		if (txtData == null) {
			System.out.println("No data <null>");
			return;
		}
		
		System.out.println(txtData);
		
		if (!txtData.startsWith("#" + txtFile)) {
			System.out.println("No data");
			return;
		}
		
		try {
			BufferedWriter writer = new BufferedWriter(new FileWriter(fileName));
			 writer.write(txtData);
			 writer.close();
			 textArea.append(txtFile + " <written>\n");
		} catch (IOException e) {
			 textArea.append("Backup " + txtFile + " <failed>\n");
			e.printStackTrace();
		}	
	}
	
	private void DoBackup() {
		btnBackup.setEnabled(false);
		
		Iterator<OrangePi> it = hashSet.iterator();
		
		while (it.hasNext()) {
			OrangePi node = it.next();
			textNodeInfo.setText(node.getNodeId());
			String nodeDirectory = textBackupDirectory.getText() + node.getAddress();
			System.out.println(nodeDirectory);
			textArea.append("Directory " + nodeDirectory + " ");

			File file = new File(nodeDirectory);

			if (file.mkdir()) {
				textArea.append("<created>\n");
			} else {
				textArea.append("<already exist>\n");
			}

			DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyyMMdd.HHmmss");
			LocalDateTime now = LocalDateTime.now();
			final String subDirectory = dtf.format(now);
			System.out.println(subDirectory);
			
			String backupDirectory = nodeDirectory + File.separator + subDirectory;
			textArea.append("Directory " + backupDirectory + " ");
			
			file = new File(backupDirectory);

			if (file.mkdir()) {
				textArea.append("<created>\n");
			} else {
				textArea.append("<already exist>\n");
			}
			
			System.out.println(backupDirectory);
					
			final String[] defaultsTxt = {"rconfig.txt", "network.txt"};
		
			for (int i = 0; i < defaultsTxt.length; i++) {
				doWrite(node, backupDirectory, defaultsTxt[i]);	
			}
			
			if (!textNodeInfo.getText().contains("LTC")) {
				doWrite(node, backupDirectory, "display.txt");
			}
			
			if (textNodeInfo.getText().contains("Art-Net")) {
				doWrite(node, backupDirectory, "artnet.txt");
			}
			
			if (textNodeInfo.getText().contains("sACN")) {
				doWrite(node, backupDirectory, "e131.txt");
			}
			
			if (textNodeInfo.getText().contains("DMX")) {
				doWrite(node, backupDirectory, "params.txt");
			}
			
			if (textNodeInfo.getText().contains("RDM")) {
				doWrite(node, backupDirectory, "rdm_device.txt");
			}
			
			if (textNodeInfo.getText().contains("Pixel")) {
				doWrite(node, backupDirectory, "devices.txt");
			}
			
			if (textNodeInfo.getText().contains("Monitor")) {
				doWrite(node, backupDirectory, "mon.txt");
			}
			
			if (textNodeInfo.getText().contains("Serial")) {
				doWrite(node, backupDirectory, "serial.txt");
			}
			
			if (textNodeInfo.getText().contains("OSC Server")) {
				doWrite(node, backupDirectory, "osc.txt");
			}
			
			if (textNodeInfo.getText().contains("OSC Client")) {
				doWrite(node, backupDirectory, "oscclnt.txt");
			}
			
			if (textNodeInfo.getText().contains("Showfile")) {
				doWrite(node, backupDirectory, "show.txt");
			}
					
			if (textNodeInfo.getText().contains("LTC")) {
				doWrite(node, backupDirectory, "ltc.txt");
				doWrite(node, backupDirectory, "ldisplay.txt");
				doWrite(node, backupDirectory, "tcnet.txt");	
				doWrite(node, backupDirectory, "gps.txt");	
			}
		}
		
		if (hashSet.size() != 1) {
			textNodeInfo.setText("<All>");
		}
	}
}
