// 2 november 2017
package main

import (
	"fmt"
)

type fixed1616 uint32
type fixed214 uint16

func fixed1616To214(f fixed1616) fixed214 {
	f += 0x00000002
	g := int32(f) >> 2
	return fixed214(uint32(g) & 0xFFFF)
}

func (f fixed1616) In214Range() bool {
	base := int16((f >> 16) & 0xFFFF)
	return base >= -2 && base < 2
}

func (f fixed1616) String() string {
	base := int16((f >> 16) & 0xFFFF)
	frac := float64(f & 0xFFFF) / 65536
	res := float64(base) + frac
	return fmt.Sprintf("%f 0x%08X", res, uint32(f))
}

func (f fixed214) String() string {
	base := []int16{
		0,
		1,
		-2,
		-1,
	}[(f & 0xC000) >> 14]
	frac := float64(f & 0x3FFF) / 16384
	res := float64(base) + frac
	return fmt.Sprintf("%f 0x%04X", res, uint16(f))
}

func main() {
	fmt.Println(fixed214(0x7fff))
	fmt.Println(fixed214(0x8000))
	fmt.Println(fixed214(0x4000))
	fmt.Println(fixed214(0xc000))
	fmt.Println(fixed214(0x7000))
	fmt.Println(fixed214(0x0000))
	fmt.Println(fixed214(0x0001))
	fmt.Println(fixed214(0xffff))

	fmt.Println()

	for i := uint64(0x00000000); i <= 0xFFFFFFFF; i++ {
		j := fixed1616(i)
		if !j.In214Range() { continue }
		fmt.Println(j, "->", fixed1616To214(j))
	}
}
