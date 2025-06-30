package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

const (
	NUM_READERS = 50
	NUM_WRITERS = 2
)

type SharedState struct {
	activeReaders  int
	activeWriters  int
	waitingWriters int
}

func reader(id int, state *SharedState, mutex *sync.Mutex, cond *sync.Cond, wg *sync.WaitGroup) {
	defer wg.Done()

	time.Sleep(time.Duration(rand.Intn(5)) * time.Second)

	mutex.Lock()
	for state.activeWriters > 0 || state.waitingWriters > 0 {
		fmt.Printf("Reader %d is waiting in the queue.\n", id)
		cond.Wait()
	}

	state.activeReaders++
	fmt.Printf("Reader %d starts reading.\n", id)
	mutex.Unlock()

	time.Sleep(1 * time.Second)

	mutex.Lock()
	state.activeReaders--
	fmt.Printf("Reader %d finished reading.\n", id)

	if state.activeReaders == 0 && state.waitingWriters > 0 {
		cond.Signal()
	}
	mutex.Unlock()
}

func writer(id int, state *SharedState, mutex *sync.Mutex, cond *sync.Cond, wg *sync.WaitGroup) {
	defer wg.Done()

	time.Sleep(time.Duration(rand.Intn(5)) * time.Second)

	mutex.Lock()
	state.waitingWriters++
	fmt.Printf("## Writer %d is waiting in the queue.\n", id)

	for state.activeReaders > 0 || state.activeWriters > 0 {
		cond.Wait()
	}

	state.waitingWriters--
	state.activeWriters++
	fmt.Printf("## Writer %d starts writing.\n", id)
	mutex.Unlock()

	time.Sleep(1 * time.Second)

	mutex.Lock()
	state.activeWriters--
	fmt.Printf("## Writer %d finished writing.\n", id)

	if state.waitingWriters > 0 {
		cond.Signal()
	} else {
		cond.Broadcast()
	}
	mutex.Unlock()
}

func main() {
	rand.Seed(time.Now().UnixNano())

	state := &SharedState{}
	mutex := &sync.Mutex{}
	cond := sync.NewCond(mutex)

	var wg sync.WaitGroup

	for i := 1; i <= NUM_READERS; i++ {
		wg.Add(1)
		go reader(i, state, mutex, cond, &wg)
	}

	for i := 1; i <= NUM_WRITERS; i++ {
		wg.Add(1)
		go writer(i, state, mutex, cond, &wg)
	}

	wg.Wait()
}
