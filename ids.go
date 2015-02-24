// 23 february 2015

package ui

// Go pointers cannot be passed to C safely.
// The Go runtime will move them around, tripping C up.
// However, C pointers don't have this problem.
// Solution: use the C pointer to get the Go object needed!

import (
	"fmt"
	"sync"
)

var (
	ids		map[uintptr]interface{}
	idsLock	sync.Mutex
)

func getGoObject(cptr uintptr) interface{} {
	idsLock.Lock()
	defer idsLock.Unlock()

	if i, ok := ids[cptr]; ok {
		return i
	}
	panic(fmt.Errorf("[BUG in package ui; report to andlabs immediately] C pointer %p not associated with Go object in package ui", cptr))
}

func associateGoObject(cptr uintptr, goobj interface{}) {
	idsLock.Lock()
	defer idsLock.Unlock()

	if i, ok := ids[cptr]; ok {
		panic(fmt.Errorf("[BUG in package ui; report to andlabs immediately] C pointer %p already associated with Go object of type %T but we want to associate it with Go object of type %T", cptr, i, goobj))
	}
	ids[cptr] = goobj
}

func disassociateGoObject(cptr uintptr) {
	idsLock.Lock()
	defer idsLock.Unlock()

	if _, ok := ids[cptr]; !ok {
		panic(fmt.Errorf("[BUG in package ui; report to andlabs immediately] C pointer %p not associated with any Go object but we want to disassociate it", cptr))
	}
	delete(ids, cptr)
}

func idsHandler(req *request) bool {
	switch req.id {
	case reqObjectDestroyed:
		disassociateGoObject(req.ptr)
		return true
	}
	return false
}

func init() {
	interopHandlers = append(interopHandlers, idsHandler)
}
