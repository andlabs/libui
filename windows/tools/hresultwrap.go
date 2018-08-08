// 8 august 2018
// usage: hresultwrap funclist template out
package main

import (
	"fmt"
	"os"
	"io/ioutil"

	"github.com/golang/protobuf/proto"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "usage: %s funclist\n", os.Args[0])
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
	fmt.Println(f)
}
