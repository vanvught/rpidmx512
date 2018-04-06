## Open Source cross platform C++ library for the Open Sound Control (OSC) implementation ##

The OSC Server accepts the following message formats:

 <table cellspacing="0" border="0">
  <tr>
    <th>Path</th>
    <th>OSC Message Type</th>
    <th>Data</th>
	<th>Comments</th>
  </tr>
  <tr>
    <td>/path</td>
    <td>'b'</td>
    <td>Blob of length 1 to 512.</td>
	<td>The most efficient way of sending DMX data over OSC.</td>
  </tr>
  <tr>
    <td>/path/N</td>
    <td>'i'</td>
    <td>Slot value from 0 to 255.</td>
	<td>Update a single slot. N is the slot number from 1 to 512.</td>
  </tr>
  <tr>
    <td>/path/N</td>
    <td>'f'</td>
    <td>Slot value from 0.0 to 1.0</td>
	<td>Update a single slot N is the slot number from 1 to 512.</td>
  </tr>
  <tr>
    <td>/path</td>
    <td>'ii'</td>
    <td>Slot number from 0 to 511, slot value from 0 to 255.</td>
	<td>Update a single slot.</td>
  </tr>
  <tr>
    <td>/path</td>
    <td>'if'</td>
    <td>Slot number from 0 to 511, slot value from 0.0 to 1.0</td>
	<td>Update a single slot.</td>
  </tr>
</table> 

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

