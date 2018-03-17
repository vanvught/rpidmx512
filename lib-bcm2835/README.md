## Raspberry Pi bare-metal library for the BROADCOM SoC's ##

<table cellspacing="0" border="0">
	<tr>
		<td colspan=3 height="17" align="center" valign=middle>ARM Physical</td>
		<td colspan=2 align="center">memmap</td>
		<td align="left">Comment</td>
		<td align="left">Model</td>
		<td colspan=2 align="center" valign=middle>MMU</td>
		</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0000000</font></td>
		<td rowspan=7 align="left" valign=middle sdnum="1043;0;@"><font face="Courier New">3MB</font></td>
		<td align="left" valign=middle sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td rowspan=17 align="left" valign=middle>0x0040E</td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0008000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">.init</font></td>
		<td align="left"><font face="Courier New">__ram_start</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0008040</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">.text</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">reset</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x00080..</font></td>
		<td align="left"><br></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">notmain</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">.rodata</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left"><br></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">.data</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">.bss</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0300000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">3MB</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0310000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__und_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0320000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__abt_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0330000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__fiq_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0340000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__irq_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0350000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__svc_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0360000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__sys_stack_top</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0370000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__svc_stack_top_core1</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0380000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__svc_stack_top_core2</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x0390000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">64KB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New">__svc_stack_top_core3</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x00400000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">1MB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">4MB</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left">0x10412</td>
		<td align="left">MEM_COHERENT_REGION</td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x00500000</font></td>
		<td rowspan=2 align="left" valign=middle sdnum="1043;0;@"><font face="Courier New">123MB</font></td>
		<td align="left" valign=middle sdnum="1043;0;@"><font face="Courier New">5MB</font></td>
		<td align="left"><font face="Courier New">heap_low</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td rowspan=2 align="center" valign=middle>0x0040E</td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x08000000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">128MB</font></td>
		<td align="left"><font face="Courier New">heap_top</font></td>
		<td align="left"><font face="Courier New">__ram_end</font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left">Video Core Ram</td>
		<td align="left" sdval="1" sdnum="1043;">1</td>
		<td align="left">0x10416</td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x20000000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">512MB</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left">512 MB</td>
		<td align="left" sdval="1" sdnum="1043;">1</td>
		<td align="left">0x00412</td>
		<td align="left">BCM2835_PERI_BASE</td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left">Video Core Ram</td>
		<td align="left" sdval="2,3" sdnum="1043;">2,3</td>
		<td align="left">0x10416</td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x3F000000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left" sdval="2,3" sdnum="1043;">2,3</td>
		<td align="left">0x00412</td>
		<td align="left">BCM2835_PERI_BASE</td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x40000000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">1MB</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">1GB</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left">Mailboxes multi-core</td>
		<td align="left" sdval="3" sdnum="1043;">3</td>
		<td align="left">0x10416</td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x40100000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
	<tr>
		<td height="17" align="left" sdnum="1043;0;@"><font face="Courier New">0x100000000</font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New"><br></font></td>
		<td align="left" sdnum="1043;0;@"><font face="Courier New">4GB</font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left"><font face="Courier New"><br></font></td>
		<td align="left">32-bits</td>
		<td align="left"><br></td>
		<td align="left"><br></td>
		<td align="left"><br></td>
	</tr>
</table>

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

