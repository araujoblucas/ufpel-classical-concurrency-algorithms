package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

const (
	MAX_CADEIRAS   = 2
	TOTAL_CLIENTES = 20
)

type BarberShop struct {
	mutex              sync.Mutex
	cond               *sync.Cond
	cadeirasOcupadas   int
	clientesAtendidos  int
	clientesRejeitados int
	barbeariaAberta    bool
}

func NewBarberShop() *BarberShop {
	shop := &BarberShop{
		barbeariaAberta: true,
	}
	shop.cond = sync.NewCond(&shop.mutex)
	return shop
}

func (shop *BarberShop) clienteEntra(id int) {
	shop.mutex.Lock()
	if shop.cadeirasOcupadas < MAX_CADEIRAS {
		shop.cadeirasOcupadas++
		fmt.Printf("Cliente %d entrou na barbearia. Cadeiras livres: %d\n", id, MAX_CADEIRAS-shop.cadeirasOcupadas)
		shop.cond.Signal() // Acorda o barbeiro
	} else {
		fmt.Printf("Cliente %d foi embora: barbearia cheia.\n", id)
		shop.clientesRejeitados++
	}
	shop.mutex.Unlock()
}

func (shop *BarberShop) cortarCabelo() {
	for {
		shop.mutex.Lock()
		for shop.cadeirasOcupadas == 0 && shop.barbeariaAberta {
			fmt.Println("Barbeiro está dormindo...")
			shop.cond.Wait()
		}

		if shop.cadeirasOcupadas > 0 {
			shop.cadeirasOcupadas--
			fmt.Println("Barbeiro está cortando cabelo.")
			shop.mutex.Unlock()
			time.Sleep(time.Duration(rand.Intn(3)+1) * time.Second)
			shop.mutex.Lock()
			fmt.Println("Corte de cabelo finalizado.")
			shop.clientesAtendidos++
		} else if !shop.barbeariaAberta {
			shop.mutex.Unlock()
			break
		}
		shop.mutex.Unlock()
	}
	fmt.Println("Barbeiro foi para casa.")
}

func main() {
	rand.Seed(time.Now().UnixNano())
	shop := NewBarberShop()

	// Inicia a thread do barbeiro
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		shop.cortarCabelo()
	}()

	// Criando clientes
	for i := 1; i <= TOTAL_CLIENTES; i++ {
		go shop.clienteEntra(i)
		time.Sleep(time.Millisecond * 500) // Intervalo entre chegadas
	}

	// Aguarda os clientes terminarem
	time.Sleep(time.Second * 5)
	shop.mutex.Lock()
	shop.barbeariaAberta = false
	shop.cond.Broadcast()
	shop.mutex.Unlock()

	wg.Wait()

	fmt.Printf("Total de clientes atendidos: %d\n", shop.clientesAtendidos)
	fmt.Printf("Total de clientes rejeitados: %d\n", shop.clientesRejeitados)
	fmt.Println("Barbearia fechada.")
}
