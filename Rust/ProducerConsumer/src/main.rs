use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::Duration;
use rand::{Rng, rng};

const BUFFER_SIZE: usize = 5;
const NUM_ITEMS: usize = 20;
const NUM_PRODUTORES: usize = 2;
const NUM_CONSUMIDORES: usize = 20;

struct SharedBuffer {
    buffer: Vec<Option<i32>>,
    in_index: usize,
    out_index: usize,
    count: usize,
}

fn produtor(buffer: Arc<(Mutex<SharedBuffer>, Condvar, Condvar)>, id: usize) {
    let mut rng = rng();

    // Simula tempo de chegada aleatório
    thread::sleep(Duration::from_secs(rng.random_range(0..3)));

    for _ in 0..(NUM_ITEMS / NUM_PRODUTORES) {
        let item = rng.random_range(1..100);
        let (lock, not_full, not_empty) = &*buffer;

        // Adiciona o item ao buffer
        let mut shared = lock.lock().unwrap();
        while shared.count == BUFFER_SIZE {
            shared = not_full.wait(shared).unwrap();
        }

        // Insere o item no buffer
        let index = shared.in_index; // Evitar conflito de empréstimos
        shared.buffer[index] = Some(item);
        println!("Produtor {} colocou: {} (Posição {})", id, item, index);
        shared.in_index = (shared.in_index + 1) % BUFFER_SIZE;
        shared.count += 1;

        not_empty.notify_one(); // Notifica consumidores que há um item disponível
        drop(shared); // Libera o lock antes do sleep
        thread::sleep(Duration::from_millis(rng.random_range(100..1200)));
    }
}

fn consumidor(buffer: Arc<(Mutex<SharedBuffer>, Condvar, Condvar)>, id: usize) {
    let mut rng = rng();

    // Simula tempo de chegada aleatório
    thread::sleep(Duration::from_secs(rng.random_range(0..3)));

    for _ in 0..(NUM_ITEMS / NUM_CONSUMIDORES) {
        let (lock, not_full, not_empty) = &*buffer;

        // Retira o item do buffer
        let mut shared = lock.lock().unwrap();
        while shared.count == 0 {
            shared = not_empty.wait(shared).unwrap();
        }

        // Remove o item do buffer
        let index = shared.out_index; // Evitar conflito de empréstimos
        let item = shared.buffer[index].take().unwrap();
        println!("Consumidor {} pegou: {} (Posição {})", id, item, index);
        shared.out_index = (shared.out_index + 1) % BUFFER_SIZE;
        shared.count -= 1;

        not_full.notify_one(); // Notifica produtores que há espaço disponível
        drop(shared); // Libera o lock antes do sleep
        thread::sleep(Duration::from_millis(rng.random_range(100..400)));
    }
}

fn main() {
    let buffer = Arc::new((Mutex::new(SharedBuffer {
        buffer: vec![None; BUFFER_SIZE],
        in_index: 0,
        out_index: 0,
        count: 0,
    }), Condvar::new(), Condvar::new()));

    // Criação dos produtores
    let mut producers = vec![];
    for id in 1..=NUM_PRODUTORES {
        let buffer = Arc::clone(&buffer);
        producers.push(thread::spawn(move || {
            produtor(buffer, id);
        }));
    }

    // Criação dos consumidores
    let mut consumers = vec![];
    for id in 1..=NUM_CONSUMIDORES {
        let buffer = Arc::clone(&buffer);
        consumers.push(thread::spawn(move || {
            consumidor(buffer, id);
        }));
    }

    // Aguarda o término dos produtores
    for producer in producers {
        producer.join().unwrap();
    }

    // Aguarda o término dos consumidores
    for consumer in consumers {
        consumer.join().unwrap();
    }
}
