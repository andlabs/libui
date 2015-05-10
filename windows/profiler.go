// 10 may 2015
package main

import (
	"fmt"
	"os"
	"bufio"
	"debug/pe"
	"strings"
	"strconv"
)

func getsymbols(filename string) map[uint64]string {
	dll, err := pe.Open(os.Args[1])
	if err != nil {
		panic(err)
	}
	defer dll.Close()

	imagebase := uint64(0)
	switch o := dll.OptionalHeader.(type) {
	case *pe.OptionalHeader32:
		imagebase = uint64(o.ImageBase)
	case *pe.OptionalHeader64:
		imagebase = o.ImageBase
	default:
		panic("unknown image base; can't continue")
	}

	names := make(map[uint64]string)
	for _, s := range dll.Symbols {
		switch s.SectionNumber {
		case 0, -1, -2:		// undefined, constant, debugging
			continue
		}
		addr := uint64(s.Value)
		addr += imagebase
		section := dll.Sections[s.SectionNumber - 1]
		addr += uint64(section.VirtualAddress)
		names[addr] = s.Name
	}

	return names
}

type Entry struct {
	Leave	bool
	Func		string
	PerfCtr	uint64
}

func getentries(filename string, names map[uint64]string) []Entry {
	f, err := os.Open(os.Args[2])
	if err != nil {
		panic(err)
	}
	defer f.Close()

	entries := make([]Entry, 0, 10240)

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		l := scanner.Text()
		l = strings.TrimSpace(l)
		parts := strings.Split(l, " ")
		addr, err := strconv.ParseUint(parts[1], 16, 64)
		if err != nil {
			panic(err)
		}
		e := Entry{}
		e.Leave = parts[0] == "leave"
		e.Func = names[addr]
		e.PerfCtr, err = strconv.ParseUint(parts[2], 10, 64)
		if err != nil {
			panic(err)
		}
		entries = append(entries, e)
	}
	if err := scanner.Err(); err != nil {
		panic(err)
	}

	return entries
}

type Profile struct {
	Calls			uint
	TotalTime		uint64
	PerfCtr		[]uint64
}

var profile map[string]*Profile

func run(e Entry) {
	p := profile[e.Func]
	if p == nil {
		p = new(Profile)
	}
	if !e.Leave {
		p.Calls++
		p.PerfCtr = append(p.PerfCtr, e.PerfCtr)
	} else {
		start := p.PerfCtr[len(p.PerfCtr) - 1]
		p.PerfCtr = p.PerfCtr[:len(p.PerfCtr) - 1]
		p.TotalTime += (e.PerfCtr - start)
	}
	profile[e.Func] = p
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "usage: %s dll profout\n", os.Args[0])
		os.Exit(1)
	}

	names := getsymbols(os.Args[1])
	entries := getentries(os.Args[2], names)
	profile = make(map[string]*Profile)
	for _, e := range entries {
		run(e)
	}
	for f, p := range profile {
		fmt.Printf("%s %v %v ", f, p.Calls, p.TotalTime)
		if p.Calls != 0 {
			fmt.Printf("%v", float64(p.TotalTime) / float64(p.Calls))
		} else {
			fmt.Printf("%v", float64(0))
		}
		fmt.Printf("\n")
	}
}
