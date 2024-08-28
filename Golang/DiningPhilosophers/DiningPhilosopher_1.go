package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

const numPhilosophers = 5

type Philosopher struct {
	name      string
	leftFork  int
	rightFork int
}

func (p Philosopher) dine(wg *sync.WaitGroup, forks []sync.Mutex) {
	defer wg.Done()
	for {
		p.think()
		p.eat(forks)
	}
}

func (p Philosopher) think() {
	fmt.Printf("%s está pensando...\n", p.name)
	time.Sleep(time.Duration(rand.Intn(1000)) * time.Millisecond)
}

func (p Philosopher) eat(forks []sync.Mutex) {
	left, right := p.leftFork, p.rightFork
	if left > right {
		left, right = right, left
	}

	// Lock both forks only if both are available
	for {
		forks[left].Lock()
		lockedRight := forks[right].TryLock()

		if lockedRight {
			// Successfully locked both forks
			break
		} else {
			// Couldn't lock right fork, unlock left fork and retry
			forks[left].Unlock()
			time.Sleep(time.Duration(rand.Intn(100)) * time.Millisecond)
		}
	}

	fmt.Printf("%s está comendo...\n", p.name)
	time.Sleep(time.Duration(rand.Intn(1000)) * time.Millisecond)
	fmt.Printf("%s já comeu.\n", p.name)

	// Unlock both forks
	forks[right].Unlock()
	forks[left].Unlock()
}

func main() {
	rand.Seed(time.Now().UnixNano())

	var wg sync.WaitGroup
	forks := make([]sync.Mutex, numPhilosophers)
	philosophers := []Philosopher{
		{"Filósofo 1", 0, 1},
		{"Filósofo 2", 1, 2},
		{"Filósofo 3", 2, 3},
		{"Filósofo 4", 3, 4},
		{"Filósofo 5", 4, 0},
	}

	wg.Add(numPhilosophers)
	for _, philosopher := range philosophers {
		go philosopher.dine(&wg, forks)
	}

	wg.Wait()
}
