use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::time::Duration;

const NUM_READERS: usize = 5;
const NUM_WRITERS: usize = 2;
const MAX_CONSECUTIVE_READS: usize = 3;

struct SharedData {
    data: i32,
    reader_count: usize,
    writer_waiting: bool,
    consecutive_reads: usize,
}

fn reader(id: usize, shared: Arc<(Mutex<SharedData>, Condvar)>) {
    let (lock, cvar) = &*shared;
    let mut shared_data = lock.lock().unwrap();

    // Espera enquanto há um escritor esperando
    while shared_data.writer_waiting {
        shared_data = cvar.wait(shared_data).unwrap();
    }

    // Incrementa o contador de leitores
    shared_data.reader_count += 1;

    // Se for o primeiro leitor, incrementa a contagem de leituras consecutivas
    if shared_data.reader_count == 1 {
        shared_data.consecutive_reads += 1;
    }

    // Libera o lock para permitir que outros leitores leiam simultaneamente
    drop(shared_data);

    // Simulação de leitura
    println!("Leitor {} está lendo o valor: {}", id, lock.lock().unwrap().data);
    thread::sleep(Duration::from_millis(100));

    // Decrementa o contador de leitores após a leitura
    let mut shared_data = lock.lock().unwrap();
    shared_data.reader_count -= 1;

    // Se for o último leitor, notifica os escritores
    if shared_data.reader_count == 0 {
        cvar.notify_all();
    }

    println!("Leitor {} terminou de ler o valor: {}", id, shared_data.data);
}

fn writer(id: usize, shared: Arc<(Mutex<SharedData>, Condvar)>) {
    let (lock, cvar) = &*shared;
    let mut shared_data = lock.lock().unwrap();

    // Indica que um escritor está esperando
    shared_data.writer_waiting = true;

    // Espera até que nenhum leitor esteja ativo
    while shared_data.reader_count > 0 {
        shared_data = cvar.wait(shared_data).unwrap();
    }

    // Escrita realizada
    shared_data.data += id as i32;
    println!("Escritor {} escreveu o valor: {}", id, shared_data.data);

    // Escritor termina, permite que leitores e outros escritores procedam
    shared_data.writer_waiting = false;
    shared_data.consecutive_reads = 0; // Reinicia contagem de leituras consecutivas
    cvar.notify_all();

    thread::sleep(Duration::from_millis(200)); // Simulação de tempo entre escritas
}

fn main() {
    let shared = Arc::new((
        Mutex::new(SharedData {
            data: 0,
            reader_count: 0,
            writer_waiting: false,
            consecutive_reads: 0,
        }),
        Condvar::new(),
    ));

    loop {
        let mut handles = vec![];

        // Inicia leitores
        for i in 0..NUM_READERS {
            let shared = Arc::clone(&shared);
            let handle = thread::spawn(move || {
                reader(i, shared);
            });
            handles.push(handle);
        }

        // Inicia escritores
        for i in 0..NUM_WRITERS {
            let shared = Arc::clone(&shared);
            let handle = thread::spawn(move || {
                writer(i, shared);
            });
            handles.push(handle);
        }

        for handle in handles {
            handle.join().unwrap();
        }

        thread::sleep(Duration::from_secs(1)); // Pausa antes do próximo ciclo
    }
}
