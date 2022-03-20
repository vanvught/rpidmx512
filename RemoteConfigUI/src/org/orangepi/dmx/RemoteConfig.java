/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

import java.awt.Color;
import java.awt.EventQueue;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.net.BindException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.border.EmptyBorder;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import javax.swing.border.EtchedBorder;

public class RemoteConfig extends JFrame {
	private static final long serialVersionUID = 8836780363465781413L;
	//
	TreeMap<Integer, OrangePi> treeMap = null;
	//
	private final int BUFFERSIZE = 1024;
	private final int PORT = 0x2905;
	private DatagramSocket socketReceive = null;

	private JPanel contentPane;
	private JMenuItem mntmExit;
	private JMenuItem mntmAbout;
	private JTree tree;
	private DefaultTreeModel model;
	private JScrollPane scrollPaneLeft;
	private JMenuItem mntmRefresh;
	private JMenuItem mntmExpandAll;
	private JMenuItem mntmColl;
	private JScrollPane scrollPaneRight;
	private JTextArea textArea;
	private JMenuItem mntmSave;
	private JMenu mnAction;
	private JMenuItem mntmReboot;
	private JLabel lblNodeId;
	private JLabel lblDisplayName;
	private JMenuItem mntmUptime;
	private JMenuItem mntmDisplayOnoff;
	private JMenuItem mntmTftp;
	private JMenu mnRun;
	private JMenuItem mntmTftpClient;
	private JMenuItem mntmVersion;
	private JMenuItem mntmLtcGenerator;
	private JMenuItem mntmRgbDisplay;
	private JMenu mnLTC;
	private JMenuItem mntmSytemTime;
	private JMenuItem mntmTCNet;
	private JMenuItem mntmGlobalControl;
	private JMenu mnWorkflow;
	private JMenuItem mntmFirmwareInstallation;
	private JMenu mnBackup;
	private JMenuItem mntmBackupSelected;
	private JMenuItem mntmBackupAll;

	HashSet<OrangePi> h;
	private JMenuItem mntmRestore;
	private JMenuItem mntmMIDI;
	private JMenuItem mntmPixelTextPatterns;

	private OrangePi opi = null;
	private JMenuItem mntmFactoryDefaults;
	private JMenuItem mntmDmxTransmit;

	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					RemoteConfig frame = new RemoteConfig();
					frame.setVisible(true);
					frame.constructTree();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	public RemoteConfig() {
		System.out.println(System.getProperty("os.name"));
		
		setTitle("Remote Configuration Manager");

		createReceiveSocket();

		InitComponents();
		CreateEvents();
	}

