package org.orangepi.dmx;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;

public class UdpSerial extends JDialog {
	private static final long serialVersionUID = -4391872265315903050L;
	private final static String newline = "\n";
	private final int UDP_PORT = 9999;
	private DatagramSocket serverSocket = null;
	private SocketAddress sockaddr = null;
	private InetAddress serverInetAddress = null;	
	private byte[] sendData = null;
	
	private JTextField textFieldCmd;
	private JTextField textIPAddress;
	private JButton btnConnect;
	private JTextArea textArea;
	

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		try {
			UdpSerial dialog = new UdpSerial(InetAddress.getByName("192.168.2.133"));
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Create the dialog.
	 */
	public UdpSerial(InetAddress localInetAddress) {
		setTitle("UART-SHELL DEBUG");
		InitComponents();
		CreateEvents();

		try {
			serverSocket = new DatagramSocket(null);
			sockaddr = new InetSocketAddress(localInetAddress, UDP_PORT);
			serverSocket.bind(sockaddr);
			
			sendData = new byte[1024];
		} catch (Exception e) {
			e.printStackTrace();
		}	
	}
	
	public void Show() {
		setVisible(true);
	}
	
	
	private void InitComponents() {
		setBounds(100, 100, 450, 411);
		
		textFieldCmd = new JTextField();
		textFieldCmd.setEditable(false);
		textFieldCmd.setColumns(10);
		
		JLabel lblNewLabel = new JLabel("Bridge-IP");
		
		textIPAddress = new JTextField();
		textIPAddress.setColumns(10);
	
		btnConnect = new JButton("Connect");
		
		JScrollPane scrollPane = new JScrollPane();

		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 438, Short.MAX_VALUE)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(lblNewLabel)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(textIPAddress, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnConnect))
						.addGroup(Alignment.TRAILING, groupLayout.createSequentialGroup()
							.addComponent(textFieldCmd, GroupLayout.DEFAULT_SIZE, 531, Short.MAX_VALUE)
							.addGap(9)))
					.addContainerGap())
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(15)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNewLabel, GroupLayout.PREFERRED_SIZE, 17, GroupLayout.PREFERRED_SIZE)
						.addComponent(textIPAddress, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(btnConnect))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 301, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(textFieldCmd, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addContainerGap())
		);
		
		textArea = new JTextArea();
		textArea.setEditable(false);
		scrollPane.setViewportView(textArea);
		
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		textFieldCmd.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e) {
				int key = e.getKeyCode();
				if (key == KeyEvent.VK_ENTER) {
					sendData = (textFieldCmd.getText() + newline).getBytes();
					if (sendData.length > 1) {
						DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, serverInetAddress, UDP_PORT);
						try {
							serverSocket.send(sendPacket);
							textArea.append(textFieldCmd.getText() + newline);
							textFieldCmd.setText("");
						} catch (Exception e1) {
							e1.printStackTrace();
						}
					}
				}
			}
		});
		
		btnConnect.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					if (textIPAddress.getText().length() > 6) {
						serverInetAddress = InetAddress.getByName(textIPAddress.getText());
						textIPAddress.setEditable(false);
						btnConnect.setEnabled(false);
						textFieldCmd.setEditable(true);

						Thread t = new Thread(new Runnable() {
							public void run() {
								try {
									startServer();
								} catch (Exception e) {
									e.printStackTrace();
								}
							}

							private void startServer() {
								try {
									sendData = new byte[5];
									sendData[0] = 'i';
									sendData[1] = 'n';
									sendData[2] = 'f';
									sendData[3] = 'o';
									sendData[4] = 0x0A;
									DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, serverInetAddress, UDP_PORT);
									serverSocket.send(sendPacket);
								} catch (IOException e1) {
									e1.printStackTrace();
								}
								
								byte[] receiveData = new byte[1024];
								
								while (true) {
									try {
										DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
										serverSocket.receive(receivePacket);
										String str = new String( receivePacket.getData(), 0, receivePacket.getLength() ) + newline;
										textArea.append(str);
										textArea.setCaretPosition(textArea.getDocument().getLength());
									} catch (Exception e) {
										e.printStackTrace();
									}
								}
							}
						});
						t.start();
					}
				} catch (Exception e1) {
					textArea.setText(e1.getMessage());
					e1.printStackTrace();
				}
			}
		});
	}
}
