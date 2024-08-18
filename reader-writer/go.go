package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	numReaders          = 5
	numWriters          = 2
	maxConsecutiveReads = 3
)

type SharedData struct {
	data             int
	readerCount      int
	writerWaiting    bool
	consecutiveReads int
	mu               sync.Mutex
	cv               *sync.Cond
}

func newSharedData() *SharedData {
	sd := &SharedData{}
	sd.cv = sync.NewCond(&sd.mu)
	return sd
}

func reader(id int, sd *SharedData) {
	sd.mu.Lock()
	for sd.writerWaiting {
		sd.cv.Wait()
	}
	sd.readerCount++
	if sd.readerCount == 1 {
		sd.consecutiveReads++
	}
	sd.mu.Unlock()

	fmt.Printf("Reader %d is reading value: %d\n", id, sd.data)
	time.Sleep(100 * time.Millisecond)

	sd.mu.Lock()
	sd.readerCount--
	if sd.readerCount == 0 {
		sd.cv.Broadcast()
	}
	sd.mu.Unlock()

	fmt.Printf("Reader %d finished reading value: %d\n", id, sd.data)
}

func writer(id int, sd *SharedData) {
	sd.mu.Lock()
	sd.writerWaiting = true
	for sd.readerCount > 0 {
		sd.cv.Wait()
	}
	sd.data += id
	fmt.Printf("Writer %d wrote value: %d\n", id, sd.data)
	sd.writerWaiting = false
	sd.consecutiveReads = 0
	sd.cv.Broadcast()
	sd.mu.Unlock()

	time.Sleep(200 * time.Millisecond)
}

func main() {
	sd := newSharedData()

	for {
		var wg sync.WaitGroup
		wg.Add(numReaders + numWriters)

		for i := 0; i < numReaders; i++ {
			go func(id int) {
				reader(id, sd)
				wg.Done()
			}(i)
		}

		for i := 0; i < numWriters; i++ {
			go func(id int) {
				writer(id, sd)
				wg.Done()
			}(i)
		}

		wg.Wait()

		time.Sleep(time.Second)
	}
}
