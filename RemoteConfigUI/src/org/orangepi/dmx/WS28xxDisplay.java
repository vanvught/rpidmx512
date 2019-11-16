package org.orangepi.dmx;

import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.text.ParseException;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.text.MaskFormatter;

public class WS28xxDisplay extends JDialog {
	private static final long serialVersionUID = -3879847439023085704L;
	
	static final int UDP_PORT = 0x2812;
	static final int WS28XX_NUM_OF_DIGITS = 8;
	
	private InetAddress IPAddressLtcNode; 
    private DatagramSocket socket = null;
    private JButton btnShowMessage;
    private JTextField textMessage;
    private JButton btnDigitColour;
    private JButton btnColonColour;
    private JButton btnMessageColour;
    private JFormattedTextField textDigitColour;
    private JFormattedTextField textColonColour;
    private JFormattedTextField textMessageColour;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					WS28xxDisplay dialog = new WS28xxDisplay(InetAddress.getByName("192.168.2.120"));
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
	
	/**
	 * Create the dialog.
	 */
	public WS28xxDisplay(InetAddress IPAddressLtcNode) {
		InitComponents();
		CreateEvents();
		
		this.IPAddressLtcNode = IPAddressLtcNode;
		
		 try {
			socket = new DatagramSocket();
			setTitle("LTC Node " + IPAddressLtcNode.getHostAddress());
		} catch (SocketException e) {
			setTitle("Error: LTC Node " + IPAddressLtcNode.getHostAddress());
			e.printStackTrace();
		}
	}
	
	public void Show() {
		setVisible(true);
	}
	
	private void InitComponents() {
		setBounds(100, 100, 260, 168);
		
		btnShowMessage = new JButton("Show message");	
		
		textMessage = new JTextField();
		textMessage.setDocument(new JTextFieldLimit(WS28XX_NUM_OF_DIGITS));
		textMessage.setColumns(10);
		
		btnDigitColour = new JButton("Set digit colour");
		btnColonColour = new JButton("Set colon colour");
		btnMessageColour = new JButton("Set message colour");
		
		MaskFormatter mask = null;
		try {
			mask = new MaskFormatter("HHHHHH");
			mask.setPlaceholderCharacter('0');
		} catch (ParseException e1) {
			e1.printStackTrace();
		}
		
		
		textDigitColour = new JFormattedTextField(mask);
		textDigitColour.setToolTipText("RRGGBB");
		textDigitColour.setColumns(10);
		
		textColonColour = new JFormattedTextField(mask);
		textColonColour.setToolTipText("RRGGBB");
		textColonColour.setColumns(10);
		
		textMessageColour = new JFormattedTextField(mask);
		textMessageColour.setToolTipText("RRGGBB");
		textMessageColour.setColumns(10);

		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(btnDigitColour)
						.addComponent(btnColonColour)
						.addComponent(btnShowMessage)
						.addComponent(btnMessageColour))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING, false)
						.addGroup(Alignment.LEADING, groupLayout.createParallelGroup(Alignment.TRAILING, false)
							.addComponent(textMessageColour, Alignment.LEADING, 0, 0, Short.MAX_VALUE)
							.addComponent(textColonColour, Alignment.LEADING, 0, 0, Short.MAX_VALUE)
							.addComponent(textDigitColour, Alignment.LEADING, GroupLayout.DEFAULT_SIZE, 76, Short.MAX_VALUE))
						.addComponent(textMessage, GroupLayout.PREFERRED_SIZE, 76, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(18, Short.MAX_VALUE))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnDigitColour)
						.addComponent(textDigitColour, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnColonColour)
						.addComponent(textColonColour, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnMessageColour)
						.addComponent(textMessageColour, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnShowMessage)
						.addComponent(textMessage, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(6, Short.MAX_VALUE))
		);
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		btnShowMessage.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("7seg!showmsg#".concat(textMessage.getText().trim()));
			}
		});
			
		btnDigitColour.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("7seg!rgb#0".concat(textDigitColour.getText().trim()));
			}
		});
		
		btnColonColour.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("7seg!rgb#1".concat(textColonColour.getText().trim()));
			}
		});
		
		btnMessageColour.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				SendUpd("7seg!rgb#2".concat(textMessageColour.getText().trim()));
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
