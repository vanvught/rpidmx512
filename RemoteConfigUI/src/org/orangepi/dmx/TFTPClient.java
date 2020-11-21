/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Arrays;
import java.util.prefs.Preferences;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.filechooser.FileFilter;

public class TFTPClient extends JDialog {
	 private InetAddress IPAddressTFTPServer; 
	 private byte[] receiveBuffer;
	 private DatagramSocket clientSocket;
	 private DatagramPacket UDPPacket;
	 private int TFPTport;
	 private String pathFile;
	 private File file;
	 private FileInputStream outOctet;
	 private Preferences prefs = Preferences.userRoot().node(getClass().getName());
	/**
	 * 
	 */
	private static final long serialVersionUID = 2075087707733616822L;
	private JTextField textField;
	private JButton btnPut;
	private JButton btnUimage;
	
	static final String LAST_USED_FOLDER = "org.orangepi.dmx";
	
	static final byte WRQopcode[] = {(byte) 0, (byte) 2}; 
	static final byte DATAopcode[] = {(byte) 0, (byte) 3}; 
	
	static final int DATA_SIZE = 512;
	static final int TFTP_DEFAULT_PORT = 69;
	private JButton btnClose;
	
	private int result = 0;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					TFTPClient dialog = new TFTPClient("/Volumes/development/workspace/opi_emac_artnet_dmx/orangepi_one.uImage", InetAddress.getByName("192.168.2.121") );
					dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
					dialog.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
	
	public int result() {
		return result;
	}
	
	public TFTPClient(String FileName, InetAddress IPAddressTFTPServer) {
		InitComponents();
		CreateEvents();
		
		this.IPAddressTFTPServer = IPAddressTFTPServer;
		this.pathFile = FileName;
		
		textField.setText(FileName);
		
		try {
			clientSocket = new DatagramSocket();
			clientSocket.setSoTimeout(1000);
		} catch (SocketException e) {
			displayError(e.getLocalizedMessage());
			e.printStackTrace();
		}  
		
        receiveBuffer = new byte[512];
        setTitle("TFTP Client " + IPAddressTFTPServer.getHostAddress());
        setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
	}
	
