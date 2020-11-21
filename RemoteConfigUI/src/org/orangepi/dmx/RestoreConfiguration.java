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
import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.file.Paths;
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

public class RestoreConfiguration extends JDialog {
	private static final long serialVersionUID = -1394768911279346857L;
	private final JPanel contentPanel = new JPanel();
	private static final String LAST_USED_FOLDER = "org.orangepi.dmx";
	
	private Preferences prefs = Preferences.userRoot().node(getClass().getName());
	private JButton btnRestore;
	private OrangePi node;
	private JTextField textRestoreDirectory;
	private JButton btnDirectory;
	private JTextField textStaticNode;
	private JTextField textNodeInfo;
	private JTextArea textArea;
	private File[] files;

	public static void main(String[] args) {
		try {
			RestoreConfiguration dialog = new RestoreConfiguration();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public RestoreConfiguration() {
		setTitle("Restore");
		InitComponents();
		CreateEvents();
		textRestoreDirectory.setText(prefs.get(LAST_USED_FOLDER, System.getProperty("user.home")));
		displayFiles(prefs.get(LAST_USED_FOLDER, textRestoreDirectory.getText()));
	}
	
	public void doRestoreSelected(OrangePi node) {
		this.node = node;
		
		textNodeInfo.setText(node.getNodeId());
	}
	
	private void displayFiles(String dir) {
		textArea.setText("*.txt :\n");
		
        File f = new File(dir);

        FilenameFilter filter = new FilenameFilter() {
            @Override
            public boolean accept(File f, String name) {
                return name.endsWith(".txt");
            }
        };
        
        files = f.listFiles(filter);
        
        if(files.length == 0) {
        	btnRestore.setEnabled(false);
        	textArea.append("<none>\n");
        	return;
        } 
        
        btnRestore.setEnabled(true);
        
        for (int i = 0; i < files.length; i++) {
        	textArea.append(files[i].getName() + "\n");
            System.out.println(files[i].getName());
        }
	}
	
	private void InitComponents() {
		setBounds(100, 100, 450, 300);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		btnDirectory = new JButton("Directory");

		textRestoreDirectory = new JTextField();
		textRestoreDirectory.setBackground(Color.LIGHT_GRAY);
		textRestoreDirectory.setEditable(false);
		textRestoreDirectory.setColumns(10);
		
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
								.addComponent(textRestoreDirectory, GroupLayout.DEFAULT_SIZE, 326, Short.MAX_VALUE))))
					.addContainerGap())
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnDirectory)
						.addComponent(textRestoreDirectory, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(textStaticNode, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(textNodeInfo, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE))
		);
		
		textArea = new JTextArea();
		scrollPane.setViewportView(textArea);
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				btnRestore = new JButton("Restore");
				btnRestore.setActionCommand("OK");
				buttonPane.add(btnRestore);
				getRootPane().setDefaultButton(btnRestore);
			}
		}
	}
	
	private void CreateEvents() {
		btnDirectory.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser dirChooser = new JFileChooser(prefs.get(LAST_USED_FOLDER, textRestoreDirectory.getText()));
				dirChooser.setDialogTitle("Select a backup directory");
				dirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		          
				if (dirChooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
					final File selectedFile = dirChooser.getSelectedFile();
					System.out.println(selectedFile.getAbsolutePath());
					textRestoreDirectory.setText(selectedFile.getAbsolutePath());
					prefs.put(LAST_USED_FOLDER, selectedFile.getAbsolutePath());
					
					displayFiles(selectedFile.getAbsolutePath());
				}
			}
		});
		
		btnRestore.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				for (int i = 0; i < files.length; i++) {
					  System.out.println(files[i].getAbsolutePath());
					  
					  textArea.append(files[i].getAbsolutePath() + " ");
					 
						try {
							String content = new String(java.nio.file.Files.readAllBytes(Paths.get(files[i].getAbsolutePath())));
							node.doSave(content);
							textArea.append("<saved>\n");
							 System.out.println(content);
						} catch (IOException e1) {
							textArea.append("<failed!>\n");
							e1.printStackTrace();
						}
				}
				btnRestore.setEnabled(false);
			}
		});
	}

}
