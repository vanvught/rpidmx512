package org.orangepi.dmx;

import java.awt.EventQueue;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

import javax.swing.JDialog;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.LayoutStyle.ComponentPlacement;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JRadioButton;
import javax.swing.ButtonGroup;

public class SystemTime extends JDialog {
	private static final long serialVersionUID = 5979281990683857469L;
	static final int UDP_PORT = 21571;
	
	private InetAddress IPAddressLtcNode; 
    private DatagramSocket socket = null;
    private JButton btnStart;
    private JButton btnStop;
    private final ButtonGroup buttonGroup = new ButtonGroup();
    private JRadioButton rdbtn24FPS;
    private JRadioButton rdbtn25FPS;
    private JRadioButton rdbtn29FPS;
    private JRadioButton rdbtn30FPS;

	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					SystemTime dialog = new SystemTime(InetAddress.getByName("192.168.2.120"));
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	public SystemTime(InetAddress IPAddressLtcNode) {
		InitComponents();
		CreateEvents();
	
		this.IPAddressLtcNode = IPAddressLtcNode;
		
		 try {
			socket = new DatagramSocket();
			setTitle("System-Time: LTC Node " + IPAddressLtcNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error: LTC Node " + IPAddressLtcNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	public void Show() {
		setVisible(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 251, 121);
		
		btnStart = new JButton("Start");
		btnStop = new JButton("Stop");
		
		rdbtn24FPS = new JRadioButton("24");
		buttonGroup.add(rdbtn24FPS);
	
		rdbtn25FPS = new JRadioButton("25");
		buttonGroup.add(rdbtn25FPS);
		
		rdbtn29FPS = new JRadioButton("29.97");
		buttonGroup.add(rdbtn29FPS);
		
		rdbtn30FPS = new JRadioButton("30");
		buttonGroup.add(rdbtn30FPS);
		
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING, false)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(btnStart)
							.addPreferredGap(ComponentPlacement.RELATED, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
							.addComponent(btnStop))
						.addGroup(Alignment.LEADING, groupLayout.createSequentialGroup()
							.addComponent(rdbtn24FPS)
							.addGap(17)
							.addComponent(rdbtn25FPS)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtn29FPS)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(rdbtn30FPS)))
					.addGap(27))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(17)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnStop)
						.addComponent(btnStart))
					.addGap(6)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtn25FPS)
						.addComponent(rdbtn29FPS)
						.addComponent(rdbtn30FPS)
						.addComponent(rdbtn24FPS))
					.addContainerGap(24, Short.MAX_VALUE))
		);
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		btnStart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!start");
			}
		});	
		
		btnStop.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!stop");
			}
		});
		
		rdbtn24FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!rate#24");
			}
		});
		
		rdbtn25FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!rate#25");
			}
		});
		
		rdbtn29FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!rate#29");
			}
		});
		
		rdbtn30FPS.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("ltc!rate#30");
			}
		});
	}
	
	private void SendUpd(String message) {
		byte[] buffer = message.getBytes();

		
		DatagramPacket UDPPacket = new DatagramPacket(buffer, buffer.length, IPAddressLtcNode, UDP_PORT);
		try {
			socket.send(UDPPacket);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