	private void InitComponents() {
		setTitle("TFTP Client");
		setBounds(100, 100, 450, 138);
		
		textField = new JTextField();
		textField.setEditable(false);
		textField.setColumns(10);
		
		btnPut = new JButton("TFTP put");
		btnUimage = new JButton("Select uImage");	
		btnClose = new JButton("Close");
		
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(btnUimage)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnPut))
						.addComponent(textField, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 438, Short.MAX_VALUE)
						.addComponent(btnClose, Alignment.TRAILING))
					.addContainerGap())
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(btnPut)
						.addComponent(btnUimage))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(textField, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(btnClose)
					.addContainerGap(8, Short.MAX_VALUE))
		);
		getContentPane().setLayout(groupLayout);
	}
	
	private void CreateEvents() {
		btnClose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
			}
		});
		
		btnPut.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				System.out.println(e);
				doTFTPput();
			}
		});
		
		btnUimage.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser jfc = new JFileChooser(prefs.get(LAST_USED_FOLDER, new File(".").getAbsolutePath()));
				
				FileFilter ff = new FileFilter() {

					@Override
					public String getDescription() {
						return "uImage files";
					}

					@Override
					public boolean accept(File f) {
						if (f.isDirectory())
							return true;
						if (f.getName().endsWith("uImage"))
							return true;
						if (f.getName().endsWith("uImage.gz"))
							return true;
						return false;
					}
				};
				
				jfc.setDialogTitle("Select an uImage");
				jfc.setAcceptAllFileFilterUsed(false);
				jfc.setFileFilter(ff);
				//jfc.addChoosableFileFilter(new FileNameExtensionFilter("*.uImage", "uImage"));

				int returnValue = jfc.showOpenDialog(null);

				if (returnValue == JFileChooser.APPROVE_OPTION) {
					File selectedFile = jfc.getSelectedFile();
					System.out.println(selectedFile.getAbsolutePath());
					pathFile = selectedFile.getAbsolutePath();
					textField.setText(pathFile);
					prefs.put(LAST_USED_FOLDER, jfc.getSelectedFile().getParent());
				}
			}
		});
	}
	
	private int getInt(byte[] array) {
		final int integer = ((array[0] & 0xff) << 8) | (array[1] & 0xff);
		return integer;
	}

	public static byte[] getByteArray(int integer) {
		byte[] array = new byte[2];
		array[1] = (byte) (integer & 0xFF);
		array[0] = (byte) ((integer >> 8) & 0xFF);
		return array;
	}
	

	private byte[] getDataPacket(int block, byte[] chunck) throws Exception {
        ByteArrayOutputStream outputPacket = new ByteArrayOutputStream( );
        
        outputPacket.write(DATAopcode);
        outputPacket.write(getByteArray(block));
        outputPacket.write(chunck);
 
        return outputPacket.toByteArray(); 
	}

	private void displayError(String txt) {
		textField.setText(txt);
		textField.setForeground(Color.RED);
		result = -1;
	}
	
	private void doTFTPput() {
		textField.setForeground(Color.BLACK);
		
		file = new File(pathFile);
		
		result = 1;
		
		try {
			outOctet = new FileInputStream(file);
		} catch (FileNotFoundException e) {
			displayError("File not found");
			e.printStackTrace();
		}
				
		try {
			SendRequest();
		} catch (Exception e) {
			displayError(e.getLocalizedMessage());
			e.printStackTrace();
		}
		
		try {
			outOctet.close();
		} catch (IOException e) {
			displayError(e.getLocalizedMessage());
			e.printStackTrace();
		}
	}

	private void SendRequest() throws Exception {
        ByteArrayOutputStream outputPacket = new ByteArrayOutputStream( );
        
		outputPacket.write(WRQopcode);
		outputPacket.write(file.getName().getBytes());
		outputPacket.write((byte) 0);
		outputPacket.write("octet".getBytes());
		outputPacket.write((byte) 0);

		byte[] requestBuffer = outputPacket.toByteArray();

        UDPPacket = new DatagramPacket(requestBuffer, requestBuffer.length, IPAddressTFTPServer, TFTP_DEFAULT_PORT);
		clientSocket.send(UDPPacket);
    
        // receiving Ack
        UDPPacket = new DatagramPacket(receiveBuffer, receiveBuffer.length, InetAddress.getLocalHost(), clientSocket.getLocalPort());         
        clientSocket.receive(UDPPacket);   
         
        TFPTport = UDPPacket.getPort(); 
 
        final int opCodeReceived =  getInt(Arrays.copyOfRange(receiveBuffer, 0, 2));
        
        System.out.println("opCodeReceived=" + opCodeReceived);

		if (opCodeReceived == 5) {
			displayError("TFTP: " + new String(Arrays.copyOfRange(receiveBuffer, 4, receiveBuffer.length)));
		} else if (opCodeReceived == 4) {
			// Ack
			if (getInt(Arrays.copyOfRange(receiveBuffer, 2, 4)) == 0) {
				doSendFile();
			}
		}
	}

	private void doSendFile() throws Exception {
		int block = 0;
		boolean done = false;
		int bytesSent = 0;
		byte[] chunck;
		byte[] fileBytes = new byte[(int) file.length()];
		final int maxBlocks = (fileBytes.length + DATA_SIZE)/ DATA_SIZE;
		
		Graphics g = getGraphics();
		
		outOctet.read(fileBytes);
   
		System.out.println("File size = " + fileBytes.length);


		while (!done) {
			block++;
			
			if ((file.length() - bytesSent) >= DATA_SIZE) {
				chunck = Arrays.copyOfRange(fileBytes, bytesSent, bytesSent + DATA_SIZE);
			} else {
				chunck = Arrays.copyOfRange(fileBytes, bytesSent, (int) file.length());
				done = true;
			}
			
			byte[] dataPacket = getDataPacket(block, chunck); 
			
        	UDPPacket = new DatagramPacket(dataPacket, dataPacket.length, IPAddressTFTPServer, TFPTport);
            clientSocket.send(UDPPacket);  
            
            // receiving Ack
            UDPPacket = new DatagramPacket(receiveBuffer, receiveBuffer.length, InetAddress.getLocalHost(), clientSocket.getLocalPort());         
            clientSocket.receive(UDPPacket);
            
            final int opCodeReceived =  getInt(Arrays.copyOfRange(receiveBuffer, 0, 2));
                        
			if (opCodeReceived == 5) {
				displayError("TFTP: " + new String(Arrays.copyOfRange(receiveBuffer, 4, receiveBuffer.length)));
				done = true;
			} else if (opCodeReceived == 4) {
				final int blockReceived = getInt(Arrays.copyOfRange(receiveBuffer, 2, 4));
				textField.setText("Blocks " + maxBlocks + "/" + blockReceived + " Bytes: " + bytesSent);
				bytesSent += chunck.length;
			}
    		
    		update(g); // Quick and dirty update
		}
		
		if (bytesSent == file.length()) {
			textField.setText("Switch TFTP Off for flashing the new firmware " + bytesSent + " bytes");
			textField.setForeground(Color.GREEN);
		} else {
			displayError("TFTP failed");
		}
	}
}
