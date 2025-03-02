package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

type Ingredient int

const (
	Tabaco Ingredient = iota
	Papel
	Fosforo
)

var ingredientNames = []string{"Tabaco", "Papel", "Fósforo"}

const ROUNDS = 5

type Mesa struct {
	mutex            sync.Mutex
	cond             *sync.Cond
	ingredientes     [3]bool
	disponivel       bool
	rodadasRestantes int
	rodadasAcabaram  bool
}

func NewMesa() *Mesa {
	m := &Mesa{rodadasRestantes: ROUNDS}
	m.cond = sync.NewCond(&m.mutex)
	return m
}

func (m *Mesa) colocarIngredientes() {
	rand.Seed(time.Now().UnixNano())
	for i := 0; i < ROUNDS; i++ {
		m.mutex.Lock()
		for m.disponivel {
			m.cond.Wait()
		}

		// Escolhe dois ingredientes aleatórios
		ing1 := rand.Intn(3)
		ing2 := (ing1 + 1 + rand.Intn(2)) % 3

		m.ingredientes[ing1] = true
		m.ingredientes[ing2] = true
		m.disponivel = true

		fmt.Printf("Peter colocou %s e %s na mesa. (Rodada %d)\n",
			ingredientNames[ing1], ingredientNames[ing2], i+1)

		m.cond.Broadcast()
		m.mutex.Unlock()
		time.Sleep(time.Second)
	}

	// Marca que o jogo acabou e acorda todos os fumantes
	m.mutex.Lock()
	m.rodadasAcabaram = true
	m.cond.Broadcast()
	m.mutex.Unlock()
}

func (m *Mesa) fumante(id int, meuIngrediente Ingredient, wg *sync.WaitGroup) {
	defer wg.Done()

	for {
		m.mutex.Lock()
		for !m.disponivel || m.ingredientes[meuIngrediente] {
			if m.rodadasAcabaram {
				m.mutex.Unlock()
				return
			}
			m.cond.Wait()
		}

		if !m.ingredientes[meuIngrediente] {
			fmt.Printf("Fumante %d (que tem %s) pegou %s e %s e está fumando.\n",
				id, ingredientNames[meuIngrediente], ingredientNames[(meuIngrediente+1)%3], ingredientNames[(meuIngrediente+2)%3])

			m.ingredientes[(meuIngrediente+1)%3] = false
			m.ingredientes[(meuIngrediente+2)%3] = false
			m.disponivel = false
			m.cond.Broadcast()
			m.mutex.Unlock()

			time.Sleep(time.Duration(rand.Intn(3)+1) * time.Second)
			fmt.Printf("Fumante %d terminou de fumar.\n", id)
		} else {
			m.mutex.Unlock()
		}
	}
}

func main() {
	mesa := NewMesa()
	var wg sync.WaitGroup

	wg.Add(4)

	go func() {
		defer wg.Done()
		mesa.colocarIngredientes()
	}()

	for i, ing := range []Ingredient{Tabaco, Papel, Fosforo} {
		go func(id int, ing Ingredient) {
			mesa.fumante(id, ing, &wg)
		}(i+1, ing)
	}

	wg.Wait()
	fmt.Println("Simulação concluída após 5 rodadas.")
}
