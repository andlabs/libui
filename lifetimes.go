// 18 april 2015
package main

import (
	"fmt"
	"os"
	"bufio"
	"strings"
	"strconv"
)

func toaddr(s string) uintptr {
	n, err := strconv.Itoa(s)
	if err != nil {
		panic(err)
	}
	return uintptr(n)
}

var failed = false

func report(format string, args ...interface{}) {
	fmt.Fprintf(os.Stderr, format, args...)
	failed = true
}

type Object struct {
	Type			string
	Detail		string
	Refs			int
	CanDestroy	bool
}

var objs = make(map[uintptr]*Object)
var singles = make(map[uintptr]*Object)

var newControls = map[string]string{
	"uiNewButton()":		"button",
	"uiNewCheckbox()":		"checkbox",
	"uiNewEntry()":			"entry",
	"uiNewLabel()":			"label",
	"uiNewTab()":			"tab",
}

func newControl(parts []string) {
	ty := newControls[parts[0]]
	addr := toaddr(parts[1])
	detail := strings.Join(parts[2:], " ")
	o := &Object{
		Type:	ty,
		Detail:	detail,
		Refs:		1,
	}
	if oo := objs[addr]; oo != nil {
		report("duplicate object %s %q, %s %q at 0x%X\n",
			oo.Type, oo.Detail,
			o.Type, o.Detail,
			addr)
	}
	if oo := singles[addr]; oo != nil {
		report("single %s %q and objecct %s %q coexist at 0x%X\n",
			oo.Type, oo.Detail,
			o.Type, o.Detail,
			addr)
	}
	objs[addr] = o
}

func newSingle(parts []string) {
	addrObj := toaddr(parts[1])
	addrSingle := toaddr(parts[2])
	o := objs[addrObj]
	if o == nil {
		report("single without object at 0x%X\n", addrObj)
	}
	o.Refs++
	if oo := singles[addrSingle]; oo != nil {
		report("duplicate singles %s %q, %s %q at 0x%X\n",
			oo.Type, oo.Detail,
			o.Type, o.Detail,
			addrSingle)
	}
	singles[addrSingle] = o
}

func canDestroy(parts []string) {
	addrObj := toaddr(parts[1])
	addrSingle := toaddr(parts[2])
	o := objs[addrObj]
	if o == nil {
		report("destroy object without object at 0x%X\n", addrObj)
	}
	if singles[addrSingle] != o {
		report("inconsistency: single 0x%X has different object\n", addrSingle)
	}
	o.CanDestroy = true
}

func destroy(parts []string) {
	var o *Object
	var what string

	ty := parts[1]
	addr := toaddr(parts[2])
	if ty == "single" {
		o = singles[addr]
		delete(singles, addr)
		what = "single"
	} else {
		o = objs[addr]
		delete(objs, addr)
		what = "object"
	}
	if o == nil {
		report("missing %s %p in destroy()\n", what, addr)
		return
	}
	if !o.CanDestroy {
		report("can't destroy %s %p yet\n", what, addr)
	}
	o.Refs--
	if o.Refs < 0 {
		report("not enough references to %p in destroy()\n", addr)
	}
}

func main() {
	b := bufio.NewScanner(os.Stdin)
	for b.Scan() {
		s := b.String()
		parts := strings.Split(s, " ")
		name := parts[0]
		switch {
		case newControls[name] != "":
			newControl(parts)
		case name == "newSingle":
			newSingle(parts)
		case name == "uiControlDestroy()":
			canDestroy(parts)
		case name == "OSdestroy":
			destroy(parts)
		default:
			panic("unknown line " + name)
		}
	}
	if err := b.Err(); err != nil {
		panic(err)
	}
}
