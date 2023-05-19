A freestanding implementation has an implementation-defined set of headers. The following headers are provided by the C/C++ standard:

#c11
> 4 Conformance
> 
> 6 ... A conforming freestanding implementation shall accept any strictly conforming program that does not use complex types and in which the use of the features specified in the library clause (clause 7) is confined to the contents of the standard headers &lt;float.h>, &lt;iso646.h>, &lt;limits.h>, &lt;stdalign.h>, &lt;stdarg.h>, &lt;stdbool.h>, &lt;stddef.h>, and &lt;stdint.h>. 

Reference: [https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1548.pdf](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1548.pdf)

#c++11


<table>
<tr>
	<td>Types</td>
	<td>&lt;cstddef></td>
</tr>
<tr>
	<td rowspan="3">Implementation properties</td>
	<td>&lt;limits></td>
</tr>
<tr>
	<td>&lt;cfloat></td>
</tr>	
<tr>
	<td>&lt;climits></td>
</tr>	
<tr>
	<td>Integer types</td>
	<td>&lt;cstdint></td>
</tr>
<tr>
	<td>Start and termination</td>
	<td>&ltcstdlib> (partial)</td>
</tr>
<tr>
	<td>Dynamic memory management</td>
	<td>&lt;new></td>
</tr>
<tr>
	<td>Type identification</td>
	<td>&lt;typeinfo></td>
</tr>
<tr>
	<td>Exception handling</td>
	<td>&lt;exception></td>
</tr>
<tr>
	<td>Initializer lists</td>
	<td>&lt;initializer_list></td>
</tr>
<tr>
	<td>Other runtime support</td>
	<td>&lt;cstdarg></td>
</tr>
<tr>
	<td>Type traits</td>
	<td>&lt;type_traits></td>
</tr>
<tr>
	<td>Atomics</td>
	<td>&lt;atomic></td>
</tr>
</table>
	
Reference: [https://en.cppreference.com/w/cpp/freestanding](https://en.cppreference.com/w/cpp/freestanding)

[http://www.gd32-dmx.org](http://www.gd32-dmx.org)

[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)


