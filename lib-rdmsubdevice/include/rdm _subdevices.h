/**
 * @file rdm_subdevices.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDM_SUBDEVICES_H_
#define RDM_SUBDEVICES_H_

namespace rdm {
namespace subdevices {
enum class Types {
	BW7FETS, BWDIMMER, BWDIO, BWLCD, BWRELAY,	// BitWizard
	MCP23S08, MCP23S17, 						// GPIO
	MCP4822, MCP4902,							// DAC
	UNDEFINED
};

const char* get_type_string(rdm::subdevices::Types type);
rdm::subdevices::Types get_type_string(const char *pValue);
}  // namespace subdevices
}  // namespace rdm

#endif /* RDM_SUBDEVICES_H_ */
