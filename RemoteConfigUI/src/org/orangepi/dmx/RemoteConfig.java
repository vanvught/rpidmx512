/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;

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

public class RemoteConfig extends JFrame {
	private static final long serialVersionUID = 8836780363465781413L;

	private final int BUFFERSIZE = 512;
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
	private JMenuItem mntmInterfaces;
	
	private InetAddress localAddress;
	private static final String ipv4Pattern = "(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])";
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
		
		NetworkInterface ifDefault = FirstNetworkInterface.get();
		
		if (ifDefault == null) {
			try {
				localAddress = InetAddress.getLocalHost();
			} catch (UnknownHostException e) {
				e.printStackTrace();
			}
		} else {
			Enumeration<InetAddress> enumIP = ifDefault.getInetAddresses();

			while (enumIP.hasMoreElements()) {
				InetAddress ip = (InetAddress) enumIP.nextElement();

				if (ip.getHostAddress().matches(ipv4Pattern)) {
					localAddress = ip;
					break;
				}
			}
		}
		
		setTitle("Remote Configuration Manager - " +  localAddress.getHostAddress());
		
		System.out.println("Local ip: " + localAddress.getHostAddress());
		
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
					OrangePi opi = (OrangePi) node.getUserObject();
					
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
		
		mntmInterfaces.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				NetworkInterfaces networkInterfaces = new NetworkInterfaces(localAddress);
				InetAddress currentLocalAddress = localAddress;
				
				localAddress = networkInterfaces.Show();
					
