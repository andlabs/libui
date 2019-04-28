// 28 april 2019
package main

import (
	"fmt"
)

func main() { main2(7,2,3,8) }

type op interface {
	do(x uint32) uint32
	write() string
}

type sub uint32
func (b sub) do(x uint32) uint32 { return x - uint32(b) }
func (b sub) write() string { return fmt.Sprintf("- %d", b) }

type div uint32
func (c div) do(x uint32) uint32 { return x / uint32(c) }
func (c div) write() string { return fmt.Sprintf("/ %d", c) }

type mul uint32
func (d mul) do(x uint32) uint32 { return x * uint32(d) }
func (d mul) write() string { return fmt.Sprintf("* %d", d) }

func dowrite(a uint32, b, c, d op) (uint32, string) {
	val := d.do(c.do(b.do(a)))
	
	str := fmt.Sprintf("%d %s", a, b.write())
	str = fmt.Sprintf("(%s) %s", str, c.write())
	str = fmt.Sprintf("(%s) %s", str, d.write())
	
	return val, str
}

func main2(aa, bb, cc, dd uint32) {
	a, b, c, d := uint32(aa), sub(bb), div(cc), mul(dd)
	fmt.Println(dowrite(a, b, c, d))
	fmt.Println(dowrite(a, b, d, c))
	fmt.Println(dowrite(a, c, b, d))
	fmt.Println(dowrite(a, c, d, b))
	fmt.Println(dowrite(a, d, b, c))
	fmt.Println(dowrite(a, d, c, b))
}

func try(a, b, c, d uint32) bool {
	m := make(map[uint32]uint32)
	m[a]++;m[b]++;m[c]++;m[d]++
	if len(m)!=4{return false}
	
	add := func(x uint32) uint32 { return x - b }
	mul := func(x uint32) uint32 { return x / c }
	sub := func(x uint32) uint32 { return x * d }
	m = make(map[uint32]uint32)
	m[add(mul(sub(a)))]++
	m[add(sub(mul(a)))]++
	m[mul(add(sub(a)))]++
	m[mul(sub(add(a)))]++
	m[sub(add(mul(a)))]++
	m[sub(mul(add(a)))]++
	if len(m) != 6 {
		return false
	}
	for k, v := range m {
		if int32(k) < 0 {
			return false
		}
		if v != 1 {
			return false
		}
	}
	return true
}

func main1() {
	min := uint32(2)
	max := uint32(10)
	for a := min; a <= max; a++ {
		for b := min; b <= max; b++ {
			for c := min; c <= max; c++ {
				for d := min; d <= max; d++ {
					if try(a, b, c, d) {
						fmt.Println(a, b, c, d)
					}
				}
			}
		}
	}
}
