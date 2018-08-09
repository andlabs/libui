// 8 august 2018
// usage: hresultwrap funclist template out
package main

import (
	"fmt"
	"os"
	"io/ioutil"
	"text/template"
	"strings"

	"github.com/golang/protobuf/proto"
)

func argname(arg string) string {
	fields := strings.Fields(arg)
	last := fields[len(fields) - 1]
	start := strings.LastIndexFunc(last, func(r rune) bool {
		return !(r >= 'A' && r <= 'Z') &&
			!(r >= 'a' && r <= 'z') &&
			!(r >= '0' && r <= '9') &&
			r != '_'
	})
	if start == -1 {
		return last
	}
	// TODO replace + 1 with + len of that last rune
	return last[start + 1:]
}

func argcomma(n, len int) string {
	if n == len - 1 {
		return ""
	}
	return ", "
}

var templateFuncs = template.FuncMap{
	"argname":	argname,
	"argcomma":	argcomma,
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "usage: %s funclist template\n", os.Args[0])
		os.Exit(1)
	}
	b, err := ioutil.ReadFile(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "error reading %s: %v\n", os.Args[1], err)
		os.Exit(1)
	}
	var f File
	err = proto.UnmarshalText(string(b), &f)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error parsing %s: %v\n", os.Args[1], err)
		os.Exit(1)
	}

	tmpl, err := template.New(os.Args[2]).Funcs(templateFuncs).ParseFiles(os.Args[2])
	if err != nil {
		fmt.Fprintf(os.Stderr, "error parsing %s: %v\n", os.Args[2], err)
		os.Exit(1)
	}
	err = tmpl.Execute(os.Stdout, f.Func)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error executing template: %v\n", err)
		os.Exit(1)
	}
}