				if (!localAddress.equals(currentLocalAddress)) {
					setTitle("Remote Configuration Manager - " + localAddress.getHostAddress());
					createReceiveSocket();
					constructTree();
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
						client.Show();
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for TFTP Client to run.");
				}
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
							client.Show();
						} else {
							JOptionPane.showMessageDialog(null, "The node selected is not a LTC node");
						}
					}
				} else {
					JOptionPane.showMessageDialog(null, "No node selected for LTC Generator to run.");
				}	
			}
		});
	}

	private void InitComponents() {
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		setBounds(100, 100, 459, 325);
		
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
		
		mntmLtcGenerator = new JMenuItem("LTC Generator");

		mntmLtcGenerator.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, InputEvent.ALT_MASK));
		mnRun.add(mntmLtcGenerator);
		
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
		
		JMenu mnNetwork = new JMenu("Network");
		menuBar.add(mnNetwork);
		
		mntmInterfaces = new JMenuItem("Interfaces");
		mntmInterfaces.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_I, InputEvent.ALT_MASK));
		mnNetwork.add(mntmInterfaces);
		
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
		
		scrollPaneRight = new JScrollPane();
		
		lblDisplayName = new JLabel("");
		
		lblNodeId = new JLabel("");
		
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addComponent(scrollPaneLeft, GroupLayout.DEFAULT_SIZE, 240, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(lblNodeId, GroupLayout.DEFAULT_SIZE, 203, Short.MAX_VALUE)
						.addComponent(lblDisplayName, GroupLayout.DEFAULT_SIZE, 203, Short.MAX_VALUE)
						.addGroup(Alignment.TRAILING, gl_contentPane.createSequentialGroup()
							.addGap(4)
							.addComponent(scrollPaneRight, GroupLayout.DEFAULT_SIZE, 199, Short.MAX_VALUE)))
					.addGap(0))
		);
		gl_contentPane.setVerticalGroup(
			gl_contentPane.createParallelGroup(Alignment.TRAILING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPane.createParallelGroup(Alignment.TRAILING)
						.addComponent(scrollPaneLeft, Alignment.LEADING, GroupLayout.DEFAULT_SIZE, 259, Short.MAX_VALUE)
						.addGroup(gl_contentPane.createSequentialGroup()
							.addComponent(lblDisplayName, GroupLayout.PREFERRED_SIZE, 25, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(lblNodeId, GroupLayout.PREFERRED_SIZE, 23, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(scrollPaneRight, GroupLayout.DEFAULT_SIZE, 187, Short.MAX_VALUE)))
					.addContainerGap())
		);
		
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setToolTipText("Editor for .txt files ");
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
		int n = JOptionPane.showConfirmDialog(null, "Are you sure?", "Exit", JOptionPane.OK_CANCEL_OPTION);
		if (n == JOptionPane.OK_OPTION)
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
	
	private void doUptime(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			JOptionPane.showMessageDialog(null, opi.getNodeDisplayName() + "\n" + opi.getNodeId() + "\n\n" + opi.doUptime());
		}
	}
	
	private void doVersion(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			JOptionPane.showMessageDialog(null, opi.getNodeDisplayName() + "\n" + opi.getNodeId() + "\n\n" + opi.doVersion());
		}
	}
	
	private void doSave(OrangePi opi) {
		if (lblNodeId.getText().trim().length() != 0) {
			int n = JOptionPane.showConfirmDialog(null, "Save " + textArea.getText().trim().substring(1, textArea.getText().indexOf('\n')) + " ?", lblDisplayName.getText(), JOptionPane.OK_CANCEL_OPTION);
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
	
	private void constructTree() {			
		System.out.println("Interface address: " + localAddress.getHostAddress());
		
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

		HashSet<OrangePi> h = new HashSet<OrangePi>();

		byte[] buffer = new byte[BUFFERSIZE];
		DatagramPacket dpack = new DatagramPacket(buffer, buffer.length);
		int times = 0;

		while (h.isEmpty() && times++ < 3) {

			try {
				broadcast("?list#", InetAddress.getByName("255.255.255.255"));

				while (true) {
					socketReceive.receive(dpack);

					String str = new String(dpack.getData());
					String data[] = str.split("\n");

					OrangePi opi = new OrangePi(data[0], localAddress, socketReceive);
					if (opi.getIsValid()) {
						h.add(opi);
					}
				}
			} catch (SocketTimeoutException e) {
				System.out.println("No more messages.");
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		Iterator<OrangePi> it = h.iterator();

		while (it.hasNext()) {
			child = new DefaultMutableTreeNode(it.next());
			child.add(new DefaultMutableTreeNode(((OrangePi) child.getUserObject()).getNodeRemoteConfig()));
			child.add(new DefaultMutableTreeNode(((OrangePi) child.getUserObject()).getNodeNetwork()));
			child.add(new DefaultMutableTreeNode(((OrangePi) child.getUserObject()).getNodeType()));
			
			String nodeMode = ((OrangePi) child.getUserObject()).getNodeMode();
			
			if (nodeMode != null) {
				child.add(new DefaultMutableTreeNode(nodeMode));
			}
			
			String nodeExtras =  ((OrangePi) child.getUserObject()).getNodeExtras();
			
			if (nodeExtras != null) {
				child.add(new DefaultMutableTreeNode(nodeExtras));
			}
			
			root.add(child);
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

	private void broadcast(String broadcastMessage, InetAddress address) throws IOException {
		byte[] buffer = broadcastMessage.getBytes();

		DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, PORT);

		DatagramSocket socketBroadcast = new DatagramSocket(null);
		socketBroadcast.setReuseAddress(true);
		SocketAddress sockaddr = new InetSocketAddress(localAddress, PORT);
		socketBroadcast.bind(sockaddr);
		socketBroadcast.setBroadcast(true);
		socketBroadcast.send(packet);
		socketBroadcast.close();
	}
	
	private void createReceiveSocket() {
		if (socketReceive != null) {
			socketReceive.close();
		}
		try {
			socketReceive = new DatagramSocket(null);
			socketReceive.setReuseAddress(true);
			SocketAddress sockaddr = new InetSocketAddress(localAddress, PORT);
			socketReceive.bind(sockaddr);
			socketReceive.setSoTimeout(1500);
		} catch (SocketException e) {
			e.printStackTrace();
		}
	}
	
}