	private void CreateEvents() {
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				doExit();
			}
		});

		mntmExit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doExit();
			}
		});

		mntmExpandAll.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				int row = tree.getRowCount() - 1;
				while (row >= 0) {
					tree.expandRow(row);
					row--;
				}
			}
		});

		mntmColl.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				int row = tree.getRowCount() - 1;
				while (row >= 0) {
					tree.collapseRow(row);
					row--;
				}
			}
		});

		mntmAbout.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				About about = new About();
				String buildNumber = RemoteConfig.class.getPackage().getImplementationVersion();
				about.setBuildNumber(buildNumber);
				about.setVisible(true);
			}
		});

		tree.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				int x = e.getX();
				int y = e.getY();

				TreePath path = tree.getPathForLocation(x, y);

				if (path != null) {
					DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
					opi = (OrangePi) node.getUserObject();

					if (isRightClick(e)) {
						final int cnt = path.getPathCount();
						System.out.println("Right" + cnt);
						if (cnt == 2) {
							doReboot(opi);
						} else if (cnt == 3) {
							doSave(opi);
						}
					} else {
						lblNodeId.setText(opi.getNodeId());
						lblDisplayName.setText(opi.getNodeDisplayName());

						String text = opi.getTxt(path.getLastPathComponent().toString());

						if (text != null) {
							textArea.setText(text);
							textArea.setEditable(true);
						} else {
							textArea.setText("");
							textArea.setEditable(false);
						}
					}
				} else {
					System.err.printf("tree.getPathForLocation(%d, %d)=null\n", x, y);
				}
			}
		});

		textArea.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				if (isRightClick(e)) {
					final String text = textArea.getText();
					if (text.startsWith("#")) {
						final int index = text.indexOf('\n');
						if (index > 1) {
							final String txt = text.substring(1, index);
							if (txt.endsWith(".txt")) {
								if (txt.startsWith("rconfig")) {
									doWizardRemoteConfig(opi);
								}
								if (txt.startsWith("display")) {
									doWizardDisplay(opi);
								}
								if (txt.startsWith("network")) {
									doWizardNetwork(opi);
								}
								if (txt.startsWith("e131")) {
									doWizardE131(opi);
								}
								if (txt.startsWith("artnet")) {
									doWizardArtnet(opi);
								}
								if (txt.startsWith("params")) {
									doWizardDmx(opi);
								}
								if (txt.startsWith("devices")) {
									doWizardDevices(opi);
								}
								if (txt.startsWith("ddpdisp")) {
									doWizardDdpdisplay(opi);
								}
								if (txt.startsWith("gps")) {
									doWizardGps(opi);
								}
								if (txt.startsWith("tcnet")) {
									doWizardTCNet(opi);
								}
							}
						}
					}
				}
			}
		});

		mntmRefresh.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				constructTree();
			}
		});

		mntmReboot.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();
				Boolean bRebooted = false;

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doReboot((OrangePi) node.getUserObject());
						bRebooted = true;
					}
				}

				if (!bRebooted) {
					JOptionPane.showMessageDialog(null, "No node selected for reboot action.");
				}
			}
		});
		
		mntmFactoryDefaults.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doSetFactoryDefaults((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for factory defaults action.");
				}
			}
		});

		mntmDisplayOnoff.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doSetDisplay((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for display action.");
				}
			}
		});

		mntmTftp.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doSetTFTP((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for TFTP action.");
				}
			}
		});

		mntmUptime.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doUptime((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for uptime action.");
				}
			}
		});

		mntmVersion.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doVersion((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for version action.");
				}
			}
		});

		mntmSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 3) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doSave((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No txt file selected for save action.");
				}
			}
		});

		mntmTftpClient.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						TFTPClient client = new TFTPClient("", ((OrangePi) node.getUserObject()).getAddress());
						client.setVisible(true);
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for TFTP Client to run.");
				}
			}
		});
		
		mntmPixelTextPatterns.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doPixelTestPattern();
			}
		});

		mntmGlobalControl.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doBroadcastSelect();
			}
		});

		mntmLtcGenerator.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();

						if (pi.getNodeType().contains("ltc")) {
							LTCGenerator client = new LTCGenerator(pi.getAddress());
							client.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for LTC Generator to run.");
				}
			}
		});
		
		mntmDmxTransmit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();
						
						if (pi.getTxt("params.txt").contains("params.txt")) {
							UdpDmxTransmit client = new UdpDmxTransmit(pi.getAddress());
							client.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a DMX node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for DMX Transmit to run.");
				}			
			}
		});

		mntmRgbDisplay.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();

						if (pi.getNodeType().contains("ltc")) {
							RgbDisplay client = new RgbDisplay(pi.getAddress());
							client.Show();
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for RGB Display to run.");
				}
			}
		});

		mntmSytemTime.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();

						if (pi.getNodeType().contains("ltc")) {
							SystemTime client = new SystemTime(pi.getAddress());
							client.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for System Time to run.");
				}
			}
		});

		mntmMIDI.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();

						if (pi.getNodeType().contains("ltc")) {
							MIDI midi = new MIDI(pi.getAddress());
							midi.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for MIDI to run.");
				}
			}
		});

		mntmTCNet.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {

						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);

						OrangePi pi = (OrangePi) node.getUserObject();

						if (pi.getNodeType().contains("ltc")) {
							TCNet client = new TCNet(pi.getAddress());
							client.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for TCNet to run.");
				}
			}
		});

		mntmFirmwareInstallation.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						doFirmwareInstallation((OrangePi) node.getUserObject());
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for Workflow Firmware action.");
				}
			}
		});

		mntmBackupSelected.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						BackupConfiguration backup = new BackupConfiguration();
						HashSet<OrangePi> hOrangePi = new HashSet<OrangePi>();
						hOrangePi.add((OrangePi) node.getUserObject());
						backup.doSaveSelected(hOrangePi);
						backup.setVisible(true);
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for Workflow Backup action.");
				}
			}
		});

		mntmBackupAll.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				BackupConfiguration backup = new BackupConfiguration();
				backup.doSaveSelected(h);
				backup.setVisible(true);
			}
		});

		mntmRestore.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				TreePath path = tree.getSelectionPath();

				if (path != null) {
					if (path.getPathCount() == 2) {
						DefaultMutableTreeNode node = (DefaultMutableTreeNode) path.getPathComponent(1);
						RestoreConfiguration restore = new RestoreConfiguration();
						restore.doRestoreSelected((OrangePi) node.getUserObject());
						restore.setVisible(true);
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for Workflow Restore action.");
				}
			}
		});
	}

	private void InitComponents() {
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		setBounds(100, 100, 459, 337);

		tree = new JTree();
		tree.setToolTipText("Press ALT-R for refresh");
		tree.setRootVisible(false);
		tree.setModel(model);

		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);

		JMenu mnFile = new JMenu("File");
		menuBar.add(mnFile);

		mntmExit = new JMenuItem("Exit");

		mntmExit.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X, InputEvent.ALT_MASK));
		mnFile.add(mntmExit);

		mnAction = new JMenu("Action");
		menuBar.add(mnAction);

		mntmDisplayOnoff = new JMenuItem("Display On/Off");
		mntmDisplayOnoff.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_D, InputEvent.CTRL_MASK));
		mnAction.add(mntmDisplayOnoff);
		
		mntmFactoryDefaults = new JMenuItem("Factory defaults");
		mntmFactoryDefaults.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F, InputEvent.CTRL_MASK));
		mnAction.add(mntmFactoryDefaults);

		mntmReboot = new JMenuItem("Reboot");
		mntmReboot.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, InputEvent.CTRL_MASK));
		mnAction.add(mntmReboot);

		mntmSave = new JMenuItem("Save");
		mnAction.add(mntmSave);
		mntmSave.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.CTRL_MASK));

		mntmTftp = new JMenuItem("TFTP On/Off");
		mntmTftp.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T, InputEvent.CTRL_MASK));
		mnAction.add(mntmTftp);

		mntmUptime = new JMenuItem("Uptime");
		mntmUptime.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_U, InputEvent.CTRL_MASK));
		mnAction.add(mntmUptime);

		mntmVersion = new JMenuItem("Version");
		mntmVersion.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_V, InputEvent.CTRL_MASK));
		mnAction.add(mntmVersion);

		mnRun = new JMenu("Run");
		menuBar.add(mnRun);
		
				mntmGlobalControl = new JMenuItem("Global control");
				mntmGlobalControl.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_G, InputEvent.ALT_MASK));
				mnRun.add(mntmGlobalControl);
				
				mntmDmxTransmit = new JMenuItem("DMX Transmit");
				mntmDmxTransmit.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_D, InputEvent.ALT_MASK));
				mnRun.add(mntmDmxTransmit);
		
				mntmPixelTextPatterns = new JMenuItem("Pixel Controller Test Patterns");
				
						mntmPixelTextPatterns.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_P, InputEvent.ALT_MASK));
						mnRun.add(mntmPixelTextPatterns);
		
				mnLTC = new JMenu("LTC");
				mnRun.add(mnLTC);
				
						mntmLtcGenerator = new JMenuItem("Generator");
						mnLTC.add(mntmLtcGenerator);
						mntmLtcGenerator.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_G, InputEvent.ALT_MASK));
						
								mntmSytemTime = new JMenuItem("System time");
								mntmSytemTime.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.ALT_MASK));
								mnLTC.add(mntmSytemTime);
								
										mntmTCNet = new JMenuItem("TCNet");
										mntmTCNet.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, InputEvent.ALT_MASK));
										mnLTC.add(mntmTCNet);
										
												mntmRgbDisplay = new JMenuItem("RGB Display");
												mntmRgbDisplay.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, InputEvent.ALT_MASK));
												mnLTC.add(mntmRgbDisplay);
												
														mntmMIDI = new JMenuItem("MIDI");
														mntmMIDI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_M, InputEvent.ALT_MASK));
														mnLTC.add(mntmMIDI);

		mntmTftpClient = new JMenuItem("TFTP Client");
		mntmTftpClient.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T, InputEvent.ALT_MASK));
		mnRun.add(mntmTftpClient);

		JMenu mnView = new JMenu("View");
		menuBar.add(mnView);

		mntmExpandAll = new JMenuItem("Expand All");

		mntmExpandAll.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_E, InputEvent.ALT_MASK));
		mnView.add(mntmExpandAll);

		mntmColl = new JMenuItem("Collapse All");

		mntmColl.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C, InputEvent.ALT_MASK));
		mnView.add(mntmColl);

		mntmRefresh = new JMenuItem("Refresh");

		mntmRefresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, InputEvent.ALT_MASK));
		mnView.add(mntmRefresh);

		mnWorkflow = new JMenu("Workflow");
		menuBar.add(mnWorkflow);

		mntmFirmwareInstallation = new JMenuItem("Firmware installation");
		mnWorkflow.add(mntmFirmwareInstallation);

		mnBackup = new JMenu("Backup");
		mnWorkflow.add(mnBackup);

		mntmBackupAll = new JMenuItem("All");
		mnBackup.add(mntmBackupAll);

		mntmBackupSelected = new JMenuItem("Selected");
		mnBackup.add(mntmBackupSelected);

		mntmRestore = new JMenuItem("Restore");
		mnWorkflow.add(mntmRestore);

		JMenu mnHelp = new JMenu("Help");
		menuBar.add(mnHelp);

		mntmAbout = new JMenuItem("About");
		mntmAbout.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_A, InputEvent.ALT_MASK));

		mnHelp.add(mntmAbout);
		contentPane = new JPanel();
		contentPane.setToolTipText("Make sure that this application is in the same network as the nodes");

		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);

		scrollPaneLeft = new JScrollPane();
		scrollPaneLeft.setViewportBorder(new EtchedBorder(EtchedBorder.LOWERED, null, null));
		scrollPaneRight = new JScrollPane();
		lblDisplayName = new JLabel("");
		lblNodeId = new JLabel("");

		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(gl_contentPane.createParallelGroup(Alignment.LEADING).addGroup(gl_contentPane
				.createSequentialGroup().addComponent(scrollPaneLeft, GroupLayout.DEFAULT_SIZE, 240, Short.MAX_VALUE)
				.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPane.createSequentialGroup().addPreferredGap(ComponentPlacement.RELATED)
								.addGroup(gl_contentPane.createParallelGroup(Alignment.TRAILING)
										.addComponent(lblNodeId, GroupLayout.DEFAULT_SIZE, 203, Short.MAX_VALUE)
										.addComponent(lblDisplayName, GroupLayout.DEFAULT_SIZE, 203, Short.MAX_VALUE)))
						.addGroup(gl_contentPane.createSequentialGroup().addGap(10).addComponent(scrollPaneRight,
								GroupLayout.DEFAULT_SIZE, 199, Short.MAX_VALUE)))
				.addGap(0)));
		gl_contentPane.setVerticalGroup(gl_contentPane.createParallelGroup(Alignment.TRAILING).addGroup(gl_contentPane
				.createSequentialGroup().addContainerGap()
				.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(scrollPaneLeft, GroupLayout.DEFAULT_SIZE, 271, Short.MAX_VALUE)
						.addGroup(gl_contentPane.createSequentialGroup()
								.addComponent(lblDisplayName, GroupLayout.PREFERRED_SIZE, 25,
										GroupLayout.PREFERRED_SIZE)
								.addPreferredGap(ComponentPlacement.RELATED)
								.addComponent(lblNodeId, GroupLayout.PREFERRED_SIZE, 23, GroupLayout.PREFERRED_SIZE)
								.addPreferredGap(ComponentPlacement.RELATED)
								.addComponent(scrollPaneRight, GroupLayout.DEFAULT_SIZE, 211, Short.MAX_VALUE)))
				.addGap(0)));

		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setToolTipText("Editor for .txt files\nMouse right-click for Wizard");
		scrollPaneRight.setViewportView(textArea);

		scrollPaneLeft.setViewportView(tree);
		contentPane.setLayout(gl_contentPane);
	}

	private static boolean isRightClick(MouseEvent e) {
		return (e.getButton() == MouseEvent.BUTTON3 || (System.getProperty("os.name").contains("Mac OS X")
				&& (e.getModifiers() & InputEvent.BUTTON1_MASK) != 0
				&& (e.getModifiers() & InputEvent.CTRL_MASK) != 0));
	}

	private void doExit() {
		System.exit(0);
	}

	private void doReboot(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			int n = JOptionPane.showConfirmDialog(null, "Reboot", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
			if (n == JOptionPane.OK_OPTION) {
				try {
					opi.doReboot();
					JOptionPane.showMessageDialog(null, "Reboot message has been sent.");
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	private void doSetFactoryDefaults(OrangePi userObject) {
		if (lblNodeId.getText().trim().length() != 0) {
			int n = JOptionPane.showConfirmDialog(null, "Factory defaults", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
			if (n == JOptionPane.OK_OPTION) {
				try {
					opi.doFactory();
					JOptionPane.showMessageDialog(null, "Factory defaults message has been sent.");
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	private void doSetDisplay(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			String s = opi.doGetDisplay();

			if (s.contains("On")) {
				int n = JOptionPane.showConfirmDialog(null, "Display is On\nSet display Off? ", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
				opi.doSetDisplay(n != JOptionPane.OK_OPTION);
			} else {
				int n = JOptionPane.showConfirmDialog(null, "Display is Off\nSet display On? ", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
				opi.doSetDisplay(n == JOptionPane.OK_OPTION);
			}
		}
	}

	private void doSetTFTP(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			String s = opi.doGetTFTP();

			if (s.contains("On")) {
				int n = JOptionPane.showConfirmDialog(null, "TFTP is On\nSet TFTP Off? ", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
				opi.doSetTFTP(n != JOptionPane.OK_OPTION);
			} else {
				int n = JOptionPane.showConfirmDialog(null, "TFTP is Off\nSet TFTP On? ", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
				opi.doSetTFTP(n == JOptionPane.OK_OPTION);
			}
		}
	}

	private void doPixelTestPattern() {
		PixelTestPattern pixelTestPattern = new PixelTestPattern(this);
		pixelTestPattern.setModal(true);
		pixelTestPattern.setVisible(true);
	}

	private void doBroadcastSelect() {
		BroadcastSelect broadcastSelect = new BroadcastSelect(this, socketReceive);
		broadcastSelect.setVisible(true);
	}

	private void doFirmwareInstallation(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			FirmwareInstallation firmware = new FirmwareInstallation(opi, this);
			firmware.setVisible(true);
		}
	}

	private void doWizardRemoteConfig(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardRconfigTxt wizard = new WizardRconfigTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardDisplay(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardDisplayTxt wizard = new WizardDisplayTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}

	private void doWizardNetwork(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardNetworkTxt wizard = new WizardNetworkTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}

	private void doWizardDevices(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardDevicesTxt wizard = new WizardDevicesTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardE131(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardE131Txt wizard = new WizardE131Txt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardArtnet(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardArtnetTxt wizard = new WizardArtnetTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardDmx(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardParamsTxt wizard = new WizardParamsTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardDdpdisplay(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardDdpdisplayTxt wizard = new WizardDdpdisplayTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardGps(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardGpsTxt wizard = new WizardGpsTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}
	
	private void doWizardTCNet(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			WizardTCNetTxt wizard = new WizardTCNetTxt(lblNodeId.getText(), opi, this);
			wizard.setModal(true);
			wizard.setVisible(true);
		}
	}

	private void doUptime(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			try {
				String uptime = opi.doUptime().trim();
				uptime = uptime.substring(uptime.indexOf(' ') + 1, uptime.indexOf('s'));

				int nUptime = Integer.parseInt(uptime);
				int days = nUptime / (24 * 3600);
				nUptime -= days * (24 * 3600);
				int hours = nUptime / 3600;
				nUptime -= hours * 3600;
				int minutes = nUptime / 60;
				int seconds = nUptime - minutes * 60;

				String output = String.format("uptime %d day%s, %02d:%02d:%02d", days, days == 1 ? "" : "s", hours, minutes, seconds);

				JOptionPane.showMessageDialog(null, opi.getNodeDisplayName() + "\n" + opi.getNodeId() + "\n\n" + output);

			} catch (Exception e) {
				System.out.println(e);
				JOptionPane.showMessageDialog(null, opi.getNodeDisplayName() + "\n" + opi.getNodeId() + "\n\n" + opi.doUptime());
			}
		}
	}

	private void doVersion(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			JOptionPane.showMessageDialog(null, opi.getNodeDisplayName() + "\n" + opi.getNodeId() + "\n\n" + opi.doVersion());
		}
	}

	private void doSave(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			int n = JOptionPane.showConfirmDialog(null,
					"Save " + textArea.getText().trim().substring(1, textArea.getText().indexOf('\n')) + " ?",
					lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
			if (n == JOptionPane.OK_OPTION) {
				try {
					Boolean succes = opi.doSave(textArea.getText().trim());
					if (!succes) {
						JOptionPane.showMessageDialog(null, "Not saved!\nPlease check content");
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	public void setTextArea(String text) {
		System.out.println("textArea > [" + text + "]");
		textArea.setText(text);
	}
	
	public TreeMap<Integer, OrangePi> getTreeMap() {
		return treeMap;
	}

	public void constructTree() {
		Graphics g = getGraphics();

		lblDisplayName.setText("Searching ...");
		lblDisplayName.setForeground(Color.RED);
		lblNodeId.setText("");
		textArea.setText("");
		textArea.setEditable(false);
		textArea.setEnabled(false);

		update(g);

		DefaultMutableTreeNode root = new MyDefaultMutableTreeNode("Root");
		DefaultMutableTreeNode child = null;

		treeMap = new TreeMap<Integer, OrangePi>();

		for (int i = 0; i < 2; i++) {
			try {
				if (i == 0) {
					broadcast("?list#*");
					broadcast("?list#*");
				} else {
					broadcast("?list#");
				}
				while (true) {
					byte[] buffer = new byte[BUFFERSIZE];
					DatagramPacket dpack = new DatagramPacket(buffer, buffer.length);
					socketReceive.receive(dpack);

					textArea.append(dpack.getAddress().toString() + "\n");
					update(g);

					String str = new String(dpack.getData());
					final String data[] = str.split("\n");

					OrangePi opi = new OrangePi(data[0], socketReceive);

					if (opi.getIsValid()) {
						treeMap.put(ByteBuffer.wrap(dpack.getAddress().getAddress()).getInt(), opi);
					}
				}
			} catch (SocketTimeoutException e) {
				textArea.append("No replies\n");
				update(g);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (!treeMap.isEmpty()) {
			textArea.setText("");

			Set<Map.Entry<Integer, OrangePi>> entries = treeMap.entrySet();

			for (Map.Entry<Integer, OrangePi> entry : entries) {

				child = new DefaultMutableTreeNode(entry.getValue());
				child.add(new DefaultMutableTreeNode(((OrangePi) child.getUserObject()).getNodeRemoteConfig()));

				String nodeDisplay = ((OrangePi) child.getUserObject()).getNodeDisplay();

				if (nodeDisplay != null) {
					child.add(new DefaultMutableTreeNode(nodeDisplay));
				}

				child.add(new DefaultMutableTreeNode(((OrangePi) child.getUserObject()).getNodeNetwork()));

				String nodeType = ((OrangePi) child.getUserObject()).getNodeType();

				if (nodeType != null) {
					child.add(new DefaultMutableTreeNode(nodeType));
				}

				String nodeRdm = ((OrangePi) child.getUserObject()).getNodeRDM();

				if (nodeRdm != null) {
					child.add(new DefaultMutableTreeNode(nodeRdm));
				}

				String nodeLtcDisplay = ((OrangePi) child.getUserObject()).getNodeLtcDisplay();

				if (nodeLtcDisplay != null) {
					child.add(new DefaultMutableTreeNode(nodeLtcDisplay));
				}

				String nodeMode = ((OrangePi) child.getUserObject()).getNodeMode();

				if (nodeMode != null) {
					child.add(new DefaultMutableTreeNode(nodeMode));
				}

				String nodeTCNet = ((OrangePi) child.getUserObject()).getNodeTCNet();

				if (nodeTCNet != null) {
					child.add(new DefaultMutableTreeNode(nodeTCNet));
				}

				String nodeSparkFun = ((OrangePi) child.getUserObject()).getNodeSparkFun();

				if (nodeSparkFun != null) {
					child.add(new DefaultMutableTreeNode(nodeSparkFun));
				}

				int nMotorIndex = 0;
				String nodeMotor = null;

				while ((nodeMotor = ((OrangePi) child.getUserObject()).getNodeMotor(nMotorIndex)) != null) {
					child.add(new DefaultMutableTreeNode(nodeMotor));
					nMotorIndex++;
				}

				String nodeGPS = ((OrangePi) child.getUserObject()).getNodeGPS();

				if (nodeGPS != null) {
					child.add(new DefaultMutableTreeNode(nodeGPS));
				}
				
				String nodeETC = ((OrangePi) child.getUserObject()).getNodeETC();

				if (nodeETC != null) {
					child.add(new DefaultMutableTreeNode(nodeETC));
				}

				root.add(child);
			}
		} else {
			textArea.setText("No Orange Pi's found\n");
		}

		if (tree == null) {
			tree = new JTree(new DefaultTreeModel(root));
		} else {
			tree.setModel(new DefaultTreeModel(root));
		}

		lblDisplayName.setText("");
		lblDisplayName.setForeground(Color.BLACK);
		textArea.setEditable(true);
		textArea.setEnabled(true);

		update(g);
	}

	public void broadcast(String broadcastMessage) {
		byte[] buffer = broadcastMessage.getBytes();
		try {
			final DatagramPacket packet = new DatagramPacket(buffer, buffer.length, InetAddress.getByName("255.255.255.255"), PORT);
			try {
				socketReceive.send(packet);
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (UnknownHostException u) {
			u.printStackTrace();
		}
	}

	private void createReceiveSocket() {
		if (socketReceive != null) {
			socketReceive.close();
		}
		try {
			socketReceive = new DatagramSocket(null);
			SocketAddress sockaddr = new InetSocketAddress(PORT);
			socketReceive.setBroadcast(true);
			socketReceive.setSoTimeout(1000);
			socketReceive.bind(sockaddr);
		} catch (BindException e) {
			JOptionPane.showMessageDialog(null, "There is already an application running using the UDP port: " + PORT);
			doExit();
		} catch (SocketException e) {
			e.printStackTrace();
		}
	}
}
