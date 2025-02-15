package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

const (
	BUFFER_SIZE      = 5
	NUM_ITEMS        = 20
	NUM_PRODUTORES   = 2
	NUM_CONSUMIDORES = 20
)

type SharedBuffer struct {
	buffer   [BUFFER_SIZE]int
	inIndex  int
	outIndex int
	count    int
	mutex    sync.Mutex
	notFull  *sync.Cond
	notEmpty *sync.Cond
}

func produtor(buffer *SharedBuffer, id int) {
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))

	time.Sleep(time.Duration(rng.Intn(3)) * time.Second)

	for i := 0; i < NUM_ITEMS/NUM_PRODUTORES; i++ {
		item := rng.Intn(100) + 1

		buffer.mutex.Lock()
		for buffer.count == BUFFER_SIZE {
			buffer.notFull.Wait() // Libera o Mutex e espera notificação
		}

		// Insere o item no buffer
		buffer.buffer[buffer.inIndex] = item
		fmt.Printf("Produtor %d colocou: %d (Posição %d)\n", id, item, buffer.inIndex)
		buffer.inIndex = (buffer.inIndex + 1) % BUFFER_SIZE
		buffer.count++

		buffer.notEmpty.Signal() // Avisa consumidores
		buffer.mutex.Unlock()

		time.Sleep(time.Duration(rng.Intn(1100)+100) * time.Millisecond)
	}
}

func consumidor(buffer *SharedBuffer, id int) {
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))

	time.Sleep(time.Duration(rng.Intn(3)) * time.Second)

	for i := 0; i < NUM_ITEMS/NUM_CONSUMIDORES; i++ {
		buffer.mutex.Lock()
		for buffer.count == 0 {
			buffer.notEmpty.Wait() // Libera o Mutex e espera notificação
		}

		// Remove o item do buffer
		item := buffer.buffer[buffer.outIndex]
		fmt.Printf("Consumidor %d pegou: %d (Posição %d)\n", id, item, buffer.outIndex)
		buffer.outIndex = (buffer.outIndex + 1) % BUFFER_SIZE
		buffer.count--

		buffer.notFull.Signal() // Avisa produtores
		buffer.mutex.Unlock()

		time.Sleep(time.Duration(rng.Intn(300)+100) * time.Millisecond)
	}
}

func main() {
	buffer := SharedBuffer{}
	buffer.notFull = sync.NewCond(&buffer.mutex)  // Usa o mesmo Mutex
	buffer.notEmpty = sync.NewCond(&buffer.mutex) // Usa o mesmo Mutex

	var wg sync.WaitGroup

	// Inicia produtores
	for id := 1; id <= NUM_PRODUTORES; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			produtor(&buffer, id)
		}(id)
	}

	// Inicia consumidores
	for id := 1; id <= NUM_CONSUMIDORES; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			consumidor(&buffer, id)
		}(id)
	}

	wg.Wait() // Aguarda todas as goroutines finalizarem
}
