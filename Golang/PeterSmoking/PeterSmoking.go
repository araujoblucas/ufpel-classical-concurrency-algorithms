package main

import (
	"fmt"
	"sync"
	"time"
)

type Ingrediente int

const (
	Tabaco Ingrediente = iota
	Papel
	Fosforo
)

func main() {
	tabacoCh := make(chan struct{ ing1, ing2 Ingrediente })
	papelCh := make(chan struct{ ing1, ing2 Ingrediente })
	fosforoCh := make(chan struct{ ing1, ing2 Ingrediente })
	encerraCh := make(chan struct{}, 3) // buffer para evitar bloqueio

	maxCigarros := 3
	var wg sync.WaitGroup

	// Agente NÃO participa do WaitGroup
	go func() {
		ingredientes := []struct{ ing1, ing2 Ingrediente }{
			{Tabaco, Papel},
			{Tabaco, Fosforo},
			{Papel, Fosforo},
		}
		fumantesTerminados := 0

	loopAgente:
		for {
			select {
			case <-encerraCh:
				fumantesTerminados++
				if fumantesTerminados == 3 {
					fmt.Println("Todos os fumantes terminaram. Agente encerrando.")
					break loopAgente
				}
			default:
				for _, par := range ingredientes {
					fmt.Println("Agente colocou na mesa:", par)
					switch par {
					case struct{ ing1, ing2 Ingrediente }{Tabaco, Papel}:
						fosforoCh <- par
					case struct{ ing1, ing2 Ingrediente }{Tabaco, Fosforo}:
						papelCh <- par
					case struct{ ing1, ing2 Ingrediente }{Papel, Fosforo}:
						tabacoCh <- par
					}
					time.Sleep(time.Second)
				}
			}
		}

		// Fecha canais após fim do loop
		close(tabacoCh)
		close(papelCh)
		close(fosforoCh)
	}()

	// Função fumante corrigida com canal fechado
	fumante := func(nome string, meuIngrediente Ingrediente, ingredienteCh chan struct{ ing1, ing2 Ingrediente }) {
		defer wg.Done()
		cigarrosFumados := 0

		for cigarrosFumados < maxCigarros {
			par, ok := <-ingredienteCh
			if !ok { // Canal fechado pelo agente
				break
			}
			if par.ing1 != meuIngrediente && par.ing2 != meuIngrediente {
				cigarrosFumados++
				fmt.Printf("%s está fumando (%d de %d cigarros)!\n", nome, cigarrosFumados, maxCigarros)
				time.Sleep(2 * time.Second)
			}
		}
		fmt.Println(nome, "terminou de fumar!")
		encerraCh <- struct{}{}
	}

	wg.Add(3)
	go fumante("Fumante 1", Fosforo, fosforoCh)
	go fumante("Fumante 2", Papel, papelCh)
	go fumante("Fumante 3", Tabaco, tabacoCh)

	wg.Wait()
}
