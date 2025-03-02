use crossbeam::channel;
use std::thread;
use std::time::Duration;

#[derive(Clone, Copy, Debug, PartialEq)]
enum Ingrediente {
    Tabaco,
    Papel,
    Fosforo,
}

fn main() {
    let (tx_tabaco, rx_tabaco) = channel::unbounded(); // 1
    let (tx_papel, rx_papel) = channel::unbounded(); // 2
    let (tx_fosforo, rx_fosforo) = channel::unbounded(); // 3
    let (tx_encerra, rx_encerra) = channel::unbounded(); // 4
    let max_cigarros = 3;

    let agente = thread::spawn(move || { // 5
        let ingredientes = vec![
            (Ingrediente::Tabaco, Ingrediente::Papel),
            (Ingrediente::Tabaco, Ingrediente::Fosforo),
            (Ingrediente::Papel, Ingrediente::Fosforo),
        ];

        let mut fumantes_terminaram = 0;

        loop {
            if let Ok(_) = rx_encerra.try_recv() { // 6
                fumantes_terminaram += 1;
                if fumantes_terminaram == 3 { // 7
                    println!("Todos os fumantes terminaram. Agente encerrando.");
                    break; // 8
                }
            }

            for &par in &ingredientes { // 9
                match par {
                    (Ingrediente::Tabaco, Ingrediente::Papel) => {
                        if tx_fosforo.send(par).is_err() { // 10
                            return; // 11
                        }
                    }
                    (Ingrediente::Tabaco, Ingrediente::Fosforo) => {
                        if tx_papel.send(par).is_err() { // 12
                            return; // 13
                        }
                    }
                    (Ingrediente::Papel, Ingrediente::Fosforo) => {
                        if tx_tabaco.send(par).is_err() { // 14
                            return; // 15
                        }
                    }
                    _ => (),
                }
                println!("Agente colocou na mesa: {:?}", par); // 16
                thread::sleep(Duration::from_secs(1)); // 17
            }
        }
    });

    let fumante = |nome, meu_ingrediente, ingrediente_rx: channel::Receiver<(Ingrediente, Ingrediente)>, max_cigarros, tx_encerra: channel::Sender<()>| {
        thread::spawn(move || { // 18
            let mut cigarros_fumados = 0;

            while cigarros_fumados < max_cigarros { // 19
                if let Ok((ing1, ing2)) = ingrediente_rx.recv() { // 20
                    if ing1 != meu_ingrediente && ing2 != meu_ingrediente { // 21
                        cigarros_fumados += 1; // 22
                        println!("{} está fumando ({} de {} cigarros)!", nome, cigarros_fumados, max_cigarros); // 23
                        thread::sleep(Duration::from_secs(2)); // 24
                    }
                }
            }

            println!("{} terminou de fumar!", nome); // 25
            let _ = tx_encerra.send(()); // 26
        })
    };

    let fumante1 = fumante("Fumante 1", Ingrediente::Fosforo, rx_fosforo, max_cigarros, tx_encerra.clone()); // 27
    let fumante2 = fumante("Fumante 2", Ingrediente::Papel, rx_papel, max_cigarros, tx_encerra.clone()); // 28
    let fumante3 = fumante("Fumante 3", Ingrediente::Tabaco, rx_tabaco, max_cigarros, tx_encerra.clone()); // 29

    agente.join().unwrap(); // 30
    fumante1.join().unwrap(); // 31
    fumante2.join().unwrap(); // 32
    fumante3.join().unwrap(); // 33
}
